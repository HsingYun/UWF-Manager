// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractScrollArea>
#include <QApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QLayout>
#include <QScrollBar>
#include <QTimer>
#include <QWidget>
#include <utility>
#include <vector>

namespace uwf::test {
namespace layout_settling {
struct Geometry {
  const QWidget* widget;
  QRect bounds;
  QSize minimum;
  bool visible;
  int horizontalMaximum = 0;
  int verticalMaximum = 0;
  bool operator==(const Geometry&) const = default;
};

inline void activate(QWidget& widget) {
  if (widget.layout()) widget.layout()->activate();
  for (auto children = widget.findChildren<QWidget*>(QString{}, Qt::FindDirectChildrenOnly); auto* child : std::as_const(children))
    if (!child->isWindow() && child->isVisibleTo(&widget)) activate(*child);
}

inline std::vector<Geometry> snapshot(QWidget& root) {
  auto widgets = root.findChildren<QWidget*>();
  widgets.prepend(&root);
  std::vector<Geometry> result;
  result.reserve(static_cast<std::size_t>(widgets.size()));
  for (const auto* widget : std::as_const(widgets)) {
    if (widget->window() != root.window()) continue;
    Geometry geometry{widget, widget->geometry(), widget->minimumSize(), widget->isVisibleTo(&root)};
    if (const auto* area = qobject_cast<const QAbstractScrollArea*>(widget)) {
      geometry.horizontalMaximum = area->horizontalScrollBar()->maximum();
      geometry.verticalMaximum = area->verticalScrollBar()->maximum();
    }
    result.push_back(geometry);
  }
  return result;
}
}  // namespace layout_settling

// Process layout events and require two unchanged event-loop rounds of geometry and scrollbar ranges.
// An unchanged first sample may be stale; this barrier does not wait for animations.
inline bool waitForLayoutSettled(QWidget& widget, const int timeoutMs = 2000) {
  QElapsedTimer elapsed;
  elapsed.start();
  std::vector<layout_settling::Geometry> previous;
  int stableRounds = 0;
  do {
    layout_settling::activate(widget);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QEventLoop round;
    QTimer::singleShot(0, &round, &QEventLoop::quit);
    round.exec(QEventLoop::AllEvents);
    QApplication::processEvents(QEventLoop::AllEvents);
    layout_settling::activate(widget);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    auto current = layout_settling::snapshot(widget);
    stableRounds = current == previous ? stableRounds + 1 : 0;
    if (stableRounds >= 2) return true;
    previous = std::move(current);
  } while (elapsed.elapsed() < timeoutMs);
  return false;
}
}  // namespace uwf::test
