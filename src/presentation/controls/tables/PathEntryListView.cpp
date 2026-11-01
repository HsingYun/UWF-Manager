// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/tables/PathEntryListView.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStyledItemDelegate>

#include "presentation/controls/layout/RoundedCornerOverlay.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/tables/PagedViewController.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/painting/IconPainting.h"
#include "presentation/design/painting/TextPainting.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/models/PageProxyModel.h"
namespace uwf::ui {
namespace {
constexpr int statusWidth = 120;
class PathEntryDelegate final : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;
  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override { return {0, design::tableRowHeight}; }
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    using design::Color;
    using presentation::ItemStatus;
    const auto& system = DesignSystem::instance();
    const auto status = static_cast<ItemStatus>(index.data(presentation::StatusRole).toInt());
    const bool selected = option.state.testFlag(QStyle::State_Selected);
    const bool removed = status == ItemStatus::Removed;
    const bool added = status == ItemStatus::Added;
    const QRect row = option.rect;
    painter->save();
    painter->setClipRect(row);
    QColor fill = system.color(Color::Surface);
    if (removed) fill = DesignSystem::mix(fill, system.color(Color::DangerBg), .55);
    if (option.state.testFlag(QStyle::State_MouseOver)) fill = system.color(Color::Hover);
    if (selected) fill = system.color(Color::Selected);
    painter->fillRect(row, fill);
    const auto* view = qobject_cast<const QAbstractItemView*>(option.widget);
    if (!view || row.bottom() < view->viewport()->rect().bottom()) {
      painter->setPen(system.color(Color::LineSoft));
      painter->drawLine(row.bottomLeft(), row.bottomRight());
    }
    if (selected) painter->fillRect(QStyle::visualRect(option.direction, row, QRect(row.left(), row.top(), 3, row.height())), system.color(Color::Accent));
    QRectF glyph(row.left() + 14, QRectF(row).center().y() - 16 / 2.0, 16, 16);
    if (option.direction == Qt::RightToLeft) glyph.moveLeft(row.left() + row.right() + 1 - glyph.right());
    const auto resource = index.data(presentation::IconResourceRole).toString();
    paintIcon(*painter, QIcon(resource), glyph, system.color(removed ? Color::Danger : Color::Muted));
    auto type = option.font;
    type.setFamilies(DesignSystem::monospaceFamilies());
    type.setPixelSize(13);
    type.setLetterSpacing(QFont::AbsoluteSpacing, -.15);
    type.setStrikeOut(removed);
    painter->setFont(type);
    painter->setPen(system.color(removed ? Color::Danger : Color::Text));
    const QRect text =
        QStyle::visualRect(option.direction, row, QRect(row.left() + 39, row.top(), std::max(0, row.width() - 39 - statusWidth - 12), row.height()));
    painter->setLayoutDirection(Qt::LeftToRight);
    paintTextLine(*painter, text, QFontMetrics(type).elidedText(index.data().toString(), Qt::ElideMiddle, text.width()), type,
                  Qt::AlignVCenter | (option.direction == Qt::RightToLeft ? Qt::AlignRight : Qt::AlignLeft));
    const bool required = index.data(presentation::RequiredRole).toBool();
    const QString caption = index.data(presentation::StatusTextRole).toString();
    const auto ink = removed ? Color::Danger : added ? Color::Success : required ? Color::Accent : Color::Muted;
    const auto background = removed ? Color::DangerBg : added ? Color::SuccessBg : required ? Color::Selected : Color::SurfaceSoft;
    type = option.font;
    type.setPixelSize(12);
    type.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    type.setStrikeOut(false);
    painter->setFont(type);
    const int width = std::min(statusWidth - 10, QFontMetrics(type).horizontalAdvance(caption) + 16);
    QRectF badge(row.right() - statusWidth + (statusWidth - width) / 2.0, QRectF(row).center().y() - 11.5, width, 23);
    if (option.direction == Qt::RightToLeft) badge.moveLeft(row.left() + row.right() + 1 - badge.right());
    painter->setRenderHint(QPainter::Antialiasing);
    if (removed || added || required) {
      painter->setBrush(system.color(background));
      painter->setPen(system.color(Color::Line));
      painter->drawRoundedRect(badge, 4, 4);
    }
    painter->setPen(system.color(ink));
    painter->setLayoutDirection(option.direction);
    paintTextLine(*painter, badge, QFontMetricsF(type).elidedText(caption, Qt::ElideRight, std::max(qreal{0}, badge.width() - 4)), type, Qt::AlignCenter);
    painter->restore();
  }
};
}  // namespace
PathEntryListView::PathEntryListView(const QString& heading, const QString& statusHeading, QWidget* parent)
    : SelectionListView(parent),
      m_header(new Surface(design::Color::SurfaceSoft, this)),
      m_filter(new QSortFilterProxyModel(this)),
      m_pages(new presentation::PageProxyModel(this)) {
  m_filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_pages->setSourceModel(m_filter);
  m_scrolling = new PagedViewController(this, *m_pages, design::tableRowHeight);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setFrameShape(QFrame::NoFrame);
  setViewportMargins(0, design::tableHeaderHeight, 0, 0);
  setUniformItemSizes(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setItemDelegate(new PathEntryDelegate(this));
  auto* row = new QHBoxLayout(m_header);
  row->setContentsMargins(layoutDirection() == Qt::RightToLeft ? 0 : 14, 0, layoutDirection() == Qt::RightToLeft ? 14 : 0, 0);
  row->setSpacing(0);
  m_heading = new TextLabel(heading, TextStyle::Caption, m_header);
  m_heading->setObjectName(QStringLiteral("entryHeading"));
  m_heading->setProperty("designTone", "muted");
  row->addWidget(m_heading, 1);
  m_statusHeading = new TextLabel(statusHeading, TextStyle::Caption, m_header);
  m_statusHeading->setObjectName(QStringLiteral("entryStatusHeading"));
  m_statusHeading->setProperty("designTone", "muted");
  m_statusHeading->setAlignment(Qt::AlignCenter);
  m_statusHeading->setFixedWidth(statusWidth);
  row->addWidget(m_statusHeading);
  static_cast<Surface*>(m_header)->setBorder(Qt::BottomEdge);
  m_header->show();
  m_cornerOverlay = new RoundedCornerOverlay(
      this, 0, 6, [] { return DesignSystem::instance().color(design::Color::Window); }, [] { return DesignSystem::instance().color(design::Color::Line); }, 1);
  connect(verticalScrollBar(), &QAbstractSlider::valueChanged, m_cornerOverlay, qOverload<>(&QWidget::update));
  connect(&DesignSystem::instance(), &DesignSystem::changed, m_cornerOverlay, qOverload<>(&QWidget::update));
}
void PathEntryListView::setHeading(const QString& text) { m_heading->setText(text); }
void PathEntryListView::setStatusHeading(const QString& text) { m_statusHeading->setText(text); }
void PathEntryListView::changeEvent(QEvent* event) {
  SelectionListView::changeEvent(event);
  if (event->type() == QEvent::LayoutDirectionChange) {
    m_header->layout()->setContentsMargins(layoutDirection() == Qt::RightToLeft ? 0 : 14, 0, layoutDirection() == Qt::RightToLeft ? 14 : 0, 0);
    viewport()->update();
  }
}
void PathEntryListView::resizeEvent(QResizeEvent* event) {
  SelectionListView::resizeEvent(event);
  m_header->setGeometry(0, 0, viewport()->width(), design::tableHeaderHeight);
  m_scrolling->updateGeometry();
}
void PathEntryListView::setModel(QAbstractItemModel* model) {
  m_filter->setSourceModel(model);
  SelectionListView::setModel(m_pages);
}
void PathEntryListView::setFilterText(const QString& text) {
  m_filter->setFilterFixedString(text);
  m_pages->goFirst();
}
}  // namespace uwf::ui
