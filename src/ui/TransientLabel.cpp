#include "TransientLabel.h"

#include <QLabel>
#include <QTimer>

namespace uwf::ui {

TransientLabel::TransientLabel(QLabel* label, QObject* parent) : QObject(parent), m_label(label), m_timer(new QTimer(this)) {
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout, this, [this]() {
    m_showing = false;
    if (m_label) m_label->setText(m_baseline);
  });
}

void TransientLabel::setBaseline(const QString& text) {
  m_baseline = text;
  // 处于 transient 时不立刻刷 label——临时提示还在显示，等它到点恢复时自然
  // 用新基线。
  if (!m_showing && m_label) m_label->setText(m_baseline);
}

void TransientLabel::show(const QString& text) {
  m_timer->stop();
  m_showing = true;
  if (m_label) m_label->setText(text);
}

void TransientLabel::restoreAfter(int delayMs) {
  if (delayMs <= 0) {
    m_timer->stop();
    m_showing = false;
    if (m_label) m_label->setText(m_baseline);
    return;
  }
  m_timer->start(delayMs);
}

void TransientLabel::flash(const QString& text, int msec) {
  show(text);
  restoreAfter(msec);
}

}  // namespace uwf::ui
