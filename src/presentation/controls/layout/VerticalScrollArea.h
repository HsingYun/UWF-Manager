// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QScrollArea>

namespace uwf::ui {

// Preserves the content minimum width without imposing its full height on the window.
// Size hints follow current font, text and banner geometry.
class VerticalScrollArea final : public QScrollArea {
 public:
  explicit VerticalScrollArea(QWidget* parent = nullptr);
  void setWidget(QWidget* content);
  [[nodiscard]] QSize minimumSizeHint() const override;
  [[nodiscard]] QSize sizeHint() const override;

 protected:
  bool event(QEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  void scheduleScrollUpdate();
  void updateScrollPolicy();
  bool m_updatingScrollPolicy = false;
};

}  // namespace uwf::ui
