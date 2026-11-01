// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <memory>
#include <optional>

#include "presentation/dialogs/PanelDialog.h"
#include "presentation/viewmodels/registry/RegistryPickerViewModel.h"

class QLineEdit;
class QPushButton;
class QTableView;
class QTreeView;
class QStackedWidget;

namespace uwf::ui {

class ContentHintBox;
class VerticalScrollArea;
class ContentState;
class TextLabel;

// The view renders model snapshots; registry I/O and navigation run asynchronously.

class RegistryPickerDialog : public PanelDialog {
  Q_OBJECT
 public:
  using Mode = presentation::RegistryPickerViewModel::Mode;
  using Result = presentation::RegistryPickerViewModel::Result;
  using KeyAvailability = presentation::RegistryPickerViewModel::KeyAvailability;
  using AvailabilityChecker = presentation::RegistryPickerViewModel::AvailabilityChecker;

  RegistryPickerDialog(std::shared_ptr<presentation::RegistryPickerViewModel> model, const QString& title, QWidget* parent = nullptr);

  // Overrides node selection, expansion and acceptance policy.
  void setAvailabilityChecker(AvailabilityChecker checker);

  // Selects the path or its deepest accessible ancestor.
  void preselectKey(const QString& key);

  // Cancellation returns nullopt.
  [[nodiscard]] std::optional<Result> selection() const { return m_viewModel->selection(); }

  // Runs the modal picker with an optional availability policy.
  [[nodiscard]] static std::optional<Result> pick(std::shared_ptr<presentation::RegistryPickerViewModel> model, const QString& title, QWidget* parent = nullptr,
                                                  AvailabilityChecker checker = {}, const QString& preselectKey = {});

 private:
  bool requestRejection() override;

  void buildUi();
  void populateRootHives();
  void onTreeSelectionChanged();
  void onValueSelectionChanged();
  void onAddressBarReturnPressed();
  void onRefresh();
  void refreshValueTable(const QString& keyPath);
  void renderAccessState();
  void navigateTo(const QString& keyPath);
  void refreshSelectionLabels();
  void updateOkButton();

  [[nodiscard]] QString currentKeyPath() const;
  [[nodiscard]] std::optional<QString> currentValueName() const;

  [[nodiscard]] bool supportsValueSelection() const { return m_viewModel->supportsValueSelection(); }

 protected:
  // Blank-area clicks clear the value selection and restore whole-key semantics.
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  std::shared_ptr<presentation::RegistryPickerViewModel> m_viewModel;

  QLineEdit* m_addressBar = nullptr;
  QTreeView* m_tree = nullptr;
  QTableView* m_valueTable = nullptr;
  QStackedWidget* m_details = nullptr;
  ContentState* m_accessError = nullptr;
  ContentHintBox* m_errorPath = nullptr;
  ContentHintBox* m_errorDetail = nullptr;
  TextLabel* m_keyLabel = nullptr;
  TextLabel* m_valueLabel = nullptr;
  QPushButton* m_wholeKey = nullptr;
  QPushButton* m_okBtn = nullptr;
};

}  // namespace uwf::ui
