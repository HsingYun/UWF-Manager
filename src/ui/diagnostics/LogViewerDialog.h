// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "presentation/diagnostics/LogViewModel.h"
#include "ui/dialogs/PanelDialog.h"

class QPushButton;
class QStackedWidget;
class QTableView;

namespace uwf::presentation {
class PageProxyModel;
}

namespace uwf::ui {
class ContentHintBox;
class ContentState;
class PaginationBar;
class SearchField;
class SelectBox;
class Surface;
class RecordInspector;
class TextLabel;

class LogViewerDialog : public PanelDialog {
  Q_OBJECT
 public:
  explicit LogViewerDialog(std::shared_ptr<presentation::LogViewModel> model, QWidget* parent = nullptr);

 private:
  void render();
  void updateSelection();
  void updatePagination();
  std::shared_ptr<presentation::LogViewModel> m_viewModel;
  presentation::PageProxyModel* m_pages;
  QStackedWidget* m_content;
  QWidget* m_records;
  QTableView* m_table;
  Surface* m_tableHost;
  RecordInspector* m_inspector;
  ContentState* m_state;
  ContentState* m_empty;
  ContentHintBox* m_errorDetails;
  ContentHintBox* m_details;
  QWidget* m_errorDisclosure;
  SelectBox* m_level;
  SearchField* m_search;
  PaginationBar* m_pagination;
  QPushButton* m_copy;
  QPushButton* m_refresh;
  QPushButton* m_copyAll;
  QPushButton* m_clear;
};

}  // namespace uwf::ui
