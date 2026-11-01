// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenu>
#include <QSet>
#include <QTimer>
#include <QToolButton>
#include <QtMath>
#include <utility>

#include "presentation/controls/layout/AlignedGraphic.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/models/ItemRoles.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

namespace uwf::test {
inline QList<QRectF> inlineInkRuns(const QImage& image, const QRectF& region, const int minimumGap) {
  const qreal scale = image.devicePixelRatio();
  QList<QRectF> runs;
  const auto rasterRuns = contrastingInkRuns(image, pixelCenterRegion(region, scale), minimumGap);
  for (const auto& ink : std::as_const(rasterRuns)) runs.append(QRectF(ink.x() / scale, ink.y() / scale, ink.width() / scale, ink.height() / scale));
  return runs;
}
inline QRectF renderedInlineInk(const QImage& image, const QRectF& region) {
  QRectF ink;
  for (const auto runs = inlineInkRuns(image, region, 1); const auto& run : runs) ink = ink.united(run);
  return ink;
}
inline bool inlineRegionFullyVisible(QWidget& widget, const QRectF& region, QWidget& root) {
  if (!widget.visibleRegion().contains(region.toAlignedRect())) return false;
  const auto bounds = region.translated(widget.mapTo(&root, QPoint()));
  for (auto* ancestor = widget.parentWidget(); ancestor && ancestor != &root; ancestor = ancestor->parentWidget()) {
    const auto* area = qobject_cast<QAbstractScrollArea*>(ancestor);
    if (!area || !area->viewport()->isAncestorOf(&widget)) continue;
    // A graphics effect can expose the complete source widget to visibleRegion(),
    // while the final composited pixels are still clipped by this viewport.
    const QRectF viewport(area->viewport()->mapTo(&root, QPoint()), area->viewport()->size());
    if (!viewport.contains(bounds)) return false;
  }
  return true;
}

// Inspect actual adjacent production controls rather than rebuilding their text/icon layout in the test.
inline QString probeInlineIconText(QWidget& root, const QImage& frame, const QString& name, const QString& tag, QWidget* scope = nullptr) {
  QString errors;
  QJsonArray pairs;
  QJsonArray clippedPairs;
  QSet<ui::IconView*> inspected;
  const qreal scale = root.devicePixelRatioF();
  auto* inspectedRoot = scope ? scope : &root;
  const auto firstLabel = [&](QLayoutItem* item) -> ui::TextLabel* {
    if (auto* direct = dynamic_cast<ui::TextLabel*>(item->widget())) return direct;
    return nullptr;
  };
  const auto findIcon = [&](QWidget* widget) -> ui::IconView* {
    if (!widget || !widget->isVisibleTo(&root)) return nullptr;
    if (auto* icon = dynamic_cast<ui::IconView*>(widget)) return icon;
    ui::IconView* result = nullptr;
    for (auto widgets = widget->findChildren<QWidget*>(); auto* child : std::as_const(widgets)) {
      if (dynamic_cast<ui::TextLabel*>(child)) return nullptr;
      auto* icon = dynamic_cast<ui::IconView*>(child);
      if (!icon || !icon->isVisibleTo(&root)) continue;
      if (result) return nullptr;
      result = icon;
    }
    return result;
  };
  for (auto layouts = inspectedRoot->findChildren<QHBoxLayout*>(); auto* layout : std::as_const(layouts)) {
    for (int index = 0; index + 1 < layout->count(); ++index) {
      auto* icon = findIcon(layout->itemAt(index)->widget());
      if (!icon || inspected.contains(icon)) continue;
      const auto* alignment = dynamic_cast<ui::AlignedGraphic*>(layout->itemAt(index)->widget());
      const bool wholeBlock = alignment && alignment->textAnchor() == ui::TextAnchor::VisibleBlock;
      QWidget* reference = alignment ? alignment->reference() : firstLabel(layout->itemAt(index + 1));
      if (!reference) continue;
      if (!inlineRegionFullyVisible(*icon, icon->rect(), root)) {
        clippedPairs.append(QJsonObject{{QStringLiteral("icon"), icon->objectName()},
                                        {QStringLiteral("reference"), reference->objectName()},
                                        {QStringLiteral("reason"), QStringLiteral("not-fully-visible")}});
        continue;
      }
      QList<ui::TextLabel*> labels;
      if (auto* text = qobject_cast<ui::TextLabel*>(reference))
        labels.append(text);
      else if (wholeBlock)
        labels = reference->findChildren<ui::TextLabel*>();
      const auto iconInk = renderedInlineInk(frame, QRectF(icon->mapTo(&root, QPoint()), icon->size()));
      QRectF textInk;
      QRectF textReference;
      bool fontMetrics = false;
      QStringList captions;
      for (auto* text : std::as_const(labels)) {
        if (!text->isVisibleTo(&root) || text->text().isEmpty()) continue;
        const QRectF textRegion = wholeBlock ? text->rect() : text->firstLineBoxBounds();
        if (!inlineRegionFullyVisible(*text, textRegion, root)) {
          clippedPairs.append(QJsonObject{{QStringLiteral("text"), text->text()}, {QStringLiteral("reason"), QStringLiteral("not-fully-visible")}});
          continue;
        }
        // Render a complete block transparently at its real window origin so unrelated
        // overlapping controls cannot enter the text union (for example, a close button).
        const auto ink = wholeBlock ? renderedInkBounds(*text, scale).translated(text->mapTo(&root, QPoint()))
                                    : renderedInlineInk(frame, textRegion.translated(text->mapTo(&root, QPoint())));
        if (ink.isEmpty()) continue;
        textInk = textInk.united(ink);
        const bool stable = text->verticalReference() == ui::TextVerticalReference::FontMetrics;
        fontMetrics = fontMetrics || stable;
        textReference = textReference.united(stable ? text->alignmentBounds(!wholeBlock).translated(text->mapTo(&root, QPoint())) : ink);
        captions.append(text->text());
      }
      if (iconInk.isEmpty() || textReference.isEmpty()) continue;
      const QString caption = captions.join(u'\n');
      inspected.insert(icon);
      const qreal difference = qAbs(qRound((iconInk.center().y() - textReference.center().y()) * scale * 2)) / 2.0;
      const bool passed = difference <= 1;
      pairs.append(QJsonObject{{QStringLiteral("icon"), icon->objectName()},
                               {QStringLiteral("text"), caption},
                               {QStringLiteral("textObject"), reference->objectName()},
                               {QStringLiteral("anchor"), wholeBlock ? QStringLiteral("visible-block") : QStringLiteral("first-line")},
                               {QStringLiteral("fontMetricsReference"), fontMetrics},
                               {QStringLiteral("iconCenter"), iconInk.center().y()},
                               {QStringLiteral("textCenter"), textInk.center().y()},
                               {QStringLiteral("referenceCenter"), textReference.center().y()},
                               {QStringLiteral("differencePhysicalPixels"), difference},
                               {QStringLiteral("passed"), passed}});
      if (!passed) errors += QStringLiteral("%1: %2 icon/text reference differ by %3 physical pixels\n").arg(name, caption).arg(difference);
    }
  }
  for (auto buttons = inspectedRoot->findChildren<QAbstractButton*>(); auto* button : std::as_const(buttons)) {
    if (button->icon().isNull() || button->text().isEmpty() || !button->isVisibleTo(&root)) continue;
    if (const auto* tool = qobject_cast<QToolButton*>(button); tool && tool->toolButtonStyle() == Qt::ToolButtonIconOnly) continue;
    const QRect interior = button->rect().adjusted(4, 3, -4, -3);
    if (!inlineRegionFullyVisible(*button, interior, root)) continue;
    const auto runs = inlineInkRuns(frame, QRectF(interior.translated(button->mapTo(&root, QPoint()))), qCeil(3 * scale));
    if (runs.size() < 2) {
      errors += QStringLiteral("%1: cannot separate actual button icon/text for %2\n").arg(name, button->text());
      continue;
    }
    const bool rtl = button->layoutDirection() == Qt::RightToLeft;
    const auto iconInk = rtl ? runs.back() : runs.front();
    QRectF textInk;
    for (qsizetype index = rtl ? 0 : 1; index < runs.size() - (rtl ? 1 : 0); ++index) textInk = textInk.united(runs.at(index));
    const bool stable = button->property("designTextVerticalReference").value<ui::TextVerticalReference>() == ui::TextVerticalReference::FontMetrics;
    qreal referenceCenter = textInk.center().y();
    if (stable) {
      auto font = button->font();
      font.setWeight(button->property("designRole").toString() == QLatin1String("link") ? QFont::Normal : QFont::Medium);
      if (qobject_cast<QToolButton*>(button)) font.setPixelSize(ui::design::smallFontSize);
      const QFontMetricsF metrics(font);
      const qreal halfStrut = (metrics.ascent() - metrics.descent()) / 2.0;
      const qreal center = button->mapTo(&root, QPoint()).y() + button->height() / 2.0 + button->property("designVerticalAlignmentOffset").toReal();
      referenceCenter = qRound((center + halfStrut) * scale) / scale - metrics.capHeight() / 2.0;
    }
    const qreal difference = qAbs(qRound((iconInk.center().y() - referenceCenter) * scale * 2)) / 2.0;
    const bool passed = difference <= 1;
    pairs.append(QJsonObject{{QStringLiteral("kind"), QStringLiteral("button")},
                             {QStringLiteral("icon"), button->objectName()},
                             {QStringLiteral("text"), button->text()},
                             {QStringLiteral("fontMetricsReference"), stable},
                             {QStringLiteral("iconCenter"), iconInk.center().y()},
                             {QStringLiteral("textCenter"), textInk.center().y()},
                             {QStringLiteral("referenceCenter"), referenceCenter},
                             {QStringLiteral("differencePhysicalPixels"), difference},
                             {QStringLiteral("passed"), passed}});
    if (!passed) errors += QStringLiteral("%1: button %2 icon/text reference differ by %3 physical pixels\n").arg(name, button->text()).arg(difference);
  }
  for (auto fields = inspectedRoot->findChildren<QLineEdit*>(); auto* field : std::as_const(fields)) {
    if (field->actions().isEmpty() || field->text().isEmpty() || !field->isVisibleTo(&root)) continue;
    for (auto buttons = field->findChildren<QAbstractButton*>(); auto* button : std::as_const(buttons)) {
      const bool rtl = field->layoutDirection() == Qt::RightToLeft;
      if (button->icon().isNull() || !button->isVisibleTo(&root) || (button->geometry().center().x() < field->width() / 2) == rtl) continue;
      if (!inlineRegionFullyVisible(*field, field->rect(), root)) continue;
      const auto offset = field->mapTo(&root, QPoint());
      const auto iconInk = renderedInlineInk(frame, QRectF(button->mapTo(&root, QPoint()), button->size()));
      QRect textRegion = field->rect().adjusted(4, 3, -4, -3);
      if (rtl)
        textRegion.setRight(button->geometry().left() - 4);
      else
        textRegion.setLeft(button->geometry().right() + 4);
      const auto textInk = renderedInlineInk(frame, textRegion.translated(offset));
      if (iconInk.isEmpty() || textInk.isEmpty()) continue;
      const qreal difference = qAbs(qRound((iconInk.center().y() - textInk.center().y()) * scale * 2)) / 2.0;
      const bool passed = difference <= 1;
      pairs.append(QJsonObject{{QStringLiteral("kind"), QStringLiteral("field-leading-action")},
                               {QStringLiteral("icon"), field->objectName()},
                               {QStringLiteral("text"), field->text()},
                               {QStringLiteral("iconCenter"), iconInk.center().y()},
                               {QStringLiteral("textCenter"), textInk.center().y()},
                               {QStringLiteral("differencePhysicalPixels"), difference},
                               {QStringLiteral("passed"), passed}});
      if (!passed) errors += QStringLiteral("%1: field %2 icon/text differ by %3 physical pixels\n").arg(name, field->objectName()).arg(difference);
    }
  }
  for (auto views = inspectedRoot->findChildren<QAbstractItemView*>(); auto* view : std::as_const(views)) {
    if (!view->isVisibleTo(&root) || !view->model()) continue;
    for (int row = 0; row < view->model()->rowCount(); ++row)
      for (int column = 0; column < view->model()->columnCount(); ++column) {
        const auto index = view->model()->index(row, column);
        if (index.data().toString().isEmpty() || index.data(presentation::SeparatorRole).toBool() ||
            (index.data(presentation::IconResourceRole).toString().isEmpty() && !index.data(presentation::StatusIconRole).toBool() &&
             index.data(Qt::DecorationRole).value<QIcon>().isNull()))
          continue;
        const auto cell = view->visualRect(index);
        if (!view->viewport()->rect().contains(cell)) continue;
        if (!inlineRegionFullyVisible(*view->viewport(), cell, root)) continue;
        auto region = QRectF(cell.translated(view->viewport()->mapTo(&root, QPoint()))).adjusted(5, 2, -5, -2);
        // This delegate draws an independent status column inside the same model cell.
        if (auto* paths = dynamic_cast<ui::PathEntryListView*>(view)) {
          if (auto* status = paths->findChild<ui::TextLabel*>(QStringLiteral("entryStatusHeading"))) {
            const QRectF statusColumn(status->mapTo(&root, QPoint()), status->size());
            if (view->layoutDirection() == Qt::RightToLeft)
              region.setLeft(std::max(region.left(), statusColumn.right()));
            else
              region.setRight(std::min(region.right(), statusColumn.left()));
          }
        }
        const auto runs = inlineInkRuns(frame, region, qCeil(4 * scale));
        if (runs.size() < 2) continue;
        const bool rtl = view->layoutDirection() == Qt::RightToLeft;
        const auto iconInk = rtl ? runs.back() : runs.front();
        QRectF textInk;
        for (qsizetype run = rtl ? 0 : 1; run < runs.size() - (rtl ? 1 : 0); ++run) textInk = textInk.united(runs.at(run));
        const qreal difference = qAbs(qRound((iconInk.center().y() - textInk.center().y()) * scale * 2)) / 2.0;
        const bool passed = difference <= 1;
        pairs.append(QJsonObject{{QStringLiteral("kind"), QStringLiteral("item")},
                                 {QStringLiteral("view"), view->objectName()},
                                 {QStringLiteral("icon"), index.data(presentation::IconResourceRole).toString()},
                                 {QStringLiteral("text"), index.data().toString()},
                                 {QStringLiteral("iconCenter"), iconInk.center().y()},
                                 {QStringLiteral("textCenter"), textInk.center().y()},
                                 {QStringLiteral("differencePhysicalPixels"), difference},
                                 {QStringLiteral("passed"), passed}});
        if (!passed) errors += QStringLiteral("%1: item %2 icon/text differ by %3 physical pixels\n").arg(name, index.data().toString()).arg(difference);
      }
  }
  if (auto* menu = qobject_cast<QMenu*>(&root)) {
    for (auto actions = menu->actions(); auto* action : std::as_const(actions)) {
      if (!action->isVisible() || action->isSeparator() || action->icon().isNull() || action->text().isEmpty()) continue;
      const auto bounds = menu->actionGeometry(action);
      if (bounds.isEmpty()) continue;
      const auto runs = inlineInkRuns(frame, bounds.adjusted(4, 2, -4, -2), qCeil(3 * scale));
      if (runs.size() < 2) continue;
      const bool rtl = menu->layoutDirection() == Qt::RightToLeft;
      const auto iconInk = rtl ? runs.back() : runs.front();
      QRectF textInk;
      for (qsizetype run = rtl ? 0 : 1; run < runs.size() - (rtl ? 1 : 0); ++run) textInk = textInk.united(runs.at(run));
      const qreal difference = qAbs(qRound((iconInk.center().y() - textInk.center().y()) * scale * 2)) / 2.0;
      const bool passed = difference <= 1;
      pairs.append(QJsonObject{{QStringLiteral("kind"), QStringLiteral("menu")},
                               {QStringLiteral("icon"), action->objectName()},
                               {QStringLiteral("text"), action->text()},
                               {QStringLiteral("iconCenter"), iconInk.center().y()},
                               {QStringLiteral("textCenter"), textInk.center().y()},
                               {QStringLiteral("differencePhysicalPixels"), difference},
                               {QStringLiteral("passed"), passed}});
      if (!passed) errors += QStringLiteral("%1: menu %2 icon/text differ by %3 physical pixels\n").arg(name, action->text()).arg(difference);
    }
  }
  if (const auto directory = qEnvironmentVariable("UWF_INLINE_AUDIT_DIR"); !directory.isEmpty()) {
    QDir output(directory);
    if (!output.mkpath(QStringLiteral("."))) return errors + QStringLiteral("Cannot create audit directory\n");
    const QString stem = name + u'-' + tag + u'-' + QString::number(scale);
    const QString screenshot = output.filePath(stem + QStringLiteral(".png"));
    if (!frame.save(screenshot)) errors += QStringLiteral("Cannot save audit screenshot\n");
    QFile file(output.filePath(stem + QStringLiteral(".json")));
    if (!file.open(QIODevice::WriteOnly)) return errors + QStringLiteral("Cannot save audit results\n");
    file.write(QJsonDocument(QJsonObject{{QStringLiteral("view"), name},
                                         {QStringLiteral("variant"), tag},
                                         {QStringLiteral("dpr"), scale},
                                         {QStringLiteral("screenshot"), screenshot},
                                         {QStringLiteral("pairs"), pairs},
                                         {QStringLiteral("clippedPairs"), clippedPairs}})
                   .toJson());
  }
  return errors;
}

inline QImage captureInlineIconText(QWidget& root) {
  for (auto fields = root.findChildren<QLineEdit*>(); auto* field : std::as_const(fields))
    if (!field->actions().isEmpty()) field->clearFocus();
  return root.grab().toImage();
}

inline QString probeInlineIconText(QWidget& root, const QString& name, const QString& tag, QWidget* scope = nullptr) {
  return probeInlineIconText(root, captureInlineIconText(root), name, tag, scope);
}

// Enter the production popup handlers, then close without triggering any action.
inline QString probeInlineMenus(QWidget& owner, const QString& name, const QString& tag, QWidget* scope = nullptr) {
  auto* inspectedRoot = scope ? scope : &owner;
  QString errors;
  bool selectedItem = false;
  int ordinal = 0;
  for (auto menus = inspectedRoot->findChildren<QMenu*>(); auto* menu : std::as_const(menus)) {
    if (menu->actions().isEmpty()) continue;
    menu->popup(QPoint(100, 100));
    QApplication::processEvents();
    errors += probeInlineIconText(*menu, name + QStringLiteral("-owned-menu-%1").arg(ordinal++), tag);
    menu->close();
  }
  for (auto views = inspectedRoot->findChildren<QAbstractItemView*>(); auto* view : std::as_const(views)) {
    if (!view->isVisibleTo(&owner) || view->contextMenuPolicy() != Qt::CustomContextMenu || !view->model()) continue;
    auto index = view->indexAt(QPoint(20, 8));
    if (!index.isValid()) index = view->model()->index(0, 0);
    if (!index.isValid() || !view->viewport()->rect().contains(view->visualRect(index).center())) continue;
    view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    selectedItem = true;
    QTimer capture;
    capture.setSingleShot(true);
    QObject::connect(&capture, &QTimer::timeout, &owner, [&] {
      auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      if (!menu) return;
      errors += probeInlineIconText(*menu, name + QStringLiteral("-context-") + view->objectName(), tag);
      menu->close();
    });
    capture.start(0);
    emit view->customContextMenuRequested(view->visualRect(index).center());
  }
  if (selectedItem) {
    QApplication::processEvents();
    errors += probeInlineIconText(owner, name, tag + QStringLiteral("-selected"), scope);
  }
  return errors;
}
}  // namespace uwf::test
