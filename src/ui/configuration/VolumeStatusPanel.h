// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>
#include <memory>
#include <optional>

#include "core/UwfModel.h"
#include "presentation/configuration/VolumeStatusViewModel.h"

class QLabel;
class QComboBox;
class QHBoxLayout;

namespace uwf::ui {
class Notice;

class SwitchButton;

class VolumeStatusPanel : public QWidget {
  Q_OBJECT
 public:
  VolumeStatusPanel(std::shared_ptr<presentation::VolumeStatusViewModel> model, QWidget* parent = nullptr);

  void setUnsupported(const QString& reason);
  // 比 setUnsupported 弱：只显示一条警告横幅，不灰化任何控件。
  // 用于"卷可保护但有局部限制"的情况（如 exFAT 不支持文件排除）。
  void setNotice(const QString& text);
  void setLayoutSize(const QSize& size);

 private:
  void renderEditor();
  void renderAvailability() const;

  Notice* m_banner = nullptr;

  QLabel* m_protectCur = nullptr;
  SwitchButton* m_protectNext = nullptr;
  QComboBox* m_bindNext = nullptr;

  std::shared_ptr<presentation::VolumeStatusViewModel> m_viewModel;
  // 常驻提示在后续 setData 中保留。
  bool m_hasStickyNotice = false;
  QLayout* m_cardLayout = nullptr;
};

}  // namespace uwf::ui
