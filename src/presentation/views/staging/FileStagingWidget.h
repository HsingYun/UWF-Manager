// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QWidget>

#include "presentation/controls/tables/PathEntryControls.h"
#include "presentation/viewmodels/staging/FileStagingViewModel.h"

class QAction;
class QModelIndex;

namespace uwf::ui {

namespace dialogs {
class FileDialogProvider;
}

// Shows one volume from the shared staging draft. Edits preserve other volumes
// and reach persistent storage only after configuration approval.
class FileStagingWidget : public QWidget {
  Q_OBJECT
 public:
  [[nodiscard]] QWidget* commandWidget() const { return m_entries.commands; }
  FileStagingWidget(std::shared_ptr<presentation::FileStagingViewModel> viewModel, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);
  [[nodiscard]] QString driveLetter() const { return m_viewModel->driveLetter(); }
  [[nodiscard]] presentation::FileStagingViewModel& viewModel() const { return *m_viewModel; }
  // Enhanced-mode availability affects Windows shutdown integration, not draft editing
  // or the application's safe power actions.
  [[nodiscard]] bool enhancedModeAvailable() const { return m_viewModel->enhancedModeAvailable(); }

 signals:
  void selectionDetailChanged(const QString& detail);
  void copiedToClipboard();
  void enhancedModeAvailabilityChanged();

 private slots:
  void addFiles();
  void addDirectory();
  void removeSelected();
  void filterChanged(const QString& text);
  void copySelectedPath(const QModelIndex& index);

 private:
  void rebuild();
  void renderState();
  void refreshThemedIcons();

  std::shared_ptr<presentation::FileStagingViewModel> m_viewModel;
  dialogs::FileDialogProvider& m_fileDialogs;
  PathEntryControls m_entries;
  QAction* m_addFileAction = nullptr;
  QAction* m_addDirectoryAction = nullptr;
};

}  // namespace uwf::ui
