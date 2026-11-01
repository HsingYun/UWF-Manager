// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <memory>

#include "presentation/dialogs/InformationDialog.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"

class QPushButton;
class QVBoxLayout;
namespace uwf::presentation {
class OverlaySettingsEditViewModel;
}

namespace uwf::ui {
class IconView;
class TextLabel;
class DialogLayout;

class OverlayAdvancedDetailsDialog final : public InformationDialog {
  Q_OBJECT
 public:
  using Model = presentation::OverlayAdvancedViewModel;
  using Field = Model::Field;
  using Edit = Model::Edit;
  enum class Purpose { Feature, Restriction, Conflicts };
  Q_ENUM(Purpose)
  explicit OverlayAdvancedDetailsDialog(std::shared_ptr<Model> model, const Edit& edit, Purpose purpose, Field field = Field::Passthrough,
                                        QWidget* parent = nullptr);
  explicit OverlayAdvancedDetailsDialog(std::shared_ptr<presentation::OverlaySettingsEditViewModel> session, QWidget* parent = nullptr);

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  struct ConditionLabels {
    TextLabel* title;
    TextLabel* text;
  };
  struct ConflictRow {
    QWidget* widget;
    TextLabel* text;
  };
  void render();
  void updateResponsiveLayout();
  std::shared_ptr<Model> m_model;
  std::shared_ptr<presentation::OverlaySettingsEditViewModel> m_session;
  const Edit m_edit;
  const Purpose m_purpose;
  const Field m_field;
  DialogLayout* m_frame = nullptr;
  IconView* m_icon;
  TextLabel* m_introduction;
  TextLabel* m_official;
  TextLabel* m_description = nullptr;
  TextLabel* m_reason = nullptr;
  TextLabel* m_featureHeading = nullptr;
  QList<ConditionLabels> m_conditions;
  QList<ConflictRow> m_conflictRows;
  QVBoxLayout* m_conflicts = nullptr;
  QPushButton* m_back;
  int m_layoutProfile = -1;
};
}  // namespace uwf::ui
