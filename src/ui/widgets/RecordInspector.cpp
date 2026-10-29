// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RecordInspector.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtMath>
#include <algorithm>

#include "ContentHintBox.h"
#include "TextLabel.h"
#include "ui/design/DesignSystem.h"

namespace uwf::ui {
RecordInspector::RecordInspector(const QString& title, const int detailLines, QWidget* parent)
    : Surface(design::Color::Surface, parent),
      m_title(new TextLabel(title, TextStyle::Caption, this)),
      m_metadata(new TextLabel({}, TextStyle::Caption, this)),
      m_note(new TextLabel({}, TextStyle::Eyebrow, this)),
      m_text(new ContentHintBox(this)),
      m_actions(new QHBoxLayout()) {
  setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* content = new QVBoxLayout(this);
  content->setContentsMargins(18, 16, 18, 16);
  content->setSpacing(7);
  auto* heading = new QHBoxLayout();
  heading->setSpacing(10);
  m_title->setColorRole(design::Color::Muted);
  m_title->setWordWrap(true);
  auto headingFont = m_title->font();
  headingFont.setWeight(QFont::DemiBold);
  m_title->setFont(headingFont);
  m_metadata->setColorRole(design::Color::Faint);
  m_metadata->setElideMode(Qt::ElideRight);
  m_metadata->hide();
  heading->addWidget(m_title);
  heading->addWidget(m_metadata);
  heading->addStretch();
  m_actions->setContentsMargins(0, 0, 0, 0);
  m_actions->setSpacing(14);
  heading->addLayout(m_actions);
  content->addLayout(heading);
  constexpr qreal lineHeight = 23.4;
  auto font = m_text->font();
  font.setFamilies(DesignSystem::monospaceFamilies());
  font.setPixelSize(13);
  m_text->setFont(font);
  m_text->setLineHeight(lineHeight);
  m_text->setMinimumHeight(qCeil(lineHeight) * std::max(1, detailLines));
  // The enclosing page owns scrolling; selected details retain their complete wrapped height.
  m_text->setProperty("designPlainDocument", true);
  m_text->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  content->addWidget(m_text);
  m_note->setColorRole(design::Color::Faint);
  m_note->setWordWrap(true);
  m_note->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_note->hide();
  content->addWidget(m_note);
}
void RecordInspector::setTitle(const QString& title) { m_title->setText(title); }
void RecordInspector::setMetadata(const QString& text) {
  m_metadata->setText(text);
  m_metadata->setToolTip(text);
  m_metadata->setVisible(!text.isEmpty());
}
void RecordInspector::setNote(const QString& text) {
  m_note->setText(text);
  m_note->setToolTip(text);
  m_note->setVisible(!text.isEmpty());
}
void RecordInspector::setText(const QString& text) { m_text->setPlainText(text); }
QPushButton* RecordInspector::addAction(const QString& text, const QIcon& icon) {
  auto* button = new QPushButton(icon, text, this);
  button->setProperty("designRole", "link");
  button->setProperty("designPaddingX", 0);
  button->setIconSize(QSize(14, 14));
  button->setFixedHeight(20);
  auto type = button->font();
  type.setPixelSize(12);
  button->setFont(type);
  m_actions->addWidget(button);
  return button;
}
}  // namespace uwf::ui
