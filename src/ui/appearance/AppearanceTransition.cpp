// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "AppearanceTransition.h"

#include <qt_windows.h>

#include <QApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QPainter>
#include <QPointer>
#include <QScopeGuard>
#include <QScopedValueRollback>
#include <QTimer>
#include <QVariantAnimation>
#include <QWidget>
#include <utility>

#include "shared/Log.h"
#include "ui/design/DesignSystem.h"

namespace uwf::ui {
namespace {
class TransitionLayer final : public QWidget {
 public:
  TransitionLayer(QWidget* window, QPixmap snapshot) : QWidget(window), m_snapshot(std::move(snapshot)) {
    setObjectName(QStringLiteral("appearanceTransition"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    setGeometry(window->rect());
    m_animation.setDuration(design::appearanceDurationMs);
    m_animation.setStartValue(1.0);
    m_animation.setEndValue(0.0);
    m_animation.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
      m_opacity = value.toReal();
      update();
    });
    connect(&m_animation, &QVariantAnimation::finished, this, [this] {
      hide();
      m_snapshot = {};
      deleteLater();
    });
    window->installEventFilter(this);
  }
  void start() {
    show();
    raise();
    m_animation.start();
  }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    painter.setOpacity(m_opacity);
    painter.drawPixmap(QPoint(), m_snapshot);
  }
  bool eventFilter(QObject*, QEvent* event) override {
    if (event->type() == QEvent::Resize || event->type() == QEvent::Hide) {
      m_animation.stop();
      hide();
      m_snapshot = {};
      deleteLater();
    }
    return false;
  }

 private:
  QPixmap m_snapshot;
  QVariantAnimation m_animation;
  qreal m_opacity = 1.0;
};
bool animationsEnabled() {
  BOOL enabled = TRUE;
  return !SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &enabled, 0) || enabled;
}
}  // namespace
void AppearanceTransition::run(const std::function<void()>& update, const bool animateRequested) {
  static bool active = false;
  if (active) {
    update();
    return;
  }
  const QScopedValueRollback guard(active, true);
  QElapsedTimer elapsed;
  elapsed.start();
  struct Frame {
    QPointer<QWidget> window;
    QPixmap snapshot;
  };
  QList<Frame> frames;
  const bool animate = animateRequested && animationsEnabled();
  for (const auto windows = QApplication::topLevelWidgets(); auto* window : windows) {
    if (!window->isVisible() || window->isMinimized() || !window->updatesEnabled() || window->windowType() == Qt::Popup || window->windowType() == Qt::ToolTip)
      continue;
    const bool animateWindow = animate && (window->windowType() == Qt::Window || window->windowType() == Qt::Dialog);
    auto snapshot = animateWindow ? window->grab() : QPixmap{};
    if (!snapshot.isNull()) {
      // QWidget snapshots omit the native Mica backdrop. Give transparent
      // pixels the previous theme's base so text keeps its contrast in flight.
      QPainter background(&snapshot);
      background.setCompositionMode(QPainter::CompositionMode_DestinationOver);
      background.fillRect(window->rect(), window->palette().color(QPalette::Window));
    }
    for (const auto layers = window->findChildren<QWidget*>(QStringLiteral("appearanceTransition"), Qt::FindDirectChildrenOnly); auto* layer : layers) {
      layer->hide();
      layer->deleteLater();
    }
    frames.append({window, snapshot});
    window->setUpdatesEnabled(false);
  }
  const auto captureMs = elapsed.elapsed();
  qint64 updateMs = 0;
  const auto finish = qScopeGuard([&] {
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    for (const auto& frame : std::as_const(frames)) {
      if (!frame.window) continue;
      frame.window->setUpdatesEnabled(true);
      if (animate && frame.window->isVisible() && frame.snapshot.size() / frame.snapshot.devicePixelRatio() == frame.window->size())
        (new TransitionLayer(frame.window, frame.snapshot))->start();
      frame.window->update();
    }
    UWF_LOG_D("appearance") << "display update completed: windows=" << frames.size() << " captureMs=" << captureMs << " updateMs=" << updateMs
                            << " elapsedMs=" << elapsed.elapsed();
  });
  update();
  updateMs = elapsed.elapsed() - captureMs;
}
void AppearanceTransition::request(QObject* owner, std::function<void()> update, std::function<bool()> needsPaint) {
  struct Change {
    QPointer<QObject> owner;
    std::function<void()> update;
    std::function<bool()> needsPaint;
  };
  static QList<Change> pending;
  const bool scheduled = !pending.isEmpty();
  for (auto& change : pending) {
    if (change.owner != owner) continue;
    change.update = std::move(update);
    change.needsPaint = std::move(needsPaint);
    return;
  }
  pending.append({owner, std::move(update), std::move(needsPaint)});
  if (scheduled) return;
  QTimer::singleShot(0, qApp, [] {
    const auto changes = std::exchange(pending, {});
    bool repaint = false;
    for (const auto& change : changes) repaint |= change.owner && (!change.needsPaint || change.needsPaint());
    const auto apply = [&] {
      for (const auto& change : changes)
        if (change.owner) change.update();
    };
    if (repaint)
      run(apply);
    else
      apply();
  });
}
}  // namespace uwf::ui
