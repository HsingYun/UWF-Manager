// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPointer>
#include <QWidget>

namespace uwf::ui {
class TextLabel;
enum class TextAnchor { FirstLine, VisibleBlock };

// Preserves the fractional remainder that QWidget geometry cannot represent.
void setGraphicVerticalAlignmentOffset(QWidget& graphic, qreal offset);

// Owns a graphic and follows the first line's or complete block's text reference.
class AlignedGraphic final : public QWidget {
 public:
  AlignedGraphic(QWidget* graphic, TextLabel* reference, QWidget* parent = nullptr);
  AlignedGraphic(QWidget* graphic, QWidget* reference, TextAnchor anchor, QWidget* parent = nullptr);
  void setReference(TextLabel* reference);
  [[nodiscard]] TextAnchor textAnchor() const { return m_anchor; }
  [[nodiscard]] QWidget* reference() const { return m_reference.data(); }

 protected:
  bool event(QEvent* event) override;
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  void syncGraphicSize();
  void alignGraphic();
  void setReferenceWidget(QWidget* reference, TextAnchor anchor);
  [[nodiscard]] QRectF referenceBounds() const;
  QWidget* m_graphic;
  QPointer<QWidget> m_reference;
  TextAnchor m_anchor = TextAnchor::FirstLine;
};
}  // namespace uwf::ui
