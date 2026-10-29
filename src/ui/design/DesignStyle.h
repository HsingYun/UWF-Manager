// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QProxyStyle>
namespace uwf::ui {
class DesignStyle final : public QProxyStyle {
 public:
  DesignStyle();
  using QProxyStyle::polish;
  void polish(QWidget* widget) override;
  void polish(QPalette& palette) override;
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
  void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
  void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget = nullptr) const override;
  [[nodiscard]] QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;
  [[nodiscard]] int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
  [[nodiscard]] int styleHint(StyleHint hint, const QStyleOption* option = nullptr, const QWidget* widget = nullptr,
                              QStyleHintReturn* result = nullptr) const override;
  [[nodiscard]] QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl sub, const QWidget* widget = nullptr) const override;
  [[nodiscard]] QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget = nullptr) const override;

 private:
  void paintButton(const QStyleOption* option, QPainter* painter, const QWidget* widget, const QString& text, const QIcon& icon, bool tool) const;
  void paintField(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
}  // namespace uwf::ui
