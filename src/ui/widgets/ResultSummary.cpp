// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ResultSummary.h"

#include <QHBoxLayout>
#include <array>

#include "IconView.h"
#include "TextLabel.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
namespace uwf::ui {
ResultSummary::ResultSummary(const int succeeded, const int failed, const int skipped, QWidget* parent) : QWidget(parent) {
  auto* row = new QHBoxLayout(this);
  row->setContentsMargins(0, 3, 0, 9);
  row->setSpacing(32);
  struct Metric {
    int count;
    QString caption;
    QString icon;
    design::Color color;
  };
  const std::array metrics{
      Metric{succeeded, I18n::tr("Succeeded"), QStringLiteral(":/icons/status_success.svg"), design::Color::Success},
      Metric{failed, I18n::tr("Failed"), QStringLiteral(":/icons/status_error.svg"), failed ? design::Color::Danger : design::Color::Muted},
      Metric{skipped, I18n::tr("Skipped"), QStringLiteral(":/icons/info.svg"), design::Color::Muted}};
  for (const auto& metric : metrics) {
    auto* group = new QHBoxLayout();
    group->setSpacing(9);
    group->addWidget(new IconView(ThemeManager::instance().icon(metric.icon), 19, metric.color, this));
    auto* number = new TextLabel(QString::number(metric.count), TextStyle::Metric, this);
    auto font = number->font();
    font.setPixelSize(25);
    font.setWeight(QFont::DemiBold);
    number->setFont(font);
    number->setColorRole(metric.color);
    number->setVerticalReference(TextVerticalReference::GlyphBounds);
    group->addWidget(number);
    auto* label = new TextLabel(metric.caption, TextStyle::Caption, this);
    label->setColorRole(metric.color);
    label->setVerticalReference(TextVerticalReference::GlyphBounds);
    group->addWidget(label);
    row->addLayout(group);
  }
  row->addStretch();
}
}  // namespace uwf::ui
