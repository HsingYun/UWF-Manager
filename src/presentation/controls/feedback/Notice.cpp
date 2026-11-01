// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/feedback/Notice.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QVBoxLayout>

#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/controls/layout/AlignedGraphic.h"
#include "presentation/design/controls/ControlShape.h"
#include "presentation/design/theme/DesignSystem.h"
namespace uwf::ui {
namespace {
design::Color foreground(const Notice::Tone tone) {
  return tone == Notice::Tone::Warning   ? design::Color::Warning
         : tone == Notice::Tone::Error   ? design::Color::Danger
         : tone == Notice::Tone::Success ? design::Color::Success
         : tone == Notice::Tone::Accent  ? design::Color::Accent
                                         : design::Color::Muted;
}
design::Color background(const Notice::Tone tone) {
  return tone == Notice::Tone::Warning   ? design::Color::WarningBg
         : tone == Notice::Tone::Error   ? design::Color::DangerBg
         : tone == Notice::Tone::Success ? design::Color::SuccessBg
         : tone == Notice::Tone::Accent  ? design::Color::Selected
                                         : design::Color::SurfaceSoft;
}
}  // namespace
Notice::Notice(QWidget* parent)
    : QWidget(parent),
      m_icon(new IconView({}, 18, design::Color::Muted, this)),
      m_iconHost(nullptr),
      m_heading(new TextLabel({}, TextStyle::Caption, this)),
      m_text(new TextLabel({}, TextStyle::Caption, this)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* row = new QHBoxLayout(this);
  row->setContentsMargins(15, 14, 15, 14);
  row->setSpacing(11);
  m_iconHost = new AlignedGraphic(m_icon, m_text, this);
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
  }
  copy->addWidget(m_heading);
  copy->addWidget(m_text);
  row->addLayout(copy, 1);
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
  setTone(Tone::Neutral);
}
void Notice::setText(const QString& text) { m_text->setText(text); }
void Notice::setFlat(const bool flat) {
  m_flat = flat;
  update();
}
void Notice::setIcon(const QIcon& icon) { m_icon->setIcon(icon); }
QString Notice::text() const { return m_text->text(); }
void Notice::setHeading(const QString& text) {
  m_heading->setText(text);
  m_heading->setVisible(!text.isEmpty());
  m_iconHost->setReference(text.isEmpty() ? m_text : m_heading);
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
  if (m_flat) border.setAlpha(0);
  paintControlShape(painter, rect(), 6, colors.color(background(m_tone)), border);
}
}  // namespace uwf::ui
#include "moc_Notice.cpp"
