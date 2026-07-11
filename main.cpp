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
#include <QApplication>

#include "src/app/CrashHandler.h"
#include "src/app/SecureSingleInstance.h"
#include "src/ui/CenteredTextStyle.h"
#include "src/ui/I18n.h"
#include "src/ui/MainWindow.h"
#include "src/ui/ThemeManager.h"
#include "src/util/Log.h"
#include "src/util/PostGate.h"
#include "src/uwf/SystemCheck.h"

namespace {

bool handleSingleInstanceStartup(uwf::app::SecureSingleInstance& singleInstance) {
  // 单实例：已有实例在运行则切到它并退出，不再启动第二个窗口。放在最前面——
  // 系统检查等重活之前；若只是把任务转交给已有实例，没必要白做这些。
  const auto acquireResult = singleInstance.acquire();
  if (acquireResult == uwf::app::SecureSingleInstance::AcquireResult::ActivatedExisting) {
    UWF_LOG_I("main") << "another instance is already running; activated it and exiting";
    return false;
  }
  if (acquireResult == uwf::app::SecureSingleInstance::AcquireResult::Unprotected) {
    UWF_LOG_W("main") << "single-instance server failed to listen: " << singleInstance.errorString().toStdString();
  }
  return true;
}

void initializeUserInterface(QApplication& app) {
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

  // 启动时跟随系统主题（读 HKCU\...\Personalize\AppsUseLightTheme），用户
  // 之后可以通过工具栏右上角的按钮自由切换。不持久化偏好——每次启动重新探测。
  auto& theme = uwf::ui::ThemeManager::instance();
  theme.apply(uwf::ui::ThemeManager::detectSystemTheme());
}

uwf::SystemCheckResult checkRuntimeEnvironment() {
  // 系统校验不再是硬性拦截：版本不在受支持清单内时只记一条兼容模式提示，
  // 程序照常启动，提示通过 GlobalStatusPanel 的信息框告知用户。提示文案不在
  // 这里翻译——交给 MainWindow::buildUi 按当前语言现翻译，否则切语言后文案
  // 不会跟着变（详见 MainWindow 构造函数注释）。
  const auto check = uwf::runSystemChecks();
  switch (check.status) {
    case uwf::CheckStatus::UnsupportedSystem:
      UWF_LOG_W("main") << "unsupported Windows family or edition; running in compatibility mode; product=" << check.productName
                        << " edition=" << check.editionId;
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
  return check;
}

int runMainWindow(QApplication& app, uwf::app::SecureSingleInstance& singleInstance, const uwf::SystemCheckResult& check) {
  uwf::ui::MainWindow w(check.status == uwf::CheckStatus::UnsupportedSystem, QString::fromStdString(check.productName),
                        QString::fromStdString(check.editionId));

  QObject::connect(&singleInstance, &uwf::app::SecureSingleInstance::activationRequested, &w, &uwf::ui::MainWindow::raiseToFront);
  singleInstance.enableActivationNotifications();

  w.show();
  const int rc = app.exec();
  // 事件循环已退出、QApplication 即将析构——关闭"可投递"门闸，让仍在跑的
  // 后台 worker 不再向 qApp 投递结果（见 src/util/PostGate.h）。
  uwf::postgate::close();
  return rc;
}

}  // namespace

int main(int argc, char* argv[]) {
  uwf::app::CrashHandler::install();
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication app(argc, argv);
  app.setApplicationName("UWF Manager");
  app.setOrganizationName("UWF");
  app.setWindowIcon(QIcon(":/icons/app.svg"));

  uwf::app::SecureSingleInstance singleInstance;
  if (!handleSingleInstanceStartup(singleInstance)) return 0;

  initializeUserInterface(app);
  UWF_LOG_I("main") << "UWF manager started; pid=" << QCoreApplication::applicationPid();
  const auto check = checkRuntimeEnvironment();
  return runMainWindow(app, singleInstance, check);
}
