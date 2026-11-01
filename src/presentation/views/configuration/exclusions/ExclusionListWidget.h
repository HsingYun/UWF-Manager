// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QWidget>
#include <memory>

#include "presentation/controls/tables/PathEntryControls.h"
#include "presentation/viewmodels/configuration/ExclusionListViewModel.h"

class QModelIndex;
class QAction;

namespace uwf::ui {

namespace dialogs {
class FileDialogProvider;
}

class ExclusionListWidget : public QWidget {
  Q_OBJECT
 public:
  [[nodiscard]] QWidget* commandWidget() const { return m_entries.commands; }
  using Kind = presentation::ExclusionListViewModel::Kind;
  // The widget shares ownership of model; fileDialogs is borrowed and must
  // remain alive for the widget's lifetime.
  ExclusionListWidget(std::shared_ptr<presentation::ExclusionListViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);
  [[nodiscard]] presentation::ExclusionListViewModel& viewModel() const { return *m_viewModel; }

  void setReadOnly(bool ro);

 public slots:
  void addSelectedPath(const QString& selectedPath);
 signals:
  void selectionDetailChanged(const QString& detail);
  void registryPickerRequested();
  void copiedToClipboard();
  // The host executes the requested file commit.
  void commitFileRequested(const QString& absPath);

 private slots:
  void onAddFile();
  void onAddDir();
  void onAddRegistry();
  void onRemove();
  void onFilterChanged(const QString& text);
  void onItemDoubleClicked(const QModelIndex& index);

 private:
  void rebuild();
  // Refresh menu icons and the list's semantic colors after a theme change.
  void refreshThemedIcons();

  void openContainingFolder(const QString& entry) const;
  // File entries resolve to absolute paths; registry entries retain their full key paths.
  [[nodiscard]] QString entryFullPath(const QModelIndex& index) const;
  [[nodiscard]] QString absoluteFilePath(const QString& path) const;
  void renderState();

  void copyPathToClipboard(const QString& path);

  // Stages enablement or reverses pending disablement of a persistence option.
  void enablePersistFlag(presentation::ExclusionListViewModel::PersistenceItem item);

  void updateAddMenuState();

  [[nodiscard]] QVector<presentation::ExclusionListViewModel::Selection> selectedEntries() const;
  [[nodiscard]] bool canRemoveSelection() const;
  void updateRemoveButtonState();

  std::shared_ptr<presentation::ExclusionListViewModel> m_viewModel;
  Kind m_kind;
  dialogs::FileDialogProvider& m_fileDialogs;

  PathEntryControls m_entries;
  QAction* m_addFileAct = nullptr;
  QAction* m_addDirAct = nullptr;
  QAction* m_addRegKeyAct = nullptr;
  QAction* m_addDomainSecretAct = nullptr;
  QAction* m_addTscalAct = nullptr;
};

}  // namespace uwf::ui
