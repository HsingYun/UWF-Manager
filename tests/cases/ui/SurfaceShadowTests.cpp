// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QTest>
#include <QWidget>

#include "ui/design/DesignSystem.h"
#include "ui/design/SurfaceShadow.h"

using namespace uwf::ui;
class SurfaceShadowTests final : public QObject {
  Q_OBJECT
 private slots:
  void transparentInteriorAndSmoothColoredFalloff_data() {
    QTest::addColumn<bool>("dark");
    QTest::addColumn<qreal>("dpr");
    QTest::addColumn<bool>("popup");
    for (const bool dark : {false, true})
      for (const qreal dpr : {1.0, 1.25, 2.0})
        for (const bool popup : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(dark).arg(dpr).arg(popup))) << dark << dpr << popup;
  }
  void transparentInteriorAndSmoothColoredFalloff() {
    QFETCH(bool, dark);
    QFETCH(qreal, dpr);
    QFETCH(bool, popup);
    DesignSystem::instance().setLight(!dark);
    const auto kind = popup ? ShadowKind::Popup : ShadowKind::Window;
    const auto margins = surfaceShadowMargins(kind);
    const QSize content(520, popup ? 160 : 400);
    const QSize logical = content + QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    QImage image(QSize(qCeil(logical.width() * dpr), qCeil(logical.height() * dpr)), QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dpr);
    image.fill(Qt::transparent);
    const QRectF surface(margins.left(), margins.top(), content.width(), content.height());
    {
      QPainter painter(&image);
      paintSurfaceShadow(painter, surface, kind);
    }
    const auto pixel = [&](const int x, const int y) { return image.pixelColor(qRound(x * dpr), qRound(y * dpr)); };
    for (int y = margins.top() + 12; y < margins.top() + content.height() - 12; y += 7)
      for (int x = margins.left() + 12; x < margins.left() + content.width() - 12; x += 7) QCOMPARE(pixel(x, y).alpha(), 0);
    const int center = margins.left() + content.width() / 2;
    const int bottom = margins.top() + content.height();
    QVERIFY(pixel(center, bottom + 2).alpha() > pixel(center, bottom + 40).alpha());
    QVERIFY(pixel(center, bottom + 2).alpha() > 0);
    QCOMPARE(pixel(0, 0).alpha(), 0);
    for (int y = 0; y < image.height(); ++y)
      for (int x = 0; x < image.width(); ++x) {
        const auto color = image.pixelColor(x, y);
        if (color.alpha() < 12) continue;
        QVERIFY2(color.red() < 80 && color.green() < 80 && color.blue() < 90, "Shadow contains a light source-mask fringe");
      }
    const QString root = qEnvironmentVariable("UWF_SHADOW_CAPTURE_ROOT");
    if (!root.isEmpty()) {
      QVERIFY(QDir().mkpath(root));
      QVERIFY(image.save(QDir(root).filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
  }
  void companionFollowsOwnerAndCannotTakeFocus() {
    QWidget owner;
    owner.resize(520, 400);
    installSurfaceShadow(&owner, ShadowKind::Window);
    owner.show();
    QVERIFY(QTest::qWaitForWindowExposed(&owner));
    auto* shadow = owner.findChild<QWidget*>(QStringLiteral("designSurfaceShadow"));
    QVERIFY(shadow);
    QTRY_VERIFY(shadow->isVisible());
    QVERIFY(shadow->windowFlags().testFlag(Qt::WindowTransparentForInput));
    QVERIFY(shadow->windowFlags().testFlag(Qt::WindowDoesNotAcceptFocus));
    const auto margins = surfaceShadowMargins(ShadowKind::Window);
    QTRY_COMPARE(shadow->geometry(), QRect(owner.mapToGlobal(QPoint()), owner.size()).marginsAdded(margins));
    owner.resize(600, 450);
    QTRY_COMPARE(shadow->geometry(), QRect(owner.mapToGlobal(QPoint()), owner.size()).marginsAdded(margins));
    owner.hide();
    QTRY_VERIFY(!shadow->isVisible());
  }
};
QTEST_MAIN(SurfaceShadowTests)
#include "SurfaceShadowTests.moc"
