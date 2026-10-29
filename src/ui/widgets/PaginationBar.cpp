// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PaginationBar.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <array>

#include "presentation/common/PageProxyModel.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/widgets/TextLabel.h"
namespace uwf::ui {
PaginationBar::PaginationBar(presentation::PageProxyModel& pages, QWidget* parent, const TextStyle textStyle)
    : QWidget(parent), m_summary(new TextLabel({}, textStyle, this)) {
  setMinimumHeight(28);
  auto* row = new QHBoxLayout(this);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(6);
  m_summary->setProperty("designTone", "muted");
  m_summary->setObjectName(QStringLiteral("paginationSummary"));
  m_summary->setElideMode(Qt::ElideRight);
  row->addWidget(m_summary, 1);
  auto* first = new QPushButton(this);
  auto* previous = new QPushButton(this);
  auto* next = new QPushButton(this);
  auto* last = new QPushButton(this);
  const std::array buttons{first, previous, next, last};
  const std::array names{I18n::tr("First page"), I18n::tr("Previous page"), I18n::tr("Next page"), I18n::tr("Last page")};
  const QStringList resources{QStringLiteral(":/icons/page_first.svg"), QStringLiteral(":/icons/page_previous.svg"), QStringLiteral(":/icons/page_next.svg"),
                              QStringLiteral(":/icons/page_last.svg")};
  auto* position = new TextLabel({}, textStyle, this);
  position->setAlignment(Qt::AlignCenter);
  position->setMinimumWidth(68);
  position->setContentsMargins(10, 0, 10, 0);
  position->setProperty("designTone", "muted");
  for (std::size_t index = 0; index < buttons.size(); ++index) {
    auto* button = buttons[index];
    button->setProperty("designRole", "quiet");
    button->setProperty("accessibleName", names[index]);
    button->setToolTip(names[index]);
    button->setIcon(ThemeManager::instance().icon(resources[static_cast<qsizetype>(index)]));
    button->setFixedSize(28, 28);
    row->addWidget(button);
    if (index == 1) row->addWidget(position);
  }
  connect(first, &QPushButton::clicked, &pages, &presentation::PageProxyModel::goFirst);
  connect(previous, &QPushButton::clicked, &pages, &presentation::PageProxyModel::goPrevious);
  connect(next, &QPushButton::clicked, &pages, &presentation::PageProxyModel::goNext);
  connect(last, &QPushButton::clicked, &pages, &presentation::PageProxyModel::goLast);
  const auto update = [&pages, position, first, previous, next, last] {
    const auto& pager = pages.pager();
    const int count = pager.pageCount(pages.totalRows());
    position->setText(QStringLiteral("%1 / %2").arg(count ? pager.currentPage + 1 : 0).arg(count));
    first->setEnabled(pager.hasPrev());
    previous->setEnabled(pager.hasPrev());
    next->setEnabled(pager.hasNext(pages.totalRows()));
    last->setEnabled(pager.hasNext(pages.totalRows()));
  };
  connect(&pages, &presentation::PageProxyModel::pageChanged, this, update);
  update();
  I18n::bind(this, [buttons] {
    const std::array translated{I18n::tr("First page"), I18n::tr("Previous page"), I18n::tr("Next page"), I18n::tr("Last page")};
    for (std::size_t index = 0; index < buttons.size(); ++index) {
      buttons[index]->setProperty("accessibleName", translated[index]);
      buttons[index]->setToolTip(translated[index]);
    }
  });
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [buttons, resources] {
    for (std::size_t index = 0; index < buttons.size(); ++index)
      buttons[index]->setIcon(ThemeManager::instance().icon(resources[static_cast<qsizetype>(index)]));
  });
}
void PaginationBar::setSummary(const QString& text) {
  m_summary->setText(text.simplified());
  m_summary->setToolTip(text);
}
}  // namespace uwf::ui
