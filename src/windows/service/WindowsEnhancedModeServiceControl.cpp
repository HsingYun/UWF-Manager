// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
// MinGW 的 shellapi.h 依赖 windows.h 先定义基础 Win32 声明。
// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on

#include "WindowsEnhancedModeServiceControl.h"

#include <QDir>
#include <QScopeGuard>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cwchar>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "app/enhanced/EnhancedModeProtocol.h"
#include "app/ports/RegistryCommitPlan.h"
#include "app/ports/RegistryKey.h"
#include "app/uwf/UwfFilter.h"
#include "windows/security/ExecutableIdentity.h"
#include "windows/system/WindowsSystemError.h"

namespace uwf::windows {

namespace {

constexpr auto kServiceStateTimeout = std::chrono::seconds{30};
constexpr auto kServiceStatePollInterval = std::chrono::milliseconds{100};

class ServiceHandle final {
 public:
  explicit ServiceHandle(SC_HANDLE handle = nullptr) : m_handle(handle) {}
  ~ServiceHandle() {
    if (m_handle) CloseServiceHandle(m_handle);
  }
  ServiceHandle(const ServiceHandle&) = delete;
  ServiceHandle& operator=(const ServiceHandle&) = delete;
  ServiceHandle(ServiceHandle&& other) noexcept : m_handle(std::exchange(other.m_handle, nullptr)) {}
  ServiceHandle& operator=(ServiceHandle&& other) noexcept {
    if (this == &other) return *this;
    if (m_handle) CloseServiceHandle(m_handle);
    m_handle = std::exchange(other.m_handle, nullptr);
    return *this;
  }

  [[nodiscard]] SC_HANDLE get() const { return m_handle; }
  [[nodiscard]] explicit operator bool() const { return m_handle != nullptr; }

 private:
  SC_HANDLE m_handle = nullptr;
};

class ServiceDeletionBarrier final : public app::EnhancedModeServiceControl::DeletionBarrier {
 public:
  explicit ServiceDeletionBarrier(ServiceHandle service) : m_service(std::move(service)) {}

 private:
  ServiceHandle m_service;
};

[[noreturn]] void throwSystemError(const char* operation, const DWORD error = GetLastError()) { throw WindowsSystemError(error, operation); }

std::wstring serviceCommandLine() { return L"\"" + currentExecutablePath() + L"\" --service"; }

ServiceHandle openServiceManager(const DWORD access) {
  ServiceHandle manager(OpenSCManagerW(nullptr, nullptr, access));
  if (!manager) throwSystemError("open service control manager");
  return manager;
}

ServiceHandle findEnhancedService(const SC_HANDLE manager, const DWORD access) {
  ServiceHandle service(OpenServiceW(manager, app::kEnhancedServiceName, access));
  if (!service) {
    const DWORD error = GetLastError();
    const bool logicallyAbsent = error == ERROR_SERVICE_DOES_NOT_EXIST || error == ERROR_SERVICE_MARKED_FOR_DELETE;
    if (!logicallyAbsent) throwSystemError("open enhanced mode service", error);
  }
  return service;
}

std::vector<std::byte> queryServiceConfig(const SC_HANDLE service) {
  DWORD bytesNeeded = 0;
  QueryServiceConfigW(service, nullptr, 0, &bytesNeeded);
  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || bytesNeeded == 0) throwSystemError("measure enhanced mode service configuration");
  std::vector<std::byte> storage(bytesNeeded);
  auto* config = reinterpret_cast<QUERY_SERVICE_CONFIGW*>(storage.data());
  if (!QueryServiceConfigW(service, config, bytesNeeded, &bytesNeeded)) throwSystemError("read enhanced mode service configuration");
  return storage;
}

std::vector<std::byte> queryServiceConfig2(const SC_HANDLE service, const DWORD informationLevel) {
  DWORD bytesNeeded = 0;
  QueryServiceConfig2W(service, informationLevel, nullptr, 0, &bytesNeeded);
  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || bytesNeeded == 0) throwSystemError("measure enhanced mode extended service configuration");
  std::vector<std::byte> storage(bytesNeeded);
  if (!QueryServiceConfig2W(service, informationLevel, reinterpret_cast<LPBYTE>(storage.data()), bytesNeeded, &bytesNeeded)) {
    throwSystemError("read enhanced mode extended service configuration");
  }
  return storage;
}

bool configuredCommandMatches(const wchar_t* commandLine) {
  if (!commandLine) return false;
  int argumentCount = 0;
  LPWSTR* arguments = CommandLineToArgvW(commandLine, &argumentCount);
  if (!arguments) return false;
  const auto release = qScopeGuard([arguments] { LocalFree(static_cast<void*>(arguments)); });
  if (argumentCount != 2 || std::wstring_view(arguments[1]) != L"--service") return false;

  const QString configured = QDir::cleanPath(QString::fromWCharArray(arguments[0]));
  const QString current = QDir::cleanPath(QString::fromStdWString(currentExecutablePath()));
  return configured.compare(current, Qt::CaseInsensitive) == 0;
}

bool configuredForLocalSystem(const wchar_t* accountName) {
  if (!accountName || accountName[0] == L'\0') return false;

  // LocalSystem 是 SCM 识别的预定义服务账户，并不是 LookupAccountNameW
  // 能解析的普通账户。直接按 SCM 接受的三个、且不受系统语言影响的名称
  // 判断，避免把实际以 SYSTEM 运行的服务误报为配置异常。
  const QString configured = QString::fromWCharArray(accountName);
  if (configured.compare(QStringLiteral("LocalSystem"), Qt::CaseInsensitive) == 0 ||
      configured.compare(QStringLiteral(".\\LocalSystem"), Qt::CaseInsensitive) == 0) {
    return true;
  }

  std::wstring computerName(static_cast<std::size_t>(MAX_COMPUTERNAME_LENGTH) + 1, L'\0');
  DWORD length = static_cast<DWORD>(computerName.size());
  if (!GetComputerNameW(computerName.data(), &length)) return false;
  computerName.resize(length);
  const QString qualifiedName = QString::fromStdWString(computerName) + QStringLiteral("\\LocalSystem");
  return configured.compare(qualifiedName, Qt::CaseInsensitive) == 0;
}

bool privilegeListContains(const wchar_t* privileges, const std::wstring_view expected) {
  if (!privileges) return false;
  for (const wchar_t* privilege = privileges; privilege[0] != L'\0'; privilege += std::wcslen(privilege) + 1) {
    if (std::wstring_view(privilege) == expected) return true;
  }
  return false;
}

bool configuredWithRequiredPrivileges(const SC_HANDLE service) {
  const auto storage = queryServiceConfig2(service, SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO);
  const auto* config = reinterpret_cast<const SERVICE_REQUIRED_PRIVILEGES_INFOW*>(storage.data());
  return privilegeListContains(config->pmszRequiredPrivileges, SE_TCB_NAME) &&
         privilegeListContains(config->pmszRequiredPrivileges, SE_ASSIGNPRIMARYTOKEN_NAME) &&
         privilegeListContains(config->pmszRequiredPrivileges, SE_INCREASE_QUOTA_NAME);
}

bool configuredWithPreshutdownTimeout(const SC_HANDLE service) {
  const auto storage = queryServiceConfig2(service, SERVICE_CONFIG_PRESHUTDOWN_INFO);
  const auto* config = reinterpret_cast<const SERVICE_PRESHUTDOWN_INFO*>(storage.data());
  return config->dwPreshutdownTimeout == std::numeric_limits<DWORD>::max();
}

SERVICE_STATUS_PROCESS queryStatus(const SC_HANDLE service) {
  SERVICE_STATUS_PROCESS status{};
  DWORD bytesNeeded = 0;
  if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&status), sizeof(status), &bytesNeeded)) {
    throwSystemError("read enhanced mode service status");
  }
  return status;
}

void waitForState(const SC_HANDLE service, const DWORD desiredState, const std::chrono::seconds timeout) {
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  for (;;) {
    const auto status = queryStatus(service);
    if (status.dwCurrentState == desiredState) return;
    if (status.dwCurrentState == SERVICE_STOPPED && desiredState != SERVICE_STOPPED) {
      throw std::runtime_error("enhanced mode service stopped before reaching the requested state");
    }
    if (std::chrono::steady_clock::now() >= deadline) throw std::runtime_error("timed out waiting for enhanced mode service state");
    std::this_thread::sleep_for(kServiceStatePollInterval);
  }
}

SERVICE_STATUS_PROCESS waitForStartToSettle(const SC_HANDLE service, const std::chrono::seconds timeout) {
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  for (;;) {
    const auto status = queryStatus(service);
    if (status.dwCurrentState != SERVICE_START_PENDING) return status;
    if (std::chrono::steady_clock::now() >= deadline) {
      throw std::runtime_error("timed out waiting for enhanced mode service startup to settle");
    }
    std::this_thread::sleep_for(kServiceStatePollInterval);
  }
}

void stopService(const SC_HANDLE service) {
  auto status = queryStatus(service);
  if (status.dwCurrentState == SERVICE_STOPPED) return;
  if (status.dwCurrentState == SERVICE_STOP_PENDING) {
    waitForState(service, SERVICE_STOPPED, kServiceStateTimeout);
    return;
  }
  if (status.dwCurrentState == SERVICE_START_PENDING) {
    status = waitForStartToSettle(service, kServiceStateTimeout);
    if (status.dwCurrentState == SERVICE_STOPPED) return;
    if (status.dwCurrentState == SERVICE_STOP_PENDING) {
      waitForState(service, SERVICE_STOPPED, kServiceStateTimeout);
      return;
    }
  }
  SERVICE_STATUS ignored{};
  if (!ControlService(service, SERVICE_CONTROL_STOP, &ignored) && GetLastError() != ERROR_SERVICE_NOT_ACTIVE) {
    throwSystemError("stop enhanced mode service");
  }
  waitForState(service, SERVICE_STOPPED, kServiceStateTimeout);
}

}  // namespace

app::EnhancedModeStatus WindowsEnhancedModeServiceControl::query() const {
  app::EnhancedModeStatus result;
  result.serviceRegistryPresent = regkey::keyExists(app::kEnhancedServiceRegistryKey);

  const auto manager = openServiceManager(SC_MANAGER_CONNECT);
  const auto service = findEnhancedService(manager.get(), SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
  if (!service) {
    result.state = result.serviceRegistryPresent ? app::EnhancedModeState::RepairRequired : app::EnhancedModeState::Disabled;
    return result;
  }

  result.serviceExists = true;
  const auto storage = queryServiceConfig(service.get());
  const auto* config = reinterpret_cast<const QUERY_SERVICE_CONFIGW*>(storage.data());
  result.ownProcess = config->dwServiceType == SERVICE_WIN32_OWN_PROCESS;
  result.automaticStart = config->dwStartType == SERVICE_AUTO_START;
  result.localSystemAccount = configuredForLocalSystem(config->lpServiceStartName);
  result.executableMatches = configuredCommandMatches(config->lpBinaryPathName);
  result.preshutdownTimeoutConfigured = configuredWithPreshutdownTimeout(service.get());
  result.requiredPrivilegesConfigured = configuredWithRequiredPrivileges(service.get());

  const auto state = queryStatus(service.get());
  result.running = state.dwCurrentState == SERVICE_RUNNING;
  result.preshutdownAccepted = result.running && (state.dwControlsAccepted & SERVICE_ACCEPT_PRESHUTDOWN) != 0;
  if (result.serviceContractSatisfied()) {
    result.state = app::EnhancedModeState::Enabled;
  } else if (result.serviceConfigurationSatisfied() && !result.running) {
    result.state = app::EnhancedModeState::Stopped;
  } else {
    result.state = app::EnhancedModeState::RepairRequired;
  }
  return result;
}

void WindowsEnhancedModeServiceControl::installAndStart(const QString& description) {
  if (description.trimmed().isEmpty()) throw std::invalid_argument("enhanced mode service description cannot be empty");
  const auto manager = openServiceManager(SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
  ServiceHandle service = findEnhancedService(manager.get(), SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP);
  const std::wstring commandLine = serviceCommandLine();
  if (!service) {
    service = ServiceHandle(CreateServiceW(manager.get(), app::kEnhancedServiceName, app::kEnhancedServiceDisplayName,
                                           SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS | SERVICE_START, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
                                           SERVICE_ERROR_NORMAL, commandLine.c_str(), nullptr, nullptr, nullptr, L"LocalSystem", nullptr));
    if (!service) throwSystemError("create enhanced mode service");
  } else {
    // 即使配置路径文字没有变化，磁盘上的单文件程序也可能已被新版本替换。
    // 修复必须重启服务进程，不能把“旧映像仍在运行”误判成当前二进制已生效。
    stopService(service.get());
    if (!ChangeServiceConfigW(service.get(), SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_NO_CHANGE, commandLine.c_str(), nullptr, nullptr, nullptr,
                              L"LocalSystem", nullptr, app::kEnhancedServiceDisplayName)) {
      throwSystemError("repair enhanced mode service configuration");
    }
  }

  std::wstring descriptionText = description.toStdWString();
  SERVICE_DESCRIPTIONW descriptionConfig{descriptionText.data()};
  if (!ChangeServiceConfig2W(service.get(), SERVICE_CONFIG_DESCRIPTION, &descriptionConfig)) {
    throwSystemError("configure enhanced mode service description");
  }

  SERVICE_PRESHUTDOWN_INFO preshutdown{std::numeric_limits<DWORD>::max()};
  if (!ChangeServiceConfig2W(service.get(), SERVICE_CONFIG_PRESHUTDOWN_INFO, &preshutdown)) {
    throwSystemError("configure enhanced mode preshutdown timeout");
  }

  wchar_t requiredPrivileges[] = SE_TCB_NAME L"\0" SE_ASSIGNPRIMARYTOKEN_NAME L"\0" SE_INCREASE_QUOTA_NAME L"\0";
  SERVICE_REQUIRED_PRIVILEGES_INFOW privileges{requiredPrivileges};
  if (!ChangeServiceConfig2W(service.get(), SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO, &privileges)) {
    throwSystemError("configure enhanced mode service privileges");
  }

  if (!StartServiceW(service.get(), 0, nullptr) && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING) {
    throwSystemError("start enhanced mode service");
  }
  waitForState(service.get(), SERVICE_RUNNING, kServiceStateTimeout);
}

void WindowsEnhancedModeServiceControl::start() {
  const auto manager = openServiceManager(SC_MANAGER_CONNECT);
  const auto service = findEnhancedService(manager.get(), SERVICE_QUERY_STATUS | SERVICE_START);
  if (!service) throw std::runtime_error("enhanced mode service does not exist");
  if (!StartServiceW(service.get(), 0, nullptr) && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING) {
    throwSystemError("start enhanced mode service");
  }
  waitForState(service.get(), SERVICE_RUNNING, kServiceStateTimeout);
}

std::vector<app::RegistryCommitTarget> WindowsEnhancedModeServiceControl::planRegistryCommit() const {
  return app::planRegistryTreeCommit(app::kEnhancedServiceRegistryKey);
}

std::vector<app::RegistryCommitTarget> WindowsEnhancedModeServiceControl::planRegistryDeletion() const {
  return app::planRegistryTreeDeletion(app::kEnhancedServiceRegistryKey);
}

bool WindowsEnhancedModeServiceControl::registryTargetExists(const app::RegistryCommitTarget& target) const {
  return target.valueName.empty() ? regkey::keyExists(target.key) : regkey::valueExists(target.key, target.valueName);
}

void WindowsEnhancedModeServiceControl::deleteRegistryRemnants() { regkey::deleteTree(app::kEnhancedServiceRegistryKey); }

app::EnhancedModeServiceControl::Removal WindowsEnhancedModeServiceControl::stopAndMarkForDeletion() {
  const auto manager = openServiceManager(SC_MANAGER_CONNECT);
  ServiceHandle service(OpenServiceW(manager.get(), app::kEnhancedServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE));
  if (!service) {
    const DWORD error = GetLastError();
    if (error == ERROR_SERVICE_DOES_NOT_EXIST) return {RemovalState::Absent, {}};
    if (error == ERROR_SERVICE_MARKED_FOR_DELETE) return {RemovalState::AlreadyMarkedForDeletion, {}};
    throwSystemError("open enhanced mode service for removal", error);
  }

  stopService(service.get());
  if (!DeleteService(service.get()) && GetLastError() != ERROR_SERVICE_MARKED_FOR_DELETE) throwSystemError("delete enhanced mode service");
  // DeleteService 只做标记；屏障继续持有最后一个已知服务句柄，使服务注册表
  // 项在递归 CommitRegistryDeletion 完成前不会被 SCM 抢先移除。
  return {RemovalState::MarkedForDeletion, std::make_unique<ServiceDeletionBarrier>(std::move(service))};
}

}  // namespace uwf::windows
