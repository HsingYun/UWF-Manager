// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <memory>

#include "presentation/dialogs/PanelDialog.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"

class QComboBox;
class QResizeEvent;
class QGridLayout;
class QHBoxLayout;

namespace uwf::ui {
class BusyIndicator;
class DialogLayout;
class Notice;
class TextLabel;

// Shared presentation chrome only. The view model owns observation and draft
// eligibility; accepted plans own execution. No dialog accesses a provider.
class OverlayMaintenanceDialog : public PanelDialog {
 public:
  using Model = presentation::OverlayMaintenanceViewModel;

 protected:
  OverlayMaintenanceDialog(std::shared_ptr<Model> model, QWidget* parent);
  void finishBody();
  void renderDialogState(const Model::DialogState& state);
  void resizeEvent(QResizeEvent* event) override;
  std::shared_ptr<Model> m_model;
  DialogLayout* m_frame;
  Notice* m_feedback;
  QWidget* m_activity;
  TextLabel* m_activityText;
  bool m_hormChrome = false;
  QPushButton* m_cancel;
  QPushButton* m_submit;
  QHBoxLayout* m_leadingActions;
};

class FullCommitDialog final : public OverlayMaintenanceDialog {
  Q_OBJECT
 public:
  explicit FullCommitDialog(std::shared_ptr<Model> model, QWidget* parent = nullptr);

 private:
  void render();
};

class SwapDialog final : public OverlayMaintenanceDialog {
  Q_OBJECT
 public:
  explicit SwapDialog(std::shared_ptr<Model> model, QWidget* parent = nullptr);

 private:
  void render();
  QComboBox* m_volumes;
  TextLabel* m_filter;
  TextLabel* m_currentType;
  TextLabel* m_target;
  TextLabel* m_nextType;
  TextLabel* m_capacity;
  TextLabel* m_previousHost;
  QWidget* m_statusGap;
};

class HormDialog final : public OverlayMaintenanceDialog {
  Q_OBJECT
 public:
  explicit HormDialog(std::shared_ptr<Model> model, QWidget* parent = nullptr);

 private:
  void render();
  TextLabel* m_effect;
  QWidget* m_effectHost;
  QWidget* m_blockers;
  QGridLayout* m_blockerRows;
  QWidget* m_checklist;
  QPushButton* m_protectAll;
  TextLabel* m_protectionPending;
  QList<TextLabel*> m_requirementLabels;
  QList<TextLabel*> m_requirementStates;
  QList<class IconView*> m_requirementIcons;
};
}  // namespace uwf::ui
