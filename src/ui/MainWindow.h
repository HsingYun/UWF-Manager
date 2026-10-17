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
#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QVector>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../service/EnhancedModeService.h"
#include "../uwf/UwfSnapshot.h"
#include "../uwf/wmi/WmiClient.h"
#include "CommitDispatcher.h"
#include "FileStagingCoordinator.h"

namespace uwf::app {
class FileStagingStore;
}

namespace uwf::service {
class EnhancedModeAgent;
class EnhancedModeAgentConnection;
}  // namespace uwf::service

class QTabWidget;
class QLabel;
class QAction;
class QCloseEvent;
class QEvent;
class QTimer;

namespace uwf::ui {

class DiskTab;
class GlobalStatusPanel;
class HoverHintController;
class OverlayPresentationController;
class PowerController;
class TrayController;
class TransientLabel;
class WindowChromeController;
enum class Theme;

struct ApplicationState {
  std::vector<core::DiskInfo> disks;
  core::UwfSnapshot snapshot;
};

// 主窗口只依赖“读取一份完整应用状态”的能力，不关心数据来自本机 WMI、远程
// 代理还是离线快照。实现必须完整返回磁盘与 UWF 快照，或在失败时抛出异常；
// read 在 GUI 线程同步执行，MainWindow 负责以两阶段提交保证旧界面不被部分
// 结果污染。
class ApplicationStateSource {
 public:
  virtual ~ApplicationStateSource() = default;
  [[nodiscard]] virtual ApplicationState read(UwfCapability capability) = 0;
};

// 服务对象均由调用方拥有，生命周期必须覆盖 MainWindow。
struct MainWindowServices {
  WmiOperations& uwf;
  ApplicationStateSource& state;
  app::FileStagingStore& fileStaging;
  service::EnhancedModeServiceControl* enhancedMode = nullptr;
  // 可选的代理传输由调用方拥有；为空时 MainWindow 创建生产命名管道实现。
  // 注入对象的生命周期必须覆盖 MainWindow。
  service::EnhancedModeAgentConnection* enhancedModeAgent = nullptr;
};

struct MainWindowStartup {
  UwfCapability uwfCapability;
  bool compatibilityMode = false;
  QString osProductName;
  QString osEditionId;
};

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  // compatibilityMode 为 true 时（系统版本不在受支持清单内），在 GlobalStatusPanel
  // 的信息框常驻显示兼容模式提示。提示文案在 buildUi 里按当前语言翻译，故这里
  // 只收原始数据（系统名 / 版本 ID），不收已翻译好的字符串——否则切语言后
  // 文案不会跟着变。
  explicit MainWindow(UwfCapability uwfCapability, bool compatibilityMode = false, const QString& osProductName = {}, const QString& osEditionId = {},
                      QWidget* parent = nullptr);
  MainWindow(MainWindowServices services, MainWindowStartup startup, QWidget* parent = nullptr);
  ~MainWindow() override;

  // 静默启动入口：在不显示主窗口、不创建任务栏窗口项的前提下完成首轮 UI、
  // 数据、托盘与 HUB 初始化。首次从托盘/HUB 打开时仍走正常 shown 状态下的
  // rebuildUi，以维持可见窗口的 polish 与几何稳定性。
  void startInTray();

  // 由"单实例"机制调用：另一个实例被启动时，把本窗口从最小化恢复并带到前台。
  void raiseToFront();
  void requestFileStagingCommit(FileStagingCoordinator::Completion completion);

 public slots:
  void refresh();
  void showPlan();
  void showImport();
  void showAbout();
  void showLogs();
  void showEnhancedMode();
  // 单文件 / 单目录提交：按 QFileInfo::isDir 自动分发——目录走 QDirIterator
  // 递归遍历挨个 CommitFile，文件直接 CommitFile。DiskTab.onCommitFile /
  // onCommitDir、ExclusionListWidget 右键 commit、覆盖层文件对话框右键 commit
  // 都共用这一个槽。
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

 private:
  static constexpr int kEnhancedModeStatusRetryInitialMs = 1000;
  static constexpr int kEnhancedModeStatusRetryMaximumMs = 30'000;

  void buildUi();
  // **切换主题 / 切换语言的唯一刷新入口**——整体重建 toolbar + central widget，
  // 让 tr() 拿到新译文、let QSS 在新 widget 上从干净状态应用。两个低频操作
  // 共用同一套机制，避免分两套各自的几何抖动。调用方在进入这里前会确认是否
  // 丢弃 widget 状态里的 pending changes。
  void rebuildUi();
  // 把已经完整提交到 m_disks / m_snapshot 的状态渲染到当前这套控件。刷新先在
  // 局部候选对象中完成全部读取，只有成功后才调用这里，避免残缺数据进入 UI。
  void applyCommittedState();
  // ApplyPlan 发生过写尝试或确认过收敛状态后，旧快照与 pending
  // 都不再是可写基线。在下一次完整刷新成功前锁住配置交互，避免
  // 部分成功的批次被重放。
  void reconcileAfterApply();
  void updateInteractionAvailability();
  [[nodiscard]] bool configurationWritesAllowed() const;
  // 首次启动尚无可保留的旧状态时，读取失败需要给出不可用占位；已有已提交
  // 状态时刷新失败不会调用本函数，当前 UI 原样保留。
  void showInitialRefreshFailure(const std::string& reason);
  void rebuildTabs(const std::vector<core::DiskInfo>& disks);
  void updatePendingSummary();
  bool confirmDiscardPendingChanges();
  void showTransientHint(const QString& text, int msec) const;
  // buildUi 内部的初始 icon / RichText 设置助手——给 toolbar action 按当前主题
  // 染色 svg、给 hoverHint 默认文案塞主题相关色。仅供 buildUi 末尾调用一次。
  // 不再作为对外的"主题切换刷新"路径——切主题统一走 rebuildUi。
  void refreshThemedUi();
  void refreshEnhancedModeStatus();
  void recordEnhancedModeStatusFailure(const QString& detail);
  void applyEnhancedModeStatus(const service::EnhancedModeStatus& status);
  void requestExit();

  QTabWidget* m_tabs = nullptr;
  GlobalStatusPanel* m_global = nullptr;
  QLabel* m_statusText = nullptr;
  // 状态栏与右下角悬停提示框都按"基线文本 + 临时覆盖、到点回基线"的模式跑——
  // 各自包给一个 TransientLabel，把过去散在 buildUi / rebuildUi / eventFilter /
  // updatePendingSummary 几处的 QTimer + 基线字符串成员收成对象。
  // 父对象设为对应 widget（状态栏 QLabel / 悬停框 QTextBrowser）：rebuildUi 删
  // widget 时控制器自动跟着 deleteLater，不必单独管理生命周期。
  TransientLabel* m_statusCtl = nullptr;
  TransientLabel* m_hoverCtl = nullptr;
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
  // 首次 showEvent 标记：true 表示已经走过一次"shown 状态下的 rebuildUi"，
  // 后续 show 不再重复。让首屏的几何和样式与切换路径完全一致——polish /
  // QStyle 一些计算只有在 widget 真正进入 shown 状态后才稳定。
  bool m_firstShowDone = false;
  bool m_initialGeometryApplied = false;
  bool m_exitRequested = false;

  // 系统托盘（图标 + 右键菜单）——独立组件，由本窗口编排。
  // 这些控制器存在明确的依赖顺序，使用成员声明的逆序析构表达：
  // Power → FileStaging → Overlay → Tray → Chrome → Hover。
  std::unique_ptr<TrayController> m_tray;
  std::unique_ptr<OverlayPresentationController> m_overlayPresentation;
  std::unique_ptr<FileStagingCoordinator> m_fileStagingCoordinator;
  std::unique_ptr<PowerController> m_power;
  service::EnhancedModeAgentConnection* m_enhancedModeAgent = nullptr;
  QTimer* m_enhancedModeStatusRetry = nullptr;

  // UWF 能力在启动期探测一次并固定。当前 UI 线程随后只复用 embedded
  // namespace session 读取动态状态；内部代理断线重建不会改变能力结论。
  const UwfCapability m_uwfCapability;
  WmiOperations& m_session;
  ApplicationStateSource& m_stateSource;
  app::FileStagingStore& m_fileStaging;

  QVector<QPointer<DiskTab>> m_diskTabs;
  std::vector<core::DiskInfo> m_disks;
  core::UwfSnapshot m_snapshot;
  bool m_hasCommittedState = false;
  bool m_reconciliationRequired = false;
  // 4 个 commit{File,FileDeletion,Registry,RegistryDeletion}Path 槽实际工作都
  // 在这里头：CommitDispatcher 自己持有 UwfVolume / UwfRegistryFilter 包装，
  // 共享 m_session + 引用 m_snapshot + Overlay 控制器的 usage timer。
  std::unique_ptr<CommitDispatcher> m_commit;
  std::unique_ptr<service::EnhancedModeManager> m_enhancedModeManager;
  std::unique_ptr<service::EnhancedModeAgentConnection> m_ownedEnhancedModeAgent;
  service::EnhancedModeStatus m_enhancedModeStatus;
  int m_enhancedModeStatusRetryDelayMs = kEnhancedModeStatusRetryInitialMs;

  // 兼容模式标志与系统标识（系统版本未通过校验时为 true）。提示文案每次
  // buildUi 按当前语言现翻译，故只存原始数据：rebuildUi 重建面板时连带重译。
  bool m_compatibilityMode = false;
  QString m_osProductName;
  QString m_osEditionId;
};

}  // namespace uwf::ui
