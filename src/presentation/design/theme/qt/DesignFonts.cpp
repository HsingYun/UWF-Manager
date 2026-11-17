// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QFontDatabase>

#include "presentation/design/theme/DesignSystem.h"

namespace uwf::ui {
QFont DesignSystem::platformFont() { return QFontDatabase::systemFont(QFontDatabase::GeneralFont); }
QStringList DesignSystem::monospaceFamilies() { return QFontDatabase::systemFont(QFontDatabase::FixedFont).families(); }
}  // namespace uwf::ui
