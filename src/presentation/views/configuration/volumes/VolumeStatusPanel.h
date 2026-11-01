// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>
#include <memory>
#include <optional>

#include "domain/uwf/model/UwfModel.h"
#include "presentation/viewmodels/configuration/VolumeStatusViewModel.h"

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
  // Adds a persistent warning without changing control availability.
  void setNotice(const QString& text);
  void setLayoutSize(const QSize& size);

 private:
  void renderEditor();
  void renderState() const;

  Notice* m_banner = nullptr;

  QLabel* m_protectCur = nullptr;
  SwitchButton* m_protectNext = nullptr;
  QComboBox* m_bindNext = nullptr;

  std::shared_ptr<presentation::VolumeStatusViewModel> m_viewModel;
  // Retained across editor rendering.
  bool m_hasStickyNotice = false;
  QLayout* m_cardLayout = nullptr;
};

}  // namespace uwf::ui
