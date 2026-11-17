// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/design/theme/DesignSystem.h"

namespace uwf::ui {
QFont DesignSystem::platformFont() {
  QFont font;
  font.setFamilies(
      {QStringLiteral("Segoe UI Variable Text"), QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei UI"), QStringLiteral("Microsoft YaHei")});
  return font;
}
QStringList DesignSystem::monospaceFamilies() {
  // CJK fallbacks remain sans serif when the primary monospace font lacks a glyph.
  return {QStringLiteral("Cascadia Mono"), QStringLiteral("Consolas"), QStringLiteral("Microsoft YaHei UI"), QStringLiteral("Microsoft YaHei")};
}
}  // namespace uwf::ui
