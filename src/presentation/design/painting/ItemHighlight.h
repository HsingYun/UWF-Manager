// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QHash>
#include <QObject>
#include <QPoint>

class QVariantAnimation;
class QWidget;

namespace uwf::ui {
class ItemHighlight final : public QObject {
  Q_OBJECT
 public:
  [[nodiscard]] static qreal opacity(const QWidget* viewport, const QPoint& itemPosition, bool active);

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  explicit ItemHighlight(QWidget* viewport);
  struct Transition {
    QVariantAnimation* animation;
    bool active = false;
  };
  QHash<QPoint, Transition> m_items;
};
}  // namespace uwf::ui
