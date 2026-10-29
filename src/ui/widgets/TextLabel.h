// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QLabel>
#include <optional>

#include "design/DesignTokens.h"
#include "ui/design/TextPainting.h"
namespace uwf::ui {
enum class TextStyle { Body, Label, Caption, Eyebrow, Subtitle, Title, Section, Description, Metric, MessageTitle };
class TextLabel : public QLabel {
  Q_OBJECT
 public:
  explicit TextLabel(const QString& text = {}, TextStyle style = TextStyle::Body, QWidget* parent = nullptr);
  void setTextStyle(TextStyle style);
  void setElideMode(Qt::TextElideMode mode);
  [[nodiscard]] Qt::TextElideMode elideMode() const { return m_elide; }
  void setColorRole(design::Color role);
  void setLineHeight(qreal height);
  void setVerticalReference(TextVerticalReference reference);
  [[nodiscard]] QRectF firstLineBoxBounds() const;
  [[nodiscard]] QSize sizeHint() const override;
  [[nodiscard]] QSize minimumSizeHint() const override;
  [[nodiscard]] bool hasHeightForWidth() const override;
  [[nodiscard]] int heightForWidth(int width) const override;
 public slots:
  void setText(const QString& text);

 protected:
  bool event(QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  [[nodiscard]] QSize measure(int width) const;
  std::optional<design::Color> m_colorRole;
  std::optional<design::Color> m_toneRole;
  Qt::TextElideMode m_elide = Qt::ElideNone;
  qreal m_lineHeight = 1.6;
  TextVerticalReference m_verticalReference = TextVerticalReference::FontMetrics;
};
}  // namespace uwf::ui
