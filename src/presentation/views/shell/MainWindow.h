// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QVector>
#include <memory>
#include <optional>

#include "application/uwf/contracts/UwfCapability.h"

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

// Shared services retain ownership; fileDialogs is borrowed for the window's lifetime.
struct MainWindowServices {
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

  // Initializes the UI and workflows without showing the main window or a taskbar entry.
  void startInTray();

  void raiseToFront();
  void setVisible(bool visible) override;

 public slots:
  // Every user refresh entry confirms discarding the draft. Initialization and
  // ViewModel-owned reconciliation read state directly without this UI prompt.
  void refresh();
  void showPlan();
  void showImport();
  void showAbout();
  void showSystemInformation();
  void showLogs();
  void showEnhancedMode();
  // Directory requests are traversed into individual file commits by the workflow.
  void commitFilePath(const QString& path);
  void commitFileDeletionPath(const QString& path);
  // nullopt selects the whole key; a present empty name selects its default value.
  void commitRegistryKey(const QString& key, const std::optional<QString>& valueName);
  void commitRegistryDeletionKey(const QString& key, const std::optional<QString>& valueName);

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
  // Renders only complete snapshots committed by the window model.
  void applyCommittedState();
  void completeInitialShow();
  // Applies model admission, including the reconciliation requirement after attempted writes.
  void updateInteractionAvailability();
  void renderWorkspace();
  void rebuildTabs();
  void updatePendingSummary();
  [[nodiscard]] std::size_t pendingChangeCount() const;
  bool confirmDiscardPendingChanges();
  bool confirmPendingChangesDiscard();
  void showTransientHint(const QString& text, int msec) const;
  void updateStatusBaseline();
  void setStatusDetail(const QString& text);

  void refreshThemedUi();
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

  bool m_initialUiPrepared = false;
  bool m_initialGeometryPending = false;
  bool m_showAfterRefresh = false;
  QSize m_layoutDensitySize;
  bool m_exitRequested = false;

  // Declaration order releases Overlay before Tray, Chrome and Hover controllers.
  std::unique_ptr<TrayController> m_tray;
  std::unique_ptr<OverlayPresentationController> m_overlayPresentation;

  std::shared_ptr<presentation::ViewModelFactory> m_viewModelFactory;
  std::shared_ptr<WindowWorkflowFactory> m_workflowFactory;
  std::shared_ptr<presentation::SystemInformationViewModel> m_systemInformation;
  dialogs::FileDialogProvider& m_fileDialogs;

  std::unique_ptr<presentation::MainWindowViewModel> m_viewModel;
  QVector<QPointer<VolumeTab>> m_volumeTabs;

  std::unique_ptr<CommitDispatcher> m_commit;

  bool m_compatibilityMode = false;
};

}  // namespace uwf::ui
