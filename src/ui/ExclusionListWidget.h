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

#include <QSet>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <optional>

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QLabel;
class QPushButton;
class QAction;

namespace uwf::ui {

class RoundedCornerOverlay;
namespace dialogs {
class FileDialogProvider;
}

class ExclusionListWidget : public QWidget {
  Q_OBJECT
 public:
  enum class Kind { File, Registry };
  explicit ExclusionListWidget(Kind kind, QWidget* parent = nullptr);
  // 注入对象不转移所有权，生命周期必须覆盖本控件。
  ExclusionListWidget(Kind kind, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);

  void setDriveLetter(const QString& dl);
  void setBaseline(const QStringList& currentSession, const QStringList& nextSession);
  // Registry kind 专用：把 UWF_RegistryFilter 的两个全局持久化开关
  // （PersistDomainSecretKey / PersistTSCAL）作为伪条目纳入本列表，置于最前。
  // current 或 next 任一为 true 才显示；染色复用排除项的 current/next 逻辑。
  // 须在 setBaseline 之后调用。
  void setPersistBaseline(bool domainSecretKeyCurrent, bool domainSecretKeyNext, bool tscalCurrent, bool tscalNext);
  void resetPending();

  [[nodiscard]] QStringList pendingAdded() const;
  [[nodiscard]] QStringList pendingRemoved() const;

  // 两个持久化开关的待应用值：nullopt = 未改动，否则为用户期望的下次会话值。
  [[nodiscard]] std::optional<bool> pendingPersistDomainSecretKey() const;
  [[nodiscard]] std::optional<bool> pendingPersistTSCAL() const;

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
  // 主题切换时刷新顶部按钮 / 菜单项的 icon，并触发 list rebuild 让条目
  // 重新染色（行前景、徽章色都依赖当前主题）。
  void refreshThemedIcons();
  // 用资源管理器打开 entry 所在文件夹，并高亮该条目。仅 File kind 用。
  void openContainingFolder(const QString& entry) const;
  // item 对应的完整路径：文件列表拼成带盘符的绝对路径，注册表列表即键全路径。
  [[nodiscard]] QString entryFullPath(const QListWidgetItem* item) const;
  // 把 path 复制到剪贴板并发出 copiedToClipboard 提示。
  void copyPathToClipboard(const QString& path);

  // Registry kind：UWF_RegistryFilter 持久化开关伪条目的状态。baseCurrent /
  // baseNext 来自快照，pendingNext 为用户的待应用改动（nullopt = 未改）。
  struct PersistFlag {
    QString name;
    bool baseCurrent = false;
    bool baseNext = false;
    std::optional<bool> pendingNext;
    [[nodiscard]] bool visible() const { return baseCurrent || baseNext || pendingNext == true; }
    [[nodiscard]] bool effectiveNext() const { return pendingNext.value_or(baseNext); }
  };
  // 菜单"开启 X"：把伪条目标记为待开启（或撤销待关闭）。
  void enablePersistFlag(PersistFlag& flag);
  // entry 是否为某个持久化开关伪条目。
  [[nodiscard]] bool isPersistRow(const QString& entry) const;
  // Registry "添加" 菜单弹出前：已开启的开关项置灰。
  void updateAddMenuState();

  Kind m_kind;
  dialogs::FileDialogProvider& m_fileDialogs;
  QString m_driveLetter;
  QStringList m_current;
  QStringList m_next;
  QSet<QString> m_added;
  QSet<QString> m_removed;

  QListWidget* m_list = nullptr;
  // 盖在列表容器上、抗锯齿补圆角 + 重描边框的遮罩层（首尾项 / 滚动时角部不再穿帮）。
  RoundedCornerOverlay* m_cornerOverlay = nullptr;
  QLineEdit* m_filter = nullptr;
  QLabel* m_summary = nullptr;
  // 主题切换时这些按钮 / 菜单项的 svg 图标要重染色，需要持有引用。
  QPushButton* m_addBtn = nullptr;
  QPushButton* m_rmBtn = nullptr;
  QAction* m_addFileAct = nullptr;
  QAction* m_addDirAct = nullptr;
  // Registry kind 的"添加"下拉菜单三项。
  QAction* m_addRegKeyAct = nullptr;
  QAction* m_addDomainSecretAct = nullptr;
  QAction* m_addTscalAct = nullptr;
  PersistFlag m_persistDomainSecretKey;
  PersistFlag m_persistTSCAL;
  bool m_readOnly = false;
  bool m_commitEnabled = false;
};

}  // namespace uwf::ui
