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
#include <QCoreApplication>
#include <QScopeGuard>
#include <QTimer>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <optional>
#include <stdexcept>

#include "src/app/CrashHandler.h"
#include "src/app/FileStagingStore.h"
#include "src/app/SecureSingleInstance.h"
#include "src/app/StartupOptions.h"
#include "src/app/UserConfig.h"
#include "src/service/EnhancedModeService.h"
#include "src/ui/CenteredTextStyle.h"
#include "src/ui/I18n.h"
#include "src/ui/MainWindow.h"
#include "src/ui/ThemeManager.h"
#include "src/util/Log.h"
#include "src/uwf/SystemCheck.h"
#include "src/uwf/UwfSnapshot.h"
#include "src/uwf/wmi/WmiClient.h"

namespace {

void restrictDllSearchPath() {
  // Release 构建还通过 /DEPENDENTLOADFLAG:0x800 约束进入 main 前的静态
  // 导入；这里负责之后没有显式 LOAD_LIBRARY_SEARCH 标志的动态加载。
  // 正式包是单文件程序，不需要从应用目录、当前目录或 PATH 查找 DLL。
  if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32)) {
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "restrict process DLL search path");
  }
}

std::optional<int> handleSingleInstanceStartup(uwf::app::SecureSingleInstance& singleInstance, const uwf::app::StartupOptions& options) {
  // 单实例：已有实例在运行则切到它并退出，不再启动第二个窗口。放在最前面——
  // 系统检查等重活之前；若只是把任务转交给已有实例，没必要白做这些。
  uwf::app::ApplicationCommandResult forwardedResult;
  const auto acquireResult = singleInstance.acquire(options.initialCommand(), &forwardedResult);
  if (acquireResult == uwf::app::SecureSingleInstance::AcquireResult::ForwardedExisting) {
    UWF_LOG_I("main") << "startup command forwarded to existing instance: command=" << static_cast<int>(options.initialCommand())
                      << " outcome=" << static_cast<int>(forwardedResult.outcome);
    // CompletedWithFailures 表示所有目标均已得到最终结果，但显式
    // --commit-stage 命令仍应返回非零退出码，不能向脚本隐藏逐项失败。
    return forwardedResult.outcome == uwf::app::ApplicationCommandOutcome::Succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  if (acquireResult == uwf::app::SecureSingleInstance::AcquireResult::Unprotected) {
    UWF_LOG_W("main") << "single-instance server unavailable: error=" << singleInstance.errorString().toStdString();
  }
  return std::nullopt;
}

void initializeUserInterface(QApplication& app, const uwf::app::UserConfig& config) {
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

  uwf::ui::I18n::instance().applyStartupLanguage(config.language);

  // 在创建窗口前应用启动偏好，避免先显示默认语言或主题再切换。工具栏的
  // 语言与主题选择只影响本次运行，不回写用户手工维护的配置。
  uwf::ui::ThemeManager::instance().applyStartupTheme(config.theme);
}

uwf::SystemCheckResult checkRuntimeEnvironment() {
  // 系统校验不再是硬性拦截：版本不在受支持清单内时只记一条兼容模式提示，
  // 程序照常启动，提示通过 GlobalStatusPanel 的信息框告知用户。提示文案不在
  // 这里翻译——交给 MainWindow::buildUi 按当前语言现翻译，否则切语言后文案
  // 不会跟着变（详见 MainWindow 构造函数注释）。
  const auto check = uwf::runSystemChecks();
  switch (check.status) {
    case uwf::CheckStatus::UnsupportedSystem:
      UWF_LOG_W("main") << "unsupported Windows edition: mode=compatibility product=" << check.productName << " edition=" << check.editionId;
      break;
    case uwf::CheckStatus::Ok:
      UWF_LOG_I("main") << "system check completed: product=" << check.productName << " edition=" << check.editionId;
      break;
  }

  // 未提权不再弹模态框拦截——GlobalStatusPanel 会常驻一条红色"需要管理员
  // 权限"横幅，程序照常启动，可读但不可改。
  if (!uwf::isElevated()) {
    UWF_LOG_I("main") << "process is not elevated: mode=read-only";
  }
  return check;
}

int runMainWindow(QApplication& app, uwf::app::SecureSingleInstance& singleInstance, const uwf::app::StartupOptions& options,
                  const uwf::SystemCheckResult& check, const uwf::UwfCapability uwfCapability) {
  uwf::ui::MainWindow w(uwfCapability, check.status == uwf::CheckStatus::UnsupportedSystem, QString::fromStdString(check.productName),
                        QString::fromStdString(check.editionId));

  QObject::connect(&singleInstance, &uwf::app::SecureSingleInstance::activationRequested, &w, &uwf::ui::MainWindow::raiseToFront);
  QObject::connect(&singleInstance, &uwf::app::SecureSingleInstance::commitStageRequested, &w, [&](const std::uint64_t requestToken) {
    w.requestFileStagingCommit(
        [&singleInstance, requestToken](const uwf::ui::FileStagingBatchResult& result) { singleInstance.completeCommand(requestToken, result.command); });
  });
  singleInstance.enableCommandNotifications();

  switch (options.mode) {
    case uwf::app::StartupMode::Interactive:
      w.show();
      break;
    case uwf::app::StartupMode::Quiet:
      UWF_LOG_I("main") << "startup mode selected: mode=quiet presentation=system-tray";
      w.startInTray();
      break;
    case uwf::app::StartupMode::CommitStage:
      UWF_LOG_I("main") << "startup mode selected: mode=commit-stage presentation=system-tray";
      w.startInTray();
      QTimer::singleShot(0, &w, [&app, &w] {
        w.requestFileStagingCommit([&app](const uwf::ui::FileStagingBatchResult& result) {
          UWF_LOG_I("main") << "startup file staging command completed: outcome=" << static_cast<int>(result.command.outcome)
                            << " committed=" << result.command.committedFiles << " failures=" << result.command.failedFiles;
          app.exit(result.command.outcome == uwf::app::ApplicationCommandOutcome::Succeeded ? EXIT_SUCCESS : EXIT_FAILURE);
        });
      });
      break;
    case uwf::app::StartupMode::Service:
    case uwf::app::StartupMode::InstallService:
    case uwf::app::StartupMode::UninstallService:
      throw std::logic_error("service startup mode reached the UI dispatcher");
  }
  return app.exec();
}

int runApplication(int argc, char* argv[], const uwf::app::StartupOptions& options) {
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication app(argc, argv);
  app.setApplicationName("UWF Manager");
  app.setOrganizationName("UWF");
  app.setWindowIcon(QIcon(":/icons/app.svg"));
  uwf::app::SecureSingleInstance singleInstance;
  if (const auto forwardedExitCode = handleSingleInstanceStartup(singleInstance, options)) return *forwardedExitCode;

  initializeUserInterface(app, uwf::app::readUserConfig(QCoreApplication::applicationDirPath()));
  UWF_LOG_I("main") << "application started: pid=" << QCoreApplication::applicationPid();
  const auto check = checkRuntimeEnvironment();
  // WMI 运行时守卫覆盖完整 UI 事件循环。窗口及其控制器先销毁，随后守卫在
  // 普通栈展开阶段主动释放本线程 WMI 代理与 COM apartment，不等待 TLS 清理。
  uwf::initializeWmiRuntime();
  const auto shutdownWmi = qScopeGuard([] { uwf::shutdownWmiRuntime(); });
  // 兼容模式只表达“系统不在官方支持清单中”，不能替代实际能力探测。用户
  // 仍可能自行安装 UWF 驱动与 provider，因此所有系统都以 Embedded namespace
  // 和 UWF_Filter 的真实注册状态决定功能是否可用。
  const auto uwfCapability = uwf::probeUwfCapability();
  if (uwfCapability == uwf::UwfCapability::Unavailable) {
    UWF_LOG_W("main") << "UWF unavailable: reason=filter-class-or-namespace-not-registered";
  }
  // 文件暂存注册表的排除关系属于启动维护，不属于 UI 刷新。这里只执行一次；
  // 失败保留现有数据并记录，窗口随后仍可启动供用户检查和修复。
  if (uwf::isElevated()) {
    try {
      uwf::app::registryFileStagingStore(uwf::embeddedWmiSession(), uwfCapability).reconcileDurability();
    } catch (const std::exception& error) {
      UWF_LOG_W("staging") << "file staging durability maintenance failed: error=" << error.what();
    } catch (...) {
      UWF_LOG_W("staging") << "file staging durability maintenance failed: error=non-standard-exception";
    }
  }
  return runMainWindow(app, singleInstance, options, check, uwfCapability);
}

QStringList startupArguments(const int argc, char* argv[]) {
  QStringList arguments;
  arguments.reserve(argc);
  for (int index = 0; index < argc; ++index) arguments.append(QString::fromLocal8Bit(argv[index]));
  return arguments;
}

int runServiceControlCommand(int argc, char* argv[], const uwf::app::StartupMode mode) {
  QCoreApplication app(argc, argv);
  app.setApplicationName("UWF Manager");
  (void)uwf::ui::I18n::instance();
  uwf::initializeWmiRuntime();
  const auto shutdownWmi = qScopeGuard([] { uwf::shutdownWmiRuntime(); });
  const auto capability = uwf::probeUwfCapability();
  uwf::service::WindowsEnhancedModeServiceControl serviceControl;
  uwf::service::EnhancedModeManager manager(serviceControl, uwf::embeddedWmiSession(), capability);
  uwf::service::EnhancedModeChangeResult result;
  switch (mode) {
    case uwf::app::StartupMode::InstallService:
      result = manager.enable(uwf::ui::I18n::enhancedModeServiceDescription());
      break;
    case uwf::app::StartupMode::UninstallService:
      result = manager.disable();
      break;
    case uwf::app::StartupMode::Interactive:
    case uwf::app::StartupMode::Quiet:
    case uwf::app::StartupMode::CommitStage:
    case uwf::app::StartupMode::Service:
      throw std::logic_error("non-control startup mode reached the service-control dispatcher");
  }
  if (!result.persistenceWarning.isEmpty()) {
    UWF_LOG_W("service") << "enhanced mode changed with UWF persistence warning: error=" << result.persistenceWarning.toStdString();
  }
  const bool reachedTarget = mode == uwf::app::StartupMode::InstallService ? result.status.state == uwf::service::EnhancedModeState::Enabled
                                                                           : result.status.state == uwf::service::EnhancedModeState::Disabled;
  return reachedTarget ? EXIT_SUCCESS : EXIT_FAILURE;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    restrictDllSearchPath();
    const auto options = uwf::app::parseStartupOptions(startupArguments(argc, argv));
    switch (options.mode) {
      case uwf::app::StartupMode::Service:
        return uwf::service::runEnhancedModeService();
      case uwf::app::StartupMode::InstallService:
      case uwf::app::StartupMode::UninstallService:
        return runServiceControlCommand(argc, argv, options.mode);
      case uwf::app::StartupMode::Interactive:
      case uwf::app::StartupMode::Quiet:
      case uwf::app::StartupMode::CommitStage:
        uwf::app::CrashHandler::install();
        return runApplication(argc, argv, options);
    }
    throw std::logic_error("unknown startup mode");
  } catch (const std::exception& error) {
    UWF_LOG_E("main") << "fatal application error: error=" << error.what();
  } catch (...) {
    UWF_LOG_E("main") << "fatal application error: error=non-standard-exception";
  }
  return EXIT_FAILURE;
}
