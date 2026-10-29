// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommandBar.h"

#include <QAction>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QToolButton>
#include <utility>
namespace uwf::ui {
CommandBar::CommandBar(QWidget* parent) : Surface(design::Color::Window, parent), m_layout(new QHBoxLayout(this)) {
  setProperty("windowDragSurface", true);
  setBorder(Qt::BottomEdge);
  m_layout->setContentsMargins(17, 8, 17, 14);
  m_layout->setSpacing(4);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}
QAction* CommandBar::addCommand(const QString& text, const bool optional) {
  auto* action = new QAction(text, this);
  auto* button = new QToolButton(this);
  button->setAutoRaise(true);
  button->setProperty("designRole", "quiet");
  button->setDefaultAction(action);
  button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  button->setIconSize({16, 16});
  button->setFixedHeight(32);
  auto font = button->font();
  font.setPixelSize(design::smallFontSize);
  font.setWeight(QFont::Medium);
  button->setFont(font);
  button->setProperty("optionalCommand", optional);
  m_buttons.append(button);
  m_layout->addWidget(button);
  connect(action, &QAction::changed, this, [this] { arrange(); });
  return action;
}
void CommandBar::addSeparator() {
  auto* line = new Surface(design::Color::Line, this);
  line->setFixedSize(1, 20);
  m_layout->addSpacing(8);
  m_layout->addWidget(line);
  m_layout->addSpacing(8);
}
void CommandBar::addStretch() { m_layout->addStretch(); }
QToolButton* CommandBar::buttonForAction(QAction* action) const {
  for (auto* button : std::as_const(m_buttons))
    if (button->defaultAction() == action) return button;
  return nullptr;
}
void CommandBar::resizeEvent(QResizeEvent* event) {
  Surface::resizeEvent(event);
  arrange();
}
void CommandBar::arrange() {
  for (auto* button : std::as_const(m_buttons)) {
    const bool compact = window()->width() <= design::compactWindowWidth;
    const bool iconOnly = compact && button->property("optionalCommand").toBool();
    button->setToolButtonStyle(iconOnly ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextBesideIcon);
    button->setProperty("designPaddingX", compact ? 7 : 10);
    button->setFixedWidth(button->sizeHint().width());
  }
}
}  // namespace uwf::ui

#include "moc_CommandBar.cpp"
