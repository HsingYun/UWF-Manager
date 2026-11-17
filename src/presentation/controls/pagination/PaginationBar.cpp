// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/pagination/PaginationBar.h"

#include <QCoreApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <array>

#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/painting/IconPainting.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/models/PageProxyModel.h"

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
  m_summary->setVerticalReference(TextVerticalReference::FontMetrics);
  row->addWidget(m_summary, 1);
  auto* first = new QPushButton(this);
  auto* previous = new QPushButton(this);
  auto* next = new QPushButton(this);
  auto* last = new QPushButton(this);
  m_buttons = {first, previous, next, last};
  auto* position = new TextLabel({}, textStyle, this);
  position->setAlignment(Qt::AlignCenter);
  position->setVerticalReference(TextVerticalReference::FontMetrics);
  position->setMinimumWidth(68);
  position->setContentsMargins(10, 0, 10, 0);
  position->setProperty("designTone", "muted");
  for (std::size_t index = 0; index < m_buttons.size(); ++index) {
    auto* button = m_buttons[index];
    button->setProperty("designRole", "quiet");
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
  retranslate();
  refreshIcons();
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, &PaginationBar::refreshIcons);
}
void PaginationBar::changeEvent(QEvent* event) {
  QWidget::changeEvent(event);
  if (event->type() == QEvent::LanguageChange && m_buttons.front()) retranslate();
}
void PaginationBar::retranslate() {
  const std::array names{QCoreApplication::translate("uwf", "First page"), QCoreApplication::translate("uwf", "Previous page"),
                         QCoreApplication::translate("uwf", "Next page"), QCoreApplication::translate("uwf", "Last page")};
  for (std::size_t index = 0; index < m_buttons.size(); ++index) {
    m_buttons[index]->setProperty("accessibleName", names[index]);
    m_buttons[index]->setToolTip(names[index]);
  }
}
void PaginationBar::refreshIcons() {
  const QStringList resources{QStringLiteral(":/icons/page_first.svg"), QStringLiteral(":/icons/page_previous.svg"), QStringLiteral(":/icons/page_next.svg"),
                              QStringLiteral(":/icons/page_last.svg")};
  for (std::size_t index = 0; index < m_buttons.size(); ++index)
    m_buttons[index]->setIcon(coloredSvgIcon(resources[static_cast<qsizetype>(index)], DesignSystem::instance().color(design::Color::Text)));
}
void PaginationBar::setSummary(const QString& text) {
  m_summary->setText(text.simplified());
  m_summary->setToolTip(text);
}
}  // namespace uwf::ui
