// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QColor>
#include <QFont>
#include <QObject>
#include <QPalette>
#include <QString>
#include <QStringList>

#include "design/DesignTokens.h"

namespace uwf::ui {
class DesignSystem final : public QObject {
  Q_OBJECT
 public:
  static DesignSystem& instance();
  [[nodiscard]] static QFont defaultFont();
  [[nodiscard]] static QStringList monospaceFamilies();
  [[nodiscard]] bool isLight() const { return m_light; }
  void setLight(bool light);
  [[nodiscard]] QColor color(design::Color role) const;
  [[nodiscard]] QPalette palette() const;
  [[nodiscard]] QString resolveStyle(QString text) const;
  [[nodiscard]] static QColor mix(const QColor& from, const QColor& to, qreal amount);
 signals:
  void changed();

 private:
  bool m_light = false;
};
}  // namespace uwf::ui
