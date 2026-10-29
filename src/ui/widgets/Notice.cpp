// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "Notice.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QVBoxLayout>

#include "IconView.h"
#include "TextLabel.h"
#include "ui/design/ControlShape.h"
#include "ui/design/DesignSystem.h"
namespace uwf::ui {
namespace {
design::Color foreground(const Notice::Tone tone) {
  return tone == Notice::Tone::Warning   ? design::Color::Warning
         : tone == Notice::Tone::Error   ? design::Color::Danger
         : tone == Notice::Tone::Success ? design::Color::Success
                                         : design::Color::Muted;
}
design::Color background(const Notice::Tone tone) {
  return tone == Notice::Tone::Warning   ? design::Color::WarningBg
         : tone == Notice::Tone::Error   ? design::Color::DangerBg
         : tone == Notice::Tone::Success ? design::Color::SuccessBg
                                         : design::Color::SurfaceSoft;
}
}  // namespace
Notice::Notice(QWidget* parent)
    : QWidget(parent),
      m_icon(new IconView({}, 18, design::Color::Muted, this)),
      m_iconHost(new QWidget(this)),
      m_heading(new TextLabel({}, TextStyle::Caption, this)),
      m_text(new TextLabel({}, TextStyle::Caption, this)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* row = new QHBoxLayout(this);
  row->setContentsMargins(15, 14, 15, 14);
  row->setSpacing(11);
  m_icon->setParent(m_iconHost);
  m_icon->show();
  m_iconHost->setFixedWidth(18);
  row->addWidget(m_iconHost);
  auto* copy = new QVBoxLayout();
  copy->setContentsMargins(0, 0, 0, 0);
  copy->setSpacing(3);
  auto font = m_heading->font();
  font.setWeight(QFont::DemiBold);
  m_heading->setFont(font);
  m_heading->setWordWrap(true);
  m_heading->hide();
  m_text->setWordWrap(true);
  for (auto* text : {m_heading, m_text}) {
    text->setLineHeight(1.7);
    text->setAlignment(Qt::AlignLeading | Qt::AlignTop);
    text->installEventFilter(this);
  }
  copy->addWidget(m_heading);
  copy->addWidget(m_text);
  row->addLayout(copy, 1);
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
  setTone(Tone::Neutral);
}
void Notice::setText(const QString& text) { m_text->setText(text); }
void Notice::alignIcon() {
  auto* text = m_heading->isHidden() ? m_text : m_heading;
  const auto line = text->firstLineBoxBounds();
  m_icon->move(0, qRound(text->mapTo(this, QPoint()).y() - m_iconHost->y() + line.center().y() - m_icon->height() / 2.0));
}
bool Notice::event(QEvent* event) {
  const bool handled = QWidget::event(event);
  if (event->type() == QEvent::LayoutRequest) alignIcon();
  return handled;
}
bool Notice::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::Resize || event->type() == QEvent::Show || event->type() == QEvent::FontChange) alignIcon();
  return QWidget::eventFilter(object, event);
}
QString Notice::text() const { return m_text->text(); }
void Notice::setHeading(const QString& text) {
  m_heading->setText(text);
  m_heading->setVisible(!text.isEmpty());
}
void Notice::setTone(const Tone tone) {
  m_tone = tone;
  const QString resource = tone == Tone::Warning   ? QStringLiteral(":/icons/status_warning.svg")
                           : tone == Tone::Error   ? QStringLiteral(":/icons/status_error.svg")
                           : tone == Tone::Success ? QStringLiteral(":/icons/status_success.svg")
                                                   : QStringLiteral(":/icons/info.svg");
  m_icon->setIcon(QIcon(resource));
  m_icon->setColorRole(foreground(tone));
  update();
}
void Notice::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  const auto& colors = DesignSystem::instance();
  auto border = colors.color(m_tone == Tone::Neutral ? design::Color::Line : foreground(m_tone));
  if (m_tone != Tone::Neutral) border.setAlphaF(.2F);
  paintControlShape(painter, rect(), 6, colors.color(background(m_tone)), border);
}
}  // namespace uwf::ui
#include "moc_Notice.cpp"
