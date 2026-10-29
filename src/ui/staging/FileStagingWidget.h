// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QStringList>
#include <QWidget>

#include "app/ports/FileStagingStore.h"
#include "presentation/staging/FileStagingViewModel.h"

class QAction;
class QEvent;
class QFrame;
class QLabel;
class QLineEdit;
class QListView;
class QModelIndex;
class QPushButton;

namespace uwf::ui {

class RoundedCornerOverlay;
class PaginationBar;
namespace dialogs {
class FileDialogProvider;
}

// 安全关机/重启前自动提交的持久化路径列表。每个磁盘页只呈现本卷条目，但
// FileStagingStore 仍保存全部卷的统一快照；修改本卷时必须保留其它卷条目。
// 修改仅更新待应用草稿；确认应用后才写入持久存储。
class FileStagingWidget : public QWidget {
  Q_OBJECT
 public:
  [[nodiscard]] QWidget* commandWidget() const { return m_commands; }
  FileStagingWidget(std::shared_ptr<presentation::FileStagingViewModel> viewModel, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);
  [[nodiscard]] QString driveLetter() const { return m_viewModel->driveLetter(); }
  [[nodiscard]] presentation::FileStagingViewModel& viewModel() const { return *m_viewModel; }
  // 只读状态只禁止修改持久化列表；筛选、选择、复制和打开所在目录仍可用。
  // 存储读取失败是另一条状态，不得用 setEnabled(false) 把两者混为一谈。
  void setReadOnly(bool readOnly);
  // 增强模式仅影响 Windows 关机时的自动提交；不可用时提示安全电源入口，
  // 不限制列表编辑和程序内的安全关机 / 重启。
  void setEnhancedModeAvailable(bool available);
  [[nodiscard]] bool enhancedModeAvailable() const { return m_viewModel->enhancedModeAvailable(); }

 signals:
  void selectionDetailChanged(const QString& detail);
  void copiedToClipboard();
  void enhancedModeAvailabilityChanged();

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

 private slots:
  void addFiles();
  void addDirectory();
  void removeSelected();
  void filterChanged(const QString& text);
  void copySelectedPath(const QModelIndex& index);

 private:
  void rebuild();
  void renderSummary();
  void refreshThemedIcons();
  void updateModificationControls();
  void setStorageUnavailable(const QString& error);

  std::shared_ptr<presentation::FileStagingViewModel> m_viewModel;
  dialogs::FileDialogProvider& m_fileDialogs;
  QWidget* m_commands = nullptr;
  QLineEdit* m_filter = nullptr;
  QListView* m_list = nullptr;
  PaginationBar* m_pagination = nullptr;
  QPushButton* m_addButton = nullptr;
  QPushButton* m_removeButton = nullptr;
  QAction* m_addFileAction = nullptr;
  QAction* m_addDirectoryAction = nullptr;
  RoundedCornerOverlay* m_cornerOverlay = nullptr;
};

}  // namespace uwf::ui
