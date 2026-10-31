// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "EnhancedModeServiceRuntime.h"

#include "windows/commit/WindowsCommitTraversalSource.h"
#include "windows/storage/RegistryFileStagingStore.h"
// MinGW 的 sddl.h 不会自行定义 WINADVAPI / WINBOOL，windows.h 必须保持在前。
// clang-format off
#include <windows.h>
#include <sddl.h>
#include <userenv.h>
#include <wtsapi32.h>
// clang-format on

#include <QScopeGuard>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include "app/enhanced/EnhancedModeProtocol.h"
#include "app/ports/FileStagingStore.h"
#include "app/staging/FileStagingTask.h"
#include "app/uwf/UwfStateReader.h"
#include "shared/Log.h"
#include "shared/StringUtil.h"
#include "windows/ipc/EnhancedModePipeIo.h"
#include "windows/resources/UniqueHandle.h"
#include "windows/security/ExecutableIdentity.h"
#include "windows/wmi/WmiSession.h"

namespace uwf::service {

namespace {

constexpr DWORD kServiceWaitHintMs = 60'000;
constexpr auto kServiceHeartbeatInterval = std::chrono::seconds{15};
constexpr auto kAgentStartupGrace = std::chrono::seconds{30};
constexpr auto kAgentRetryInitial = std::chrono::seconds{5};
constexpr auto kAgentRetryMaximum = std::chrono::seconds{60};
constexpr auto kCommitPreparationPollInterval = std::chrono::milliseconds{16};
constexpr auto kPreshutdownSkipLifetime = std::chrono::minutes{5};

using windows::UniqueHandle;

class LocalMemory final {
 public:
  ~LocalMemory() {
    if (m_memory) LocalFree(m_memory);
  }
  [[nodiscard]] void** put() { return &m_memory; }
  [[nodiscard]] void* get() const { return m_memory; }

 private:
  void* m_memory = nullptr;
};

class EnvironmentBlock final {
 public:
  explicit EnvironmentBlock(const HANDLE token) {
    if (!CreateEnvironmentBlock(&m_environment, token, FALSE)) {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "create user environment");
    }
  }
  ~EnvironmentBlock() { DestroyEnvironmentBlock(m_environment); }
  [[nodiscard]] void* get() const { return m_environment; }

 private:
  void* m_environment = nullptr;
};

[[noreturn]] void throwSystemError(const char* operation, const DWORD error = GetLastError()) {
  throw std::system_error(static_cast<int>(error), std::system_category(), operation);
}

struct AuthenticatedAgent {
  DWORD processId = 0;
  DWORD sessionId = 0;
  UniqueHandle process;
};

bool tokenIsElevatedAdministrator(const HANDLE token) {
  TOKEN_ELEVATION elevation{};
  DWORD bytes = sizeof(elevation);
  if (!GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &bytes) || elevation.TokenIsElevated == 0) return false;

  BYTE administrators[SECURITY_MAX_SID_SIZE]{};
  DWORD sidSize = sizeof(administrators);
  if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, administrators, &sidSize)) return false;

  // OpenProcessToken、WTSQueryUserToken 和 DuplicateTokenEx(TokenPrimary)
  // 在这里提供的都是主令牌；CheckTokenMembership 显式接收的令牌却必须是
  // 模拟令牌。直接读取 TokenGroups 可按令牌本来的类型完成同一项判断，也
  // 避免为了查询组成员关系再申请 TOKEN_DUPLICATE 权限。
  bytes = 0;
  GetTokenInformation(token, TokenGroups, nullptr, 0, &bytes);
  if (bytes == 0) return false;
  std::vector<BYTE> groupBuffer(bytes);
  if (!GetTokenInformation(token, TokenGroups, groupBuffer.data(), bytes, &bytes)) return false;

  const auto* groups = reinterpret_cast<const TOKEN_GROUPS*>(groupBuffer.data());
  for (DWORD index = 0; index < groups->GroupCount; ++index) {
    const auto& group = groups->Groups[index];
    const bool enabled = (group.Attributes & SE_GROUP_ENABLED) != 0;
    const bool denyOnly = (group.Attributes & SE_GROUP_USE_FOR_DENY_ONLY) != 0;
    if (enabled && !denyOnly && EqualSid(group.Sid, administrators)) return true;
  }
  return false;
}

bool sessionChangeRequiresAgent(const DWORD eventType) {
  // 登录和重新连接会话表示出现了新的交互桌面；锁定/解锁等状态变化不能
  // 重新拉起用户已经主动退出的 UI。
  return eventType == WTS_SESSION_LOGON || eventType == WTS_CONSOLE_CONNECT || eventType == WTS_REMOTE_CONNECT;
}

DWORD activeInteractiveSessionId() {
  const DWORD console = WTSGetActiveConsoleSessionId();
  PWTS_SESSION_INFOW sessions = nullptr;
  DWORD count = 0;
  if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessions, &count)) {
    return console != 0 ? console : std::numeric_limits<DWORD>::max();
  }
  const auto release = std::unique_ptr<WTS_SESSION_INFOW, decltype(&WTSFreeMemory)>(sessions, &WTSFreeMemory);
  DWORD firstActive = std::numeric_limits<DWORD>::max();
  for (DWORD index = 0; index < count; ++index) {
    const auto& session = sessions[index];
    if (session.SessionId == 0 || session.State != WTSActive) continue;
    if (session.SessionId == console) return session.SessionId;
    if (firstActive == std::numeric_limits<DWORD>::max()) firstActive = session.SessionId;
  }
  return firstActive;
}

std::optional<AuthenticatedAgent> authenticateAgent(const HANDLE pipe) {
  ULONG processId = 0;
  if (!GetNamedPipeClientProcessId(pipe, &processId) || processId == 0) return std::nullopt;
  UniqueHandle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, static_cast<DWORD>(processId)));
  if (!process.valid() ||
      !windows::sameFile(windows::processImagePath(process.get()), windows::currentExecutablePath(), windows::FileIdentityFormat::FileId128))
    return std::nullopt;

  DWORD sessionId = 0;
  if (!ProcessIdToSessionId(static_cast<DWORD>(processId), &sessionId) || sessionId == 0 || sessionId != activeInteractiveSessionId()) {
    return std::nullopt;
  }
  HANDLE rawToken = nullptr;
  if (!OpenProcessToken(process.get(), TOKEN_QUERY, &rawToken)) return std::nullopt;
  const UniqueHandle token(rawToken);
  return tokenIsElevatedAdministrator(token.get()) ? std::optional{AuthenticatedAgent{static_cast<DWORD>(processId), sessionId, std::move(process)}}
                                                   : std::nullopt;
}

void enablePrivilege(const wchar_t* name) {
  HANDLE rawToken = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &rawToken)) throwSystemError("open service process token");
  const UniqueHandle token(rawToken);
  TOKEN_PRIVILEGES privileges{};
  privileges.PrivilegeCount = 1;
  if (!LookupPrivilegeValueW(nullptr, name, &privileges.Privileges[0].Luid)) throwSystemError("resolve service privilege");
  privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  SetLastError(ERROR_SUCCESS);
  if (!AdjustTokenPrivileges(token.get(), FALSE, &privileges, 0, nullptr, nullptr) || GetLastError() != ERROR_SUCCESS) {
    throwSystemError("enable service privilege");
  }
}

UniqueHandle elevatedInteractiveToken(const DWORD sessionId) {
  HANDLE rawUserToken = nullptr;
  if (!WTSQueryUserToken(sessionId, &rawUserToken)) throwSystemError("query interactive user token");
  UniqueHandle userToken(rawUserToken);

  TOKEN_ELEVATION_TYPE elevationType = TokenElevationTypeDefault;
  DWORD bytes = sizeof(elevationType);
  if (!GetTokenInformation(userToken.get(), TokenElevationType, &elevationType, sizeof(elevationType), &bytes)) {
    throwSystemError("read interactive token elevation type");
  }

  HANDLE sourceToken = userToken.get();
  UniqueHandle linkedToken;
  if (elevationType == TokenElevationTypeLimited) {
    TOKEN_LINKED_TOKEN linked{};
    bytes = sizeof(linked);
    if (!GetTokenInformation(userToken.get(), TokenLinkedToken, &linked, sizeof(linked), &bytes)) {
      throwSystemError("read linked administrator token");
    }
    linkedToken.reset(linked.LinkedToken);
    sourceToken = linkedToken.get();
  }

  HANDLE duplicated = nullptr;
  if (!DuplicateTokenEx(sourceToken, TOKEN_ALL_ACCESS, nullptr, SecurityImpersonation, TokenPrimary, &duplicated)) {
    throwSystemError("duplicate interactive token");
  }
  UniqueHandle elevatedToken(duplicated);
  if (!tokenIsElevatedAdministrator(elevatedToken.get())) {
    throw std::runtime_error("the active session has no elevated administrator token");
  }
  DWORD targetSessionId = sessionId;
  if (!SetTokenInformation(elevatedToken.get(), TokenSessionId, &targetSessionId, sizeof(targetSessionId))) {
    throwSystemError("assign interactive token session");
  }
  return elevatedToken;
}

struct LaunchedAgent {
  UniqueHandle process;
  DWORD processId = 0;
  DWORD sessionId = 0;
};

LaunchedAgent launchInteractiveAgent() {
  const DWORD sessionId = activeInteractiveSessionId();
  if (sessionId == 0 || sessionId == std::numeric_limits<DWORD>::max()) {
    throw std::runtime_error("no active interactive session is available");
  }

  enablePrivilege(SE_TCB_NAME);
  enablePrivilege(SE_ASSIGNPRIMARYTOKEN_NAME);
  enablePrivilege(SE_INCREASE_QUOTA_NAME);
  auto token = elevatedInteractiveToken(sessionId);
  EnvironmentBlock environment(token.get());
  const std::wstring image = windows::currentExecutablePath();
  std::wstring commandLine = L"\"" + image + L"\" --quiet";

  STARTUPINFOW startup{};
  startup.cb = sizeof(startup);
  wchar_t desktop[] = L"winsta0\\default";
  startup.lpDesktop = desktop;
  PROCESS_INFORMATION process{};
  if (!CreateProcessAsUserW(token.get(), image.c_str(), commandLine.data(), nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_PROCESS_GROUP,
                            environment.get(), nullptr, &startup, &process)) {
    throwSystemError("launch elevated UWF Manager agent");
  }
  UniqueHandle processHandle(process.hProcess);
  const UniqueHandle threadHandle(process.hThread);
  return {std::move(processHandle), process.dwProcessId, sessionId};
}

struct AgentConnection {
  AgentConnection(UniqueHandle pipeHandle, AuthenticatedAgent identity)
      : pipe(std::move(pipeHandle)), processId(identity.processId), sessionId(identity.sessionId), process(std::move(identity.process)) {}
  UniqueHandle pipe;
  DWORD processId = 0;
  DWORD sessionId = 0;
  UniqueHandle process;
};

enum class AgentLaunchObservation {
  Connected,
  ProcessExited,
  TimedOut,
  SessionChanged,
  ServiceStopping,
};

struct AgentLaunchResult {
  AgentLaunchObservation observation = AgentLaunchObservation::TimedOut;
  DWORD processId = 0;
  DWORD exitCode = STILL_ACTIVE;
};

app::FileStagingCommitResult commitStagedFiles() {
  bool taskFailure = false;
  try {
    // 提交任务在自己的工作线程内使用该线程唯一的 WMI 上下文。作用域守卫会在
    // result 完整构造后、函数真正返回前主动释放 COM/WMI，避免把代理释放拖到
    // CRT 的线程退出清理阶段。
    windows::initializeWmiRuntime();
    const auto shutdownWmi = qScopeGuard([] { windows::shutdownWmiRuntime(); });
    auto& session = windows::embeddedWmiSession();
    const auto capability = app::probeUwfCapability(session);
    windows::RegistryFileStagingStore store;
    app::FileStagingTask task(session, store, capability, uwf::windows::systemCommitTraversalSource());
    try {
      while (!task.pollPreparation()) std::this_thread::sleep_for(kCommitPreparationPollInterval);
      while (!task.finished()) {
        static_cast<void>(task.advance());
        task.waitForWorker();
      }
      return task.result();
    } catch (...) {
      taskFailure = task.failed();
      throw;
    }
  } catch (const std::exception& error) {
    if (!taskFailure) UWF_LOG_E("service") << "enhanced mode staged commit startup failed: error=" << error.what();
    throw;
  } catch (...) {
    if (!taskFailure) UWF_LOG_E("service") << "enhanced mode staged commit startup failed: error=non-standard-exception";
    throw;
  }
}

class ServiceRuntime final {
 public:
  static ServiceRuntime& instance() {
    static ServiceRuntime runtime;
    return runtime;
  }

  void run() {
    SERVICE_TABLE_ENTRYW table[] = {{const_cast<LPWSTR>(app::kEnhancedServiceName), &ServiceRuntime::serviceMainThunk}, {nullptr, nullptr}};
    if (!StartServiceCtrlDispatcherW(table)) throwSystemError("start service control dispatcher");
  }

 private:
  struct StatusUpdate {
    DWORD state;
    DWORD controls = 0;
    DWORD checkpoint = 0;
    DWORD waitHint = 0;
  };

  struct ControlRequest {
    DWORD control;
    DWORD eventType;
  };

  static void WINAPI serviceMainThunk(DWORD, LPWSTR*) {
    try {
      instance().serviceMain();
    } catch (...) {
      // ERROR_SERVICE_SPECIFIC_ERROR 要求同时提供非零的
      // dwServiceSpecificExitCode；这里没有可对外承诺的私有错误码，使用通用
      // Win32 失败状态，避免向 SCM 发布内部不一致的停止记录。
      instance().reportStopped(ERROR_GEN_FAILURE);
    }
  }

  static DWORD WINAPI handlerThunk(const DWORD control, const DWORD eventType, void*, void* context) {
    try {
      return static_cast<ServiceRuntime*>(context)->handleControl({.control = control, .eventType = eventType});
    } catch (...) {
      return ERROR_EXCEPTION_IN_SERVICE;
    }
  }

  void serviceMain() {
    m_statusHandle = RegisterServiceCtrlHandlerExW(app::kEnhancedServiceName, &ServiceRuntime::handlerThunk, this);
    if (!m_statusHandle) throwSystemError("register enhanced mode service handler");
    report({.state = SERVICE_START_PENDING, .checkpoint = 1, .waitHint = kServiceWaitHintMs});
    try {
      m_stopEvent.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
      m_preshutdownEvent.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
      m_agentNeededEvent.reset(CreateEventW(nullptr, FALSE, FALSE, nullptr));
      m_agentConnectedEvent.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
      if (!m_stopEvent.valid() || !m_preshutdownEvent.valid() || !m_agentNeededEvent.valid() || !m_agentConnectedEvent.valid()) {
        throwSystemError("create enhanced mode service events");
      }

      m_acceptThread =
          std::jthread([this](const std::stop_token stopToken) { runInfrastructureThread("enhanced mode agent listener", [&] { acceptAgents(stopToken); }); });
      m_supervisorThread = std::jthread(
          [this](const std::stop_token stopToken) { runInfrastructureThread("enhanced mode agent supervisor", [&] { superviseAgent(stopToken); }); });
      SetEvent(m_agentNeededEvent.get());
      report({.state = SERVICE_RUNNING, .controls = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE});

      const HANDLE events[] = {m_stopEvent.get(), m_preshutdownEvent.get()};
      const DWORD wait = WaitForMultipleObjects(2, events, FALSE, INFINITE);
      if (wait != WAIT_OBJECT_0 && wait != WAIT_OBJECT_0 + 1) throwSystemError("wait for enhanced mode service shutdown");
      if (wait == WAIT_OBJECT_0 + 1) {
        // 关机提交不依赖交互会话。先关闭 UI 身份基础设施，防止服务在关机阶段
        // 启动或等待 UI，再由本服务进程内的工作线程独立完成整个批次。
        stopInfrastructure();
        runPreshutdownCommit();
      } else {
        stopInfrastructure();
      }
      reportStopped(m_stopExitCode.load(std::memory_order_acquire));
    } catch (...) {
      if (m_stopEvent.valid()) SetEvent(m_stopEvent.get());
      stopInfrastructure();
      throw;
    }
  }

  DWORD handleControl(const ControlRequest& request) {
    switch (request.control) {
      case SERVICE_CONTROL_STOP:
        if (!m_preshutdownStarted.load(std::memory_order_acquire)) {
          report({.state = SERVICE_STOP_PENDING, .checkpoint = 1, .waitHint = kServiceWaitHintMs});
          SetEvent(m_stopEvent.get());
        }
        return NO_ERROR;
      case SERVICE_CONTROL_PRESHUTDOWN: {
        bool expected = false;
        if (!m_preshutdownStarted.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) return NO_ERROR;
        try {
          report({.state = SERVICE_STOP_PENDING, .checkpoint = nextCheckpoint(), .waitHint = kServiceWaitHintMs});
          if (!SetEvent(m_preshutdownEvent.get())) throwSystemError("request enhanced mode preshutdown");
        } catch (...) {
          m_preshutdownStarted.store(false, std::memory_order_release);
          throw;
        }
        return NO_ERROR;
      }
      case SERVICE_CONTROL_SESSIONCHANGE:
        if (sessionChangeRequiresAgent(request.eventType)) SetEvent(m_agentNeededEvent.get());
        return NO_ERROR;
      case SERVICE_CONTROL_INTERROGATE:
        publishCurrentStatus();
        return NO_ERROR;
      default:
        return ERROR_CALL_NOT_IMPLEMENTED;
    }
  }

  template <typename Work>
  void runInfrastructureThread(const char* operation, Work&& work) noexcept {
    try {
      std::forward<Work>(work)();
    } catch (const std::system_error& error) {
      const auto code = static_cast<DWORD>(error.code().value());
      stopForInfrastructureFailure(operation, code, error.what());
    } catch (const std::exception& error) {
      stopForInfrastructureFailure(operation, ERROR_GEN_FAILURE, error.what());
    } catch (...) {
      stopForInfrastructureFailure(operation, ERROR_GEN_FAILURE, "non-standard-exception");
    }
  }

  void report(const StatusUpdate& update) {
    std::scoped_lock lock(m_statusMutex);
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = update.state;
    m_status.dwControlsAccepted = update.controls;
    m_status.dwWin32ExitCode = NO_ERROR;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = update.checkpoint;
    m_status.dwWaitHint = update.waitHint;
    if (m_statusHandle && !SetServiceStatus(m_statusHandle, &m_status)) throwSystemError("publish enhanced mode service status");
  }

  void publishCurrentStatus() {
    std::scoped_lock lock(m_statusMutex);
    if (m_statusHandle && !SetServiceStatus(m_statusHandle, &m_status)) throwSystemError("republish enhanced mode service status");
  }

  void reportStopped(const DWORD exitCode) noexcept {
    std::scoped_lock lock(m_statusMutex);
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = 0;
    m_status.dwWin32ExitCode = exitCode;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
    if (m_statusHandle) SetServiceStatus(m_statusHandle, &m_status);
  }

  DWORD nextCheckpoint() { return m_checkpoint.fetch_add(1, std::memory_order_relaxed) + 1; }

  void reportRealProgress() { report({.state = SERVICE_STOP_PENDING, .checkpoint = nextCheckpoint(), .waitHint = kServiceWaitHintMs}); }

  void acceptAgents(const std::stop_token stopToken) {
    LocalMemory descriptor;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:P(A;;GA;;;SY)(A;;GA;;;BA)", SDDL_REVISION_1, descriptor.put(), nullptr)) {
      stopForInfrastructureFailure("create enhanced mode pipe security descriptor", GetLastError());
      return;
    }
    SECURITY_ATTRIBUTES security{sizeof(security), descriptor.get(), FALSE};

    while (!stopToken.stop_requested()) {
      // 逻辑上仍只接受一个已认证代理，但原生管道实例的寿命可能略长于逻辑
      // 连接：监督线程持有的 shared_ptr 会让刚断开的 HANDLE 延迟关闭。允许
      // 退场实例与下一监听实例短暂重叠，不能把正常 UI 退出误判成
      // ERROR_PIPE_BUSY 并停止整个服务。
      UniqueHandle pipe(CreateNamedPipeW(app::kEnhancedPipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                         PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, PIPE_UNLIMITED_INSTANCES, 64 * 1024,
                                         64 * 1024, 0, &security));
      if (!pipe.valid()) {
        stopForInfrastructureFailure("create enhanced mode agent pipe", GetLastError());
        return;
      }

      const auto connectionResult = windows::connectEnhancedPipe(pipe.get(), m_stopEvent.get());
      if (connectionResult == windows::EnhancedPipeIoResult::Stopped || stopToken.stop_requested()) {
        DisconnectNamedPipe(pipe.get());
        return;
      }
      if (connectionResult == windows::EnhancedPipeIoResult::Disconnected) continue;
      if (stopToken.stop_requested()) return;
      auto identity = authenticateAgent(pipe.get());
      if (!identity) {
        DisconnectNamedPipe(pipe.get());
        continue;
      }
      const auto handshakeResult = windows::writeEnhancedPipe(pipe.get(), std::as_bytes(std::span{app::kEnhancedAgentHandshake}), m_stopEvent.get());
      if (handshakeResult == windows::EnhancedPipeIoResult::Stopped) {
        DisconnectNamedPipe(pipe.get());
        return;
      }
      if (handshakeResult == windows::EnhancedPipeIoResult::Disconnected) {
        DisconnectNamedPipe(pipe.get());
        continue;
      }

      auto connection = std::make_shared<AgentConnection>(std::move(pipe), std::move(*identity));
      {
        std::scoped_lock lock(m_connectionMutex);
        m_connection = connection;
        SetEvent(m_agentConnectedEvent.get());
      }

      // 管道不承载 UWF 任务；唯一控制消息是安全电源流程在完成 UI 预提交后
      // 武装或撤销一次性 PRESHUTDOWN 跳过令牌。服务原样回显包含请求 ID
      // 的控制帧，确保 UI 不会把延迟确认误认成下一次请求的结果。
      for (;;) {
        app::EnhancedAgentControlFrame frame{};
        const auto readResult = windows::readEnhancedPipe(connection->pipe.get(), std::as_writable_bytes(std::span{frame}), m_stopEvent.get());
        if (readResult != windows::EnhancedPipeIoResult::Completed) break;
        const auto message = app::decodeEnhancedAgentControl(frame);
        if (!message) {
          UWF_LOG_W("service") << "enhanced mode identity channel rejected invalid client control";
          break;
        }
        applyPreshutdownControl(message->control);
        if (windows::writeEnhancedPipe(connection->pipe.get(), std::as_bytes(std::span{frame}), m_stopEvent.get()) != windows::EnhancedPipeIoResult::Completed)
          break;
      }
      discardConnection(connection);
      if (stopToken.stop_requested() || WaitForSingleObject(m_stopEvent.get(), 0) == WAIT_OBJECT_0) return;
    }
  }

  void superviseAgent(const std::stop_token stopToken) {
    auto retryDelay = kAgentRetryInitial;
    bool delayBeforeLaunch = false;

    while (!stopToken.stop_requested()) {
      if (retainHealthyAgent()) {
        retryDelay = kAgentRetryInitial;
        delayBeforeLaunch = false;
        const HANDLE events[] = {m_stopEvent.get(), m_agentNeededEvent.get()};
        // 正常运行期间不轮询管道。用户主动关闭 UI 后，服务保留这个选择；
        // 仅新的交互会话事件或 PRESHUTDOWN 会唤醒监督线程重新评估。
        const DWORD wait = WaitForMultipleObjects(2, events, FALSE, INFINITE);
        if (wait == WAIT_OBJECT_0) return;
        if (wait != WAIT_OBJECT_0 + 1) {
          stopForInfrastructureFailure("wait for enhanced mode agent request", GetLastError());
          return;
        }
        continue;
      }

      if (delayBeforeLaunch) {
        const HANDLE events[] = {m_stopEvent.get(), m_agentNeededEvent.get(), m_agentConnectedEvent.get()};
        const DWORD wait = WaitForMultipleObjects(3, events, FALSE, static_cast<DWORD>(std::chrono::milliseconds{retryDelay}.count()));
        if (wait == WAIT_OBJECT_0) return;
        if (wait == WAIT_OBJECT_0 + 1 || wait == WAIT_OBJECT_0 + 2) {
          delayBeforeLaunch = false;
          continue;
        }
        if (wait != WAIT_TIMEOUT) {
          stopForInfrastructureFailure("wait for enhanced mode agent retry", GetLastError());
          return;
        }
        retryDelay = std::min(retryDelay * 2, kAgentRetryMaximum);
      }

      try {
        auto launched = launchInteractiveAgent();
        UWF_LOG_I("service") << "enhanced mode UI launch started: pid=" << launched.processId << " session=" << launched.sessionId;
        const auto result = observeLaunchedAgent(launched);
        switch (result.observation) {
          case AgentLaunchObservation::Connected:
            UWF_LOG_I("service") << "enhanced mode UI launch confirmed: launchedPid=" << launched.processId << " connectedPid=" << result.processId;
            retryDelay = kAgentRetryInitial;
            delayBeforeLaunch = false;
            continue;
          case AgentLaunchObservation::ServiceStopping:
            return;
          case AgentLaunchObservation::SessionChanged:
            UWF_LOG_I("service") << "enhanced mode UI launch superseded by active-session change: pid=" << launched.processId;
            retryDelay = kAgentRetryInitial;
            delayBeforeLaunch = false;
            continue;
          case AgentLaunchObservation::ProcessExited:
            UWF_LOG_W("service") << "enhanced mode UI exited before agent authentication: pid=" << launched.processId << " exitCode=" << result.exitCode;
            break;
          case AgentLaunchObservation::TimedOut:
            UWF_LOG_W("service") << "enhanced mode UI did not authenticate before timeout: pid=" << launched.processId;
            break;
        }
      } catch (const std::exception& error) {
        UWF_LOG_W("service") << "enhanced mode UI launch failed: error=" << error.what();
      } catch (...) {
        UWF_LOG_W("service") << "enhanced mode UI launch failed: error=non-standard-exception";
      }
      delayBeforeLaunch = true;
    }
  }

  bool retainHealthyAgent() {
    std::shared_ptr<AgentConnection> connection;
    {
      std::scoped_lock lock(m_connectionMutex);
      connection = m_connection;
    }
    if (!connection) return false;
    DWORD available = 0;
    const bool sessionMatches = connection->sessionId == activeInteractiveSessionId();
    const bool processAlive = WaitForSingleObject(connection->process.get(), 0) == WAIT_TIMEOUT;
    if (processAlive && sessionMatches && PeekNamedPipe(connection->pipe.get(), nullptr, 0, nullptr, &available, nullptr)) return true;
    discardConnection(connection);
    return false;
  }

  AgentLaunchResult observeLaunchedAgent(const LaunchedAgent& launched) {
    const auto deadline = std::chrono::steady_clock::now() + kAgentStartupGrace;
    for (;;) {
      if (auto connection = currentConnection(); connection && retainHealthyAgent()) {
        return {AgentLaunchObservation::Connected, connection->processId, STILL_ACTIVE};
      }

      const auto now = std::chrono::steady_clock::now();
      if (now >= deadline) return {AgentLaunchObservation::TimedOut, launched.processId, STILL_ACTIVE};
      const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
      const auto timeout = static_cast<DWORD>(std::min<std::int64_t>(remaining.count(), std::numeric_limits<DWORD>::max()));
      const HANDLE events[] = {m_stopEvent.get(), m_agentNeededEvent.get(), m_agentConnectedEvent.get(), launched.process.get()};
      const DWORD wait = WaitForMultipleObjects(4, events, FALSE, timeout);
      if (wait == WAIT_OBJECT_0) return {AgentLaunchObservation::ServiceStopping, launched.processId, STILL_ACTIVE};
      if (wait == WAIT_OBJECT_0 + 1) {
        if (activeInteractiveSessionId() != launched.sessionId) {
          return {AgentLaunchObservation::SessionChanged, launched.processId, STILL_ACTIVE};
        }
        continue;
      }
      if (wait == WAIT_OBJECT_0 + 2) continue;
      if (wait == WAIT_OBJECT_0 + 3) {
        DWORD exitCode = 0;
        if (!GetExitCodeProcess(launched.process.get(), &exitCode)) exitCode = GetLastError();
        return {AgentLaunchObservation::ProcessExited, launched.processId, exitCode};
      }
      if (wait == WAIT_TIMEOUT) return {AgentLaunchObservation::TimedOut, launched.processId, STILL_ACTIVE};
      throwSystemError("wait for enhanced mode UI authentication");
    }
  }

  std::shared_ptr<AgentConnection> currentConnection() {
    std::scoped_lock lock(m_connectionMutex);
    return m_connection;
  }

  void stopForInfrastructureFailure(const char* operation, const DWORD error, const std::string_view detail = {}) noexcept {
    const DWORD exitCode = error == NO_ERROR ? ERROR_GEN_FAILURE : error;
    DWORD expected = NO_ERROR;
    if (m_stopExitCode.compare_exchange_strong(expected, exitCode, std::memory_order_acq_rel)) {
      try {
        if (detail.empty())
          UWF_LOG_E("service") << operation << " failed: error=" << exitCode;
        else
          UWF_LOG_E("service") << operation << " failed: error=" << detail;
      } catch (...) {
      }
    }
    if (m_stopEvent.valid()) SetEvent(m_stopEvent.get());
  }

  void discardConnection(const std::shared_ptr<AgentConnection>& connection) {
    std::scoped_lock lock(m_connectionMutex);
    if (m_connection == connection) {
      CancelIoEx(connection->pipe.get(), nullptr);
      DisconnectNamedPipe(connection->pipe.get());
      m_connection.reset();
      ResetEvent(m_agentConnectedEvent.get());
    }
  }

  void applyPreshutdownControl(const app::EnhancedAgentControl control) {
    std::scoped_lock lock(m_preshutdownSkipMutex);
    switch (control) {
      case app::EnhancedAgentControl::PreshutdownCommitHandled:
        m_preshutdownSkipDeadline = std::chrono::steady_clock::now() + kPreshutdownSkipLifetime;
        break;
      case app::EnhancedAgentControl::PreshutdownCommitRequired:
        m_preshutdownSkipDeadline.reset();
        break;
    }
  }

  [[nodiscard]] bool consumePreshutdownCommitHandled() {
    std::scoped_lock lock(m_preshutdownSkipMutex);
    if (!m_preshutdownSkipDeadline) return false;
    const bool valid = std::chrono::steady_clock::now() <= *m_preshutdownSkipDeadline;
    m_preshutdownSkipDeadline.reset();
    return valid;
  }

  void runPreshutdownCommit() {
    if (consumePreshutdownCommitHandled()) {
      UWF_LOG_I("service") << "enhanced mode staged commit skipped: reason=recent-ui-precommit";
      return;
    }

    std::promise<app::FileStagingCommitResult> resultPromise;
    auto resultFuture = resultPromise.get_future();
    std::jthread worker([promise = std::move(resultPromise)]() mutable {
      try {
        promise.set_value(commitStagedFiles());
      } catch (...) {
        promise.set_exception(std::current_exception());
      }
    });

    bool heartbeatAvailable = true;
    while (resultFuture.wait_for(kServiceHeartbeatInterval) != std::future_status::ready) {
      if (!heartbeatAvailable) continue;
      try {
        reportRealProgress();
      } catch (const std::exception& error) {
        heartbeatAvailable = false;
        UWF_LOG_W("service") << "enhanced mode SCM heartbeat failed while staged commit continues: error=" << error.what();
      } catch (...) {
        heartbeatAvailable = false;
        UWF_LOG_W("service") << "enhanced mode SCM heartbeat failed while staged commit continues: error=non-standard-exception";
      }
    }
    worker.join();
    try {
      const auto result = resultFuture.get();
      UWF_LOG_I("service") << "enhanced mode staged commit completed: discovered=" << result.discoveredFiles << " committed=" << result.committedFiles
                           << " skippedFiles=" << result.skippedFiles << " skippedEntries=" << result.skippedEntries << " failures=" << result.failures.size();
    } catch (...) {
      // commitStagedFiles (or its task) already recorded this actual failure.
    }
  }

  void stopInfrastructure() {
    if (m_supervisorThread.joinable()) {
      m_supervisorThread.request_stop();
      SetEvent(m_stopEvent.get());
      m_supervisorThread.join();
    }
    if (m_acceptThread.joinable()) {
      m_acceptThread.request_stop();
      SetEvent(m_stopEvent.get());
      m_acceptThread.join();
    }
  }

  SERVICE_STATUS_HANDLE m_statusHandle = nullptr;
  SERVICE_STATUS m_status{};
  std::mutex m_statusMutex;
  std::atomic_uint32_t m_checkpoint{1};
  std::atomic_bool m_preshutdownStarted{false};
  std::atomic<DWORD> m_stopExitCode{NO_ERROR};
  UniqueHandle m_stopEvent;
  UniqueHandle m_preshutdownEvent;
  UniqueHandle m_agentNeededEvent;
  UniqueHandle m_agentConnectedEvent;
  std::jthread m_acceptThread;
  std::jthread m_supervisorThread;
  std::mutex m_connectionMutex;
  std::shared_ptr<AgentConnection> m_connection;
  std::mutex m_preshutdownSkipMutex;
  std::optional<std::chrono::steady_clock::time_point> m_preshutdownSkipDeadline;
};

}  // namespace

int runEnhancedModeService() {
  ServiceRuntime::instance().run();
  return 0;
}

}  // namespace uwf::service
