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
#include <windows.h>

#include <QApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <vector>

#include "src/ui/CenteredTextStyle.h"
#include "src/ui/I18n.h"
#include "src/ui/MainWindow.h"
#include "src/ui/ThemeManager.h"
#include "src/util/Log.h"
#include "src/util/PostGate.h"
#include "src/uwf/SystemCheck.h"

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

// 尝试连接已在运行的实例并请它把窗口带到前台。确有实例在跑（连接成功）返回
// true，否则返回 false。不能用 QLocalSocket 直接连接未知服务端：Windows 命名
// 管道默认允许服务端 impersonate 客户端，高权限进程连接被抢注的管道会泄露
// 自己的安全上下文。CreateFile 的 SQOS 把服务端限制在 Identification 级别，
// 随后再校验服务端 PID、用户、完整性级别、会话和可执行文件身份。
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

int main(int argc, char* argv[]) {
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication app(argc, argv);
  app.setApplicationName("UWF Manager");
  app.setOrganizationName("UWF");
  app.setWindowIcon(QIcon(":/icons/app.svg"));

  // 单实例：已有实例在运行则切到它并退出，不再启动第二个窗口。放在最前面——
  // 系统检查等重活之前；若只是把任务转交给已有实例，没必要白做这些。
  const QString instanceName = instanceServerName();
  if (forwardToRunningInstance(instanceName)) {
    UWF_LOG_I("main") << "another instance is already running; activated it and exiting";
    return 0;
  }

  // 用自定义 QProxyStyle 抵消 QToolButton / QTabBar 文字基线偏下的问题。
  app.setStyle(new uwf::ui::CenteredTextStyle());

  // 字体渲染：关 hinting + 关次像素 AA。Microsoft YaHei 在 9-10pt 小字号
  // 下，hinter 把笔画对齐像素网格时整除不下来的几条会落在分数像素被 ClearType
  // 反锯齿成"半透明 2 像素"，看起来比对齐到整数行的笔画细。关掉 hinting 让
  // 所有笔画用同一种次像素 AA 处理，粗细就一致；NoSubpixelAntialias 进一步
  // 把次像素 AA 退回灰度 AA，避免 ClearType 在某些 DPI 下产生彩边。代价是
  // 字看起来略软（"Mac 风"），但更整齐。
  {
    QFont f = app.font();
    f.setHintingPreference(QFont::PreferNoHinting);
    f.setStyleStrategy(static_cast<QFont::StyleStrategy>(QFont::PreferAntialias | QFont::NoSubpixelAntialias));
    app.setFont(f);
  }

  (void)uwf::ui::I18n::instance();

  UWF_LOG_I("main") << "UWF manager started; pid=" << QCoreApplication::applicationPid();

  // 启动时跟随系统主题（读 HKCU\...\Personalize\AppsUseLightTheme），用户
  // 之后可以通过工具栏右上角的按钮自由切换。不持久化偏好——每次启动重新探测。
  auto& theme = uwf::ui::ThemeManager::instance();
  theme.apply(uwf::ui::ThemeManager::detectSystemTheme());

  // 系统校验不再是硬性拦截：版本不在受支持清单内时只记一条兼容模式提示，
  // 程序照常启动，提示通过 GlobalStatusPanel 的信息框告知用户。提示文案不在
  // 这里翻译——交给 MainWindow::buildUi 按当前语言现翻译，否则切语言后文案
  // 不会跟着变（详见 MainWindow 构造函数注释）。
  const auto check = uwf::runSystemChecks();
  switch (check.status) {
    case uwf::CheckStatus::UnsupportedEdition:
      UWF_LOG_W("main") << "unsupported edition; running in compatibility mode; product=" << check.productName << " edition=" << check.editionId;
      break;
    case uwf::CheckStatus::Ok:
      UWF_LOG_I("main") << "system check ok: product=" << check.productName << " edition=" << check.editionId;
      break;
  }

  // 未提权不再弹模态框拦截——GlobalStatusPanel 会常驻一条红色"需要管理员
  // 权限"横幅，程序照常启动，可读但不可改。
  if (!uwf::isElevated()) {
    UWF_LOG_W("main") << "process is not elevated; UWF settings cannot be modified";
  }

  // 单实例：本实例已通过系统检查、即将真正运行，注册监听端占住实例名。
  QLocalServer instanceServer;
  instanceServer.setSocketOptions(QLocalServer::UserAccessOption);
  if (!instanceServer.listen(instanceName)) {
    // probe 与 listen 之间被另一实例抢注（启动竞态）——交给它并退出。
    if (forwardToRunningInstance(instanceName)) {
      UWF_LOG_I("main") << "lost single-instance startup race; activated the other instance and exiting";
      return 0;
    }
    UWF_LOG_W("main") << "single-instance server failed to listen: " << instanceServer.errorString().toStdString();
  }

  uwf::ui::MainWindow w(check.status == uwf::CheckStatus::UnsupportedEdition, QString::fromStdString(check.productName),
                        QString::fromStdString(check.editionId));

  // 收到其他实例的连接 → 把本窗口带到前台。
  if (instanceServer.isListening()) {
    auto activate = [&instanceServer, &w] {
      bool trustedClient = false;
      while (QLocalSocket* c = instanceServer.nextPendingConnection()) {
        if (isTrustedLocalSocket(c)) {
          if (c->bytesAvailable() == 0) c->waitForReadyRead(100);
          trustedClient = c->readAll() == QByteArrayLiteral("raise") || trustedClient;
        }
        c->deleteLater();
      }
      if (trustedClient) w.raiseToFront();
    };
    QObject::connect(&instanceServer, &QLocalServer::newConnection, &w, activate);
    // 处理在 listen 之后、newConnection 挂上之前（构造 MainWindow 的间隙）
    // 就已到达的连接。
    if (instanceServer.hasPendingConnections()) activate();
  }

  w.show();
  const int rc = app.exec();
  // 事件循环已退出、QApplication 即将析构——关闭"可投递"门闸，让仍在跑的
  // 后台 worker 不再向 qApp 投递结果（见 src/util/PostGate.h）。
  uwf::postgate::close();
  return rc;
}
