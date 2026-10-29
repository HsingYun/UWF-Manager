// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "app/ports/FileBrowser.h"
#include "ui/dialogs/PanelDialog.h"

class QAbstractButton;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTableView;
namespace uwf::presentation {
class FilePickerViewModel;
}
namespace uwf::ui {
class FilePickerState;
class SearchField;
class SelectBox;
class TextLabel;
class FilePickerDialog final : public PanelDialog {
  Q_OBJECT
 public:
  explicit FilePickerDialog(std::shared_ptr<presentation::FilePickerViewModel> model, const QString& title, QWidget* parent = nullptr);
  [[nodiscard]] QStringList selectedPaths() const;
  [[nodiscard]] QVector<app::FileBrowserEntry> selectedItems() const;

 private:
  bool requestRejection() override;
  void renderState();
  void renderSelection();
  void acceptSelection();
  void createDirectory();
  void renameEntry(int row);
  std::shared_ptr<presentation::FilePickerViewModel> m_model;
  QLineEdit* m_address;
  SearchField* m_search;
  QLineEdit* m_name;
  SelectBox* m_filter;
  QTableView* m_table;
  QStackedWidget* m_pages;
  FilePickerState* m_state;
  TextLabel* m_count;
  QPushButton* m_accept;
  QAbstractButton* m_back;
  QAbstractButton* m_up;
  QAbstractButton* m_newFolder;
  bool m_resetting = false;
};
}  // namespace uwf::ui
