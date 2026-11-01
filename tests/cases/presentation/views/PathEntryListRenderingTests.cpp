// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QItemSelectionModel>
#include <QPainter>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTest>

#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/design/theme/DesignStyle.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/models/ItemRoles.h"
#include "support/visual/VisualGeometry.h"

namespace {
void addRenderingRows() {
  QTest::addColumn<qreal>("scale");
  QTest::addColumn<bool>("light");
  QTest::addColumn<bool>("scrolled");
  QTest::addColumn<bool>("rtl");
  for (const qreal scale : {1.0, 1.25, 1.5, 2.0})
    for (const bool light : {true, false})
      for (const bool scrolled : {false, true})
        for (const bool rtl : {false, true})
          QTest::newRow(
              qUtf8Printable(QStringLiteral("%1-%2-%3-%4").arg(scale).arg(light ? "light" : "dark", scrolled ? "scrolled" : "top", rtl ? "rtl" : "ltr")))
              << scale << light << scrolled << rtl;
}

void populate(QStandardItemModel& model) {
  const QStringList paths{QStringLiteral("C:\\Program Files (x86)\\Microsoft\\EdgeWebView"), QStringLiteral("C:\\Program Files (x86)\\Ubisoft"),
                          QStringLiteral("C:\\Program Files\\Windows Defender")};
  for (int row = 0; row < 12; ++row) {
    auto* item = new QStandardItem(paths.at(row % paths.size()));
    item->setData(QStringLiteral(":/icons/folder.svg"), uwf::presentation::IconResourceRole);
    item->setData(static_cast<int>(uwf::presentation::ItemStatus::Normal), uwf::presentation::StatusRole);
    item->setData(QStringLiteral("Protected"), uwf::presentation::StatusTextRole);
    model.appendRow(item);
  }
}

QImage render(QWidget& widget, const qreal scale) {
  QImage image(QSize(qCeil(widget.width() * scale), qCeil(widget.height() * scale)), QImage::Format_ARGB32_Premultiplied);
  image.setDevicePixelRatio(scale);
  image.fill(Qt::transparent);
  QPainter painter(&image);
  widget.render(&painter);
  return image;
}

class PathEntryListRenderingTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() {
    QApplication::setStyle(new uwf::ui::DesignStyle());
    QApplication::setFont(uwf::ui::DesignSystem::defaultFont());
    QVERIFY(!QIcon(QStringLiteral(":/icons/folder.svg")).isNull());
  }

  void folderIconsMatchPathInk_data() { addRenderingRows(); }
  void folderIconsMatchPathInk() {
    QFETCH(qreal, scale);
    QFETCH(bool, light);
    QFETCH(bool, scrolled);
    QFETCH(bool, rtl);
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QStandardItemModel model;
    populate(model);
    uwf::ui::PathEntryListView view(QStringLiteral("Path"), QStringLiteral("Status"));
    view.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
    view.setModel(&model);
    view.resize(820, 220);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    view.selectionModel()->select(view.model()->index(1, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    if (scrolled) {
      view.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      view.verticalScrollBar()->setValue(13);
      QCOMPARE(view.visualRect(view.model()->index(0, 0)).top(), -13);
    }
    const auto image = render(*view.viewport(), scale);
    if (const auto directory = qEnvironmentVariable("UWF_UI_SCREENSHOT_ROOT"); !directory.isEmpty()) {
      QDir output(directory);
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(render(view, scale).save(output.filePath(QStringLiteral("paths-%1.png").arg(QString::fromLatin1(QTest::currentDataTag())))));
    }
    int checked = 0;
    for (int row = 0; row < view.model()->rowCount(); ++row) {
      const auto index = view.model()->index(row, 0);
      const QRect bounds = view.visualRect(index);
      if (!view.viewport()->rect().contains(bounds)) continue;
      const auto background = design.color(row == 1 ? Color::Selected : Color::Surface);
      const QRect iconRegion = QStyle::visualRect(view.layoutDirection(), bounds, QRect(bounds.left() + 12, bounds.top(), 20, bounds.height()));
      const QRect textRegion =
          QStyle::visualRect(view.layoutDirection(), bounds, QRect(bounds.left() + 39, bounds.top(), bounds.width() - 39 - 120 - 12, bounds.height()));
      const auto iconInk = uwf::test::visibleInkBounds(image, design.color(Color::Muted), background, iconRegion);
      const auto textInk = uwf::test::visibleInkBounds(image, design.color(Color::Text), background, textRegion);
      QVERIFY(!iconInk.isEmpty());
      QVERIFY(!textInk.isEmpty());
      const qreal difference = qAbs(iconInk.center().y() - textInk.center().y());
      QVERIFY2(difference <= 1 / scale, qUtf8Printable(QStringLiteral("row %1: folder center %2, path center %3, difference %4 logical pixels")
                                                           .arg(row)
                                                           .arg(iconInk.center().y())
                                                           .arg(textInk.center().y())
                                                           .arg(difference)));
      ++checked;
    }
    QVERIFY(checked >= 2);
  }

  void selectedIndicatorsStayContinuous_data() { addRenderingRows(); }
  void selectedIndicatorsStayContinuous() {
    QFETCH(qreal, scale);
    QFETCH(bool, light);
    QFETCH(bool, scrolled);
    QFETCH(bool, rtl);
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QStandardItemModel model;
    populate(model);
    uwf::ui::PathEntryListView view(QStringLiteral("Path"), QStringLiteral("Status"));
    view.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
    view.setModel(&model);
    view.resize(820, 220);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    view.selectAll();
    if (scrolled) {
      view.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      view.verticalScrollBar()->setValue(13);
      QCOMPARE(view.visualRect(view.model()->index(0, 0)).top(), -13);
    }
    const auto image = render(*view.viewport(), scale);
    const int x = qFloor((rtl ? view.viewport()->width() - 1.5 : 1.5) * scale);
    // Avoid the viewport's rounded bottom corners; every intermediate row boundary must retain the accent.
    for (int y = qCeil(6 * scale); y < qFloor((view.viewport()->height() - 6) * scale); ++y) QCOMPARE(image.pixelColor(x, y), design.color(Color::Accent));
  }
};
}  // namespace
QTEST_MAIN(PathEntryListRenderingTests)
#include "PathEntryListRenderingTests.moc"
