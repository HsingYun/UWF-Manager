// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>
#include <memory>
#include <optional>
#include <vector>

#include "core/UwfModel.h"
#include "presentation/configuration/GlobalConfigurationViewModel.h"

class QLabel;
class QComboBox;
class QSpinBox;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class QSpacerItem;

namespace uwf::ui {
class Notice;
class IconView;

class SwitchButton;
class OverlayUsageBar;

class GlobalStatusPanel : public QWidget {
  Q_OBJECT
 public:
  GlobalStatusPanel(std::shared_ptr<presentation::GlobalConfigurationViewModel> model, QWidget* parent = nullptr);
  [[nodiscard]] presentation::GlobalConfigurationViewModel& viewModel() const { return *m_viewModel; }

  void setLayoutSize(const QSize& size);
  [[nodiscard]] QString usageWarningText() const { return m_usageWarning; }

 signals:
  void usageWarningChanged(const QString& message);

 private:
  // 统一重算大小与阈值范围，限制越界值后再由调用方发布变更。
  void renderEditor() const;
  void renderAvailability() const;
  // 根据当前类型更新 "最大大小" 标签后缀与进度条 scale。
  void refreshTypeDependentUi();
  // 主题切换时刷新 pixmap、RichText 等含主题色的元素。
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

  // 仅禁用设置区，不禁用标题与警告；滚动由宿主窗口统一管理。
  QWidget* m_controls = nullptr;

  QLabel* m_filterCur = nullptr;
  IconView* m_filterCurrentIcon = nullptr;
  SwitchButton* m_filterNext = nullptr;

  QComboBox* m_overlayTypeNext = nullptr;
  QSpinBox* m_maxNext = nullptr;
  QSpinBox* m_warnNext = nullptr;
  QSpinBox* m_critNext = nullptr;
  QLabel* m_maxLabel = nullptr;
  QLabel* m_usedLabel = nullptr;
  QLabel* m_usagePercent = nullptr;
  QLabel* m_filterNextText = nullptr;
  // "?" 锁定提示：UWF 当前会话启用时显示，hover 提示要先禁用 UWF 才能改。
  QLabel* m_typeLockedHint = nullptr;
  // 主题切换时需要重设 RichText 的图例 QLabel。
  QLabel* m_overlayLegend = nullptr;
  QLabel* m_criticalLegend = nullptr;
  OverlayUsageBar* m_usageBar = nullptr;

  std::shared_ptr<presentation::GlobalConfigurationViewModel> m_viewModel;
  uint32_t m_totalRamMb = 0;
  QString m_usageWarning;
};

}  // namespace uwf::ui
