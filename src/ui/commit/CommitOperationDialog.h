// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "presentation/commit/CommitOperationViewModel.h"
#include "ui/dialogs/InformationDialog.h"
class QDialogButtonBox;
class QProgressBar;
class QPushButton;
class QStackedWidget;
namespace uwf::ui {
namespace dialogs {
class FileDialogProvider;
}
class BusyIndicator;
class IconView;
class TextLabel;
class CommitOperationDialog final : public InformationDialog {
  Q_OBJECT
 public:
  CommitOperationDialog(presentation::CommitOperationViewModel& model, dialogs::FileDialogProvider& files, app::TextFileWriter writer,
                        QWidget* parent = nullptr);

 private:
  bool requestRejection() override;
  void render();
  presentation::CommitOperationViewModel& m_model;
  TextLabel *m_target, *m_heading, *m_description, *m_count, *m_path, *m_detail, *m_note;
  IconView *m_icon, *m_noteIcon;
  BusyIndicator* m_busy;
  QProgressBar* m_progress;
  QStackedWidget *m_glyph, *m_details;
  QWidget* m_noteContainer;
  QPushButton *m_primary, *m_retry, *m_close, *m_export, *m_report;
  presentation::CommitOperationViewModel::Phase m_previous = presentation::CommitOperationViewModel::Phase::Idle;
};
}  // namespace uwf::ui
