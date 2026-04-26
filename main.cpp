#include <QApplication>

#include "src/ui/CenteredTextStyle.h"
#include "src/ui/I18n.h"
#include "src/ui/MainWindow.h"
#include "src/ui/MessageDialog.h"
#include "src/ui/ThemeManager.h"
#include "src/util/Log.h"
#include "src/uwf/SystemCheck.h"

static QString describeCheck(const uwf::SystemCheckResult& r) {
  switch (r.status) {
    case uwf::CheckStatus::NotWindows:
      return uwf::I18n::tr("Unified Write Filter (UWF) is only available on Windows.");
    case uwf::CheckStatus::UnsupportedEdition:
      return uwf::I18n::tr("UWF is only supported on Windows Enterprise, Education or IoT Enterprise editions.\n\nCurrent system: %1 (%2)")
          .arg(QString::fromStdString(r.productName), QString::fromStdString(r.editionId));
    case uwf::CheckStatus::UwfNotInstalled:
      return uwf::I18n::tr(
          "Unified Write Filter (UWF) was not detected. Open \"Control Panel → Programs → Turn Windows features on or off\", enable \"Device Lockdown → "
          "Unified Write Filter\", reboot, and try again.");
    case uwf::CheckStatus::Ok:
      return {};
  }
  return {};
}

int main(int argc, char* argv[]) {
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication app(argc, argv);
  app.setApplicationName("UWF Manager");
  app.setOrganizationName("UWF");
  app.setWindowIcon(QIcon(":/icons/app.svg"));

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

  const auto check = uwf::runSystemChecks();
  const QString detail = describeCheck(check);
  switch (check.status) {
    case uwf::CheckStatus::NotWindows:
    case uwf::CheckStatus::UnsupportedEdition:
      UWF_LOG_E("main") << "system check failed: status=" << static_cast<int>(check.status) << " product=" << check.productName
                        << " edition=" << check.editionId;
      uwf::ui::dialogs::warning(nullptr, uwf::I18n::tr("System version not supported"), detail);
      return 2;
    case uwf::CheckStatus::UwfNotInstalled:
      UWF_LOG_E("main") << "system check failed: UWF feature not installed; product=" << check.productName << " edition=" << check.editionId;
      uwf::ui::dialogs::warning(nullptr, uwf::I18n::tr("UWF feature not enabled"), detail);
      return 3;
    case uwf::CheckStatus::Ok:
      UWF_LOG_I("main") << "system check ok: product=" << check.productName << " edition=" << check.editionId;
      break;
  }

  if (!uwf::isElevated()) {
    UWF_LOG_W("main") << "process is not elevated; UWF settings cannot be modified";
    uwf::ui::dialogs::warning(nullptr, uwf::I18n::tr("Administrator privileges required"),
                              uwf::I18n::tr("This program is not running as administrator.\n\nRight-click and choose \"Run as administrator\" to restart it; "
                                            "otherwise UWF settings cannot be read or modified."));
  }

  uwf::ui::MainWindow w;
  w.show();
  return app.exec();
}
