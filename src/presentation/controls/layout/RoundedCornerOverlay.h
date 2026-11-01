// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QColor>
#include <QWidget>
#include <functional>

namespace uwf::ui {

// Paints antialiased corners over scroll content without intercepting input.
// Stylesheets do not clip child content, and QRegion masks have no antialiasing.
class RoundedCornerOverlay : public QWidget {
 public:
  // Geometry uses logical pixels. colorFn supplies the exterior fill; border styling is optional.
  // An absent edgesFn selects every edge and corner.
  RoundedCornerOverlay(QWidget* host, qreal inset, qreal radius, std::function<QColor()> colorFn, std::function<QColor()> borderFn = {}, qreal borderWidth = 0,
                       std::function<Qt::Edges()> edgesFn = {});

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;
  void paintEvent(QPaintEvent* ev) override;

 private:
  void syncToParent();
  qreal m_inset;
  qreal m_radius;
  std::function<QColor()> m_colorFn;
  std::function<QColor()> m_borderFn;
  qreal m_borderWidth;
  std::function<Qt::Edges()> m_edgesFn;
};

}  // namespace uwf::ui
