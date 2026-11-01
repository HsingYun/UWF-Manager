// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/service/EnhancedModeAgentHost.h"

// MinGW's sddl.h requires Win32 declarations first.
// clang-format off
#include <sddl.h>
#include <userenv.h>
#include <wtsapi32.h>
// clang-format on

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
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

#include "foundation/logging/Log.h"
#include "infrastructure/windows/ipc/EnhancedModeProtocol.h"
#include "infrastructure/windows/ipc/NamedPipeIo.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/security/ExecutableIdentity.h"
#include "infrastructure/windows/service/AgentSupervision.h"
#include "infrastructure/windows/service/PreshutdownCommitGate.h"

namespace uwf::windows {
namespace {
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

  // These callers supply primary tokens. Inspect TokenGroups directly because
  // CheckTokenMembership requires an impersonation token when one is supplied.
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

}  // namespace

class EnhancedModeAgentHost::Impl final {
 public:
  explicit Impl(PreshutdownCommitGate& commitGate) : m_commitGate(commitGate) {
    m_stopEvent.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    m_agentNeededEvent.reset(CreateEventW(nullptr, FALSE, FALSE, nullptr));
    m_agentConnectedEvent.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    m_failureEvent.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    if (!m_stopEvent.valid() || !m_agentNeededEvent.valid() || !m_agentConnectedEvent.valid() || !m_failureEvent.valid())
      throwSystemError("create enhanced mode agent events");
  }
  ~Impl() { stop(); }

  void start() {
    m_acceptThread =
        std::jthread([this](const std::stop_token stopToken) { runInfrastructureThread("enhanced mode agent listener", [&] { acceptAgents(stopToken); }); });
    m_supervisorThread = std::jthread(
        [this](const std::stop_token stopToken) { runInfrastructureThread("enhanced mode agent supervisor", [&] { superviseAgent(stopToken); }); });
    requestAgent();
  }

  void requestAgent() { SetEvent(m_agentNeededEvent.get()); }
  [[nodiscard]] HANDLE failureEvent() const noexcept { return m_failureEvent.get(); }
  [[nodiscard]] std::uint32_t failureCode() const noexcept { return m_failureCode.load(std::memory_order_acquire); }

  void stop() {
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

 private:
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

  void acceptAgents(const std::stop_token stopToken) {
    LocalMemory descriptor;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:P(A;;GA;;;SY)(A;;GA;;;BA)", SDDL_REVISION_1, descriptor.put(), nullptr)) {
      stopForInfrastructureFailure("create enhanced mode pipe security descriptor", GetLastError());
      return;
    }
    SECURITY_ATTRIBUTES security{sizeof(security), descriptor.get(), FALSE};

    while (!stopToken.stop_requested()) {
      // A supervisor-held connection may outlive disconnection. Allow retiring
      // and listening pipe instances to overlap while authenticating one agent.
      UniqueHandle pipe(CreateNamedPipeW(kEnhancedPipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                         PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, PIPE_UNLIMITED_INSTANCES, 64 * 1024,
                                         64 * 1024, 0, &security));
      if (!pipe.valid()) {
        stopForInfrastructureFailure("create enhanced mode agent pipe", GetLastError());
        return;
      }

      const auto connectionResult = windows::connectNamedPipe(pipe.get(), {m_stopEvent.get()});
      if (connectionResult == windows::NamedPipeIoResult::Stopped || stopToken.stop_requested()) {
        DisconnectNamedPipe(pipe.get());
        return;
      }
      if (connectionResult != windows::NamedPipeIoResult::Completed) continue;
      if (stopToken.stop_requested()) return;
      auto identity = authenticateAgent(pipe.get());
      if (!identity) {
        DisconnectNamedPipe(pipe.get());
        continue;
      }
      const auto handshakeResult = windows::writeNamedPipe(pipe.get(), std::as_bytes(std::span{kEnhancedAgentHandshake}),
                                                           {m_stopEvent.get(), std::chrono::steady_clock::now() + kEnhancedAgentExchangeTimeout});
      if (handshakeResult == windows::NamedPipeIoResult::Stopped) {
        DisconnectNamedPipe(pipe.get());
        return;
      }
      if (handshakeResult != windows::NamedPipeIoResult::Completed) {
        DisconnectNamedPipe(pipe.get());
        continue;
      }

      auto connection = std::make_shared<AgentConnection>(std::move(pipe), std::move(*identity));
      {
        std::scoped_lock lock(m_connectionMutex);
        m_connection = connection;
        SetEvent(m_agentConnectedEvent.get());
      }

      // Control frames arm or revoke the one-shot PRESHUTDOWN skip token.
      // Echo the request ID so delayed acknowledgements cannot satisfy a later request.
      for (;;) {
        EnhancedAgentControlFrame frame{};
        const auto frameRead =
            windows::readNamedPipeFrame(connection->pipe.get(), std::as_writable_bytes(std::span{frame}), m_stopEvent.get(), kEnhancedAgentExchangeTimeout);
        if (frameRead.result != windows::NamedPipeIoResult::Completed) break;
        const auto message = decodeEnhancedAgentControl(frame);
        if (!message) {
          UWF_LOG_W("service") << "enhanced mode identity channel rejected invalid client control";
          break;
        }
        applyPreshutdownControl(message->control);
        if (windows::writeNamedPipe(connection->pipe.get(), std::as_bytes(std::span{frame}), frameRead.exchange) != windows::NamedPipeIoResult::Completed)
          break;
      }
      discardConnection(connection);
      if (stopToken.stop_requested() || WaitForSingleObject(m_stopEvent.get(), 0) == WAIT_OBJECT_0) return;
    }
  }

  void superviseAgent(const std::stop_token stopToken) {
    const auto healthyAgent = [this]() -> std::optional<std::uint32_t> {
      if (const auto connection = currentConnection(); connection && retainHealthyAgent()) return connection->processId;
      return {};
    };
    const auto launch = [this] {
      auto launched = std::make_shared<LaunchedAgent>(launchInteractiveAgent());
      return AgentLaunch{launched->processId, launched->sessionId,
                         [this, launched = std::move(launched)](const std::chrono::milliseconds timeout) { return waitForLaunchedAgent(*launched, timeout); }};
    };
    const AgentSupervisionEnvironment environment{.now = [] { return std::chrono::steady_clock::now(); },
                                                  .activeSession = activeInteractiveSessionId,
                                                  .healthyAgent = healthyAgent,
                                                  .launch = launch,
                                                  .waitForRequest = [this] { return waitForAgentRequest(); },
                                                  .waitBeforeRetry = [this](const std::chrono::milliseconds delay) { return waitBeforeAgentRetry(delay); }};
    runAgentSupervision(environment, stopToken);
  }

  AgentSupervisionEvent waitForAgentRequest() {
    const HANDLE events[] = {m_stopEvent.get(), m_agentNeededEvent.get()};
    const DWORD wait = WaitForMultipleObjects(2, events, FALSE, INFINITE);
    if (wait == WAIT_OBJECT_0) return AgentSupervisionEvent::Stopped;
    if (wait == WAIT_OBJECT_0 + 1) return AgentSupervisionEvent::Requested;
    throwSystemError("wait for enhanced mode agent request");
  }

  AgentSupervisionEvent waitBeforeAgentRetry(const std::chrono::milliseconds delay) {
    const HANDLE events[] = {m_stopEvent.get(), m_agentNeededEvent.get(), m_agentConnectedEvent.get()};
    const DWORD wait = WaitForMultipleObjects(3, events, FALSE, static_cast<DWORD>(delay.count()));
    if (wait == WAIT_OBJECT_0) return AgentSupervisionEvent::Stopped;
    if (wait == WAIT_OBJECT_0 + 1) return AgentSupervisionEvent::Requested;
    if (wait == WAIT_OBJECT_0 + 2) return AgentSupervisionEvent::Connected;
    if (wait == WAIT_TIMEOUT) return AgentSupervisionEvent::TimedOut;
    throwSystemError("wait for enhanced mode agent retry");
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

  AgentLaunchWaitResult waitForLaunchedAgent(const LaunchedAgent& launched, const std::chrono::milliseconds remaining) {
    const auto timeout = static_cast<DWORD>(std::min<std::int64_t>(remaining.count(), std::numeric_limits<DWORD>::max()));
    const HANDLE events[] = {m_stopEvent.get(), m_agentNeededEvent.get(), m_agentConnectedEvent.get(), launched.process.get()};
    const DWORD wait = WaitForMultipleObjects(4, events, FALSE, timeout);
    if (wait == WAIT_OBJECT_0) return {AgentSupervisionEvent::Stopped};
    if (wait == WAIT_OBJECT_0 + 1) return {AgentSupervisionEvent::Requested};
    if (wait == WAIT_OBJECT_0 + 2) return {AgentSupervisionEvent::Connected};
    if (wait == WAIT_OBJECT_0 + 3) {
      DWORD exitCode = 0;
      if (!GetExitCodeProcess(launched.process.get(), &exitCode)) exitCode = GetLastError();
      return {AgentSupervisionEvent::ProcessExited, exitCode};
    }
    if (wait == WAIT_TIMEOUT) return {AgentSupervisionEvent::TimedOut};
    throwSystemError("wait for enhanced mode UI authentication");
  }

  std::shared_ptr<AgentConnection> currentConnection() {
    std::scoped_lock lock(m_connectionMutex);
    return m_connection;
  }

  void stopForInfrastructureFailure(const char* operation, const DWORD error, const std::string_view detail = {}) noexcept {
    const DWORD exitCode = error == NO_ERROR ? ERROR_GEN_FAILURE : error;
    DWORD expected = NO_ERROR;
    if (m_failureCode.compare_exchange_strong(expected, exitCode, std::memory_order_acq_rel)) {
      try {
        if (detail.empty())
          UWF_LOG_E("service") << operation << " failed: error=" << exitCode;
        else
          UWF_LOG_E("service") << operation << " failed: error=" << detail;
      } catch (...) {
      }
    }
    if (m_stopEvent.valid()) SetEvent(m_stopEvent.get());
    SetEvent(m_failureEvent.get());
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

  void applyPreshutdownControl(const EnhancedAgentControl control) {
    switch (control) {
      case EnhancedAgentControl::PreshutdownCommitHandled:
        m_commitGate.markHandled();
        break;
      case EnhancedAgentControl::PreshutdownCommitRequired:
        m_commitGate.requireCommit();
        break;
    }
  }

  PreshutdownCommitGate& m_commitGate;
  std::atomic<DWORD> m_failureCode{NO_ERROR};
  UniqueHandle m_stopEvent;
  UniqueHandle m_agentNeededEvent;
  UniqueHandle m_agentConnectedEvent;
  UniqueHandle m_failureEvent;
  std::jthread m_acceptThread;
  std::jthread m_supervisorThread;
  std::mutex m_connectionMutex;
  std::shared_ptr<AgentConnection> m_connection;
};

EnhancedModeAgentHost::EnhancedModeAgentHost(PreshutdownCommitGate& commitGate) : m_impl(std::make_unique<Impl>(commitGate)) { m_impl->start(); }
EnhancedModeAgentHost::~EnhancedModeAgentHost() = default;
void EnhancedModeAgentHost::requestAgent() { m_impl->requestAgent(); }
void EnhancedModeAgentHost::stop() { m_impl->stop(); }
HANDLE EnhancedModeAgentHost::failureEvent() const noexcept { return m_impl->failureEvent(); }
std::uint32_t EnhancedModeAgentHost::failureCode() const noexcept { return m_impl->failureCode(); }
}  // namespace uwf::windows
