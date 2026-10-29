// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ContentState.h"

#include <QHBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

#include "BusyIndicator.h"
#include "FlexibleColumnLayout.h"
#include "IconView.h"
#include "TextLabel.h"
#include "VerticalScrollArea.h"

namespace uwf::ui {
ContentState::ContentState(QWidget* parent, const ScrollMode mode)
    : QWidget(parent),
      m_icon(new IconView({}, 34, design::Color::Faint, this)),
      m_spinner(new BusyIndicator(this)),
      m_title(new TextLabel({}, TextStyle::Section, this)),
      m_description(new TextLabel({}, TextStyle::Caption, this)),
      m_progressHost(new QWidget(this)),
      m_progress(new QProgressBar(m_progressHost)),
      m_action(new QPushButton(this)),
      m_scrollMode(mode),
      m_layout(new FlexibleColumnLayout(nullptr)) {
  if (mode == ScrollMode::SelfManaged) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new VerticalScrollArea(this);
    scroll->setProperty("designTransparent", true);
    m_layoutHost = new QWidget(scroll);
    m_layoutHost->setLayout(m_layout);
    scroll->setWidget(m_layoutHost);
    m_layoutHost->setAutoFillBackground(false);
    outer->addWidget(scroll);
  } else {
    m_layoutHost = this;
    setLayout(m_layout);
  }
  setMinimumHeight(180);
  m_layout->setContentsMargins(24, 36, 24, 36);
  m_layout->setSpacing(0);
  m_layout->addStretch();
  auto* symbols = new QWidget(this);
  auto* symbolLayout = new QVBoxLayout(symbols);
  symbolLayout->setContentsMargins(0, 0, 0, 16);
  symbolLayout->setSpacing(0);
  m_spinner->setFixedSize(34, 34);
  m_spinner->setStrokeWidth(1.3);
  m_spinner->setColorRole(design::Color::Faint);
  symbolLayout->addWidget(m_icon, 0, Qt::AlignHCenter);
  symbolLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
  m_layout->addWidget(symbols);
  auto title = m_title->font();
  title.setPixelSize(16);
  m_title->setFont(title);
  for (auto* text : {m_title, m_description}) {
    text->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    text->setWordWrap(true);
    text->setMaximumWidth(420);
  }
  m_description->setColorRole(design::Color::Muted);
  m_layout->addWidget(m_title, 0, Qt::AlignHCenter);
  m_layout->addSpacing(7);
  m_layout->addWidget(m_description, 0, Qt::AlignHCenter);
  m_layout->addSpacing(18);
  auto* progressLayout = new QVBoxLayout(m_progressHost);
  progressLayout->setContentsMargins(0, 8, 0, 20);
  m_progress->setFixedSize(220, 4);
  m_progress->setTextVisible(false);
  m_progress->setRange(0, 0);
  progressLayout->addWidget(m_progress, 0, Qt::AlignHCenter);
  m_layout->addWidget(m_progressHost);
  m_layout->addWidget(m_action, 0, Qt::AlignHCenter);
  m_layout->addStretch();
  connect(m_action, &QPushButton::clicked, this, &ContentState::actionRequested);
  setLoading(false);
  setAction({});
}
void ContentState::setText(const QString& title, const QString& description) {
  m_title->setText(title);
  m_description->setText(description);
}
void ContentState::setIcon(const QIcon& icon, const design::Color color) {
  m_icon->setIcon(icon);
  m_icon->setColorRole(color);
}
void ContentState::setLoading(const bool loading) {
  m_spinner->setVisible(loading);
  m_icon->setVisible(!loading);
  m_progressHost->setVisible(loading);
}
void ContentState::setAction(const QString& text, const QIcon& icon) {
  m_action->setText(text);
  m_action->setIcon(icon);
  m_action->setVisible(!text.isEmpty());
}
void ContentState::setDetails(QWidget* details) {
  if (m_details == details) return;
  if (m_detailsRow) {
    m_layout->removeWidget(m_detailsRow);
    m_detailsRow->hide();
    m_detailsRow->deleteLater();
    m_detailsRow = nullptr;
  }
  m_details = details;
  if (details) {
    details->setContentsMargins(0, 24, 0, 0);
    details->setMaximumWidth(460);
    m_detailsRow = new QWidget(m_layoutHost);
    auto* row = new QHBoxLayout(m_detailsRow);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(0);
    row->addStretch();
    // Fill the available width up to the cap; the side stretches center any remainder.
    // Keeping the minimum width free lets the scroll host shrink and remeasure wrapping.
    row->addWidget(details, 1);
    row->addStretch();
    m_layout->insertWidget(m_layout->count() - 1, m_detailsRow);
  }
}
bool ContentState::hasHeightForWidth() const { return m_scrollMode == ScrollMode::HostManaged; }
int ContentState::heightForWidth(const int width) const { return hasHeightForWidth() ? m_layout->totalHeightForWidth(width) : QWidget::heightForWidth(width); }
}  // namespace uwf::ui
#include "moc_ContentState.cpp"
