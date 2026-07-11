/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SecureSingleInstance.h"

#include <windows.h>

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QString>
#include <string>
#include <vector>

namespace uwf::app {

namespace {

class UniqueHandle final {
 public:
  explicit UniqueHandle(HANDLE handle = nullptr) : m_handle(handle) {}
  ~UniqueHandle() {
    if (m_handle && m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
  }
  UniqueHandle(const UniqueHandle&) = delete;
  UniqueHandle& operator=(const UniqueHandle&) = delete;
  [[nodiscard]] HANDLE get() const { return m_handle; }
  [[nodiscard]] bool valid() const { return m_handle && m_handle != INVALID_HANDLE_VALUE; }

 private:
  HANDLE m_handle = nullptr;
};

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

std::wstring processImagePath(const HANDLE process) {
  std::wstring path(32768, L'\0');
  DWORD size = static_cast<DWORD>(path.size());
  if (!QueryFullProcessImageNameW(process, 0, path.data(), &size)) return {};
  path.resize(size);
  return path;
}

bool sameFile(const std::wstring& lhs, const std::wstring& rhs) {
  const DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  const UniqueHandle left(CreateFileW(lhs.c_str(), FILE_READ_ATTRIBUTES, share, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  const UniqueHandle right(CreateFileW(rhs.c_str(), FILE_READ_ATTRIBUTES, share, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!left.valid() || !right.valid()) return false;

  BY_HANDLE_FILE_INFORMATION leftInfo{};
  BY_HANDLE_FILE_INFORMATION rightInfo{};
  if (!GetFileInformationByHandle(left.get(), &leftInfo) || !GetFileInformationByHandle(right.get(), &rightInfo)) return false;
  return leftInfo.dwVolumeSerialNumber == rightInfo.dwVolumeSerialNumber && leftInfo.nFileIndexHigh == rightInfo.nFileIndexHigh &&
         leftInfo.nFileIndexLow == rightInfo.nFileIndexLow;
}

QByteArray fileSha256(const std::wstring& path) {
  QFile file(QString::fromStdWString(path));
  if (!file.open(QIODevice::ReadOnly)) return {};
  QCryptographicHash hash(QCryptographicHash::Sha256);
  while (!file.atEnd()) {
    const QByteArray chunk = file.read(64 * 1024);
    if (file.error() != QFileDevice::NoError) return {};
    hash.addData(chunk);
  }
  return hash.result();
}

bool sameExecutable(const std::wstring& lhs, const std::wstring& rhs) {
  if (lhs.empty() || rhs.empty()) return false;
  if (sameFile(lhs, rhs)) return true;
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
  return sameExecutable(processImagePath(GetCurrentProcess()), processImagePath(peer.get()));
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
QString instanceServerName() {
  DWORD sessionId = 0;
  ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
  const QString sid = currentUserSid();
  return QStringLiteral("UWFManager.SingleInstance.%1.%2").arg(sid.isEmpty() ? QStringLiteral("UnknownSid") : sid).arg(sessionId);
}

// 不能用 QLocalSocket 直接连接未知服务端：Windows 命名管道默认允许服务端
// impersonate 客户端，高权限进程连接被抢注的管道会泄露自己的安全上下文。
bool forwardToRunningInstance(const QString& serverName) {
  const std::wstring pipeName = (QStringLiteral("\\\\.\\pipe\\") + serverName).toStdWString();
  if (!WaitNamedPipeW(pipeName.c_str(), 300)) return false;

  constexpr DWORD kSecurityFlags = SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION | SECURITY_EFFECTIVE_ONLY;
  const UniqueHandle pipe(CreateFileW(pipeName.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, kSecurityFlags, nullptr));
  if (!pipe.valid()) return false;

  ULONG serverProcessId = 0;
  if (!GetNamedPipeServerProcessId(pipe.get(), &serverProcessId) || !isTrustedPeerProcess(static_cast<DWORD>(serverProcessId))) return false;

  AllowSetForegroundWindow(static_cast<DWORD>(serverProcessId));
  constexpr char kRaiseCommand[] = "raise";
  DWORD written = 0;
  return WriteFile(pipe.get(), kRaiseCommand, static_cast<DWORD>(sizeof(kRaiseCommand) - 1), &written, nullptr) != FALSE &&
         written == static_cast<DWORD>(sizeof(kRaiseCommand) - 1);
}

bool isTrustedLocalSocket(QLocalSocket* socket) {
  if (!socket || socket->socketDescriptor() == -1) return false;
  ULONG clientProcessId = 0;
  const HANDLE pipe = reinterpret_cast<HANDLE>(socket->socketDescriptor());
  return GetNamedPipeClientProcessId(pipe, &clientProcessId) != FALSE && isTrustedPeerProcess(static_cast<DWORD>(clientProcessId));
}

}  // namespace

class SecureSingleInstance::Private final {
 public:
  QLocalServer server;
  QString error;
  bool acquired = false;
  AcquireResult result = AcquireResult::Unprotected;
  bool notificationsEnabled = false;
  bool activationPending = false;
};

SecureSingleInstance::SecureSingleInstance(QObject* parent) : QObject(parent), d(std::make_unique<Private>()) {
  connect(&d->server, &QLocalServer::newConnection, this, &SecureSingleInstance::processPendingConnections);
}

SecureSingleInstance::~SecureSingleInstance() = default;

SecureSingleInstance::AcquireResult SecureSingleInstance::acquire() {
  if (d->acquired) return d->result;
  d->acquired = true;
  const QString serverName = instanceServerName();

  if (forwardToRunningInstance(serverName)) {
    d->result = AcquireResult::ActivatedExisting;
    return d->result;
  }

  d->server.setSocketOptions(QLocalServer::UserAccessOption);
  if (d->server.listen(serverName)) {
    d->result = AcquireResult::Primary;
    return d->result;
  }

  // probe 与 listen 之间可能被另一实例抢注，认证并重试一次转交。
  if (forwardToRunningInstance(serverName)) {
    d->result = AcquireResult::ActivatedExisting;
    return d->result;
  }
  d->error = d->server.errorString();
  return d->result;
}

QString SecureSingleInstance::errorString() const { return d->error; }

void SecureSingleInstance::enableActivationNotifications() {
  d->notificationsEnabled = true;
  // newConnection 可能在外部连接 activationRequested 之前已经发出；主动清空
  // server 队列，并与此前认证成功但尚未投递的请求合并。
  processPendingConnections();
}

void SecureSingleInstance::processPendingConnections() {
  while (QLocalSocket* socket = d->server.nextPendingConnection()) {
    if (isTrustedLocalSocket(socket)) {
      if (socket->bytesAvailable() == 0) socket->waitForReadyRead(100);
      if (socket->readAll() == QByteArrayLiteral("raise")) d->activationPending = true;
    }
    socket->deleteLater();
  }
  deliverPendingActivation();
}

void SecureSingleInstance::deliverPendingActivation() {
  if (!d->notificationsEnabled || !d->activationPending) return;
  d->activationPending = false;
  emit activationRequested();
}

}  // namespace uwf::app
