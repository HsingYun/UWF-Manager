// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DesignStyle.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QCursor>
#include <QFocusFrame>
#include <QFontMetricsF>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTabBar>
#include <QTabWidget>
#include <QTableView>
#include <QTextEdit>
#include <QToolButton>
#include <algorithm>

#include "BadgePainting.h"
#include "ContextMenus.h"
#include "ControlAnimation.h"
#include "ControlShape.h"
#include "DesignSystem.h"
#include "FocusIndicator.h"
#include "IconPainting.h"
#include "ItemHighlight.h"
#include "PopupAppearance.h"
#include "TextPainting.h"
#include "ui/widgets/SwitchButton.h"

namespace uwf::ui {
namespace {
using design::Color;
QColor color(const Color role) { return DesignSystem::instance().color(role); }
QString menuDisplayLabel(QString label) {
  static const QRegularExpression suffix(QStringLiteral(R"(\s*[（(]&[A-Za-z0-9][）)])"));
  return label.remove(suffix);
}
bool thinScrollBar(const QWidget* widget) {
  return widget && (widget->property("designThinScrollBar").toBool() || widget->objectName() == QLatin1String("entryScrollOverlay"));
}

bool enabled(const QStyleOption* option) { return option->state.testFlag(QStyle::State_Enabled); }
void rounded(QPainter* painter, const QRectF& rect, const QColor& fill, const QColor& border, const qreal radius) {
  paintControlShape(*painter, rect, radius, fill, border);
}
void icon(QPainter* painter, const QIcon& value, const QRectF& rect, const QColor& ink) { paintIcon(*painter, value, rect, ink); }
void arrow(QPainter* painter, const QRectF& rect, const QColor& ink, const Qt::ArrowType direction) {
  const QPointF center = QRectF(rect).center();
  QPainterPath path;
  if (direction == Qt::DownArrow) {
    path.moveTo(center + QPointF(-3, -1.5));
    path.lineTo(center + QPointF(0, 1.5));
    path.lineTo(center + QPointF(3, -1.5));
  } else if (direction == Qt::UpArrow) {
    path.moveTo(center + QPointF(-3, 1.5));
    path.lineTo(center + QPointF(0, -1.5));
    path.lineTo(center + QPointF(3, 1.5));
  } else if (direction == Qt::LeftArrow) {
    path.moveTo(center + QPointF(1.5, -3));
    path.lineTo(center + QPointF(-1.5, 0));
    path.lineTo(center + QPointF(1.5, 3));
  } else {
    path.moveTo(center + QPointF(-1.5, -3));
    path.lineTo(center + QPointF(1.5, 0));
    path.lineTo(center + QPointF(-1.5, 3));
  }
  paintIconPath(*painter, path, rect, ink, 1.2);
}
void check(QPainter* painter, const QRectF& rect, const QColor& ink) {
  const QRectF r(rect);
  QPainterPath path;
  path.moveTo(r.left() + r.width() * .22, r.top() + r.height() * .52);
  path.lineTo(r.left() + r.width() * .43, r.top() + r.height() * .72);
  path.lineTo(r.left() + r.width() * .80, r.top() + r.height() * .28);
  paintIconPath(*painter, path, rect, ink, 1.8);
}
QString buttonRole(const QWidget* widget) {
  if (!widget) return {};
  const auto explicitRole = widget->property("designRole").toString();
  return explicitRole.isEmpty() ? widget->property("dialogAction").toString() : explicitRole;
}
QColor textColor(const QStyleOption* option) {
  return enabled(option) ? color(Color::Text) : DesignSystem::mix(color(Color::Text), color(Color::Surface), .58);
}
}  // namespace
DesignStyle::DesignStyle() : QProxyStyle(QStyleFactory::create(QStringLiteral("Fusion"))) {
  FocusIndicator::install();
  installContextMenuPresentation();
}
void DesignStyle::polish(QWidget* widget) {
  QProxyStyle::polish(widget);
  if (!widget->testAttribute(Qt::WA_SetFont)) widget->setFont(QApplication::font());
  if (auto* area = qobject_cast<QAbstractScrollArea*>(widget)) {
    const bool transparent = widget->property("designTransparent").toBool() || widget->property("designPlainDocument").toBool();
    if (transparent) {
      widget->setAutoFillBackground(false);
      area->viewport()->setAutoFillBackground(false);
    } else if (qobject_cast<QTextEdit*>(widget) || qobject_cast<QPlainTextEdit*>(widget)) {
      widget->setBackgroundRole(QPalette::Base);
      widget->setAutoFillBackground(true);
    }
    if (widget->property("designSoftSurface").toBool()) {
      widget->setBackgroundRole(QPalette::Midlight);
      widget->setAutoFillBackground(true);
      area->viewport()->setBackgroundRole(QPalette::Midlight);
    }
  }
  if (auto* header = qobject_cast<QHeaderView*>(widget)) header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  if (qobject_cast<QAbstractButton*>(widget) || qobject_cast<QComboBox*>(widget) || qobject_cast<QLineEdit*>(widget) ||
      qobject_cast<QAbstractSpinBox*>(widget) || qobject_cast<QProgressBar*>(widget))
    ControlAnimation::attach(widget);
  if (qobject_cast<QScrollBar*>(widget)) {
    widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    widget->setAttribute(Qt::WA_Hover);
    widget->setAutoFillBackground(false);
  }
  if (qobject_cast<QMenu*>(widget)) {
    configurePopupAppearance(widget);
    auto font = QApplication::font();
    font.setPixelSize(design::smallFontSize);
    widget->setFont(font);
    widget->setAttribute(Qt::WA_Hover);
  }
}
void DesignStyle::polish(QPalette& palette) { palette = DesignSystem::instance().palette(); }
void DesignStyle::paintButton(const QStyleOption* option, QPainter* painter, const QWidget* widget, const QString& text, const QIcon& glyph,
                              const bool tool) const {
  painter->save();
  painter->setLayoutDirection(option->direction);
  const auto role = buttonRole(widget);
  const bool link = role == QLatin1String("link");
  const bool primary = role == QLatin1String("primary"), destructive = role == QLatin1String("destructive"), close = role == QLatin1String("close");
  const bool danger = destructive || role == QLatin1String("dangerQuiet") || role == QLatin1String("destructiveSecondary") || role == QLatin1String("danger");
  const bool quiet = tool || link || role == QLatin1String("quiet") || role == QLatin1String("dangerQuiet");
  QColor fill = quiet ? QColor(Qt::transparent) : color(Color::Surface), ink = danger ? color(Color::Danger) : color(Color::Text),
         border = quiet ? QColor(Qt::transparent) : color(Color::Line);
  QColor hover = color(Color::ButtonHover), pressed = color(Color::ButtonPressed);
  if (link) {
    ink = color(Color::Accent);
    hover = pressed = Qt::transparent;
  }
  if (danger && !destructive) hover = pressed = color(Color::DangerBg);
  if (primary) {
    fill = color(Color::Accent);
    ink = color(Color::AccentText);
    border = Qt::transparent;
    hover = color(Color::AccentHover);
    pressed = color(Color::AccentPressed);
  }
  if (destructive) {
    fill = color(Color::DangerFill);
    ink = Qt::white;
    border = Qt::transparent;
    hover = color(Color::DangerFillHover);
    pressed = color(Color::DangerFillPressed);
  }
  if (close) {
    fill = color(Color::Close);
    border = Qt::transparent;
    ink = color(Color::Muted);
    hover = color(Color::CloseHover);
    pressed = color(Color::ClosePressed);
  }
  const bool checkedIconOnly = widget && widget->property("designCheckedIconOnly").toBool();
  bool down = option->state.testFlag(State_Sunken) || (!checkedIconOnly && option->state.testFlag(State_On));
  const auto* pushButton = qobject_cast<const QPushButton*>(widget);
  const auto* toolButton = qobject_cast<const QToolButton*>(widget);
  const auto* menu = pushButton ? pushButton->menu() : toolButton ? toolButton->menu() : nullptr;
  if (menu && menu->isVisible() && !option->state.testFlag(State_On))
    down = QApplication::mouseButtons().testFlag(Qt::LeftButton) && widget->rect().contains(widget->mapFromGlobal(QCursor::pos()));
  if (enabled(option)) {
    const qreal amount = ControlAnimation::hover(widget, option->state.testFlag(State_MouseOver));
    fill = DesignSystem::mix(fill, hover, amount);
    const qreal pressAmount = !checkedIconOnly && option->state.testFlag(State_On) ? 1.0 : ControlAnimation::press(widget, down);
    fill = DesignSystem::mix(fill, pressed, pressAmount);
    if (close && amount > .0) ink = DesignSystem::mix(ink, Qt::white, amount);
  } else
    painter->setOpacity(painter->opacity() * .42);
  const qreal radius = close ? design::closeRadius : design::buttonRadius;
  const QColor bottom = !quiet && !primary && !destructive && !close ? DesignSystem::mix(color(Color::Line), color(Color::ButtonEdge), .2) : border;
  paintControlShape(*painter, option->rect, radius, fill, border, DesignSystem::mix(bottom, border, ControlAnimation::press(widget, down)));
  QFont font = widget ? widget->font() : QApplication::font();
  font.setWeight(link ? QFont::Normal : QFont::Medium);
  font.setUnderline(link && option->state.testFlag(State_MouseOver));
  if (tool) font.setPixelSize(design::smallFontSize);
  painter->setFont(font);
  const bool iconOnly =
      tool && qobject_cast<const QToolButton*>(widget) && qobject_cast<const QToolButton*>(widget)->toolButtonStyle() == Qt::ToolButtonIconOnly;
  const QString label = iconOnly ? QString{} : text;
  const auto* abstractButton = qobject_cast<const QAbstractButton*>(widget);
  const int side = close ? design::closeIconSize : abstractButton ? abstractButton->iconSize().width() : design::iconSize;
  const int iconWidth = glyph.isNull() ? 0 : side;
  const int gap = iconWidth && !label.isEmpty() ? (link ? 5 : design::buttonIconGap) : 0;
  const QIcon trailing = widget ? widget->property("designTrailingIcon").value<QIcon>() : QIcon{};
  const int trailingWidth = trailing.isNull() ? 0 : side;
  const int trailingGap = trailingWidth && !label.isEmpty() ? design::buttonIconGap : 0;
  const qreal textWidth = textLineSize(label, font, true).width();
  const qreal total = iconWidth + gap + textWidth + trailingGap + trailingWidth;
  const qreal left = option->rect.left() + (option->rect.width() - total) / 2.0;
  const auto visual = [&](const QRectF& rect) {
    if (option->direction != Qt::RightToLeft) return rect;
    return QRectF(QRectF(option->rect).right() - (rect.right() - option->rect.left()), rect.top(), rect.width(), rect.height());
  };
  auto iconInk = widget ? widget->property("designIconColor").value<QColor>() : QColor{};
  if (!iconInk.isValid()) iconInk = checkedIconOnly && option->state.testFlag(State_On) ? color(Color::Accent) : ink;
  if (iconWidth) icon(painter, glyph, visual(QRectF(left, option->rect.top() + (option->rect.height() - side) / 2.0, side, side)), iconInk);
  painter->setPen(ink);
  paintTextLine(*painter, visual(QRectF(left + iconWidth + gap, option->rect.top(), textWidth + 2, option->rect.height())), label, font,
                Qt::AlignVCenter | Qt::AlignLeading, true);
  if (trailingWidth)
    icon(painter, trailing, visual(QRectF(left + total - trailingWidth, option->rect.top() + (option->rect.height() - side) / 2.0, side, side)), ink);
  if (const auto* button = qobject_cast<const QToolButton*>(widget); button && button->menu() && button->popupMode() == QToolButton::MenuButtonPopup)
    arrow(painter, QRectF(option->rect.right() - 15, QRectF(option->rect).center().y() - 5, 10, 10), ink, Qt::DownArrow);
  painter->restore();
}
void DesignStyle::paintField(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  if (qobject_cast<const QLineEdit*>(widget) &&
      (qobject_cast<const QAbstractSpinBox*>(widget->parentWidget()) || qobject_cast<const QComboBox*>(widget->parentWidget())))
    return;
  painter->save();
  if (!enabled(option)) painter->setOpacity(painter->opacity() * .5);
  const bool combo = qobject_cast<const QComboBox*>(widget);
  const bool focused = combo ? widget->property("designExpanded").toBool() : option->state.testFlag(State_HasFocus) || (widget && widget->hasFocus());
  QColor fill = color(enabled(option) ? Color::Surface : Color::SurfaceSoft);
  if (combo && enabled(option)) {
    fill = DesignSystem::mix(fill, color(Color::Hover), ControlAnimation::hover(widget, option->state.testFlag(State_MouseOver)));
    fill = DesignSystem::mix(fill, color(Color::Pressed), ControlAnimation::press(widget, option->state.testFlag(State_Sunken)));
  }
  paintControlShape(*painter, option->rect, design::fieldRadius, fill, color(Color::Line), color(focused ? Color::Accent : Color::Faint));
  if (focused) {
    QPainterPath clip;
    clip.addRoundedRect(QRectF(option->rect), design::fieldRadius, design::fieldRadius);
    painter->setClipPath(clip);
    painter->fillRect(QRectF(option->rect.left(), option->rect.top() + option->rect.height() - 2, option->rect.width(), 2), color(Color::Accent));
  }
  painter->restore();
}
void DesignStyle::drawPrimitive(const PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  painter->save();
  switch (element) {
    case PE_PanelButtonCommand:
    case PE_PanelButtonTool:
    case PE_FrameDefaultButton:
      painter->restore();
      return;
    case PE_PanelLineEdit:
      paintField(option, painter, widget);
      painter->restore();
      return;
    case PE_FrameMenu:
    case PE_FrameLineEdit:
      painter->restore();
      return;
    case PE_PanelScrollAreaCorner:
      // Preserve the scrolling surface underneath the intersection, including
      // the alpha backing of Mica dialogs. A separate fill creates a rectangle.
      painter->restore();
      return;
    case PE_PanelMenu:
      // QMenu's event filter also covers stylesheet-owned native menus. Draw
      // that shared surface once, so antialiased corners do not accumulate.
      if (!widget || !widget->property("designPopupSurface").toBool()) paintPopupSurface(*painter, option->rect);
      painter->restore();
      return;
    case PE_PanelTipLabel:
      rounded(painter, option->rect, color(Color::Surface), color(Color::Line), 4);
      painter->restore();
      return;
    case PE_FrameFocusRect: {
      if (qobject_cast<const QFocusFrame*>(widget)) drawControl(CE_FocusFrame, option, painter, widget);
      painter->restore();
      return;
    }
    case PE_IndicatorCheckBox:
    case PE_IndicatorItemViewItemCheck: {
      if (!enabled(option)) painter->setOpacity(painter->opacity() * .42);
      const bool on = option->state.testFlag(State_On) || option->state.testFlag(State_NoChange);
      const QRect box(option->rect.center().x() - 7, option->rect.center().y() - 7, 15, 15);
      rounded(painter, box, color(on ? Color::Accent : Color::Surface), color(on ? Color::Accent : Color::Faint), 3);
      if (option->state.testFlag(State_NoChange)) {
        painter->setPen(QPen(color(Color::AccentText), 1.8));
        painter->drawLine(box.center() + QPoint(-3, 0), box.center() + QPoint(3, 0));
      } else if (on)
        check(painter, box, color(Color::AccentText));
      painter->restore();
      return;
    }
    case PE_IndicatorRadioButton: {
      const bool on = option->state.testFlag(State_On);
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setPen(QPen(color(on ? Color::Accent : Color::Faint), 1));
      painter->setBrush(color(on ? Color::Accent : Color::Surface));
      painter->drawEllipse(QRectF(option->rect).adjusted(.5, .5, -.5, -.5));
      if (on) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(color(Color::AccentText));
        painter->drawEllipse(QRectF(QRectF(option->rect).center() - QPointF(3, 3), QSizeF(6, 6)));
      }
      painter->restore();
      return;
    }
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
      arrow(painter, option->rect, textColor(option),
            element == PE_IndicatorArrowDown   ? Qt::DownArrow
            : element == PE_IndicatorArrowUp   ? Qt::UpArrow
            : element == PE_IndicatorArrowLeft ? Qt::LeftArrow
                                               : Qt::RightArrow);
      painter->restore();
      return;
    case PE_IndicatorBranch:
      if (option->state.testFlag(State_Children))
        arrow(painter, option->rect, color(Color::Muted), option->state.testFlag(State_Open) ? Qt::DownArrow : Qt::RightArrow);
      painter->restore();
      return;
    case PE_FrameTabWidget:
    case PE_FrameTabBarBase:
    case PE_IndicatorToolBarHandle:
    case PE_IndicatorToolBarSeparator:
      painter->restore();
      return;
    case PE_PanelItemViewItem: {
      const bool selected = option->state.testFlag(State_Selected), hovered = option->state.testFlag(State_MouseOver);
      if (selected || hovered) painter->fillRect(option->rect, color(selected ? Color::Selected : Color::Hover));
      painter->restore();
      return;
    }
    default:
      break;
  }
  painter->restore();
  QProxyStyle::drawPrimitive(element, option, painter, widget);
}
void DesignStyle::drawControl(const ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  if (element == CE_ToolBar || element == CE_Splitter) {
    painter->fillRect(option->rect, color(element == CE_ToolBar ? Color::Surface : Color::Line));
    return;
  }
  if (element == CE_ShapedFrame && (qobject_cast<const QTextEdit*>(widget) || qobject_cast<const QPlainTextEdit*>(widget))) {
    if (!widget->property("designPlainDocument").toBool() && !widget->property("designCodeEditor").toBool())
      rounded(painter, option->rect, Qt::transparent, color(Color::Line), 5);
    return;
  }
  if (element == CE_CheckBoxLabel || element == CE_RadioButtonLabel) {
    if (const auto* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
      painter->save();
      if (!enabled(option)) painter->setOpacity(painter->opacity() * .5);
      painter->setPen(color(Color::Text));
      QRectF textBounds = option->rect;
      if (!button->icon.isNull()) {
        const int side = button->iconSize.width();
        const bool rtl = option->direction == Qt::RightToLeft;
        const qreal left = rtl ? textBounds.right() - side : textBounds.left();
        icon(painter, button->icon, QRectF(left, textBounds.center().y() - side / 2.0, side, side), color(Color::Muted));
        textBounds.adjust(rtl ? 0 : side + 8, 0, rtl ? -side - 8 : 0, 0);
      }
      paintTextLine(*painter, textBounds, button->text, widget ? widget->font() : painter->font(), Qt::AlignVCenter | Qt::AlignLeading, true);
      painter->restore();
      return;
    }
  }
  if (element == CE_MenuEmptyArea) return;
  if (element == CE_FocusFrame) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(color(Color::Accent), 2));
    painter->setBrush(Qt::NoBrush);
    const auto* frame = qobject_cast<const QFocusFrame*>(widget);
    const auto* target = frame ? frame->widget() : nullptr;
    const qreal radius = qobject_cast<const SwitchButton*>(target) ? design::switchHeight / 2.0
                         : qobject_cast<const QComboBox*>(target) || qobject_cast<const QLineEdit*>(target) || qobject_cast<const QAbstractSpinBox*>(target)
                             ? design::fieldRadius
                         : buttonRole(target) == QLatin1String("close") ? design::closeRadius
                         : buttonRole(target) == QLatin1String("link")  ? 0
                                                                        : design::buttonRadius;
    const auto customRadius = target ? target->property("designRadius") : QVariant{};
    const qreal baseRadius = customRadius.isValid() ? customRadius.toReal() : radius;
    const qreal outlineRadius = baseRadius > 0 ? baseRadius + design::focusOffset + design::focusWidth / 2.0 : 0;
    painter->drawRoundedRect(QRectF(option->rect).adjusted(1, 1, -1, -1), outlineRadius, outlineRadius);
    painter->restore();
    return;
  }
  if (element == CE_ItemViewItem) {
    const auto* item = qstyleoption_cast<const QStyleOptionViewItem*>(option);
    if (item && !item->features.testFlag(QStyleOptionViewItem::WrapText)) {
      painter->save();
      painter->setLayoutDirection(item->direction);
      painter->setClipRect(item->rect, Qt::IntersectClip);
      const bool selected = item->state.testFlag(State_Selected);
      if (item->backgroundBrush.style() != Qt::NoBrush)
        painter->fillRect(item->rect, item->backgroundBrush);
      else if (item->features.testFlag(QStyleOptionViewItem::Alternate))
        painter->fillRect(item->rect, item->palette.alternateBase());
      if (selected || item->state.testFlag(State_MouseOver)) painter->fillRect(item->rect, color(selected ? Color::Selected : Color::Hover));
      if (item->features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
        QStyleOption indicator(*item);
        indicator.rect = subElementRect(SE_ItemViewItemCheckIndicator, item, widget);
        indicator.state &= ~(State_On | State_Off | State_NoChange);
        indicator.state |= item->checkState == Qt::Checked ? State_On : item->checkState == Qt::PartiallyChecked ? State_NoChange : State_Off;
        drawPrimitive(PE_IndicatorItemViewItemCheck, &indicator, painter, widget);
      }
      if (!item->icon.isNull()) {
        QRectF decoration = subElementRect(SE_ItemViewItemDecoration, item, widget);
        decoration.moveTop(QRectF(item->rect).center().y() - decoration.height() / 2);
        painter->save();
        if (!enabled(item)) painter->setOpacity(painter->opacity() * .5);
        paintIcon(*painter, item->icon, decoration);
        painter->restore();
      }
      auto textBounds = subElementRect(SE_ItemViewItemText, item, widget);
      textBounds.setTop(item->rect.top());
      textBounds.setHeight(item->rect.height());
      const auto group = enabled(item) ? QPalette::Active : QPalette::Disabled;
      painter->setPen(item->palette.color(group, selected ? QPalette::HighlightedText : QPalette::Text));
      const auto text = QFontMetricsF(item->font).elidedText(item->text, item->textElideMode, textBounds.width());
      paintTextLine(*painter, textBounds, text, item->font, item->displayAlignment);
      painter->restore();
    } else {
      QProxyStyle::drawControl(element, option, painter, widget);
    }
    painter->save();
    const auto* view = qobject_cast<const QAbstractItemView*>(widget);
    const bool spaceBelow = !view || option->rect.bottom() < view->viewport()->rect().bottom();
    if (spaceBelow && (!item || !item->index.isValid() || item->index.row() + 1 < item->index.model()->rowCount(item->index.parent()))) {
      painter->setPen(color(Color::LineSoft));
      painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
    }
    if (item && item->state.testFlag(State_Selected) && item->index.column() == 0)
      painter->fillRect(QRect(option->rect.left(), option->rect.top(), 3, option->rect.height()), color(Color::Accent));
    painter->restore();
    return;
  }
  if (element == CE_PushButton) {
    if (const auto* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
      paintButton(option, painter, widget, button->text, button->icon, false);
      return;
    }
  }
  if (element == CE_ToolButtonLabel) {
    if (const auto* button = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
      paintButton(option, painter, widget, button->text, button->icon, true);
      return;
    }
  }
  if (element == CE_Header || element == CE_HeaderSection || element == CE_HeaderLabel || element == CE_HeaderEmptyArea) {
    const auto* header = qstyleoption_cast<const QStyleOptionHeader*>(option);
    if (header) {
      painter->save();
      if (element != CE_HeaderLabel) {
        painter->fillRect(header->rect, color(Color::SurfaceSoft));
        painter->setPen(color(Color::Line));
        painter->drawLine(header->rect.bottomLeft(), header->rect.bottomRight());
      }
      if (element == CE_HeaderSection || element == CE_HeaderEmptyArea) {
        painter->restore();
        return;
      }
      painter->setPen(color(Color::Muted));
      QFont font = widget ? widget->font() : QApplication::font();
      font.setPixelSize(design::smallFontSize);
      font.setWeight(QFont::Medium);
      painter->setFont(font);
      const auto bounds = element == CE_HeaderLabel ? header->rect : header->rect.adjusted(design::tableCellPaddingX, 0, -design::tableCellPaddingX, 0);
      paintTextLine(*painter, bounds, header->text, font, Qt::AlignVCenter | header->textAlignment);
      painter->restore();
      return;
    }
  }
  if (element == CE_MenuItem) {
    const auto* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
    if (item) {
      painter->save();
      painter->setLayoutDirection(item->direction);
      const QRect r = item->rect;
      const auto visual = [&](QRectF bounds) {
        if (item->direction == Qt::RightToLeft) bounds.moveLeft(r.left() + r.right() + 1 - bounds.right());
        return bounds;
      };
      if (item->menuItemType == QStyleOptionMenuItem::Separator) {
        painter->setPen(color(Color::LineSoft));
        painter->drawLine(r.left() + 5, r.center().y(), r.right() - 5, r.center().y());
        painter->restore();
        return;
      }
      const auto* menu = qobject_cast<const QMenu*>(widget);
      const auto* action = menu ? menu->actionAt(item->rect.center()) : nullptr;
      const bool heading = action && action->property("designMenuHeading").toBool();
      if (!enabled(item) && !heading) painter->setOpacity(painter->opacity() * .4);
      const qreal highlight = ItemHighlight::opacity(widget, r.topLeft(), enabled(item) && item->state.testFlag(State_Selected));
      rounded(painter, r, DesignSystem::mix(Qt::transparent, color(Color::Hover), highlight), Qt::transparent, 4);
      const bool destructive = action && action->property("designRole").toString() == QLatin1String("destructive");
      const QColor ink = color(destructive ? Color::Danger : Color::Text);
      painter->setFont(item->font);
      const QRectF glyph = visual(QRectF(r.left() + 10, QRectF(r).center().y() - 8, 16, 16));
      if (!item->icon.isNull()) icon(painter, item->icon, glyph, color(destructive ? Color::Danger : Color::Muted));
      const QString label = menuDisplayLabel(item->text.section(QLatin1Char('\t'), 0, 0)), shortcut = item->text.section(QLatin1Char('\t'), 1);
      QFont shortcutFont = item->font;
      shortcutFont.setPixelSize(11);
      const int leading = heading || item->icon.isNull() ? 10 : 36;
      const bool submenu = item->menuItemType == QStyleOptionMenuItem::SubMenu;
      const int trailing = shortcut.isEmpty() ? 10 + (item->checkType != QStyleOptionMenuItem::NotCheckable ? 26 : 0) + (submenu ? 26 : 0)
                                              : 15 + qCeil(textLineSize(shortcut, shortcutFont).width()) + 24;
      painter->setPen(ink);
      paintTextLine(*painter, visual(r.adjusted(leading, 0, -trailing, 0)), label, item->font, Qt::AlignVCenter | Qt::AlignLeading, true);
      painter->setPen(color(Color::Faint));
      paintTextLine(*painter, visual(r.adjusted(leading, 0, -15, 0)), shortcut, shortcutFont, Qt::AlignVCenter | Qt::AlignTrailing);
      if (submenu)
        arrow(painter, visual(QRectF(r.right() - 20, QRectF(r).center().y() - 5, 10, 10)), color(Color::Muted),
              item->direction == Qt::RightToLeft ? Qt::LeftArrow : Qt::RightArrow);
      if (item->checked) {
        const auto mark = visual(QRectF(r.right() - (submenu ? 51 : 25), QRectF(r).center().y() - 7.5, 15, 15));
        QPainterPath path;
        path.moveTo(mark.left() + 12.5, mark.top() + 3.75);
        path.lineTo(mark.left() + 5.625, mark.top() + 10.625);
        path.lineTo(mark.left() + 2.5, mark.top() + 7.5);
        paintIconPath(*painter, path, mark, color(Color::Accent), 1.65 * 15 / 24);
      }
      painter->restore();
      return;
    }
  }
  if (element == CE_TabBarTab) {
    if (const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
      painter->save();
      painter->setLayoutDirection(tab->direction);
      const auto kind = widget ? widget->property("designTabKind").toString() : QString{};
      const bool volume = kind == QLatin1String("volumes");
      const bool page = kind == QLatin1String("pages");
      QRect bounds = tab->rect;
      const bool compact = widget && widget->property("designCompact").toBool();
      bounds.setWidth(std::max(0, bounds.width() - (page ? 26 : compact && !volume ? 14 : 22)));
      bounds = QStyle::visualRect(tab->direction, tab->rect, bounds);
      const auto visual = [&](const QRectF& logical) {
        return tab->direction == Qt::RightToLeft
                   ? QRectF(bounds.right() + bounds.left() + 1 - logical.right(), logical.top(), logical.width(), logical.height())
                   : logical;
      };
      const bool selected = tab->state.testFlag(State_Selected);
      if (tab->state.testFlag(State_MouseOver)) rounded(painter, bounds, color(Color::Hover), Qt::transparent, 4);
      if (selected) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(color(Color::Accent));
        painter->drawRoundedRect(QRectF(bounds.left(), bounds.bottom() - 2, bounds.width(), 3), 2, 2);
      }
      QFont font = widget ? widget->font() : QApplication::font();
      font.setPixelSize(volume || (widget && widget->property("designCompact").toBool()) ? 12 : 13);
      font.setWeight(selected ? QFont::DemiBold : QFont::Normal);
      painter->setFont(font);
      const QColor ink = color(selected ? Color::Text : Color::Muted);
      int x = bounds.left() + (volume ? 4 : 0);
      if (!tab->icon.isNull()) {
        const auto accentTab = widget ? widget->property("designAccentTab") : QVariant{};
        const QColor iconColor = volume && accentTab.isValid() && accentTab.toInt() == tab->tabIndex ? color(Color::Accent) : ink;
        icon(painter, tab->icon, visual(QRectF(x, QRectF(bounds).center().y() - 8 - (volume ? 0 : page ? 6.5 : 3), 16, 16)), iconColor);
        x += 23;
      }
      painter->setPen(ink);
      const auto counts = widget ? widget->property("designTabCounts").toMap() : QVariantMap{};
      const auto count = counts.value(QString::number(tab->tabIndex));
      const int copyHeight = bounds.height() - (volume ? 0 : page ? 13 : 6);
      paintTextLine(*painter, visual(QRectF(x, bounds.top(), textLineSize(tab->text, font, true).width() + 2, copyHeight)), tab->text, font,
                    Qt::AlignVCenter | Qt::AlignLeading, true);
      const auto indicator = widget ? widget->property("designTabIndicators").toMap().value(QString::number(tab->tabIndex)).toString() : QString{};
      if (!indicator.isEmpty()) {
        const qreal indicatorX = x + textLineSize(tab->text, font, true).width() + 6;
        icon(painter, QIcon(indicator), visual(QRectF(indicatorX, bounds.top() + (copyHeight - 14) / 2.0, 14, 14)), color(Color::Warning));
      }
      if (count.isValid()) {
        QFont badgeFont = font;
        badgeFont.setPixelSize(11);
        badgeFont.setWeight(QFont::Normal);
        const QString caption = count.toString();
        const auto badgeSize = countBadgeSize(caption, badgeFont);
        const qreal badgeWidth = badgeSize.width();
        const qreal badgeX = x + textLineSize(tab->text, font, true).width() + 6;
        const QRectF badge(badgeX, bounds.top() + (copyHeight - badgeSize.height()) / 2.0, badgeWidth, badgeSize.height());
        paintCountBadge(*painter, visual(badge), caption, badgeFont);
      }
      painter->restore();
      return;
    }
  }
  if (element == CE_ComboBoxLabel) {
    if (const auto* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
      if (combo->editable) return;
      painter->save();
      painter->setPen(textColor(option));
      paintTextLine(*painter, subControlRect(CC_ComboBox, combo, SC_ComboBoxEditField, widget), combo->currentText, widget ? widget->font() : painter->font());
      painter->restore();
      return;
    }
  }
  if (element == CE_ProgressBarLabel) return;
  if (element == CE_ProgressBar || element == CE_ProgressBarGroove || element == CE_ProgressBarContents) {
    if (const auto* progress = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
      painter->save();
      const QRectF track(option->rect.left(), QRectF(option->rect).center().y() - 2, option->rect.width(), 4);
      if (element != CE_ProgressBarContents) rounded(painter, track, color(Color::Line), Qt::transparent, 2);
      if (element == CE_ProgressBarGroove) {
        painter->restore();
        return;
      }
      qreal fraction = 0;
      if (progress->maximum > progress->minimum) {
        const auto animated = widget ? widget->property("designProgressValue") : QVariant{};
        const qreal value = animated.isValid() ? animated.toReal() : progress->progress;
        fraction = (value - progress->minimum) / static_cast<qreal>(progress->maximum - progress->minimum);
      }
      const bool busy = progress->minimum == 0 && progress->maximum == 0;
      if (busy) fraction = .38;
      fraction = std::clamp(fraction, 0.0, 1.0);
      QRectF fill = track;
      fill.setWidth(track.width() * fraction);
      if (busy) {
        const qreal phase = widget ? widget->property("designBusyPhase").toReal() : 0;
        painter->setClipRect(track);
        fill.moveLeft(track.left() + track.width() * .38 * (-1.1 + 4.8 * phase));
      }
      const auto ink = widget ? widget->property("designProgressColor").value<QColor>() : QColor{};
      rounded(painter, fill, ink.isValid() ? ink : color(Color::Accent), Qt::transparent, 2);
      painter->restore();
      return;
    }
  }
  QProxyStyle::drawControl(element, option, painter, widget);
}
void DesignStyle::drawComplexControl(const ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  if (control == CC_ToolButton) {
    if (const auto* button = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
      paintButton(option, painter, widget, button->text, button->icon, true);
      return;
    }
  }
  if (control == CC_ComboBox) {
    paintField(option, painter, widget);
    arrow(painter, subControlRect(control, option, SC_ComboBoxArrow, widget), color(Color::Muted), Qt::DownArrow);
    return;
  }
  if (control == CC_SpinBox) {
    paintField(option, painter, widget);
    if (widget) {
      const auto unit = widget->property("unitText").toString();
      if (!unit.isEmpty() && !widget->property("designUnitChild").toBool()) {
        painter->save();
        painter->setLayoutDirection(Qt::LeftToRight);
        QFont type = widget->font();
        type.setPixelSize(11);
        painter->setFont(type);
        painter->setPen(color(Color::Muted));
        if (!enabled(option)) painter->setOpacity(.5);
        paintTextLine(*painter, option->rect.adjusted(8, 1, -8, -1), unit, painter->font(), Qt::AlignRight | Qt::AlignVCenter);
        painter->restore();
      }
    }
    if (const auto* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option); spin && spin->buttonSymbols != QAbstractSpinBox::NoButtons) {
      arrow(painter, subControlRect(control, option, SC_SpinBoxUp, widget), textColor(option), Qt::UpArrow);
      arrow(painter, subControlRect(control, option, SC_SpinBoxDown, widget), textColor(option), Qt::DownArrow);
    }
    return;
  }
  if (control == CC_ScrollBar) {
    const auto* slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
    if (!slider) return;
    const bool thin = thinScrollBar(widget), vertical = slider->orientation == Qt::Vertical;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    // The non-opaque scrollbar lets QWidget restore its parent's pixels before
    // each paint. Drawing only the thumb/buttons preserves both Mica and the
    // opaque background of text editors, without leaving old thumb pixels.
    const auto active = option->activeSubControls;
    const bool hovered = option->state.testFlag(State_MouseOver), pressed = option->state.testFlag(State_Sunken);
    const QColor resting = color(Color::ScrollbarThumb);
    QColor thumbColor = resting;
    if ((hovered || pressed) && active.testFlag(SC_ScrollBarSlider)) thumbColor = color(Color::ScrollbarThumbHover);
    const auto sliderRect = subControlRect(control, option, SC_ScrollBarSlider, widget);
    const auto inset = design::fullScrollBarThumbInset;
    const QRectF thumb = thin       ? QRectF(sliderRect).adjusted(2, 2, -2, -2)
                         : vertical ? QRectF(sliderRect).adjusted(inset, 0, -inset, 0)
                                    : QRectF(sliderRect).adjusted(0, inset, 0, -inset);
    rounded(painter, thumb, thin ? color(Color::Faint) : thumbColor, Qt::transparent, thin ? 3 : (design::fullScrollBarWidth - 2 * inset) / 2.0);
    if (!thin)
      for (const auto sub : {SC_ScrollBarSubLine, SC_ScrollBarAddLine}) {
        const QRect button = subControlRect(control, option, sub, widget);
        if (button.isEmpty()) continue;
        const bool buttonPressed = pressed && active.testFlag(sub);
        const bool buttonHovered = hovered && active.testFlag(sub);
        const auto center = QRectF(button).center();
        const QColor arrowInk = buttonHovered || buttonPressed ? color(Color::ScrollbarThumbHover) : resting;
        // H5 uses these same SVGs as masks. No installed symbol font or native
        // platform scrollbar participates in the control's appearance.
        static const QIcon up(QStringLiteral(":/icons/scroll_up.svg")), down(QStringLiteral(":/icons/scroll_down.svg")),
            left(QStringLiteral(":/icons/scroll_left.svg")), right(QStringLiteral(":/icons/scroll_right.svg"));
        const auto& glyph = vertical ? (sub == SC_ScrollBarSubLine ? up : down) : (sub == SC_ScrollBarSubLine ? left : right);
        const qreal side = buttonPressed ? design::fullScrollBarPressedArrowSize : design::fullScrollBarArrowSize;
        icon(painter, glyph, QRectF(center.x() - side / 2, center.y() - side / 2, side, side), arrowInk);
      }
    painter->restore();
    return;
  }
  QProxyStyle::drawComplexControl(control, option, painter, widget);
}
QSize DesignStyle::sizeFromContents(const ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const {
  if (type == CT_PushButton) {
    if (const auto* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
      auto typeface = widget ? widget->font() : QApplication::font();
      const bool link = buttonRole(widget) == QLatin1String("link");
      typeface.setWeight(link ? QFont::Normal : QFont::Medium);
      const bool small = widget && widget->maximumHeight() <= 32;
      const auto inset = widget ? widget->property("designPaddingX") : QVariant{};
      const int padding = link ? 0 : inset.isValid() ? inset.toInt() : small ? design::smallButtonPaddingX : design::buttonPaddingX;
      const int glyph = button->icon.isNull() ? 0 : button->iconSize.width();
      const int gap = glyph && !button->text.isEmpty() ? (link ? 5 : design::buttonIconGap) : 0;
      const qreal text = textLineSize(button->text, typeface, true).width();
      const auto trailing = widget ? widget->property("designTrailingIcon").value<QIcon>() : QIcon{};
      const int trailingWidth = trailing.isNull() ? 0 : button->iconSize.width() + (text > 0 ? design::buttonIconGap : 0);
      if (link) return QSize(qCeil(text + glyph + gap + trailingWidth), qCeil(typeface.pixelSize() * 1.6));
      return QSize(std::max(small ? 30 : design::buttonHeight, qCeil(text + glyph + gap + trailingWidth + padding * 2 + 2)), small ? 30 : design::buttonHeight);
    }
  }
  if (type == CT_ToolButton) {
    if (const auto* button = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
      auto font = widget ? widget->font() : QApplication::font();
      font.setPixelSize(design::smallFontSize);
      font.setWeight(QFont::Medium);
      const bool iconOnly = button->toolButtonStyle == Qt::ToolButtonIconOnly;
      const qreal text = iconOnly ? 0 : textLineSize(button->text, font, true).width();
      const int glyph = button->icon.isNull() ? 0 : design::iconSize;
      const int gap = glyph && text > 0 ? design::buttonIconGap : 0;
      const auto inset = widget ? widget->property("designPaddingX") : QVariant{};
      return {std::max(32, qCeil(text + glyph + gap + (inset.isValid() ? inset.toInt() : 10) * 2 + 2)), 32};
    }
  }
  if (type == CT_LineEdit || type == CT_ComboBox || type == CT_SpinBox)
    return QSize(size.width() + design::fieldPaddingX * 2 + (type == CT_LineEdit ? 0 : 24), std::max(design::fieldHeight, size.height() + 12));
  if (type == CT_MenuItem) {
    if (const auto* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
      if (item->menuItemType == QStyleOptionMenuItem::Separator) return QSize(10, 9);
      const auto label = item->text.section(u'\t', 0, 0);
      const int hidden = qCeil(textLineSize(label, item->font, true).width() - textLineSize(menuDisplayLabel(label), item->font, true).width());
      // QMenu adds its widest shortcut after this call. Apply the 216 px
      // minimum to the popup itself so shortcut menus keep the same width.
      const int padding = 20 + (!item->icon.isNull() ? 26 : 0) + (item->checkType != QStyleOptionMenuItem::NotCheckable ? 26 : 0) +
                          (item->menuItemType == QStyleOptionMenuItem::SubMenu ? 26 : 0) +
                          (item->reservedShortcutWidth > 0 || item->text.contains(u'\t') ? 24 : 0);
      return QSize(std::max(0, size.width() + padding - hidden), design::menuRowHeight);
    }
  }
  if (type == CT_HeaderSection) return QSize(size.width() + 28, design::tableHeaderHeight);
  if (type == CT_TabBarTab) {
    if (const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
      const auto kind = widget ? widget->property("designTabKind").toString() : QString{};
      const bool volume = kind == QLatin1String("volumes");
      const bool page = kind == QLatin1String("pages");
      QFont font = widget ? widget->font() : QApplication::font();
      font.setPixelSize(volume || (widget && widget->property("designCompact").toBool()) ? 12 : 13);
      font.setWeight(QFont::DemiBold);
      const bool compact = widget && widget->property("designCompact").toBool();
      const auto count = widget ? widget->property("designTabCounts").toMap().value(QString::number(tab->tabIndex)) : QVariant{};
      auto badgeFont = font;
      badgeFont.setPixelSize(11);
      badgeFont.setWeight(QFont::Normal);
      const int badge = count.isValid() ? countBadgeSize(count.toString(), badgeFont).width() + 6 : 0;
      const bool indicator = widget && widget->property("designTabIndicators").toMap().contains(QString::number(tab->tabIndex));
      return QSize(QFontMetrics(font).horizontalAdvance(tab->text) + (tab->icon.isNull() ? 0 : 23) + (volume ? 8 : 0) +
                       (page                 ? 26
                        : compact && !volume ? 14
                                             : 22) +
                       badge + (indicator ? 20 : 0),
                   volume ? (compact ? 38 : 47)
                   : page ? 35
                          : 42);
    }
  }
  if (type == CT_ProgressBar) return QSize(std::max(100, size.width()), 4);
  return QProxyStyle::sizeFromContents(type, option, size, widget);
}
int DesignStyle::pixelMetric(const PixelMetric metric, const QStyleOption* option, const QWidget* widget) const {
  switch (metric) {
    case PM_DefaultFrameWidth:
      return 1;
    case PM_ButtonDefaultIndicator:
      return 0;
    case PM_ButtonMargin:
      return design::buttonPaddingX;
    case PM_SmallIconSize:
    case PM_ButtonIconSize:
      return design::iconSize;
    case PM_ScrollBarExtent:
      return thinScrollBar(widget) ? design::scrollBarWidth : design::fullScrollBarWidth;
    case PM_ScrollBarSliderMin:
      return thinScrollBar(widget) ? 24 : design::fullScrollBarMinimumThumb;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
      return 15;
    case PM_CheckBoxLabelSpacing:
    case PM_RadioButtonLabelSpacing:
      return 8;
    case PM_FocusFrameHMargin:
    case PM_FocusFrameVMargin:
      return design::focusWidth + design::focusOffset;
    case PM_MenuPanelWidth:
      return 1;
    case PM_MenuHMargin:
    case PM_MenuVMargin:
      return design::menuPadding;
    case PM_TabBarBaseOverlap:
      return 0;
    case PM_TabBarTabHSpace:
      return 8;
    case PM_TabBarTabVSpace:
      return 16;
    case PM_SplitterWidth:
      return 1;
    default:
      return QProxyStyle::pixelMetric(metric, option, widget);
  }
}
int DesignStyle::styleHint(const StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* result) const {
  switch (hint) {
    case SH_Menu_Mask:
    case SH_FocusFrame_Mask:
    case SH_ComboBox_Popup:
    case SH_EtchDisabledText:
    case SH_DitherDisabledText:
    case SH_UnderlineShortcut:
      return 0;
    case SH_FocusFrame_AboveWidget:
      return 1;
    case SH_Widget_Animate:
      return 1;
    case SH_Widget_Animation_Duration:
      return design::hoverDurationMs;
    case SH_Menu_SubMenuPopupDelay:
      return 180;
    default:
      return QProxyStyle::styleHint(hint, option, widget, result);
  }
}
QRect DesignStyle::subControlRect(const ComplexControl control, const QStyleOptionComplex* option, const SubControl sub, const QWidget* widget) const {
  const QRect r = option->rect;
  if (control == CC_ComboBox) {
    if (sub == SC_ComboBoxArrow) return QRect(r.right() - 27, r.top(), 24, r.height());
    if (sub == SC_ComboBoxEditField) return r.adjusted(10, 1, -30, -1);
    if (sub == SC_ComboBoxFrame) return r;
  }
  if (control == CC_SpinBox) {
    const auto* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
    const bool buttons = spin && spin->buttonSymbols != QAbstractSpinBox::NoButtons;
    if (sub == SC_SpinBoxEditField) {
      const auto unit = widget ? widget->property("unitText").toString() : QString{};
      const int reserve = unit.isEmpty() ? 0 : QFontMetrics(widget->font()).horizontalAdvance(unit) + 10;
      return r.adjusted(10, 1, -std::max(buttons ? 20 : 8, reserve + 8), -1);
    }
    if (sub == SC_SpinBoxFrame) return r;
    if (sub == SC_SpinBoxUp) return buttons ? QRect(r.right() - 18, r.top() + 1, 17, r.height() / 2 - 1) : QRect{};
    if (sub == SC_SpinBoxDown) return buttons ? QRect(r.right() - 18, r.center().y(), 17, r.height() / 2) : QRect{};
  }
  if (control == CC_ScrollBar) {
    const auto* slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
    if (slider) {
      const bool vertical = slider->orientation == Qt::Vertical, thin = thinScrollBar(widget);
      const int length = vertical ? r.height() : r.width();
      const int button = thin ? 0 : std::min(design::fullScrollBarButtonExtent, length / 2);
      const int groove = std::max(0, length - button * 2);
      const qint64 range = static_cast<qint64>(slider->maximum) - slider->minimum;
      const int thumb = range <= 0 ? groove
                                   : std::clamp(static_cast<int>(static_cast<qint64>(groove) * slider->pageStep / (range + slider->pageStep)),
                                                std::min(thin ? 24 : design::fullScrollBarMinimumThumb, groove), groove);
      const int position = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition, std::max(0, groove - thumb), slider->upsideDown);
      const auto section = [&](const int offset, const int extent) {
        return vertical ? QRect(r.left(), r.top() + offset, r.width(), extent) : QRect(r.left() + offset, r.top(), extent, r.height());
      };
      if (sub == SC_ScrollBarGroove) return section(button, groove);
      if (sub == SC_ScrollBarSubLine) return section(0, button);
      if (sub == SC_ScrollBarAddLine) return section(length - button, button);
      if (sub == SC_ScrollBarSlider) return section(button + position, thumb);
      if (sub == SC_ScrollBarSubPage) return section(button, position);
      if (sub == SC_ScrollBarAddPage) return section(button + position + thumb, groove - position - thumb);
    }
  }
  return QProxyStyle::subControlRect(control, option, sub, widget);
}
QRect DesignStyle::subElementRect(const SubElement element, const QStyleOption* option, const QWidget* widget) const {
  if (element == SE_HeaderLabel) return option->rect.adjusted(design::tableCellPaddingX, 0, -design::tableCellPaddingX, 0);
  if (element == SE_ProgressBarGroove || element == SE_ProgressBarContents) return option->rect;
  if (const auto* item = qstyleoption_cast<const QStyleOptionViewItem*>(option); item && qobject_cast<const QTableView*>(widget)) {
    const QRect content = item->rect.adjusted(design::tableCellPaddingX, 0, -design::tableCellPaddingX, 0);
    const int checkWidth = item->features.testFlag(QStyleOptionViewItem::HasCheckIndicator) ? 22 : 0;
    const int iconWidth = item->features.testFlag(QStyleOptionViewItem::HasDecoration) ? item->decorationSize.width() + 7 : 0;
    QRect result;
    if (element == SE_ItemViewItemCheckIndicator)
      result = QRect(content.left(), content.top() + (content.height() - 15) / 2, 15, 15);
    else if (element == SE_ItemViewItemDecoration)
      result = QRect(content.left() + checkWidth, content.top() + (content.height() - item->decorationSize.height()) / 2, item->decorationSize.width(),
                     item->decorationSize.height());
    else if (element == SE_ItemViewItemText)
      result = content.adjusted(checkWidth + iconWidth, 0, 0, 0);
    else
      return QProxyStyle::subElementRect(element, option, widget);
    return QStyle::visualRect(item->direction, item->rect, result);
  }
  if (const auto* tabs = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(option); tabs && tabs->shape == QTabBar::RoundedNorth) {
    const int height = tabs->tabBarSize.height();
    const auto* tabWidget = qobject_cast<const QTabWidget*>(widget);
    const auto* corner = tabWidget ? tabWidget->cornerWidget(Qt::TopRightCorner) : nullptr;
    const int cornerWidth = std::max(tabs->rightCornerWidgetSize.width(), corner ? corner->minimumWidth() : 0);
    const QRect r = option->rect;
    if (element == SE_TabWidgetTabPane || element == SE_TabWidgetTabContents) return r.adjusted(0, height, 0, 0);
    if (element == SE_TabWidgetTabBar) return QRect(r.left(), r.top(), r.width() - cornerWidth, height);
    if (element == SE_TabWidgetRightCorner) return QRect(r.right() - cornerWidth + 1, r.top(), cornerWidth, height);
  }

  if (element == SE_LineEditContents && widget && qobject_cast<const QAbstractSpinBox*>(widget->parentWidget())) return option->rect;
  if (element == SE_LineEditContents) return option->rect.adjusted(design::fieldPaddingX - 1, 1, 1 - design::fieldPaddingX, -1);
  if (element == SE_PushButtonContents) {
    const auto padding = widget ? widget->property("designPaddingX") : QVariant{};
    const int inset = padding.isValid() ? padding.toInt() : design::buttonPaddingX;
    return option->rect.adjusted(inset, 6, -inset, -6);
  }
  if (element == SE_HeaderLabel) return option->rect.adjusted(design::tableCellPaddingX, 0, -design::tableCellPaddingX, 0);
  return QProxyStyle::subElementRect(element, option, widget);
}
}  // namespace uwf::ui
