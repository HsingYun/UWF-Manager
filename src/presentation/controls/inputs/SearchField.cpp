// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/inputs/SearchField.h"

#include <QAction>

#include "presentation/design/theme/DesignSystem.h"
#include "presentation/design/painting/IconPainting.h"

namespace uwf::ui {
SearchField::SearchField(QWidget* parent) : QLineEdit(parent) {
  auto type = font();
  type.setPixelSize(12);
  type.setWeight(QFont::Normal);
  setFont(type);
  setClearButtonEnabled(true);
  auto* search = addAction(coloredSvgIcon(QStringLiteral(":/icons/search.svg"), DesignSystem::instance().color(design::Color::Muted)), LeadingPosition);
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, [search] {
    search->setIcon(coloredSvgIcon(QStringLiteral(":/icons/search.svg"), DesignSystem::instance().color(design::Color::Muted)));
  });
}
}  // namespace uwf::ui
#include "moc_SearchField.cpp"
