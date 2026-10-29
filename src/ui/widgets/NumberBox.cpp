// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "NumberBox.h"

#include <algorithm>
#include <QEvent>
#include <QLineEdit>
#include <QResizeEvent>
#include "TextLabel.h"

#include "design/DesignTokens.h"
namespace uwf::ui {
NumberBox::NumberBox(QWidget* parent) : QSpinBox(parent) {
  setButtonSymbols(QAbstractSpinBox::NoButtons);
  // Numeric values and Latin units form one LTR field even in an RTL form.
  setLayoutDirection(Qt::LeftToRight);
  setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  setProperty("designUnitChild", true);
  m_unit = new TextLabel({}, TextStyle::Eyebrow, this);
  m_unit->setObjectName(QStringLiteral("numberUnit"));
  m_unit->setColorRole(design::Color::Muted);
  m_unit->setLayoutDirection(Qt::LeftToRight);
  m_unit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute | Qt::AlignVCenter);
  m_unit->setAttribute(Qt::WA_TransparentForMouseEvents);
  placeFields();
}
void NumberBox::setUnit(const QString& text) {
  setProperty("unitText", text);
  setSuffix({});
  m_unit->setText(text);
  m_unit->setVisible(!text.isEmpty());
  placeFields();
  updateGeometry();
  update();
}
void NumberBox::resizeEvent(QResizeEvent* event) { QSpinBox::resizeEvent(event); placeFields(); }
void NumberBox::changeEvent(QEvent* event) {
  QSpinBox::changeEvent(event);
  if (event->type() == QEvent::LayoutDirectionChange || event->type() == QEvent::LocaleChange || event->type() == QEvent::StyleChange ||
      event->type() == QEvent::FontChange) placeFields();
}
void NumberBox::placeFields() {
  if (!m_unit) return;
  if (layoutDirection() != Qt::LeftToRight) setLayoutDirection(Qt::LeftToRight);
  const int unitWidth = m_unit->sizeHint().width();
  m_unit->setGeometry(std::max(0, width() - 8 - unitWidth), 1, unitWidth, std::max(0, height() - 2));
  auto* edit = lineEdit();
  edit->setLayoutDirection(Qt::LeftToRight);
  edit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute | Qt::AlignVCenter);
  edit->setGeometry(10, 1, std::max(0, (m_unit->text().isEmpty() ? width() - 8 : m_unit->x() - 8) - 10), std::max(0, height() - 2));
  m_unit->raise();
}
QSize NumberBox::sizeHint() const {
  const int valueWidth = std::max(fontMetrics().horizontalAdvance(prefix() + textFromValue(minimum()) + suffix()),
                                  fontMetrics().horizontalAdvance(prefix() + textFromValue(maximum()) + suffix()));
  const auto unit = property("unitText").toString();
  auto unitFont = font();
  unitFont.setPixelSize(11);
  const int extra = unit.isEmpty() ? 0 : (m_unit ? m_unit->sizeHint().width() : QFontMetrics(unitFont).horizontalAdvance(unit)) + 10;
  return {valueWidth + design::fieldPaddingX * 2 + extra + (buttonSymbols() == NoButtons ? 0 : 20), design::fieldHeight};
}
QSize NumberBox::minimumSizeHint() const {
  auto result = sizeHint();
  result.setWidth(result.width() -
                  std::max(0, fontMetrics().horizontalAdvance(textFromValue(maximum())) - fontMetrics().horizontalAdvance(QStringLiteral("0000"))));
  result.setHeight(fontMetrics().height() + 2);
  return result;
}
}  // namespace uwf::ui
#include "moc_NumberBox.cpp"
