// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QVector>
#include <memory>
#include <string>
#include <vector>

#include "app/api/UwfCapability.h"
#include "app/staging/FileStagingCoordinator.h"

namespace uwf::app {
class ApplicationStateSource;
class EnhancedModeSession;
class FileStagingStore;
class FileStagingDraft;
class PowerCoordinator;
}  // namespace uwf::app

namespace uwf::core {
struct VolumeInfo;
}

namespace uwf::presentation {
class MainWindowViewModel;
class SystemInformationViewModel;
class ViewModelFactory;
}  // namespace uwf::presentation

class QTabWidget;
class QStackedWidget;
class QLabel;
class QAction;
class QCloseEvent;
class QEvent;

namespace uwf::ui {
namespace dialogs {
class FileDialogProvider;
}

class VolumeTab;
class CommitDispatcher;
class GlobalStatusPanel;
class HoverHintController;
class OverlayPresentationController;
class TrayController;
class TransientLabel;
class WindowChromeController;
class WindowWorkflowFactory;
class StatusSurface;
class ContentState;
class Surface;
class CommandBar;
class SummaryLabel;
class VerticalScrollArea;

// 引用和指针指向的服务由调用方拥有，生命周期必须覆盖 MainWindow。
struct MainWindowServices {
  app::ApplicationStateSource& state;
  std::shared_ptr<app::FileStagingStore> fileStaging;
  std::shared_ptr<presentation::ViewModelFactory> viewModelFactory;
  std::shared_ptr<WindowWorkflowFactory> workflows;
  dialogs::FileDialogProvider& fileDialogs;
};

struct MainWindowStartup {
  app::UwfCapability uwfCapability;
  bool compatibilityMode = false;
  bool hubVisible = true;
};

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  MainWindow(MainWindowServices services, MainWindowStartup startup, QWidget* parent = nullptr);
  ~MainWindow() override;

  // 静默启动入口：在不显示主窗口、不创建任务栏窗口项的前提下完成首轮 UI、
  // 数据、托盘与 HUB 初始化。首次打开时在原生窗口显示前完成最终布局与几何。
  void startInTray();

  // 由"单实例"机制调用：另一个实例被启动时，把本窗口从最小化恢复并带到前台。
  void raiseToFront();
  void requestFileStagingCommit(app::FileStagingCoordinator::Completion completion);
  void setVisible(bool visible) override;

 public slots:
  void refresh();
  void showPlan();
  void showImport();
  void showAbout();
  void showSystemInformation();
  void showLogs();
  void showEnhancedMode();
  // 文件和目录共用提交入口，目录由分派器展开为逐文件调用。
  void commitFilePath(const QString& path);
  void commitFileDeletionPath(const QString& path);
  // valueName 空串 = 递归处理整棵键子树；非空 = 操作单个命名值。(Default) 在
  // picker 层禁选，调用方不会传"空 valueName 表示 (默认) 值"的语义进来。
  void commitRegistryKey(const QString& key, const QString& valueName);
  void commitRegistryDeletionKey(const QString& key, const QString& valueName);

 protected:
  void changeEvent(QEvent* ev) override;
  void closeEvent(QCloseEvent* ev) override;
  void showEvent(QShowEvent* ev) override;
  void resizeEvent(QResizeEvent* event) override;
  bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override;

 private:
  void buildUi();
  void updateLayoutDensity();
  // Only initialization and explicit state refresh rebuild editors.
  void rebuildUi();
  void retranslateUi();
  // 把应用状态会话中已完整提交的状态渲染到当前控件。刷新先在
  // 局部候选对象中完成全部读取，只有成功后才调用这里，避免残缺数据进入 UI。
  void applyCommittedState();
  void completeInitialShow();
  // ApplyPlan 发生过写尝试或确认过收敛状态后，旧快照与 pending
  // 都不再是可写基线。在下一次完整刷新成功前锁住配置交互，避免
  // 部分成功的批次被重放。
  void updateInteractionAvailability();
  [[nodiscard]] bool configurationWritesAllowed() const;
  // 读取失败在业务区域展示诊断，保留已提交快照作为恢复基线。
  void showInitialRefreshFailure(const std::string& reason);
  void showWorkspaceDiagnostic(const QString& message);
  void rebuildTabs();
  void updatePendingSummary();
  [[nodiscard]] std::size_t pendingChangeCount() const;
  bool confirmDiscardPendingChanges();
  void showTransientHint(const QString& text, int msec) const;
  void updateStatusBaseline();
  void setStatusDetail(const QString& text);
  // 在现有视图上更新包含主题色的图标和富文本。
  void refreshThemedUi();
  void refreshEnhancedModeStatus();
  void requestExit();
  void requestSafeShutdown();
  void requestSafeRestart();
  void requestDirectRestart();

  QTabWidget* m_tabs = nullptr;
  QStackedWidget* m_workspaceStack = nullptr;
  ContentState* m_workspaceState = nullptr;
  GlobalStatusPanel* m_global = nullptr;
  StatusSurface* m_statusSurface = nullptr;
  VerticalScrollArea* m_settingsScroll = nullptr;
  Surface* m_settingsSurface = nullptr;
  CommandBar* m_commands = nullptr;
  SummaryLabel* m_systemBrief = nullptr;
  // The status surface owns the transient hint lifetime.
  TransientLabel* m_statusCtl = nullptr;
  std::unique_ptr<HoverHintController> m_hoverHints;
  std::unique_ptr<WindowChromeController> m_chrome;

  // 引用方式持有 toolbar action，主题切换时按当前主题前景色重染 svg。
  QAction* m_actRefresh = nullptr;
  QAction* m_actImport = nullptr;
  QAction* m_actPlan = nullptr;
  QAction* m_actShutdown = nullptr;
  QAction* m_actRestart = nullptr;
  QAction* m_actEnhancedMode = nullptr;
  QAction* m_actAbout = nullptr;
  QAction* m_actLog = nullptr;
  QAction* m_actLang = nullptr;
  QAction* m_actTheme = nullptr;
  // 原生窗口首次显示前完成初始化；后续显示保留用户调整的几何。
  bool m_initialUiPrepared = false;
  bool m_initialGeometryPending = false;
  bool m_showAfterRefresh = false;
  QSize m_layoutDensitySize;
  bool m_exitRequested = false;
  QString m_workspaceDiagnostic;

  // 系统托盘（图标 + 右键菜单）——独立组件，由本窗口编排。
  // 这些控制器存在明确的依赖顺序，使用成员声明的逆序析构表达：
  // Power → FileStaging → Overlay → Tray → Chrome → Hover。
  std::unique_ptr<TrayController> m_tray;
  std::unique_ptr<OverlayPresentationController> m_overlayPresentation;
  std::unique_ptr<app::FileStagingCoordinator> m_fileStagingCoordinator;
  std::unique_ptr<app::PowerCoordinator> m_power;

  // UWF 能力在启动期探测一次并固定。当前 UI 线程随后只复用 embedded
  // namespace session 读取动态状态；内部代理断线重建不会改变能力结论。
  const app::UwfCapability m_uwfCapability;
  std::shared_ptr<presentation::ViewModelFactory> m_viewModelFactory;
  std::shared_ptr<WindowWorkflowFactory> m_workflowFactory;
  std::shared_ptr<presentation::SystemInformationViewModel> m_systemInformation;
  std::shared_ptr<app::FileStagingStore> m_fileStaging;
  std::shared_ptr<app::FileStagingDraft> m_fileStagingDraft;
  dialogs::FileDialogProvider& m_fileDialogs;

  std::unique_ptr<app::EnhancedModeSession> m_enhancedMode;
  std::unique_ptr<presentation::MainWindowViewModel> m_viewModel;
  QVector<QPointer<VolumeTab>> m_volumeTabs;
  // 提交分派器持有轻量 API 包装，并引用快照和占用刷新定时器。
  std::unique_ptr<CommitDispatcher> m_commit;

  bool m_compatibilityMode = false;
};

}  // namespace uwf::ui
