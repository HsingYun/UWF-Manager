// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Importing updates the draft; the view model owns parsing and edit admission.

#include <QList>
#include <QString>
#include <array>
#include <memory>

#include "presentation/dialogs/PanelDialog.h"
#include "presentation/viewmodels/configuration/ImportViewModel.h"

class QPushButton;
class QTableView;
class QLabel;
class QTabWidget;

namespace uwf::ui {
class CodeEditor;
class TextLabel;

namespace dialogs {
class FileDialogProvider;
}

class ImportDialog : public PanelDialog {
  Q_OBJECT
 public:
  ImportDialog(std::shared_ptr<presentation::ImportViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);

  [[nodiscard]] bool reviewRequested() const { return m_reviewRequested; }

 private slots:
  void onImportClicked();

 private:
  void renderReport(const QList<app::ImportResult>& rows);
  void renderActions();
  void showResult(const QModelIndex& index);

  std::shared_ptr<presentation::ImportViewModel> m_viewModel;
  dialogs::FileDialogProvider& m_fileDialogs;
  CodeEditor* m_text = nullptr;
  QTableView* m_report = nullptr;
  QWidget* m_summary = nullptr;
  std::array<TextLabel*, 4> m_summaryLabels{};
  QPushButton* m_importBtn = nullptr;
  QPushButton* m_closeBtn = nullptr;
  QTabWidget* m_tabs = nullptr;
  bool m_reviewRequested = false;
};

}  // namespace uwf::ui
