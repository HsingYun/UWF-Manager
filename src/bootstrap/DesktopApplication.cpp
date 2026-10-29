// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include "DesktopApplication.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QScopeGuard>
#include <QTimer>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

#include "MainWindowComposition.h"
#include "app/ports/FileStagingStore.h"
#include "app/settings/UserConfig.h"
#include "app/startup/ApplicationCommand.h"
#include "app/startup/StartupOptions.h"
#include "shared/Log.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/shell/MainWindow.h"
#include "windows/process/SecureSingleInstance.h"
#include "windows/system/SystemCheck.h"
#include "windows/uwf/SystemUwfStateReader.h"
#include "windows/wmi/WmiSession.h"

namespace uwf::bootstrap {

namespace {

std::optional<int> acquireSingleInstance(windows::SecureSingleInstance& singleInstance, const app::StartupOptions& options) {
  // 在系统检查和 UI 初始化前转交启动命令，避免重复创建实例。
  app::ApplicationCommandResult forwardedResult;
  const auto acquireResult = singleInstance.acquire(options.initialCommand(), &forwardedResult);
  if (acquireResult == windows::SecureSingleInstance::AcquireResult::ForwardedExisting) {
    UWF_LOG_I("main") << "startup command forwarded to existing instance: command=" << static_cast<int>(options.initialCommand())
                      << " outcome=" << static_cast<int>(forwardedResult.outcome);
    // CompletedWithFailures 表示所有目标均已得到最终结果，但显式
    // --commit-stage 命令仍应返回非零退出码，不能向脚本隐藏逐项失败。
    return forwardedResult.outcome == app::ApplicationCommandOutcome::Succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  if (acquireResult == windows::SecureSingleInstance::AcquireResult::Unprotected) {
    UWF_LOG_W("main") << "single-instance server unavailable: error=" << singleInstance.errorString().toStdString();
  }
  return std::nullopt;
}

void initializeUserInterface(const app::UserConfig& config) {
  uwf::ui::I18n::instance().applyStartupLanguage(config.language);

  // 在创建窗口前应用启动偏好，避免先显示默认语言或主题再切换。工具栏的
  // 语言与主题选择只影响本次运行，不回写用户手工维护的配置。
  uwf::ui::ThemeManager::instance().applyStartupTheme(config.theme);
}

windows::SystemCheckResult checkRuntimeEnvironment() {
  // 系统版本不在支持清单时仍允许启动；兼容提示由主窗口按当前语言生成。
  const auto check = windows::runSystemChecks();
  switch (check.status) {
    case windows::CheckStatus::UnsupportedSystem:
      UWF_LOG_W("main") << "unsupported Windows edition: mode=compatibility product=" << check.productName << " edition=" << check.editionId;
      break;
    case windows::CheckStatus::Ok:
      UWF_LOG_I("main") << "system check completed: product=" << check.productName << " edition=" << check.editionId;
      break;
  }

  // 未提权时保留只读界面，由主面板显示权限提示。
  if (!windows::isElevated()) {
    UWF_LOG_I("main") << "process is not elevated: mode=read-only";
  }
  return check;
}

int runMainWindow(QApplication& application, windows::SecureSingleInstance& singleInstance, const app::StartupOptions& options,
                  uwf::ui::MainWindowStartup startup) {
  const auto services = uwf::bootstrap::systemMainWindowServices(startup.uwfCapability);
  uwf::ui::MainWindow window(services, std::move(startup));

  QObject::connect(&singleInstance, &windows::SecureSingleInstance::activationRequested, &window, &uwf::ui::MainWindow::raiseToFront);
  QObject::connect(&singleInstance, &windows::SecureSingleInstance::commitStageRequested, &window, [&](const std::uint64_t requestToken) {
    window.requestFileStagingCommit(
        [&singleInstance, requestToken](const app::FileStagingBatchResult& result) { singleInstance.completeCommand(requestToken, result.command); });
  });
  singleInstance.enableCommandNotifications();

  switch (options.mode) {
    case app::StartupMode::Interactive:
      window.show();
      break;
    case app::StartupMode::Quiet:
      UWF_LOG_I("main") << "startup mode selected: mode=quiet presentation=system-tray";
      window.startInTray();
      break;
    case app::StartupMode::CommitStage:
      UWF_LOG_I("main") << "startup mode selected: mode=commit-stage presentation=system-tray";
      window.startInTray();
      QTimer::singleShot(0, &window, [&application, &window] {
        window.requestFileStagingCommit([&application](const app::FileStagingBatchResult& result) {
          UWF_LOG_I("main") << "startup file staging command completed: outcome=" << static_cast<int>(result.command.outcome)
                            << " committed=" << result.command.committedFiles << " failures=" << result.command.failedFiles;
          application.exit(result.command.outcome == app::ApplicationCommandOutcome::Succeeded ? EXIT_SUCCESS : EXIT_FAILURE);
        });
      });
      break;
    case app::StartupMode::Service:
    case app::StartupMode::InstallService:
    case app::StartupMode::UninstallService:
      throw std::logic_error("service startup mode reached the UI dispatcher");
  }
  return application.exec();
}

}  // namespace

int runDesktopApplication(int argc, char* argv[], const app::StartupOptions& options) {
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication application(argc, argv);
  application.setApplicationName("UWF Manager");
  application.setOrganizationName("UWF");
  application.setWindowIcon(QIcon(":/icons/app.svg"));
  windows::SecureSingleInstance singleInstance;
  if (const auto forwardedExitCode = acquireSingleInstance(singleInstance, options)) return *forwardedExitCode;

  const auto config = app::readUserConfig(QCoreApplication::applicationDirPath());
  initializeUserInterface(config);
  UWF_LOG_I("main") << "application started: pid=" << QCoreApplication::applicationPid();
  const auto check = checkRuntimeEnvironment();
  // WMI 运行时守卫覆盖完整 UI 事件循环。窗口及其控制器先销毁，随后守卫在
  // 普通栈展开阶段主动释放本线程 WMI 代理与 COM apartment，不等待 TLS 清理。
  windows::initializeWmiRuntime();
  const auto shutdownWmi = qScopeGuard([] { windows::shutdownWmiRuntime(); });
  // 兼容模式只表达“系统不在官方支持清单中”，不能替代实际能力探测。用户
  // 仍可能自行安装 UWF 驱动与 provider，因此所有系统都以 Embedded namespace
  // 和 UWF_Filter 的真实注册状态决定功能是否可用。
  const auto uwfCapability = windows::probeUwfCapability();
  if (uwfCapability == app::UwfCapability::Unavailable) {
    UWF_LOG_W("main") << "UWF unavailable: reason=filter-class-or-namespace-not-registered";
  }
  // 文件暂存注册表的排除关系属于启动维护，不属于 UI 刷新。这里只执行一次；
  // 失败保留现有数据并记录，窗口随后仍可启动供用户检查和修复。
  if (windows::isElevated()) {
    try {
      systemFileStagingStore(uwfCapability)->reconcileDurability();
    } catch (const std::exception& error) {
      UWF_LOG_W("staging") << "file staging durability maintenance failed: error=" << error.what();
    } catch (...) {
      UWF_LOG_W("staging") << "file staging durability maintenance failed: error=non-standard-exception";
    }
  }
  return runMainWindow(
      application, singleInstance, options,
      {.uwfCapability = uwfCapability, .compatibilityMode = check.status == windows::CheckStatus::UnsupportedSystem, .hubVisible = config.hubVisible});
}

}  // namespace uwf::bootstrap
