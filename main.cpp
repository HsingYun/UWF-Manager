#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>

#include "src/ui/CenteredTextStyle.h"
#include "src/ui/I18n.h"
#include "src/ui/MainWindow.h"
#include "src/ui/ThemeManager.h"
#include "src/util/Log.h"
#include "src/uwf/SystemCheck.h"

#include <windows.h>

// 单实例机制使用的本地服务名。带当前用户名后缀——单实例限定为"当前用户"
// 范围，多用户会话 / 快速用户切换下不会把别的登录用户的实例拉过来。
static QString instanceServerName() {
  return QStringLiteral("UWFManager.SingleInstance.") + qEnvironmentVariable("USERNAME");
}

// 尝试连接已在运行的实例并请它把窗口带到前台。确有实例在跑（连接成功）返回
// true，否则返回 false。底层是 Windows 命名管道——纯内核 IPC，不写磁盘、
// 不写注册表，进程退出即消失。
static bool forwardToRunningInstance(const QString& serverName) {
  QLocalSocket socket;
  socket.connectToServer(serverName);
  if (!socket.waitForConnected(300)) return false;
  // 授予目标进程抢占前台窗口的权限——否则受 Windows 前台锁定限制，已有窗口
  // 往往只能让任务栏图标闪烁，无法真正前置。
  AllowSetForegroundWindow(ASFW_ANY);
  socket.write("raise");
  socket.flush();
  socket.waitForBytesWritten(300);
  socket.disconnectFromServer();
  return true;
}

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

  (void)uwf::I18n::instance();

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
  QLocalServer::removeServer(instanceName);  // 清理可能残留的管道名（防御性）
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
      while (QLocalSocket* c = instanceServer.nextPendingConnection()) c->deleteLater();
      w.raiseToFront();
    };
    QObject::connect(&instanceServer, &QLocalServer::newConnection, &w, activate);
    // 处理在 listen 之后、newConnection 挂上之前（构造 MainWindow 的间隙）
    // 就已到达的连接。
    if (instanceServer.hasPendingConnections()) activate();
  }

  w.show();
  return app.exec();
}
