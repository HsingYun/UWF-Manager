// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
// clang-format off
#include <Windows.h>
// clang-format on
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>
#include <QWindow>
#include <array>

#include "presentation/design/surfaces/SurfaceShadow.h"
#include "presentation/design/theme/DesignSystem.h"
#include "support/visual/VisualGeometry.h"

using namespace uwf::ui;

namespace {
class NativeClickButton final : public QPushButton {
 public:
  using QPushButton::QPushButton;
  int spontaneousPresses = 0;
  int spontaneousReleases = 0;

 protected:
  bool event(QEvent* event) override {
    if (event->spontaneous() && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) &&
        static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
      if (event->type() == QEvent::MouseButtonPress)
        ++spontaneousPresses;
      else
        ++spontaneousReleases;
    }
    return QPushButton::event(event);
  }
};
}  // namespace
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
    QVERIFY(uwf::test::colorCoverage(image, Qt::transparent, uwf::test::interiorRegion(surface)) >= uwf::test::kVisualMinimumCoverage);
    const QRectF nearShadow(surface.x() + surface.width() * .25, surface.bottom() + margins.bottom() * .05, surface.width() * .5, margins.bottom() * .1);
    const QRectF farShadow(surface.x() + surface.width() * .25, surface.bottom() + margins.bottom() * .75, surface.width() * .5, margins.bottom() * .1);
    const auto nearColor = uwf::test::regionColor(image, nearShadow);
    const auto farColor = uwf::test::regionColor(image, farShadow);
    QVERIFY(nearColor.alphaF() > farColor.alphaF());
    QVERIFY(nearColor.alphaF() > 0);
    const QRectF outside(0, 0, margins.left() * .25, margins.top() * .25);
    QVERIFY(uwf::test::colorCoverage(image, Qt::transparent, outside) >= uwf::test::kVisualMinimumCoverage);
    qreal visible = 0, lightFringe = 0;
    for (int y = 0; y < image.height(); ++y)
      for (int x = 0; x < image.width(); ++x) {
        const auto color = image.pixelColor(x, y);
        if (color.alphaF() < uwf::test::kVisualColorRatio) continue;
        ++visible;
        if (color.lightnessF() > .4) ++lightFringe;
      }
    QVERIFY(visible > 0);
    QVERIFY2(lightFringe / visible <= uwf::test::kVisualColorRatio, "Shadow contains a light source-mask fringe");
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
  void companionDoesNotInterceptNativeOrQtInput_data() {
    QTest::addColumn<int>("kind");
    QTest::addColumn<bool>("popup");
    for (const auto kind : {ShadowKind::Window, ShadowKind::Popup, ShadowKind::Hud})
      for (const bool popup : {false, true}) {
        const auto name = kind == ShadowKind::Window ? "window" : kind == ShadowKind::Popup ? "popup" : "hud";
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString::fromLatin1(name), popup ? "popup-capture" : "window")))
            << static_cast<int>(kind) << popup;
      }
  }
  void companionDoesNotInterceptNativeOrQtInput() {
    if (QGuiApplication::platformName() != QLatin1String("windows") || qEnvironmentVariable("UWF_TEST_ALLOW_DESKTOP") != QStringLiteral("1"))
      QSKIP("Native input requires the Windows platform and UWF_TEST_ALLOW_DESKTOP=1.");
    QFETCH(int, kind);
    QFETCH(bool, popup);
    QWidget owner(nullptr, popup ? Qt::Popup | Qt::FramelessWindowHint : Qt::Window);
    owner.resize(280, 180);
    NativeClickButton control(QStringLiteral("Action"), &owner);
    control.setGeometry(40, 40, 160, 40);
    installSurfaceShadow(&owner, static_cast<ShadowKind>(kind));
    owner.show();
    QVERIFY(QTest::qWaitForWindowExposed(&owner));
    auto* shadow = owner.findChild<QWidget*>(QStringLiteral("designSurfaceShadow"));
    QVERIFY(shadow);
    QTRY_VERIFY(shadow->isVisible());
    QVERIFY(shadow->windowFlags().testFlag(Qt::WindowTransparentForInput));
    QVERIFY(shadow->testAttribute(Qt::WA_TransparentForMouseEvents));
    const auto nativeShadow = reinterpret_cast<HWND>(shadow->winId());
    const auto styles = GetWindowLongPtrW(nativeShadow, GWL_EXSTYLE);
    QVERIFY((styles & WS_EX_TRANSPARENT) != 0);
    QVERIFY((styles & WS_EX_LAYERED) != 0);
    if (popup) {
      QCOMPARE(QApplication::activePopupWidget(), &owner);
      QTRY_COMPARE(GetCapture(), reinterpret_cast<HWND>(owner.winId()));
    } else {
      owner.raise();
      owner.windowHandle()->requestActivate();
      QVERIFY(QTest::qWaitForWindowActive(&owner));
    }
    // Exposed windows may still be behind another application's HWND. Establish
    // the real desktop target before checking Qt's decoration hit testing.
    const auto nativeOwner = reinterpret_cast<HWND>(owner.winId());
    const auto localPoint = control.mapTo(&owner, control.rect().center());
    const auto dpr = owner.windowHandle()->devicePixelRatio();
    POINT nativePoint{qRound(localPoint.x() * dpr), qRound(localPoint.y() * dpr)};
    QVERIFY(ClientToScreen(nativeOwner, &nativePoint));
    QTRY_COMPARE(GetAncestor(WindowFromPoint(nativePoint), GA_ROOT), nativeOwner);
    const auto point = control.mapToGlobal(control.rect().center());
    QCOMPARE(owner.childAt(localPoint), static_cast<QWidget*>(&control));
    // On Windows, Qt's global lookup can return null when its temporary 1px
    // mask hole does not change topLevelAt. The decoration must never be the
    // Qt target; actual input delivery is verified separately below.
    QVERIFY(QApplication::widgetAt(point) != shadow);
    if (popup) QCOMPARE(GetCapture(), reinterpret_cast<HWND>(owner.winId()));
    QSignalSpy clicks(&control, &QPushButton::clicked);
    QVERIFY((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0);
    QVERIFY(SetCursorPos(nativePoint.x, nativePoint.y));
    std::array<INPUT, 2> inputs{};
    inputs[0].type = inputs[1].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    const auto sent = SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
    if (sent != inputs.size()) (void)SendInput(1, &inputs[1], sizeof(INPUT));
    QCOMPARE(sent, static_cast<UINT>(inputs.size()));
    QTRY_COMPARE(control.spontaneousPresses, 1);
    QTRY_COMPARE(control.spontaneousReleases, 1);
    QCOMPARE(clicks.count(), 1);
  }
};
QTEST_MAIN(SurfaceShadowTests)
#include "SurfaceShadowTests.moc"
