#pragma once

#include <QSet>
#include <QString>
#include <QStringList>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QLabel;
class QPushButton;
class QAction;

namespace uwf::ui {

class ExclusionListWidget : public QWidget {
  Q_OBJECT
 public:
  enum class Kind { File, Registry };
  explicit ExclusionListWidget(Kind kind, QWidget* parent = nullptr);

  void setDriveLetter(const QString& dl);
  void setBaseline(const QStringList& currentSession, const QStringList& nextSession);
  void resetPending();

  [[nodiscard]] QStringList pendingAdded() const;
  [[nodiscard]] QStringList pendingRemoved() const;

  void setReadOnly(bool ro);

  // 文件列表右键"提交改动到磁盘"的总开关：仅当本卷当前会话存在活动覆盖层
  // （全局筛选器开 + 本卷当前会话受保护）时才为 true，由 DiskTab 每次快照后
  // 下发。为 false 时右键菜单不列出"提交"项。Registry kind 无此项，设之无副作用。
  void setCommitEnabled(bool enabled);

  // 给"导入 uwfmgr 命令"流程用的静默 add / remove。和 addPendingEntry 的差异：
  //   - 不弹任何对话框；通过返回值告诉 caller 结果，caller 自己汇总到导入报告里；
  //   - 复用同一组黑名单 / 卷归属校验，保证导入的路径和 UI 手动加进来的具备
  //     同样的合法性约束。
  enum class ImportOutcome {
    Applied,              // 状态发生了变化（加入 m_added 或 m_removed，或撤销了对应集合里的旧条目）
    NoOp,                 // 已经在目标状态，无须再动
    RejectedNotOnVolume,  // 仅 File：路径不在本卷
    RejectedForbidden,    // 触发 UWF 不允许排除的黑名单
  };
  ImportOutcome importAdd(const QString& raw);
  ImportOutcome importRemove(const QString& raw);

 signals:
  void pendingChanged();
  void copiedToClipboard(const QString& hint);
  // 用户在文件列表上右键 → "提交文件/文件夹改动" 时发出，DiskTab 把绝对
  // 路径转发到 MainWindow，最终走 UWF_Volume.CommitFile。
  void commitFileRequested(const QString& absPath);

 private slots:
  void onAddFile();
  void onAddDir();
  void onAddRegistry();
  void onRemove();
  void onFilterChanged(const QString& text);
  void onItemDoubleClicked(QListWidgetItem* item);

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  void rebuild();
  void addPendingEntry(const QString& raw);
  void applyViewportMask();
  // 主题切换时刷新顶部按钮 / 菜单项的 icon，并触发 list rebuild 让条目
  // 重新染色（行前景、徽章色都依赖当前主题）。
  void refreshThemedIcons();
  // 用资源管理器打开 entry 所在文件夹，并高亮该条目。仅 File kind 用。
  void openContainingFolder(const QString& entry) const;
  // item 对应的完整路径：文件列表拼成带盘符的绝对路径，注册表列表即键全路径。
  [[nodiscard]] QString entryFullPath(const QListWidgetItem* item) const;
  // 把 path 复制到剪贴板并发出 copiedToClipboard 提示。
  void copyPathToClipboard(const QString& path);

  Kind m_kind;
  QString m_driveLetter;
  QStringList m_current;
  QStringList m_next;
  QSet<QString> m_added;
  QSet<QString> m_removed;

  QListWidget* m_list = nullptr;
  QLineEdit* m_filter = nullptr;
  QLabel* m_summary = nullptr;
  // 主题切换时这些按钮 / 菜单项的 svg 图标要重染色，需要持有引用。
  QPushButton* m_addBtn = nullptr;
  QPushButton* m_rmBtn = nullptr;
  QAction* m_addFileAct = nullptr;
  QAction* m_addDirAct = nullptr;
  bool m_readOnly = false;
  bool m_commitEnabled = false;
};

}  // namespace uwf::ui
