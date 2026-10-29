// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "StatusSurface.h"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {
StatusSurface::StatusSurface(QWidget* parent)
    : Surface(design::Color::Footer, parent),
      m_icon(new IconView({}, 14, design::Color::Muted, this)),
      m_text(new TextLabel({}, TextStyle::Eyebrow, this)),
      m_pending(new TextLabel({}, TextStyle::Eyebrow, this)),
      m_access(new TextLabel({}, TextStyle::Eyebrow, this)),
      m_compatibility(new TextLabel(I18n::tr("Compatibility mode"), TextStyle::Eyebrow, this)),
      m_copy(new QPushButton(I18n::tr("Copy"), this)),
      m_discard(new QPushButton(I18n::tr("Discard changes"), this)),
      m_pendingGroup(new QWidget(this)) {
  setObjectName(QStringLiteral("mainStatusSurface"));
  setBorder(Qt::TopEdge);
  setFixedHeight(design::mainStatusHeight);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto* row = new QHBoxLayout(this);
  row->setContentsMargins(21, 7, 21, 7);
  row->setSpacing(8);
  auto* left = new QWidget(this);
  auto* leading = new QHBoxLayout(left);
  leading->setContentsMargins(0, 0, 0, 0);
  leading->setSpacing(8);
  leading->addWidget(m_icon, 0, Qt::AlignVCenter);
  m_text->setObjectName(QStringLiteral("statusBarLabel"));
  m_text->setElideMode(Qt::ElideRight);
  m_text->setProperty("designTone", "muted");
  for (auto* label : {m_text, m_pending, m_access, m_compatibility}) label->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
  leading->addWidget(m_text);
  for (auto* button : {m_copy, m_discard}) {
    button->setProperty("designRole", "link");
    button->setFixedHeight(24);
    auto font = button->font();
    font.setPixelSize(11);
    button->setFont(font);
    button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  }
  m_copy->setObjectName(QStringLiteral("statusCopyButton"));
  m_copy->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
  m_copy->setIconSize({14, 14});
  leading->addSpacing(4);
  leading->addWidget(m_copy, 0, Qt::AlignVCenter);
  leading->addStretch();
  row->addWidget(left, 1);
  auto* pending = new QHBoxLayout(m_pendingGroup);
  pending->setContentsMargins(0, 0, 0, 0);
  pending->setSpacing(6);
  auto* dot = new Surface(design::Color::Warning, m_pendingGroup);
  dot->setFixedSize(6, 6);
  dot->setBorder({}, 3);
  pending->addWidget(dot, 0, Qt::AlignVCenter);
  pending->addWidget(m_pending);
  auto* trailing = new QHBoxLayout();
  trailing->setSpacing(16);
  trailing->addWidget(m_pendingGroup, 0, Qt::AlignVCenter);
  trailing->addWidget(m_discard, 0, Qt::AlignVCenter);
  trailing->addWidget(m_access, 0, Qt::AlignVCenter);
  m_compatibility->setObjectName(QStringLiteral("compatibilityModeBadge"));
  m_compatibility->setColorRole(design::Color::Muted);
  m_compatibility->hide();
  trailing->addWidget(m_compatibility, 0, Qt::AlignVCenter);
  row->addLayout(trailing);
  connect(m_copy, &QPushButton::clicked, this, [this] {
    QApplication::clipboard()->setText(m_detail);
    emit copiedToClipboard();
  });
  connect(m_discard, &QPushButton::clicked, this, &StatusSurface::discardRequested);
  setPending(0);
  setText(I18n::tr("Ready"));
  I18n::bind(this, [this] {
    m_compatibility->setText(I18n::tr("Compatibility mode"));
    m_copy->setText(I18n::tr("Copy"));
    m_discard->setText(I18n::tr("Discard changes"));
    setPending(m_pendingCount);
    setAccess(m_writable);
    if (m_ready) setText(I18n::tr("Ready"));
  });
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
    m_copy->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
    setText(m_displayText);
  });
}
void StatusSurface::setText(const QString& text) {
  m_displayText = text;
  m_ready = text == I18n::tr("Ready");
  // Hover hints and selected records can contain multiple lines. Keep the
  // footer a single line; m_detail retains the original text for copying.
  auto display = text;
  display.replace(QStringLiteral("\r\n"), QStringLiteral(" "));
  display.replace(u'\r', u' ').replace(u'\n', u' ').replace(QChar::LineSeparator, u' ').replace(QChar::ParagraphSeparator, u' ');
  m_text->setText(display);
  m_copy->setVisible(!m_detail.isEmpty() && text == m_detail);
  const auto& theme = ThemeManager::instance();
  m_icon->setIcon(theme.icon(text == I18n::tr("Ready") ? QStringLiteral(":/icons/status_success.svg") : QStringLiteral(":/icons/info.svg")));
}
void StatusSurface::setDetail(const QString& detail) {
  m_detail = detail;
  setText(detail.isEmpty() ? I18n::tr("Ready") : detail);
}
void StatusSurface::setPending(const std::size_t count) {
  m_pendingCount = count;
  m_pending->setText(I18n::tr("%1 pending change(s)").arg(count));
  m_pendingGroup->setVisible(count > 0);
  m_discard->setVisible(count > 0);
}
void StatusSurface::setAccess(const bool writable) {
  m_writable = writable;
  m_access->setText(writable ? QString{} : I18n::tr("Read-only"));
  m_access->setVisible(!writable);
}
void StatusSurface::setCompatibility(const bool compatible) { m_compatibility->setVisible(compatible); }
}  // namespace uwf::ui
#include "moc_StatusSurface.cpp"
