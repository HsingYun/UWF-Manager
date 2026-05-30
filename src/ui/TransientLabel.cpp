#include "TransientLabel.h"

#include <QTimer>
#include <QWidget>

namespace uwf::ui {

TransientLabel::TransientLabel(QWidget* label, QObject* parent) : QObject(parent), m_label(label), m_timer(new QTimer(this)) {
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout, this, [this]() {
    m_showing = false;
    applyText(m_baseline);
  });
}

void TransientLabel::applyText(const QString& text) {
  // m_label 可能是 QLabel 或 QTextBrowser——两者都有 setText 槽，用 invokeMethod
  // 统一驱动，避免把本类绑死到某个具体控件类型。同线程下 AutoConnection 即同步调用。
  if (m_label) QMetaObject::invokeMethod(m_label, "setText", Q_ARG(QString, text));
}

void TransientLabel::setBaseline(const QString& text) {
  m_baseline = text;
  // 处于 transient 时不立刻刷 label——临时提示还在显示，等它到点恢复时自然
  // 用新基线。
  if (!m_showing) applyText(m_baseline);
}

void TransientLabel::show(const QString& text) {
  m_timer->stop();
  m_showing = true;
  applyText(text);
}

void TransientLabel::restoreAfter(int delayMs) {
  if (delayMs <= 0) {
    m_timer->stop();
    m_showing = false;
    applyText(m_baseline);
    return;
  }
  m_timer->start(delayMs);
}

void TransientLabel::flash(const QString& text, int msec) {
  show(text);
  restoreAfter(msec);
}

}  // namespace uwf::ui
