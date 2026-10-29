// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TabView.h"

#include <QApplication>
#include <QEvent>
#include <QLayout>
#include <QPainter>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <QTabBar>
#include <algorithm>

#include "ui/design/DesignSystem.h"
namespace uwf::ui {
namespace {
void preservePageScroll(QWidget* page, const std::shared_ptr<bool>& enabled) {
  if (!page) return;
  struct Anchor {
    int value;
    bool adjusting = false;
  };
  for (const auto bars = page->findChildren<QScrollBar*>(); auto* bar : bars) {
    if (bar->property("designPageScrollAnchor").toBool()) continue;
    bar->setProperty("designPageScrollAnchor", true);
    const auto anchor = std::make_shared<Anchor>(Anchor{bar->value()});
    // A temporary larger viewport clamps QScrollBar::value. Keep the logical
    // position until the final range is known; explicit navigation replaces it.
    // Both callbacks belong to the scrollbar and capture no parent widget.
    QObject::connect(bar, &QScrollBar::rangeChanged, bar, [bar, anchor, enabled](int, int) {
      if (!*enabled) return;
      const QScopedValueRollback adjusting(anchor->adjusting, true);
      bar->setValue(std::clamp(anchor->value, bar->minimum(), bar->maximum()));
    });
    QObject::connect(bar, &QScrollBar::valueChanged, bar, [anchor](const int value) {
      if (!anchor->adjusting) anchor->value = value;
    });
    QObject::connect(bar, &QScrollBar::actionTriggered, bar, [bar, anchor](int) { anchor->value = bar->sliderPosition(); });
  }
}
}  // namespace
TabView::TabView(QWidget* parent) : QTabWidget(parent) {
  setDocumentMode(true);
  setElideMode(Qt::ElideNone);
  setUsesScrollButtons(true);
  tabBar()->setExpanding(false);
  setKind(Kind::Pages);
  connect(this, &QTabWidget::currentChanged, this, [this] {
    if (!m_measureCurrentPage) return;
    preservePageScroll(currentWidget(), m_preserveScroll);
    updateGeometry();
  });
}
void TabView::tabInserted(const int index) {
  QTabWidget::tabInserted(index);
  if (m_measureCurrentPage) preservePageScroll(widget(index), m_preserveScroll);
}
void TabView::setMeasureCurrentPage(const bool enabled) {
  if (m_measureCurrentPage == enabled) return;
  m_measureCurrentPage = enabled;
  *m_preserveScroll = enabled;
  if (enabled)
    for (int index = 0; index < count(); ++index) preservePageScroll(widget(index), m_preserveScroll);
  updateGeometry();
}
QSize TabView::minimumSizeHint() const {
  const auto* page = currentWidget();
  if (!m_measureCurrentPage || !page || tabPosition() != North) return QTabWidget::minimumSizeHint();
  const auto minimum = page->minimumSizeHint().expandedTo(page->minimumSize());
  const auto strip = tabBar()->minimumSizeHint();
  return {std::max(minimum.width(), strip.width()), minimum.height() + tabBar()->sizeHint().height()};
}
bool TabView::hasHeightForWidth() const { return (m_measureCurrentPage && currentWidget() && tabPosition() == North) || QTabWidget::hasHeightForWidth(); }
int TabView::heightForWidth(const int width) const {
  const auto* page = currentWidget();
  if (!m_measureCurrentPage || !page || tabPosition() != North) return QTabWidget::heightForWidth(width);
  const auto* layout = page->layout();
  const int height = layout && layout->hasHeightForWidth() ? layout->totalMinimumHeightForWidth(width) : page->minimumSizeHint().height();
  return std::max(height, page->minimumHeight()) + tabBar()->sizeHint().height();
}
void TabView::setKind(const Kind kind) {
  tabBar()->setProperty("designTabKind", kind == Kind::Volumes ? "volumes" : kind == Kind::Entries ? "entries" : "pages");
  QEvent styleChange(QEvent::StyleChange);
  QApplication::sendEvent(tabBar(), &styleChange);
}
void TabView::setTabCount(const int index, const int count) {
  auto counts = tabBar()->property("designTabCounts").toMap();
  if (count < 0)
    counts.remove(QString::number(index));
  else
    counts[QString::number(index)] = count;
  tabBar()->setProperty("designTabCounts", counts);
  QEvent styleChange(QEvent::StyleChange);
  QApplication::sendEvent(tabBar(), &styleChange);
}
void TabView::setCompact(const bool compact) {
  if (tabBar()->property("designCompact").toBool() == compact) return;
  tabBar()->setProperty("designCompact", compact);
  auto type = tabBar()->font();
  type.setPixelSize(compact ? 12 : 13);
  tabBar()->setFont(type);
  tabBar()->updateGeometry();
}
void TabView::setTabIndicator(const int index, const QString& iconResource, const QString& hint) {
  auto indicators = tabBar()->property("designTabIndicators").toMap();
  if (iconResource.isEmpty())
    indicators.remove(QString::number(index));
  else
    indicators.insert(QString::number(index), iconResource);
  tabBar()->setProperty("designTabIndicators", indicators);
  setTabToolTip(index, hint);
  QEvent styleChange(QEvent::StyleChange);
  QApplication::sendEvent(tabBar(), &styleChange);
  tabBar()->updateGeometry();
  tabBar()->update();
}
void TabView::paintEvent(QPaintEvent* event) {
  QTabWidget::paintEvent(event);
  QPainter painter(this);
  painter.setPen(DesignSystem::instance().color(design::Color::Line));
  const int bottom = tabBar()->geometry().bottom();
  painter.drawLine(0, bottom, width() - 1, bottom);
}
}  // namespace uwf::ui
