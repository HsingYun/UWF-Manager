// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TransientLabel.h"

#include <QTimer>
#include <QWidget>

namespace uwf::ui {

TransientLabel::TransientLabel(QWidget* label, QObject* parent) : QObject(parent), m_label(label), m_timer(new QTimer(this)) {
  m_timer->setSingleShot(true);
  m_timer->setTimerType(Qt::PreciseTimer);
  connect(m_timer, &QTimer::timeout, this, [this]() {
    m_showing = false;
    render();
  });
}

void TransientLabel::applyText(const QString& text) {
  // 相同文本不重复设置，避免重排和重复文档测量。
  if (text == m_current) return;
  m_current = text;
  // 通过 setText 槽兼容 QLabel 和 QTextBrowser；同线程调用同步完成。
  if (m_label) QMetaObject::invokeMethod(m_label, "setText", Q_ARG(QString, text));
}

void TransientLabel::setBaseline(const QString& text) {
  m_baseline = text;
  // 临时提示期间仅更新基线，恢复时再显示新值。
  render();
}

void TransientLabel::render() { applyText(m_showing ? m_transient : m_hover.isEmpty() ? m_baseline : m_hover); }

void TransientLabel::show(const QString& text) {
  m_timer->stop();
  m_showing = true;
  m_transient = text;
  render();
}

void TransientLabel::restoreAfter(int delayMs) {
  if (delayMs <= 0) {
    m_timer->stop();
    m_showing = false;
    m_hover.clear();
    render();
    return;
  }
  m_timer->start(delayMs);
}

void TransientLabel::flash(const QString& text, int msec) {
  show(text);
  restoreAfter(msec);
}

void TransientLabel::showHover(const QString& text) {
  m_hover = text;
  render();
}

}  // namespace uwf::ui

#include "moc_TransientLabel.cpp"
