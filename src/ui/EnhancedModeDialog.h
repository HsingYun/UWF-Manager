/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <QDialog>
#include <QString>
#include <optional>

#include "../service/EnhancedModeService.h"

class QLabel;
class QPushButton;

namespace uwf::ui {

class ContentHintBox;

class EnhancedModeDialog final : public QDialog {
  Q_OBJECT
 public:
  explicit EnhancedModeDialog(service::EnhancedModeManager& manager, QWidget* parent = nullptr);

 private:
  enum class FeedbackTone {
    Neutral,
    Success,
    Warning,
    Error,
  };
  struct OperationFeedback {
    QString detail;
    FeedbackTone tone = FeedbackTone::Neutral;
  };

  void refreshStatus();
  void render(const service::EnhancedModeStatus& status);
  void setOperationFeedback(QString detail, FeedbackTone tone);
  void clearOperationFeedback();
  void renderOperationFeedback();
  void renderStatusUnavailable(const QString& detail);
  void recoverAfterOperationFailure(const QString& detail);
  void applyRequestedState();
  void removeService();

  service::EnhancedModeManager& m_manager;
  service::EnhancedModeAction m_action = service::EnhancedModeAction::Enable;
  std::optional<service::EnhancedModeStatus> m_status;
  std::optional<OperationFeedback> m_operationFeedback;
  QLabel* m_statusLabel = nullptr;
  ContentHintBox* m_issueLabel = nullptr;
  ContentHintBox* m_detailLabel = nullptr;
  QPushButton* m_changeButton = nullptr;
  QPushButton* m_removeButton = nullptr;
};

}  // namespace uwf::ui
