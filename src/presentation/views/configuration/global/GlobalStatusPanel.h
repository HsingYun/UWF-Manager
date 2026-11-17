// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>
#include <memory>
#include <vector>

#include "presentation/viewmodels/configuration/GlobalConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"

class QLabel;
class QComboBox;
class QDoubleSpinBox;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class QSpacerItem;
class QPushButton;

namespace uwf::ui {
class Notice;
class IconView;

class SwitchButton;
class OverlayUsageBar;
class OverlaySettingsButton;

class GlobalStatusPanel : public QWidget {
  Q_OBJECT
 public:
  GlobalStatusPanel(std::shared_ptr<presentation::GlobalConfigurationViewModel> model, QWidget* parent = nullptr);
  GlobalStatusPanel(std::shared_ptr<presentation::GlobalConfigurationViewModel> model, std::shared_ptr<presentation::OverlayAdvancedViewModel> overlay,
                    QWidget* parent = nullptr);
  [[nodiscard]] presentation::GlobalConfigurationViewModel& viewModel() const { return *m_viewModel; }

  void setLayoutSize(const QSize& size);
  [[nodiscard]] QString usageWarningText() const { return m_usageWarning; }

 signals:
  void usageWarningChanged(const QString& message);
  void advancedSettingsRequested();

 private:
  void renderState();

  void renderEditor() const;
  void renderAvailability() const;

  void refreshTypeDependentUi();

  void applyTheme();
  void renderUsage();
  QWidget* makeSection(const char* title, QGridLayout* rows, QWidget* hint = nullptr, QWidget* footer = nullptr);
  struct SectionLayout {
    QGridLayout* rows;
    QHBoxLayout* heading;
    int bottomMargin;
  };
  std::vector<SectionLayout> m_sections;
  QVBoxLayout* m_outerLayout = nullptr;
  QVBoxLayout* m_sectionsLayout = nullptr;
  QVBoxLayout* m_usageLayout = nullptr;
  QSpacerItem* m_usageCaptionGap = nullptr;
  QSpacerItem* m_usageBarGap = nullptr;
  QSpacerItem* m_usageLegendGap = nullptr;
  QLabel* m_title = nullptr;

  // The editable group excludes diagnostic headings and warnings.
  QWidget* m_controls = nullptr;

  QLabel* m_filterCur = nullptr;
  IconView* m_filterCurrentIcon = nullptr;
  SwitchButton* m_filterNext = nullptr;

  QComboBox* m_overlayTypeNext = nullptr;
  QDoubleSpinBox* m_maxNext = nullptr;
  QDoubleSpinBox* m_warnNext = nullptr;
  QDoubleSpinBox* m_critNext = nullptr;
  QLabel* m_maxLabel = nullptr;
  QLabel* m_usedLabel = nullptr;
  QLabel* m_usagePercent = nullptr;
  QLabel* m_filterNextText = nullptr;
  // Advanced settings remain accessible when the ordinary form is read-only.
  OverlaySettingsButton* m_advancedSettings = nullptr;

  QLabel* m_overlayLegend = nullptr;
  QLabel* m_criticalLegend = nullptr;
  OverlayUsageBar* m_usageBar = nullptr;

  std::shared_ptr<presentation::GlobalConfigurationViewModel> m_viewModel;
  std::shared_ptr<presentation::OverlayAdvancedViewModel> m_overlayPolicy;
  uint32_t m_totalRamMb = 0;
  QString m_usageWarning;
};

}  // namespace uwf::ui
