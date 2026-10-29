// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QCursor>
#include <QElapsedTimer>
#include <QEvent>
#include <QGuiApplication>
#include <QPointer>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>
#include <functional>
#include <utility>

#include "Surface.h"
#include "TextLabel.h"

namespace uwf::ui {
class HoverToolTip final : public Surface {
 public:
  static constexpr int delayMs = design::tooltipDelayMs;
  explicit HoverToolTip(QWidget* parent = nullptr) : Surface(design::Color::Surface, parent) {
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_QuitOnClose, false);
    setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 4);
    auto* row = new QVBoxLayout(this);
    row->setContentsMargins(11, 8, 11, 8);
    m_label = new TextLabel({}, TextStyle::Description, this);
    auto font = m_label->font();
    font.setPixelSize(11);
    m_label->setFont(font);
    m_label->setColorRole(design::Color::Muted);
    row->addWidget(m_label);
  }
  TextLabel* textLabel() const { return m_label; }
  void setText(const QString& text) {
    if (m_label->text() == text) return;
    m_label->setText(text);
    adjustSize();
  }
  // Floating HUBs deliberately pass pointer input through. Observe only their
  // visible bounds; no mouse interception or change to HUB dragging is needed.
  static void watchTarget(QWidget* target, std::function<QString()> text) {
    class Watch final : public QObject {
     public:
      Watch(QWidget* target, std::function<QString()> text) : QObject(target), m_target(target), m_text(std::move(text)), m_tip(new HoverToolTip(target)) {
        m_tip->setObjectName(QStringLiteral("hubHoverToolTip"));
        m_timer.setInterval(design::hoverPollIntervalMs);
        connect(&m_timer, &QTimer::timeout, this, [this] { tick(); });
        target->installEventFilter(this);
        if (target->isVisible()) m_timer.start();
      }

     protected:
      bool eventFilter(QObject* object, QEvent* event) override {
        if (event->type() == QEvent::Show) m_timer.start();
        if (event->type() == QEvent::Hide) {
          m_timer.stop();
          m_dwell.invalidate();
          m_tip->hide();
        }
        return QObject::eventFilter(object, event);
      }

     private:
      void tick() {
        const QRect bounds(m_target->mapToGlobal(QPoint()), m_target->size());
        if (!bounds.contains(QCursor::pos())) {
          m_dwell.invalidate();
          m_tip->hide();
          return;
        }
        if (!m_dwell.isValid()) m_dwell.start();
        if (m_dwell.elapsed() < delayMs) return;
        m_tip->setText(m_text());
        auto* screen = m_target->screen();
        const QRect area = screen->availableGeometry();
        const int x = qBound(area.left(), QCursor::pos().x() + 12, qMax(area.left(), area.right() - m_tip->width() + 1));
        const int y = qMax(area.top(), bounds.top() - m_tip->height() - 10);
        m_tip->move(x, y);
        m_tip->show();
      }
      QWidget* m_target;
      std::function<QString()> m_text;
      HoverToolTip* m_tip;
      QTimer m_timer;
      QElapsedTimer m_dwell;
    };
    new Watch(target, std::move(text));
  }

 private:
  TextLabel* m_label;
};
}  // namespace uwf::ui
