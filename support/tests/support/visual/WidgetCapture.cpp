// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WidgetCapture.h"

#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QEvent>
#include <QGraphicsEffect>
#include <QImage>
#include <QPixmap>
#include <QScreen>
#include <QTest>
#include <QWidget>
#include <utility>

#include "AnimationClock.h"
#include "VisualGeometry.h"
#include "application/diagnostics/ProcessLogSource.h"
#include "presentation/viewmodels/diagnostics/LogViewModel.h"

namespace uwf::test {

bool visualComparisonEnabled() { return qEnvironmentVariableIsSet("UWF_TEST_VISUAL_FIXTURE_ROOT"); }

namespace {

template <typename Capture>
QPixmap stableNativeScreenshot(Capture capture) {
  if (!visualComparisonEnabled()) return capture();
  QImage previous;
  QPixmap frame;
  int unchangedFrames = 0;
  for (int pass = 0; pass < 100; ++pass) {
    QGuiApplication::sync();
    QTest::qWait(20);
    frame = capture();
    const auto current = frame.toImage();
    unchangedFrames = !current.isNull() && current == previous ? unchangedFrames + 1 : 0;
    if (unchangedFrames >= 5) return frame;
    previous = current;
  }
  QTest::qFail("The native window did not reach a stable composited frame.", __FILE__, __LINE__);
  return frame;
}

class VisualWindowPlacement final : public QObject {
 public:
  using QObject::QObject;

 protected:
  bool eventFilter(QObject* object, QEvent* event) override {
    if (event->type() == QEvent::Show) {
      auto* widget = qobject_cast<QWidget*>(object);
      // Preserve explicit test positions; supply a capture position only for unmoved windows.
      if (widget && widget->isWindow() && !widget->testAttribute(Qt::WA_Moved) && widget->windowType() != Qt::Popup && widget->windowType() != Qt::ToolTip) {
        auto* screen = QGuiApplication::primaryScreen();
        const QString requested = qEnvironmentVariable("UWF_TEST_SCREEN_NAME");
        for (auto screens = QGuiApplication::screens(); auto* candidate : std::as_const(screens))
          if (candidate->name() == requested) screen = candidate;
        widget->setScreen(screen);
        widget->move(screen->availableGeometry().topLeft() + QPoint(30, 30));
      }
    }
    return false;
  }
};

}  // namespace

void initializeVisualComparison() {
  if (!visualComparisonEnabled()) return;
  QApplication::setCursorFlashTime(0);
  qApp->installEventFilter(new VisualWindowPlacement(qApp));
  QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));
  for (auto screens = QGuiApplication::screens(); const auto* screen : std::as_const(screens))
    qInfo() << "visual-screen" << screen->name() << screen->geometry() << "DPR" << screen->devicePixelRatio();
}

QString visualFixtureDirectory(const QString& fallback) {
  if (!visualComparisonEnabled()) return fallback;
  return QDir(qEnvironmentVariable("UWF_TEST_VISUAL_FIXTURE_ROOT")).filePath(QStringLiteral("file-dialog"));
}

namespace {

class ScreenshotLogSource final : public app::LogSource {
 public:
  std::vector<std::string> recentLines() const override {
    auto lines = m_source.recentLines();
    if (visualComparisonEnabled()) {
      for (auto& line : lines) {
        const auto space = line.find(' ');
        if (line.starts_with('[') && space != std::string::npos) line.replace(1, space - 1, "12:34:56.789");
      }
    }
    return lines;
  }
  void clear() override { m_source.clear(); }

 private:
  app::ProcessLogSource m_source;
};
}  // namespace

std::shared_ptr<presentation::LogViewModel> screenshotLogModel() {
  return std::make_shared<presentation::LogViewModel>(std::make_shared<ScreenshotLogSource>());
}

void prepareScreenshotWindow(QWidget& widget, const bool preservePointer) {
  QApplication::setCursorFlashTime(0);
  if (!preservePointer) QCursor::setPos(0, 0);
  auto* window = widget.window();
  if (window->windowType() != Qt::Popup) {
    window->raise();
    window->activateWindow();
  }
}

QPixmap screenshot(QWidget& widget, const bool preservePointer) {
  if (!visualComparisonEnabled()) return widget.grab();
  prepareScreenshotWindow(widget, preservePointer);
  // Layout and paint requests can enqueue another layout pass. Capture after
  // consecutive rendered frames agree, including lazy text layout and hover exit.
  QImage previous;
  QPixmap frame;
  for (int pass = 0; pass < 50; ++pass) {
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QTest::qWait(20);
    frame = widget.grab();
    const auto current = frame.toImage();
    if (!previous.isNull() && current == previous) return frame;
    previous = current;
  }
  QTest::qFail("The screenshot did not reach a stable rendered frame.", __FILE__, __LINE__);
  return frame;
}

QRectF renderedInkBounds(QWidget& widget, qreal scale, const QRectF& requestedRegion) {
  if (scale <= 0) scale = widget.devicePixelRatioF();
  auto* surface = widget.window();
  for (auto* ancestor = &widget; ancestor; ancestor = ancestor->parentWidget()) {
    if (const auto* effect = ancestor->graphicsEffect(); effect && effect->isEnabled()) {
      surface = ancestor;
      break;
    }
    if (ancestor->isWindow()) break;
  }
  const auto origin = widget.mapTo(surface, QPoint());
  QImage frame(QSize(qCeil(surface->width() * scale), qCeil(surface->height() * scale)), QImage::Format_ARGB32_Premultiplied);
  frame.setDevicePixelRatio(scale);
  frame.fill(Qt::transparent);
  widget.render(&frame, origin, QRegion(), QWidget::RenderFlags{});
  const auto region = requestedRegion.isEmpty() ? QRectF(origin, widget.size()) : requestedRegion.translated(origin);
  const auto area = (requestedRegion.isEmpty() ? QRectF(region.x() * scale, region.y() * scale, region.width() * scale, region.height() * scale).toAlignedRect()
                                               : pixelCenterRegion(region, scale))
                        .intersected(frame.rect());
  QRect ink;
  for (int y = area.top(); y <= area.bottom(); ++y) {
    const auto* row = reinterpret_cast<const QRgb*>(frame.constScanLine(y));
    for (int x = area.left(); x <= area.right(); ++x)
      if (qAlpha(row[x]) / 255.0 >= .12) ink = ink.united(QRect(x, y, 1, 1));
  }
  if (ink.isEmpty()) return {};
  return QRectF(ink.x() / scale - origin.x(), ink.y() / scale - origin.y(), ink.width() / scale, ink.height() / scale);
}
bool saveScreenshot(QWidget& widget, const QString& state, const bool preservePointer) {
  const QString directory = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (directory.isEmpty()) return true;
  if (!QDir().mkpath(directory)) return false;
  const QString name = state + u'-' + QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral(".png");
  return screenshot(widget, preservePointer).save(QDir(directory).filePath(name));
}

bool saveAnimatedScreenshot(QWidget& widget, const QString& state) {
  if (!visualComparisonEnabled()) return saveScreenshot(widget, state);
  AnimationClock clock;
  clock.advanceBy(400);
  return saveScreenshot(widget, state);
}

QPixmap nativeScreenshot(QWidget& widget) {
  if (visualComparisonEnabled()) {
    static_cast<void>(screenshot(widget));
  }
  return stableNativeScreenshot([&] { return widget.screen()->grabWindow(widget.winId()); });
}

QPixmap windowScreenshot(QWidget& widget) {
  static_cast<void>(screenshot(widget));
  auto* window = widget.window();
  return stableNativeScreenshot([&] {
    auto* screen = window->screen();
    const QRect frame = window->frameGeometry().translated(-screen->geometry().topLeft());
    return screen->grabWindow(0, frame.x(), frame.y(), frame.width(), frame.height());
  });
}

}  // namespace uwf::test
