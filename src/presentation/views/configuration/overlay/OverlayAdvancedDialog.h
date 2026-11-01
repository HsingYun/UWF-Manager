// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <array>
#include <memory>

#include "application/overlay/contracts/OverlayMaintenance.h"
#include "presentation/dialogs/PanelDialog.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"
#include "presentation/viewmodels/configuration/OverlaySettingsEditViewModel.h"

class QBoxLayout;
class QGridLayout;
class QPushButton;
class QSpacerItem;
class QAction;
namespace uwf::presentation {
class OverlayMaintenanceViewModel;
}
namespace uwf::ui {
class DialogLayout;
class TextLabel;
class SwitchButton;
class IconView;
class Surface;
class TabView;
class Notice;

class OverlayAdvancedDialog final : public PanelDialog {
  Q_OBJECT
 public:
  explicit OverlayAdvancedDialog(std::shared_ptr<presentation::OverlayAdvancedViewModel> model, QWidget* parent = nullptr);
  OverlayAdvancedDialog(std::shared_ptr<presentation::OverlayAdvancedViewModel> model, std::shared_ptr<presentation::OverlayMaintenanceViewModel> maintenance,
                        QWidget* parent = nullptr);

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  struct Row {
    presentation::OverlayAdvancedViewModel::Field field;
    TextLabel* name;
    Surface* currentBadge;
    IconView* currentIndicator;
    TextLabel* current;
    IconView* changed;
    QPushButton* info;
    QPushButton* reason;
    QWidget* controlColumn;
    SwitchButton* control;
  };
  void render();
  void updateResponsiveLayout();
  void showFeatureDetails(presentation::OverlayAdvancedViewModel::Field field, bool restriction, QPushButton* origin);
  void showConflicts();
  void renderMaintenance();
  void showSwap();
  void showHorm();
  std::shared_ptr<presentation::OverlayAdvancedViewModel> m_model;
  std::shared_ptr<presentation::OverlaySettingsEditViewModel> m_session;
  std::shared_ptr<presentation::OverlayMaintenanceViewModel> m_maintenance;
  TabView* m_tabs = nullptr;
  QWidget* m_configurationPage = nullptr;
  QWidget* m_maintenancePage = nullptr;
  TextLabel* m_swapName = nullptr;
  TextLabel* m_swapDescription = nullptr;
  TextLabel* m_swapStatus = nullptr;
  TextLabel* m_hormName = nullptr;
  TextLabel* m_hormDescription = nullptr;
  TextLabel* m_hormStatus = nullptr;
  TextLabel* m_volumesName = nullptr;
  TextLabel* m_volumesDescription = nullptr;
  TextLabel* m_volumesStatus = nullptr;
  Notice* m_maintenanceIssue = nullptr;
  QPushButton* m_swapConfigure = nullptr;
  QPushButton* m_hormManage = nullptr;
  QPushButton* m_volumesManage = nullptr;
  QAction* m_protectAll = nullptr;
  QAction* m_unprotectAll = nullptr;
  std::array<QGridLayout*, 3> m_maintenanceRows{};
  std::array<Row, app::overlayPolicyFields.size()> m_rows{};
  std::array<TextLabel*, 2> m_contextKeys{}, m_contextValues{};
  std::array<TextLabel*, 3> m_headings{};
  DialogLayout* m_frame;
  QGridLayout* m_grid = nullptr;
  QBoxLayout* m_contextLayout;
  QBoxLayout* m_typeLayout;
  QBoxLayout* m_footerLayout;
  QSpacerItem* m_contextGap;
  IconView* m_filterIndicator;
  TextLabel* m_filterText;
  TextLabel* m_hint;
  TextLabel* m_pendingCount;
  QPushButton* m_revert;
  QPushButton* m_conflictDetails;
  IconView* m_warning;
  QPushButton* m_confirm;
  QPushButton* m_cancel;
  int m_layoutProfile = -1;
};
}  // namespace uwf::ui
