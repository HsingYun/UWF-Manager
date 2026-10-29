// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QSet>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <memory>
#include <optional>

#include "presentation/configuration/ExclusionListViewModel.h"

class QListView;
class QModelIndex;
class QLineEdit;
class QLabel;
class QPushButton;
class QAction;

namespace uwf::app {
class FileStagingConflictPolicy;
}

namespace uwf::ui {

class RoundedCornerOverlay;
class PaginationBar;
namespace dialogs {
class FileDialogProvider;
}

class ExclusionListWidget : public QWidget {
  Q_OBJECT
 public:
  [[nodiscard]] QWidget* commandWidget() const { return m_commands; }
  using Kind = presentation::ExclusionListViewModel::Kind;
  ExclusionListWidget(std::shared_ptr<presentation::ExclusionListViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);
  [[nodiscard]] presentation::ExclusionListViewModel& viewModel() const { return *m_viewModel; }
  // 注入对象不转移所有权，生命周期必须覆盖本控件。

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
  // （全局筛选器开 + 本卷当前会话受保护）时才为 true，由 VolumeTab 每次快照后
  // 下发。为 false 时右键菜单不列出"提交"项。Registry kind 无此项，设之无副作用。
  void setCommitEnabled(bool enabled);

 public slots:
  void addPendingEntry(const QString& raw);
 signals:
  void selectionDetailChanged(const QString& detail);
  void registryPickerRequested();
  void pendingChanged();
  void copiedToClipboard();
  // 请求提交单条文件路径，由宿主执行。
  void commitFileRequested(const QString& absPath);

 private slots:
  void onAddFile();
  void onAddDir();
  void onAddRegistry();
  void onRemove();
  void onFilterChanged(const QString& text);
  void onItemDoubleClicked(const QModelIndex& index);

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  void rebuild();
  // 主题切换时刷新顶部按钮 / 菜单项的 icon，并触发 list rebuild 让条目
  // 重新染色（行前景、徽章色都依赖当前主题）。
  void refreshThemedIcons();
  // 用资源管理器打开 entry 所在文件夹，并高亮该条目。仅 File kind 用。
  void openContainingFolder(const QString& entry) const;
  // item 对应的完整路径：文件列表拼成带盘符的绝对路径，注册表列表即键全路径。
  [[nodiscard]] QString entryFullPath(const QModelIndex& index) const;
  [[nodiscard]] QString absoluteFilePath(const QString& path) const;
  void applyReadOnly();
  // 把 path 复制到剪贴板并发出 copiedToClipboard 提示。
  void copyPathToClipboard(const QString& path);

  // 菜单"开启 X"：把伪条目标记为待开启（或撤销待关闭）。
  void enablePersistFlag(presentation::ExclusionListViewModel::PersistenceItem item);
  // Registry "添加" 菜单弹出前：已开启的开关项置灰。
  void updateAddMenuState();
  // 根据只读状态、当前选区和文件暂存数据状态更新“移除所选”。
  // 只读或选区含受保护的文件暂存注册表根时返回 false。
  [[nodiscard]] bool canRemoveSelection() const;
  void updateRemoveButtonState();

  std::shared_ptr<presentation::ExclusionListViewModel> m_viewModel;
  Kind m_kind;
  dialogs::FileDialogProvider& m_fileDialogs;

  QListView* m_list = nullptr;
  // 覆盖列表容器的圆角遮罩与边框绘制层。
  RoundedCornerOverlay* m_cornerOverlay = nullptr;
  QWidget* m_commands = nullptr;
  QLineEdit* m_filter = nullptr;
  PaginationBar* m_pagination = nullptr;
  QPushButton* m_addBtn = nullptr;
  QPushButton* m_rmBtn = nullptr;
  QAction* m_addFileAct = nullptr;
  QAction* m_addDirAct = nullptr;
  QAction* m_addRegKeyAct = nullptr;
  QAction* m_addDomainSecretAct = nullptr;
  QAction* m_addTscalAct = nullptr;
};

}  // namespace uwf::ui
