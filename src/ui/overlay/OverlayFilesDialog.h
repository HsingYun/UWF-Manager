// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <memory>

#include "presentation/overlay/OverlayFilesViewModel.h"
#include "ui/dialogs/PanelDialog.h"
class QStackedWidget;
class QPushButton;
namespace uwf::presentation {
class PageProxyModel;
}
namespace uwf::ui {
namespace dialogs {
class FileDialogProvider;
}
class ContentHintBox;
class ContentState;
class Disclosure;
class PagedTableView;
class PaginationBar;
class RecordInspector;
class TextLabel;
class OverlayFilesDialog final : public PanelDialog {
  Q_OBJECT
 public:
  OverlayFilesDialog(std::shared_ptr<presentation::OverlayFilesViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);
  ~OverlayFilesDialog() override;
 signals:
  void commitFileRequested(const QString& absolutePath);

 private:
  void onContextMenu(const QPoint& pos);
  void onExportClicked();
  void showSnapshotScope();
  void renderPage();
  void renderSelection();
  static void openContainingFolder(const QString& path);
  dialogs::FileDialogProvider& m_fileDialogs;
  std::shared_ptr<presentation::OverlayFilesViewModel> m_viewModel;
  presentation::PageProxyModel* m_pages;
  QStackedWidget* m_content;
  QWidget* m_records;
  ContentState* m_state;
  PagedTableView* m_table = nullptr;
  PaginationBar* m_pagination = nullptr;
  Disclosure* m_errorDisclosure = nullptr;
  TextLabel* m_errorHint = nullptr;
  ContentHintBox* m_errorText = nullptr;
  ContentHintBox* m_selectedPath = nullptr;
  RecordInspector* m_inspector = nullptr;
  QPushButton* m_exportBtn = nullptr;
  QPushButton* m_commit = nullptr;
};
}  // namespace uwf::ui
