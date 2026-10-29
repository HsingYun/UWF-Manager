// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SecureSingleInstance.h"

#include <windows.h>

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QHash>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>
#include <QQueue>
#include <QString>
#include <algorithm>
#include <array>
#include <atomic>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "windows/resources/UniqueHandle.h"
#include "windows/security/ExecutableIdentity.h"

namespace uwf::windows {

namespace {

using windows::UniqueHandle;

struct ProcessIdentity {
  std::vector<BYTE> userBuffer;
  DWORD integrityRid = 0;
  DWORD sessionId = 0;
  bool elevated = false;

  [[nodiscard]] PSID userSid() const {
    if (userBuffer.empty()) return nullptr;
    return reinterpret_cast<const TOKEN_USER*>(userBuffer.data())->User.Sid;
  }
};

bool readProcessIdentity(const HANDLE process, ProcessIdentity& identity) {
  HANDLE rawToken = nullptr;
  if (!OpenProcessToken(process, TOKEN_QUERY, &rawToken)) return false;
  const UniqueHandle token(rawToken);

  DWORD size = 0;
  GetTokenInformation(token.get(), TokenUser, nullptr, 0, &size);
  if (size == 0) return false;
  identity.userBuffer.resize(size);
  if (!GetTokenInformation(token.get(), TokenUser, identity.userBuffer.data(), size, &size)) return false;

  TOKEN_ELEVATION elevation{};
  size = static_cast<DWORD>(sizeof(elevation));
  if (!GetTokenInformation(token.get(), TokenElevation, &elevation, size, &size)) return false;
  identity.elevated = elevation.TokenIsElevated != 0;

  size = 0;
  GetTokenInformation(token.get(), TokenIntegrityLevel, nullptr, 0, &size);
  if (size == 0) return false;
  std::vector<BYTE> integrityBuffer(size);
  if (!GetTokenInformation(token.get(), TokenIntegrityLevel, integrityBuffer.data(), size, &size)) return false;
  const auto* label = reinterpret_cast<const TOKEN_MANDATORY_LABEL*>(integrityBuffer.data());
  const UCHAR* count = GetSidSubAuthorityCount(label->Label.Sid);
  if (!count || *count == 0) return false;
  identity.integrityRid = *GetSidSubAuthority(label->Label.Sid, static_cast<DWORD>(*count - 1));
  return ProcessIdToSessionId(GetProcessId(process), &identity.sessionId) != FALSE;
}

QByteArray fileSha256(const std::wstring& path) {
  constexpr qint64 kHashChunkSize = qint64{64} * 1024;
  QFile file(QString::fromStdWString(path));
  if (!file.open(QIODevice::ReadOnly)) return {};
  QCryptographicHash hash(QCryptographicHash::Sha256);
  while (!file.atEnd()) {
    const QByteArray chunk = file.read(kHashChunkSize);
    if (file.error() != QFileDevice::NoError) return {};
    hash.addData(chunk);
  }
  return hash.result();
}

bool sameExecutable(const std::wstring& lhs, const std::wstring& rhs) {
  if (lhs.empty() || rhs.empty()) return false;
  if (windows::sameFile(lhs, rhs, windows::FileIdentityFormat::FileIndex64)) return true;
  // CMake 会把 UWF.exe 复制成带版本号的独立文件；文件 ID 不同但内容完全一致，
  // 仍应视为同一应用。只有令牌身份已经匹配后才走哈希，普通权限抢注者到不了这里。
  const QByteArray lhsHash = fileSha256(lhs);
  return !lhsHash.isEmpty() && lhsHash == fileSha256(rhs);
}

bool isTrustedPeerProcess(const DWORD processId) {
  if (processId == 0 || processId == GetCurrentProcessId()) return false;
  const UniqueHandle peer(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId));
  if (!peer.valid()) return false;

  ProcessIdentity selfIdentity;
  ProcessIdentity peerIdentity;
  if (!readProcessIdentity(GetCurrentProcess(), selfIdentity) || !readProcessIdentity(peer.get(), peerIdentity) || !selfIdentity.userSid() ||
      !peerIdentity.userSid()) {
    return false;
  }
  if (!EqualSid(selfIdentity.userSid(), peerIdentity.userSid()) || selfIdentity.elevated != peerIdentity.elevated ||
      selfIdentity.integrityRid != peerIdentity.integrityRid || selfIdentity.sessionId != peerIdentity.sessionId) {
    return false;
  }
  return sameExecutable(windows::processImagePath(GetCurrentProcess()), windows::processImagePath(peer.get()));
}

QString currentUserSid() {
  ProcessIdentity identity;
  if (!readProcessIdentity(GetCurrentProcess(), identity) || !identity.userSid()) return {};
  const DWORD sidLength = GetLengthSid(identity.userSid());
  if (sidLength == 0) return {};
  const auto* sidBytes = static_cast<const char*>(identity.userSid());
  return QString::fromLatin1(QByteArray(sidBytes, static_cast<qsizetype>(sidLength)).toHex());
}

// SID + 会话号共同限定单实例范围；不依赖可伪造的环境变量 USERNAME。
QString instanceServerName(const QString& discriminator) {
  DWORD sessionId = 0;
  ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
  const QString sid = currentUserSid();
  QString name = QStringLiteral("UWFManager.SingleInstance.%1.%2").arg(sid.isEmpty() ? QStringLiteral("UnknownSid") : sid).arg(sessionId);
  if (!discriminator.isEmpty()) {
    const QByteArray digest = QCryptographicHash::hash(discriminator.toUtf8(), QCryptographicHash::Sha256).toHex();
    name += QLatin1Char('.') + QString::fromLatin1(digest);
  }
  return name;
}

// 不能用 QLocalSocket 直接连接未知服务端：Windows 命名管道默认允许服务端
// impersonate 客户端，高权限进程连接被抢注的管道会泄露自己的安全上下文。
bool writeAll(const HANDLE pipe, const QByteArray& bytes) {
  qsizetype offset = 0;
  while (offset < bytes.size()) {
    const qsizetype remaining = bytes.size() - offset;
    const DWORD chunk = static_cast<DWORD>(std::min<qsizetype>(remaining, static_cast<qsizetype>(std::numeric_limits<DWORD>::max())));
    DWORD written = 0;
    if (!WriteFile(pipe, bytes.constData() + offset, chunk, &written, nullptr) || written == 0) return false;
    offset += static_cast<qsizetype>(written);
  }
  return true;
}

std::optional<QByteArray> readFrame(const HANDLE pipe) {
  QByteArray bytes;
  std::array<char, 4096> chunk{};
  std::size_t expectedSize = 0;
  for (;;) {
    DWORD read = 0;
    if (!ReadFile(pipe, chunk.data(), static_cast<DWORD>(chunk.size()), &read, nullptr)) return std::nullopt;
    if (read == 0) return std::nullopt;
    bytes.append(chunk.data(), static_cast<qsizetype>(read));
    if (expectedSize == 0) expectedSize = app::applicationCommandFrameSize(bytes);
    if (expectedSize != 0 && static_cast<std::size_t>(bytes.size()) >= expectedSize) {
      if (static_cast<std::size_t>(bytes.size()) != expectedSize) return std::nullopt;
      return bytes;
    }
  }
}

std::uint64_t nextWireRequestId() {
  static std::atomic_uint64_t sequence{1};
  return (static_cast<std::uint64_t>(GetCurrentProcessId()) << 32U) ^ sequence.fetch_add(1, std::memory_order_relaxed);
}

bool forwardToRunningInstance(const QString& serverName, const app::ApplicationCommandKind command, app::ApplicationCommandResult* result) {
  const std::wstring pipeName = (QStringLiteral("\\\\.\\pipe\\") + serverName).toStdWString();
  if (!WaitNamedPipeW(pipeName.c_str(), 300)) return false;

  constexpr DWORD kSecurityFlags = SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION | SECURITY_EFFECTIVE_ONLY;
  const UniqueHandle pipe(CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, kSecurityFlags, nullptr));
  if (!pipe.valid()) return false;

  ULONG serverProcessId = 0;
  if (!GetNamedPipeServerProcessId(pipe.get(), &serverProcessId) || !isTrustedPeerProcess(static_cast<DWORD>(serverProcessId))) return false;

  if (command == app::ApplicationCommandKind::Activate) AllowSetForegroundWindow(static_cast<DWORD>(serverProcessId));
  const std::uint64_t requestId = nextWireRequestId();
  if (!writeAll(pipe.get(), app::encodeCommandRequest({command, requestId}))) return false;
  const auto responseBytes = readFrame(pipe.get());
  if (!responseBytes) return false;
  const auto response = app::decodeCommandResult(*responseBytes);
  if (!response || response->first != requestId) return false;
  if (result) *result = response->second;
  return true;
}

bool isTrustedLocalSocket(QLocalSocket* socket) {
  if (!socket || socket->socketDescriptor() == -1) return false;
  ULONG clientProcessId = 0;
  const HANDLE pipe = reinterpret_cast<HANDLE>(socket->socketDescriptor());
  return GetNamedPipeClientProcessId(pipe, &clientProcessId) != FALSE && isTrustedPeerProcess(static_cast<DWORD>(clientProcessId));
}

}  // namespace

class SecureSingleInstance::Impl final {
 public:
  explicit Impl(QString name) : serverName(std::move(name)) {}

  struct PendingResponse {
    QPointer<QLocalSocket> socket;
    std::uint64_t wireRequestId = 0;
  };

  QLocalServer server;
  QString serverName;
  QString error;
  bool acquired = false;
  AcquireResult result = AcquireResult::Unprotected;
  bool notificationsEnabled = false;
  bool activationPending = false;
  std::uint64_t nextRequestToken = 1;
  QHash<QLocalSocket*, QByteArray> buffers;
  QHash<std::uint64_t, PendingResponse> pendingResponses;
  QQueue<std::uint64_t> pendingCommitNotifications;
};

SecureSingleInstance::SecureSingleInstance(QObject* parent) : SecureSingleInstance(Scope{}, parent) {}

SecureSingleInstance::SecureSingleInstance(const Scope& scope, QObject* parent)
    : QObject(parent), m_impl(std::make_unique<Impl>(instanceServerName(scope.discriminator))) {
  connect(&m_impl->server, &QLocalServer::newConnection, this, &SecureSingleInstance::processPendingConnections);
}

SecureSingleInstance::~SecureSingleInstance() = default;

SecureSingleInstance::AcquireResult SecureSingleInstance::acquire(const app::ApplicationCommandKind command, app::ApplicationCommandResult* forwardedResult) {
  if (m_impl->acquired) return m_impl->result;
  m_impl->acquired = true;
  const QString& serverName = m_impl->serverName;

  if (forwardToRunningInstance(serverName, command, forwardedResult)) {
    m_impl->result = AcquireResult::ForwardedExisting;
    return m_impl->result;
  }

  m_impl->server.setSocketOptions(QLocalServer::UserAccessOption);
  if (m_impl->server.listen(serverName)) {
    m_impl->result = AcquireResult::Primary;
    return m_impl->result;
  }

  // probe 与 listen 之间可能被另一实例抢注，认证并重试一次转交。
  if (forwardToRunningInstance(serverName, command, forwardedResult)) {
    m_impl->result = AcquireResult::ForwardedExisting;
    return m_impl->result;
  }
  m_impl->error = m_impl->server.errorString();
  return m_impl->result;
}

QString SecureSingleInstance::errorString() const { return m_impl->error; }

void SecureSingleInstance::enableCommandNotifications() {
  m_impl->notificationsEnabled = true;
  // newConnection 可能在外部建立信号连接之前已经发出；主动清空 server
  // 队列，并投递此前已完成认证的命令。
  processPendingConnections();
  deliverPendingCommands();
}

void SecureSingleInstance::processPendingConnections() {
  while (QLocalSocket* socket = m_impl->server.nextPendingConnection()) {
    if (!isTrustedLocalSocket(socket)) {
      socket->abort();
      socket->deleteLater();
      continue;
    }
    m_impl->buffers.insert(socket, {});
    connect(socket, &QLocalSocket::readyRead, this, [this, socket] { processSocket(socket); });
    connect(socket, &QLocalSocket::disconnected, this, [this, socket] {
      m_impl->buffers.remove(socket);
      for (auto it = m_impl->pendingResponses.begin(); it != m_impl->pendingResponses.end();) {
        if (it->socket == socket)
          it = m_impl->pendingResponses.erase(it);
        else
          ++it;
      }
      socket->deleteLater();
    });
    processSocket(socket);
  }
}

void SecureSingleInstance::processSocket(QLocalSocket* socket) {
  auto buffer = m_impl->buffers.find(socket);
  if (buffer == m_impl->buffers.end() || !socket) return;
  buffer.value().append(socket->readAll());

  try {
    const auto expectedSize = app::applicationCommandFrameSize(buffer.value());
    if (expectedSize == 0 || static_cast<std::size_t>(buffer.value().size()) < expectedSize) return;
    const auto request = app::decodeCommandRequest(buffer.value());
    if (!request) return;
    m_impl->buffers.erase(buffer);

    if (request->kind == app::ApplicationCommandKind::Activate) {
      m_impl->activationPending = true;
      socket->write(app::encodeCommandResult(request->requestId, {}));
      socket->flush();
      socket->disconnectFromServer();
      deliverPendingCommands();
      return;
    }
    if (request->kind == app::ApplicationCommandKind::EnsureRunning) {
      socket->write(app::encodeCommandResult(request->requestId, {}));
      socket->flush();
      socket->disconnectFromServer();
      return;
    }

    const std::uint64_t requestToken = m_impl->nextRequestToken++;
    m_impl->pendingResponses.insert(requestToken, {socket, request->requestId});
    m_impl->pendingCommitNotifications.enqueue(requestToken);
    deliverPendingCommands();
  } catch (const std::exception&) {
    m_impl->buffers.remove(socket);
    socket->abort();
  }
}

void SecureSingleInstance::deliverPendingCommands() {
  if (!m_impl->notificationsEnabled) return;
  if (m_impl->activationPending) {
    m_impl->activationPending = false;
    emit activationRequested();
  }
  while (!m_impl->pendingCommitNotifications.isEmpty()) {
    const std::uint64_t requestToken = m_impl->pendingCommitNotifications.dequeue();
    if (m_impl->pendingResponses.contains(requestToken)) emit commitStageRequested(requestToken);
  }
}

void SecureSingleInstance::completeCommand(const std::uint64_t requestToken, const app::ApplicationCommandResult& result) {
  const auto pendingIt = m_impl->pendingResponses.constFind(requestToken);
  if (pendingIt == m_impl->pendingResponses.cend()) return;
  if (!pendingIt->socket) {
    m_impl->pendingResponses.remove(requestToken);
    return;
  }
  const QPointer<QLocalSocket> socket = pendingIt->socket;
  const std::uint64_t wireRequestId = pendingIt->wireRequestId;

  QByteArray response;
  try {
    response = app::encodeCommandResult(wireRequestId, result);
  } catch (...) {
    m_impl->pendingResponses.remove(requestToken);
    if (socket) socket->abort();
    throw;
  }

  const auto pending = m_impl->pendingResponses.take(requestToken);
  if (!pending.socket) return;
  pending.socket->write(response);
  pending.socket->flush();
  pending.socket->disconnectFromServer();
}

}  // namespace uwf::windows

#include "moc_SecureSingleInstance.cpp"
