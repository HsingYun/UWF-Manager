// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>
#include <cstdint>

namespace uwf::ui {

// Overlay consumption normalized to its configured capacity, with threshold markers.
class OverlayUsageBar : public QWidget {
  Q_OBJECT
 public:
  explicit OverlayUsageBar(QWidget* parent = nullptr);

  void setData(uint32_t currentMb, uint32_t warningMb, uint32_t criticalMb, uint32_t maximumMb);

  [[nodiscard]] QSize sizeHint() const override;
  [[nodiscard]] QSize minimumSizeHint() const override;

 protected:
  void paintEvent(QPaintEvent*) override;

 private:
  uint32_t m_current = 0;
  uint32_t m_warning = 0;
  uint32_t m_critical = 0;
  uint32_t m_max = 0;
};

}  // namespace uwf::ui
