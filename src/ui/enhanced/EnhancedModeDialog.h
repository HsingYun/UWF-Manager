// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <array>
#include <optional>

#include "app/enhanced/EnhancedModeProtocol.h"
#include "presentation/enhanced/EnhancedModeViewModel.h"
#include "ui/dialogs/PanelDialog.h"

class QLabel;
class QPushButton;

namespace uwf::ui {

class ContentHintBox;
class Notice;

class EnhancedModeDialog final : public PanelDialog {
  Q_OBJECT
 public:
  EnhancedModeDialog(std::shared_ptr<presentation::EnhancedModeViewModel> viewModel, QWidget* parent = nullptr);

 private:
  void render();
  void renderOperationFeedback();
  void renderStatusUnavailable(const QString& detail);
  void applyRequestedState();
  void removeService();

  std::shared_ptr<presentation::EnhancedModeViewModel> m_viewModel;
  QLabel* m_statusLabel = nullptr;
  QLabel* m_serviceValue = nullptr;
  QLabel* m_stagingValue = nullptr;
  ContentHintBox* m_serviceDetails = nullptr;
  QWidget* m_serviceFacts = nullptr;
  std::array<ContentHintBox*, 5> m_serviceValues{};
  Notice* m_issueLabel = nullptr;
  Notice* m_detailLabel = nullptr;
  QPushButton* m_changeButton = nullptr;
  QPushButton* m_removeButton = nullptr;
};

}  // namespace uwf::ui
