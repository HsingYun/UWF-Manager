// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ItemDelegate.h"

#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <cmath>

#include "presentation/common/ItemRoles.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/BadgePainting.h"
#include "ui/design/DesignSystem.h"
#include "ui/design/IconPainting.h"
#include "ui/design/TextPainting.h"
namespace uwf::ui {
namespace {
QColor itemStatusColor(const presentation::ItemStatus status) {
  const auto& theme = ThemeManager::instance();
  switch (status) {
    case presentation::ItemStatus::Removed:
    case presentation::ItemStatus::Failed:
    case presentation::ItemStatus::Error:
      return theme.color(Sem::Danger);
    case presentation::ItemStatus::Unsupported:
    case presentation::ItemStatus::Warning:
      return theme.color(Sem::Warn);
    case presentation::ItemStatus::Added:
    case presentation::ItemStatus::Success:
      return theme.color(Sem::AddOk);
    case presentation::ItemStatus::Information:
      return theme.color(Sem::Accent);
    case presentation::ItemStatus::Normal:
      return theme.color(Sem::Fg);
    case presentation::ItemStatus::Muted:
      return theme.color(Sem::FgMuted);
    case presentation::ItemStatus::Faint:
      return DesignSystem::instance().color(design::Color::Faint);
  }
  return theme.color(Sem::Fg);
}

QIcon itemStatusIcon(const presentation::ItemStatus status) {
  const QString resource = status == presentation::ItemStatus::Error     ? QStringLiteral(":/icons/status_error.svg")
                           : status == presentation::ItemStatus::Warning ? QStringLiteral(":/icons/status_warning.svg")
                           : status == presentation::ItemStatus::Success ? QStringLiteral(":/icons/status_success.svg")
                                                                         : QStringLiteral(":/icons/info.svg");
  return ThemeManager::iconWithColor(resource, itemStatusColor(status));
}

QIcon composeWithBadge(const QIcon& base, presentation::ItemBadge badge, const qreal dpr) {
  constexpr int kLogical = 16;
  // 使用 2 倍超采样改善 16px 下的 SVG 边缘。
  const qreal renderScale = dpr * 2.0;
  const int kPx = static_cast<int>(std::lround(kLogical * renderScale));

  QPixmap pm(kPx, kPx);
  pm.setDevicePixelRatio(renderScale);
  pm.fill(Qt::transparent);

  QPainter p(&pm);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::SmoothPixmapTransform);
  paintIcon(p, base, QRectF(0, 0, kLogical, kLogical));

  if (badge != presentation::ItemBadge::None) {
    auto& tm = ThemeManager::instance();
    const QColor bg = badge == presentation::ItemBadge::Add ? tm.color(Sem::AddOk) : tm.color(Sem::Danger);
    constexpr qreal kBadge = 7.0;
    constexpr QRectF br(kLogical - kBadge, kLogical - kBadge, kBadge, kBadge);
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawEllipse(br);

    QPen sym(Qt::white, 1.2);
    sym.setCapStyle(Qt::RoundCap);
    p.setPen(sym);
    constexpr qreal cx = br.center().x();
    constexpr qreal cy = br.center().y();
    constexpr qreal half = 1.7;
    p.drawLine(QPointF(cx - half, cy), QPointF(cx + half, cy));
    if (badge == presentation::ItemBadge::Add) {
      p.drawLine(QPointF(cx, cy - half), QPointF(cx, cy + half));
    }
  }
  p.end();

  QIcon result;
  result.addPixmap(pm);
  return result;
}
}  // namespace
ItemDelegate::ItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] { m_badgedIcons.clear(); });
}
QSize ItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto size = QStyledItemDelegate::sizeHint(option, index);
  if (m_rowHeight > 0) size.setHeight(std::max(m_rowHeight, size.height()));
  return size;
}
void ItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if (index.data(presentation::StatusBadgeRole).toBool()) {
    QStyleOptionViewItem item(option);
    initStyleOption(&item, index);
    const auto text = item.text;
    item.text.clear();
    item.icon = {};
    item.features.setFlag(QStyleOptionViewItem::HasDecoration, false);
    const auto* style = item.widget ? item.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &item, painter, item.widget);
    const auto status = static_cast<presentation::ItemStatus>(index.data(presentation::StatusRole).toInt());
    const bool success = status == presentation::ItemStatus::Success;
    const bool error = status == presentation::ItemStatus::Error || status == presentation::ItemStatus::Failed;
    const bool warning = status == presentation::ItemStatus::Warning || status == presentation::ItemStatus::Unsupported;
    const auto foreground = success ? design::Color::Success : error ? design::Color::Danger : warning ? design::Color::Warning : design::Color::Muted;
    const auto background = success   ? design::Color::SuccessBg
                            : error   ? design::Color::DangerBg
                            : warning ? design::Color::WarningBg
                                      : design::Color::SurfaceSoft;
    const auto glyph = success ? ThemeManager::instance().icon(QStringLiteral(":/icons/check.svg"))
                       : error ? ThemeManager::instance().icon(QStringLiteral(":/icons/status_error.svg"))
                               : QIcon{};
    auto font = item.font;
    font.setPixelSize(11);
    font.setWeight(QFont::Normal);
    const auto size = statusBadgeSize(text, font, glyph);
    const QRect badge = QStyle::visualRect(
        item.direction, item.rect, QRect(item.rect.left() + 14, item.rect.top() + (item.rect.height() - size.height()) / 2, size.width(), size.height()));
    painter->save();
    painter->setClipRect(item.rect, Qt::IntersectClip);
    paintStatusBadge(*painter, badge, text, font, glyph, foreground, background, item.direction);
    painter->restore();
    return;
  }
  const auto secondary = index.data(presentation::SecondaryTextRole);
  if (!secondary.isValid()) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }
  QStyleOptionViewItem item(option);
  initStyleOption(&item, index);
  const auto title = item.text;
  item.text.clear();
  const auto* style = item.widget ? item.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &item, painter, item.widget);
  const int leading = 14 + (item.icon.isNull() ? 0 : item.decorationSize.width() + 7);
  const QRect text = QStyle::visualRect(item.direction, item.rect, item.rect.adjusted(leading, 0, -14, 0));
  painter->save();
  painter->setClipRect(text, Qt::IntersectClip);
  const auto& colors = DesignSystem::instance();
  painter->setPen(colors.color(design::Color::Text));
  auto font = item.font;
  font.setPixelSize(13);
  const qreal center = QRectF(item.rect).center().y();
  paintTextLine(*painter, QRectF(text.left(), center - 20, text.width(), 21), QFontMetricsF(font).elidedText(title, Qt::ElideRight, text.width()), font);
  font.setPixelSize(11);
  font.setFamilies(DesignSystem::monospaceFamilies());
  painter->setPen(colors.color(design::Color::Muted));
  paintTextLine(*painter, QRectF(text.left(), center + 3, text.width(), 17),
                QFontMetricsF(font).elidedText(secondary.toString(), Qt::ElideMiddle, text.width()), font);
  painter->restore();
}
void ItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
  QStyledItemDelegate::initStyleOption(option, index);
  const auto status = index.data(presentation::StatusRole);
  if (status.isValid()) {
    const auto ink = itemStatusColor(static_cast<presentation::ItemStatus>(status.toInt()));
    option->palette.setBrush(QPalette::Text, ink);
    option->palette.setBrush(QPalette::HighlightedText, ink);
  }
  if (index.data(presentation::MonospaceRole).toBool()) {
    option->font.setFamilies(DesignSystem::monospaceFamilies());
    option->font.setPixelSize(12);
    option->font.setLetterSpacing(QFont::AbsoluteSpacing, -.15);
    option->fontMetrics = QFontMetrics(option->font);
  }
  if (const auto rowStatus = index.data(presentation::RowStatusRole);
      rowStatus.isValid() && rowStatus.toInt() == static_cast<int>(presentation::ItemStatus::Error)) {
    const auto& colors = DesignSystem::instance();
    option->backgroundBrush = DesignSystem::mix(colors.color(design::Color::Surface), colors.color(design::Color::DangerBg), .42);
  }
  const auto resource = index.data(presentation::IconResourceRole).toString();
  if (!resource.isEmpty()) {
    option->icon = ThemeManager::instance().icon(resource);
    const auto badge = index.data(presentation::BadgeRole);
    if (badge.isValid()) {
      const auto dpr = option->widget ? option->widget->devicePixelRatioF() : 1.0;
      const QString key = resource + QLatin1Char('|') + QString::number(badge.toInt()) + QLatin1Char('|') + QString::number(dpr);
      auto found = m_badgedIcons.constFind(key);
      if (found == m_badgedIcons.cend()) {
        m_badgedIcons.insert(key, composeWithBadge(option->icon, static_cast<presentation::ItemBadge>(badge.toInt()), dpr));
        found = m_badgedIcons.constFind(key);
      }
      option->icon = *found;
    }
  } else if (index.data(presentation::StatusIconRole).toBool() && status.isValid()) {
    option->icon = itemStatusIcon(static_cast<presentation::ItemStatus>(status.toInt()));
  }
  if (!option->icon.isNull()) {
    option->features |= QStyleOptionViewItem::HasDecoration;
    option->decorationSize = option->icon.actualSize(option->decorationSize);
  }
  if (index.data(presentation::SeparatorRole).toBool()) {
    option->font.setWeight(QFont::Medium);
    option->fontMetrics = QFontMetrics(option->font);
  }
}
}  // namespace uwf::ui
