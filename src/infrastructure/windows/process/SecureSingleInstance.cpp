// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/process/SecureSingleInstance.h"

#include <windows.h>

#include <QByteArray>
#include <QByteArrayView>
#include <QCryptographicHash>
#include <QHash>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <atomic>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/windows/filesystem/WindowsFileSystem.h"
#include "infrastructure/windows/ipc/ApplicationCommandProtocol.h"
#include "infrastructure/windows/ipc/NamedPipeIo.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/security/ExecutableIdentity.h"

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
  try {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    const auto result = winfs::readChunks(std::filesystem::path(path), [&](const std::span<const std::byte> chunk) {
      hash.addData(QByteArrayView(reinterpret_cast<const char*>(chunk.data()), static_cast<qsizetype>(chunk.size())));
      return true;
    });
    return result.failure == winfs::FileIoFailure::None ? hash.result() : QByteArray{};
  } catch (const std::exception&) {
    return {};
  }
}

bool sameExecutable(const std::wstring& lhs, const std::wstring& rhs) {
  if (lhs.empty() || rhs.empty()) return false;
  if (windows::sameFile(lhs, rhs, windows::FileIdentityFormat::FileIndex64)) return true;
  // Versioned artifact copies have different file IDs. Compare their contents
  // only after user, session, elevation and integrity have matched.
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

// Scope instances by token SID and session, independently of environment names.
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

// Native connection flags limit server impersonation before peer authentication;
// connecting with QLocalSocket defaults could expose the elevated token.
std::optional<QByteArray> readAcknowledgementFrame(const HANDLE pipe, const NamedPipeIoContext& context) {
  QByteArray bytes(static_cast<qsizetype>(kApplicationCommandHeaderSize), Qt::Uninitialized);
  if (readNamedPipe(pipe, std::as_writable_bytes(std::span{bytes.data(), static_cast<std::size_t>(bytes.size())}), context) != NamedPipeIoResult::Completed)
    return std::nullopt;
  const auto expectedSize = applicationCommandFrameSize(bytes);
  if (expectedSize > kApplicationCommandHeaderSize) {
    bytes.resize(static_cast<qsizetype>(expectedSize));
    const auto body = std::span{bytes.data(), static_cast<std::size_t>(bytes.size())}.subspan(kApplicationCommandHeaderSize);
    if (readNamedPipe(pipe, std::as_writable_bytes(body), context) != NamedPipeIoResult::Completed) return std::nullopt;
  }
  // One exchange per connection; peer closure rejects trailing bytes within
  // the same deadline.
  std::byte trailing{};
  if (readNamedPipe(pipe, std::span{&trailing, 1}, context) != NamedPipeIoResult::Disconnected) return std::nullopt;
  return bytes;
}

std::uint64_t nextWireRequestId() {
  static std::atomic_uint64_t sequence{1};
  return (static_cast<std::uint64_t>(GetCurrentProcessId()) << 32U) ^ sequence.fetch_add(1, std::memory_order_relaxed);
}

bool forwardToRunningInstance(const QString& serverName, const app::ApplicationCommandKind command) {
  const std::wstring pipeName = (QStringLiteral("\\\\.\\pipe\\") + serverName).toStdWString();
  constexpr DWORD kConnectionWaitMs = 300;
  if (!WaitNamedPipeW(pipeName.c_str(), kConnectionWaitMs)) return false;

  constexpr DWORD kSecurityFlags = SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION | SECURITY_EFFECTIVE_ONLY;
  const UniqueHandle pipe(
      CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, kSecurityFlags | FILE_FLAG_OVERLAPPED, nullptr));
  if (!pipe.valid()) return false;

  ULONG serverProcessId = 0;
  if (!GetNamedPipeServerProcessId(pipe.get(), &serverProcessId) || !isTrustedPeerProcess(static_cast<DWORD>(serverProcessId))) return false;

  if (command == app::ApplicationCommandKind::Activate) AllowSetForegroundWindow(static_cast<DWORD>(serverProcessId));
  const std::uint64_t requestId = nextWireRequestId();
  const NamedPipeIoContext context{nullptr, std::chrono::steady_clock::now() + kApplicationCommandExchangeTimeout};
  const auto request = encodeCommandRequest({command, requestId});
  if (writeNamedPipe(pipe.get(), std::as_bytes(std::span{request.constData(), static_cast<std::size_t>(request.size())}), context) !=
      NamedPipeIoResult::Completed)
    return false;
  const auto responseBytes = readAcknowledgementFrame(pipe.get(), context);
  if (!responseBytes) return false;
  const auto response = decodeCommandAcknowledgement(*responseBytes);
  return response && *response == requestId;
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

  QLocalServer server;
  QString serverName;
  QString error;
  bool acquired = false;
  AcquireResult result = AcquireResult::Unprotected;
  bool notificationsEnabled = false;
  bool activationPending = false;
  QHash<QLocalSocket*, QByteArray> buffers;
};

SecureSingleInstance::SecureSingleInstance(QObject* parent) : SecureSingleInstance(Scope{}, parent) {}

SecureSingleInstance::SecureSingleInstance(const Scope& scope, QObject* parent)
    : QObject(parent), m_impl(std::make_unique<Impl>(instanceServerName(scope.discriminator))) {
  connect(&m_impl->server, &QLocalServer::newConnection, this, &SecureSingleInstance::processPendingConnections);
}

SecureSingleInstance::~SecureSingleInstance() = default;

SecureSingleInstance::AcquireResult SecureSingleInstance::acquire(const app::ApplicationCommandKind command) {
  if (command != app::ApplicationCommandKind::Activate && command != app::ApplicationCommandKind::EnsureRunning)
    throw std::invalid_argument("The UI instance channel accepts only Activate or EnsureRunning");
  if (m_impl->acquired) return m_impl->result;
  m_impl->acquired = true;
  const QString& serverName = m_impl->serverName;

  if (forwardToRunningInstance(serverName, command)) {
    m_impl->result = AcquireResult::ForwardedExisting;
    return m_impl->result;
  }

  m_impl->server.setSocketOptions(QLocalServer::UserAccessOption);
  if (m_impl->server.listen(serverName)) {
    m_impl->result = AcquireResult::Primary;
    return m_impl->result;
  }

  // Another instance may have started between forwarding and listening.
  if (forwardToRunningInstance(serverName, command)) {
    m_impl->result = AcquireResult::ForwardedExisting;
    return m_impl->result;
  }
  m_impl->error = m_impl->server.errorString();
  return m_impl->result;
}

QString SecureSingleInstance::errorString() const { return m_impl->error; }

void SecureSingleInstance::enableCommandNotifications() {
  m_impl->notificationsEnabled = true;
  // Drain connections accepted before the consumer connected its signal.
  const QPointer self(this);
  processPendingConnections();
  if (self) deliverPendingActivation();
}

void SecureSingleInstance::processPendingConnections() {
  const QPointer self(this);
  while (QLocalSocket* socket = m_impl->server.nextPendingConnection()) {
    if (!isTrustedLocalSocket(socket)) {
      socket->abort();
      socket->deleteLater();
      continue;
    }
    socket->setReadBufferSize(static_cast<qint64>(kApplicationCommandMaximumFrameSize + 1));
    QTimer::singleShot(kApplicationCommandExchangeTimeout, socket, [socket] { socket->abort(); });
    m_impl->buffers.insert(socket, {});
    connect(socket, &QLocalSocket::readyRead, this, [this, socket] { processSocket(socket); });
    connect(socket, &QLocalSocket::disconnected, this, [this, socket] {
      m_impl->buffers.remove(socket);
      socket->deleteLater();
    });
    processSocket(socket);
    if (!self) return;
  }
}

void SecureSingleInstance::processSocket(QLocalSocket* socket) {
  auto buffer = m_impl->buffers.find(socket);
  if (buffer == m_impl->buffers.end() || !socket) return;
  buffer.value().append(socket->readAll());

  try {
    const auto expectedSize = applicationCommandFrameSize(buffer.value());
    if (expectedSize == 0 || static_cast<std::size_t>(buffer.value().size()) < expectedSize) return;
    const auto request = decodeCommandRequest(buffer.value());
    if (!request) return;
    m_impl->buffers.erase(buffer);

    if (request->kind == app::ApplicationCommandKind::Activate) m_impl->activationPending = true;
    socket->write(encodeCommandAcknowledgement(request->requestId));
    socket->flush();
    socket->disconnectFromServer();
    deliverPendingActivation();
  } catch (const std::exception&) {
    m_impl->buffers.remove(socket);
    socket->abort();
  }
}

void SecureSingleInstance::deliverPendingActivation() {
  if (!m_impl->notificationsEnabled) return;
  if (m_impl->activationPending) {
    m_impl->activationPending = false;
    emit activationRequested();
  }
}

}  // namespace uwf::windows

#include "moc_SecureSingleInstance.cpp"
