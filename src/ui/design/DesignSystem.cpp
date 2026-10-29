// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DesignSystem.h"

#include <algorithm>

namespace uwf::ui {
QStringList DesignSystem::monospaceFamilies() {
  // Keep every path, report and editor on the H5 --mono fallback chain. In
  // particular, CJK fallback must remain sans serif when the Latin face has
  // no glyph for the input, including the first IME composition.
  return {QStringLiteral("Cascadia Mono"), QStringLiteral("Consolas"), QStringLiteral("Microsoft YaHei UI"), QStringLiteral("Microsoft YaHei")};
}
QFont DesignSystem::defaultFont() {
  QFont font;
  font.setFamilies(
      {QStringLiteral("Segoe UI Variable Text"), QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei UI"), QStringLiteral("Microsoft YaHei")});
  font.setPixelSize(design::fontSize);
  font.setHintingPreference(QFont::PreferNoHinting);
  font.setStyleStrategy(QFont::PreferAntialias);
  return font;
}
DesignSystem& DesignSystem::instance() {
  static DesignSystem system;
  return system;
}
void DesignSystem::setLight(const bool light) {
  if (m_light == light) return;
  m_light = light;
  emit changed();
}
QColor DesignSystem::color(const design::Color role) const {
  const auto index = static_cast<std::size_t>(role);
  return QColor::fromRgba((m_light ? design::lightColors : design::darkColors).at(index));
}
QColor DesignSystem::mix(const QColor& from, const QColor& to, qreal amount) {
  amount = std::clamp(amount, 0.0, 1.0);
  const qreal alpha = from.alphaF() * (1 - amount) + to.alphaF() * amount;
  const auto blend = [&](float first, float second) {
    return alpha > 0 ? static_cast<float>((first * from.alphaF() * (1 - amount) + second * to.alphaF() * amount) / alpha) : 0.0F;
  };
  return QColor::fromRgbF(blend(from.redF(), to.redF()), blend(from.greenF(), to.greenF()), blend(from.blueF(), to.blueF()), static_cast<float>(alpha));
}
QPalette DesignSystem::palette() const {
  using enum design::Color;
  QPalette result;
  result.setColor(QPalette::Window, color(Window));
  result.setColor(QPalette::WindowText, color(Text));
  result.setColor(QPalette::Base, color(Surface));
  result.setColor(QPalette::AlternateBase, color(Surface));
  result.setColor(QPalette::Text, color(Text));
  result.setColor(QPalette::Button, color(Surface));
  result.setColor(QPalette::ButtonText, color(Text));
  result.setColor(QPalette::Highlight, color(Selected));
  result.setColor(QPalette::HighlightedText, color(Text));
  result.setColor(QPalette::ToolTipBase, color(Surface));
  result.setColor(QPalette::ToolTipText, color(Text));
  result.setColor(QPalette::PlaceholderText, color(Faint));
  result.setColor(QPalette::Link, color(Accent));
  result.setColor(QPalette::LinkVisited, color(Accent));
  result.setColor(QPalette::Light, color(Surface));
  result.setColor(QPalette::Midlight, color(SurfaceSoft));
  result.setColor(QPalette::Mid, color(Line));
  result.setColor(QPalette::Dark, color(Line));
  result.setColor(QPalette::Shadow, color(Line));
  result.setColor(QPalette::BrightText, color(AccentText));
  for (const auto role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText})
    result.setColor(QPalette::Disabled, role, mix(color(Text), color(Surface), .58));
  return result;
}
QString DesignSystem::resolveStyle(QString text) const {
  for (std::size_t index = 0; index < design::colorNames.size(); ++index)
    text.replace(u'@' + QString::fromLatin1(design::colorNames[index]) + u'@', color(static_cast<design::Color>(index)).name());
  return text;
}
}  // namespace uwf::ui
#include "moc_DesignSystem.cpp"
