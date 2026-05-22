#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QVector>

#include "../core/UwfConfig.h"
#include "../uwf/api/UwfFilter.h"
#include "../uwf/api/UwfOverlay.h"
#include "../uwf/api/UwfOverlayConfig.h"
#include "../uwf/api/UwfRegistryFilter.h"
#include "../uwf/api/UwfVolume.h"
#include "../uwf/wmi/WmiClient.h"

class QTabWidget;
class QLabel;
class QTimer;
class QAction;

namespace uwf::ui {

class DiskTab;
class GlobalStatusPanel;
class TrayController;
enum class Theme;

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  // compatibilityMode 为 true 时（系统版本不在受支持清单内），在 GlobalStatusPanel
  // 的信息框常驻显示兼容模式提示。提示文案在 buildUi 里按当前语言翻译，故这里
  // 只收原始数据（系统名 / 版本 ID），不收已翻译好的字符串——否则切语言后
  // 文案不会跟着变。
  explicit MainWindow(bool compatibilityMode = false, const QString& osProductName = {}, const QString& osEditionId = {}, QWidget* parent = nullptr);

  // 由"单实例"机制调用：另一个实例被启动时，把本窗口从最小化恢复并带到前台。
  void raiseToFront();

 public slots:
  void refresh();
  void showPlan();
  void showImport();
  void showAbout();
  void showLogs();
  void safeShutdown();
  void safeRestart();
  // 单文件 / 单目录提交：按 QFileInfo::isDir 自动分发——目录走 QDirIterator
  // 递归遍历挨个 CommitFile，文件直接 CommitFile。DiskTab.onCommitFile /
  // onCommitDir、ExclusionListWidget 右键 commit、覆盖层文件对话框右键 commit
  // 都共用这一个槽。
  void commitFilePath(const QString& path);
  void commitFileDeletionPath(const QString& path);
  void commitRegistryKey(const QString& key, const QString& valueName);
  void commitRegistryDeletionKey(const QString& key, const QString& valueName);

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;
  void showEvent(QShowEvent* ev) override;

 private:
  void buildUi();
  // **切换主题 / 切换语言的唯一刷新入口**——整体重建 toolbar + central widget，
  // 让 tr() 拿到新译文、let QSS 在新 widget 上从干净状态应用。两个低频操作
  // 共用同一套机制，避免分两套各自的几何抖动。会丢 widget 状态里的 pending
  // changes，调用方应保证此前已 apply 过待应用变更。
  void rebuildUi();
  void rebuildTabs(const std::vector<core::DiskInfo>& disks);
  void updatePendingSummary();
  void showTransientHint(const QString& text, int msec) const;
  // buildUi 内部的初始 icon / RichText 设置助手——给 toolbar action 按当前主题
  // 染色 svg、给 hoverHint 默认文案塞主题相关色。仅供 buildUi 末尾调用一次。
  // 不再作为对外的"主题切换刷新"路径——切主题统一走 rebuildUi。
  void refreshThemedUi();

  // 5s 定时器回调：只刷新 Usage 数据——主窗口可见时刷新主面板占用条，
  // 托盘那半段交给 TrayController。
  void refreshUsage();

  // commitFilePath / commitFileDeletionPath / commitRegistryKey / commitRegistryDeletionKey
  // 共用的批处理收尾：>1 个目标弹模态进度条，逐个调用 commitOne，失败项汇总进
  // showCommitReport。commitOne 返回单个目标的 CommitResult 并自行写失败日志；
  // displayOf 给出进度条 / 报告里的展示串。模板定义在 .cpp（仅本 TU 内实例化）。
  template <typename Target, typename DisplayFn, typename CommitFn>
  void runCommitBatch(const QString& progressTitle, const QList<Target>& targets, DisplayFn displayOf, CommitFn commitOne);

  QTabWidget* m_tabs = nullptr;
  GlobalStatusPanel* m_global = nullptr;
  QLabel* m_statusText = nullptr;
  QLabel* m_hoverHint = nullptr;
  QTimer* m_hintTimer = nullptr;
  QTimer* m_hoverClearTimer = nullptr;
  QTimer* m_usageTimer = nullptr;  // 5s 周期刷新 Usage 数据
  QString m_statusBaseline;
  QString m_hoverHintDefault;

  // 引用方式持有 toolbar 6 个 action，主题切换时按当前主题前景色重染 svg。
  QAction* m_actRefresh = nullptr;
  QAction* m_actImport = nullptr;
  QAction* m_actPlan = nullptr;
  QAction* m_actShutdown = nullptr;
  QAction* m_actRestart = nullptr;
  QAction* m_actAbout = nullptr;
  QAction* m_actLog = nullptr;
  QAction* m_actLang = nullptr;
  QAction* m_actTheme = nullptr;
  // 首次 showEvent 标记：true 表示已经走过一次"shown 状态下的 rebuildUi"，
  // 后续 show 不再重复。让首屏的几何和样式与切换路径完全一致——polish /
  // QStyle 一些计算只有在 widget 真正进入 shown 状态后才稳定。
  bool m_firstShowDone = false;

  // 系统托盘（图标 + 右键菜单）——独立组件，由本窗口编排。
  TrayController* m_tray = nullptr;

  // 所有"写"操作共享同一个 WmiSession。
  WmiSession m_writeSession;
  UwfFilter m_filter{m_writeSession};
  UwfOverlay m_overlay{m_writeSession};
  UwfOverlayConfig m_overlayConfig{m_writeSession};
  UwfVolume m_volume{m_writeSession};
  UwfRegistryFilter m_registry{m_writeSession};

  QVector<QPointer<DiskTab>> m_diskTabs;
  core::UwfSnapshot m_snapshot;

  // 兼容模式标志与系统标识（系统版本未通过校验时为 true）。提示文案每次
  // buildUi 按当前语言现翻译，故只存原始数据：rebuildUi 重建面板时连带重译。
  bool m_compatibilityMode = false;
  QString m_osProductName;
  QString m_osEditionId;
};

}  // namespace uwf::ui
