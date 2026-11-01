// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/navigation/Disclosure.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include <QVariantAnimation>

#include "presentation/controls/layout/Surface.h"
#include "presentation/design/painting/BadgePainting.h"
#include "presentation/design/painting/IconPainting.h"
#include "presentation/design/painting/TextPainting.h"
#include "presentation/design/theme/DesignSystem.h"

namespace uwf::ui {
namespace {
class DisclosureToggle final : public QAbstractButton {
 public:
  explicit DisclosureToggle(const QString& title, QWidget* parent) : QAbstractButton(parent) {
    setText(title);
    setCheckable(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    auto type = font();
    type.setPixelSize(12);
    setFont(type);
    m_rotation.setDuration(design::switchDurationMs);
    QEasingCurve easing(QEasingCurve::BezierSpline);
    easing.addCubicBezierSegment(QPointF(.25, .1), QPointF(.25, 1), QPointF(1, 1));
    m_rotation.setEasingCurve(easing);
    connect(&m_rotation, &QVariantAnimation::valueChanged, this, qOverload<>(&QWidget::update));
    connect(this, &QAbstractButton::toggled, this, [this](const bool open) {
      const qreal start = m_rotation.currentValue().toReal();
      m_rotation.stop();
      m_rotation.setStartValue(start);
      m_rotation.setEndValue(open ? 90.0 : 0.0);
      m_rotation.start();
    });
    connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
  }
  void setCount(const int count) {
    m_count = count;
    updateGeometry();
    update();
  }
  QSize sizeHint() const override {
    auto font = this->font();
    font.setPixelSize(11);
    return {fontMetrics().horizontalAdvance(text()) + 23 + (m_count >= 0 ? countBadgeSize(QString::number(m_count), font).width() + 8 : 0), 20};
  }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(DesignSystem::instance().color(design::Color::Muted));
    painter.setFont(font());
    const bool rtl = layoutDirection() == Qt::RightToLeft;
    paintTextLine(painter, rect().adjusted(rtl ? 0 : 23, 0, rtl ? -23 : 0, 0), text(), font());
    if (m_count >= 0) {
      auto countFont = font();
      countFont.setPixelSize(11);
      countFont.setWeight(QFont::Normal);
      const QString value = QString::number(m_count);
      const auto size = countBadgeSize(value, countFont);
      const QRect badge = QStyle::visualRect(
          layoutDirection(), rect(), QRect(23 + fontMetrics().horizontalAdvance(text()) + 8, (height() - size.height()) / 2, size.width(), size.height()));
      paintCountBadge(painter, badge, value, countFont);
    }
    painter.translate(rtl ? width() - 7.5 : 7.5, height() / 2.0);
    painter.rotate((rtl ? 180 : 0) + m_rotation.currentValue().toReal());
    QPainterPath path;
    path.moveTo(-1.9, -3.75);
    path.lineTo(1.9, 0);
    path.lineTo(-1.9, 3.75);
    paintIconPath(painter, path, QRectF(-7.5, -7.5, 15, 15), DesignSystem::instance().color(design::Color::Muted), 1.1);
  }

 private:
  QVariantAnimation m_rotation;
  int m_count = -1;
};
}  // namespace
Disclosure::Disclosure(const QString& title, QWidget* parent) : QWidget(parent), m_toggle(new DisclosureToggle(title, this)), m_layout(new QVBoxLayout(this)) {
  m_layout->setContentsMargins(0, 0, 0, 0);
  m_layout->setSpacing(0);
  auto* line = new Surface(design::Color::Line, this);
  line->setFixedHeight(1);
  m_layout->addWidget(line);
  m_layout->addSpacing(14);
  m_layout->addWidget(m_toggle);
  connect(m_toggle, &QAbstractButton::toggled, this, [this](const bool open) {
    const QPointer self(this);
    if (m_content) m_content->setVisible(open);
    if (!self) return;
    syncContentLayout();
    if (self) emit openChanged(open);
  });
}
void Disclosure::setContent(QWidget* content) {
  if (m_content == content) return;
  const QPointer self(this);
  const QPointer previous(m_content);
  m_content = content;
  // Adopt first: the new content can itself be a child of the retired body.
  if (content) m_layout->addWidget(content);
  if (!self) return;
  if (previous && previous != m_content) {
    m_layout->removeWidget(previous);
    previous->hide();
    if (!self) return;
    // A hide observer may install a newer body or restore the previous one.
    // Keep retired widgets parent-owned until deferred destruction completes.
    if (previous && previous != m_content) previous->deleteLater();
  }
  if (m_content) m_content->setVisible(isOpen());
  if (self) syncContentLayout();
}
void Disclosure::syncContentLayout() {
  if (!isVisible()) return;
  const QPointer self(this);
  // Showing a body activates ancestor layouts at their previous heights.
  // Resolve scroll content before QAbstractButton's synchronous release repaint.
  for (auto* ancestor = parentWidget(); ancestor && !ancestor->isWindow(); ancestor = ancestor->parentWidget()) {
    const QPointer<QScrollArea> scroll = qobject_cast<QScrollArea*>(ancestor);
    if (!scroll) continue;
    m_layout->activate();
    if (!self || !scroll) return;
    if (auto* content = scroll->widget(); content && content->layout()) content->layout()->activate();
    if (self && scroll) {
      QEvent request(QEvent::LayoutRequest);
      QCoreApplication::sendEvent(scroll, &request);
    }
    return;
  }
}
void Disclosure::setOpen(const bool open) { m_toggle->setChecked(open); }
void Disclosure::setCount(const int count) { static_cast<DisclosureToggle*>(m_toggle)->setCount(count); }
bool Disclosure::isOpen() const { return m_toggle->isChecked(); }
}  // namespace uwf::ui
#include "moc_Disclosure.cpp"
