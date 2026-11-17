// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/overlay/HormUsageDialog.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>
#include <algorithm>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"

namespace uwf::ui {
namespace {
using Color = design::Color;
TextLabel* copy(QWidget* owner, const char* source, const int pixels = 12, const Color color = Color::Muted, const QFont::Weight weight = QFont::Normal) {
  auto* result = new TextLabel({}, TextStyle::Caption, owner);
  auto font = result->font();
  font.setPixelSize(pixels);
  font.setWeight(weight);
  result->setFont(font);
  result->setColorRole(color);
  result->setWordWrap(true);
  result->setLineHeight(1.85);
  I18n::text(result, [source] { return I18n::tr(source); });
  return result;
}
Surface* command(QWidget* owner, const QString& value, const QString& name) {
  auto* surface = new Surface(Color::SurfaceSoft, owner);
  surface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  auto* row = new QVBoxLayout(surface);
  row->setContentsMargins(12, 9, 12, 9);
  auto* text = new TextLabel(value, TextStyle::Caption, surface);
  text->setObjectName(name);
  auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  font.setPixelSize(12);
  text->setFont(font);
  text->setColorRole(Color::Text);
  text->setWordWrap(true);
  text->setLineHeight(1.6);
  text->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  row->addWidget(text);
  return surface;
}
}  // namespace

HormUsageDialog::HormUsageDialog(QWidget* parent) : PanelDialog(parent) {
  setObjectName(QStringLiteral("hormUsageDialog"));
  setHeaderStyle(HeaderStyle::Message);
  setWindowModality(Qt::WindowModal);
  setHeaderMetrics({QMargins(30, 28, 30, 20), 20, 1.5});
  m_frame = new DialogLayout(this, DialogContent::Scrollable, QSize(500, 0), QSize(320, 180));
  m_frame->setAutoSizeToContent(true);
  m_frame->setActionMetrics(28, 15, 68);
  auto* body = m_frame->body();
  body->setContentsMargins(30, 0, 30, 28);
  body->setSpacing(0);

  const auto hero = createDialogHero(this, ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")));
  hero.widget->setObjectName(QStringLiteral("hormUsageHeading"));
  hero.icon->setObjectName(QStringLiteral("hormUsageIcon"));
  hero.heading->setObjectName(QStringLiteral("hormUsageHeroTitle"));
  hero.technical->setObjectName(QStringLiteral("officialName"));
  I18n::bind(hero.widget, [hero] { updateDialogHeroText(hero, I18n::tr("Setup and operation"), QStringLiteral("HORM")); });
  body->addWidget(hero.widget);
  body->addSpacing(20);
  auto* divider = new Surface(Color::Line, this);
  divider->setFixedHeight(1);
  body->addWidget(divider);
  body->addSpacing(18);

  auto step = [&](const char* title, const char* text, const char* name) {
    auto* headingLabel = copy(this, title, 12, Color::Text, QFont::Medium);
    headingLabel->setObjectName(QString::fromLatin1(name) + QStringLiteral("Heading"));
    body->addWidget(headingLabel);
    body->addSpacing(4);
    auto* textLabel = copy(this, text);
    textLabel->setObjectName(QString::fromLatin1(name));
    body->addWidget(textLabel);
  };
  step(QT_TRANSLATE_NOOP("uwf", "1. Prepare the configuration"),
       QT_TRANSLATE_NOOP("uwf",
                         "With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next "
                         "startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file."),
       "hormUsagePrepare");
  body->addSpacing(14);
  step(QT_TRANSLATE_NOOP("uwf", "2. Restart and verify"),
       QT_TRANSLATE_NOOP("uwf",
                         "Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted "
                         "fixed volumes. Confirm that system hibernation is available."),
       "hormUsageVerify");
  body->addSpacing(14);
  step(QT_TRANSLATE_NOOP("uwf", "3. Enable HORM and hibernate"),
       QT_TRANSLATE_NOOP("uwf",
                         "Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:"),
       "hormUsageEnable");
  body->addSpacing(8);
  body->addWidget(command(this, QStringLiteral("uwfmgr filter enable-horm"), QStringLiteral("hormUsageEnableCommand")));
  body->addSpacing(10);
  body->addWidget(
      copy(this, QT_TRANSLATE_NOOP("uwf", "After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:")));
  body->addSpacing(8);
  body->addWidget(command(this, QStringLiteral("shutdown /h"), QStringLiteral("hormUsageHibernateCommand")));

  auto* close = new QPushButton(this);
  close->setObjectName(QStringLiteral("hormUsageClose"));
  close->setFixedHeight(36);
  setDialogAction(close, DialogAction::Secondary);
  bindRejectionButton(close);
  closeButton()->setDefault(false);
  close->setDefault(true);
  close->setFocus(Qt::OtherFocusReason);
  I18n::text(close, [] { return I18n::tr("Close"); });
  m_frame->setActions(close, static_cast<QWidget*>(nullptr));
  connect(close, &QPushButton::clicked, this, &QDialog::reject);
  I18n::bind(this, [this] {
    setWindowTitle(I18n::tr("HORM usage"));
    updateResponsiveLayout();
  });
}

void HormUsageDialog::resizeEvent(QResizeEvent* event) {
  PanelDialog::resizeEvent(event);
  if (m_frame) updateResponsiveLayout();
}
void HormUsageDialog::updateResponsiveLayout() {
  setMaximumHeight(std::max(180, std::min(720, screen()->availableGeometry().height() - 48)));
  const int profile = width() < 500 || screen()->availableGeometry().width() <= 560 ? 1 : 0;
  if (m_layoutProfile == profile) return;
  m_layoutProfile = profile;
  const int padding = profile ? 24 : 30;
  setHeaderMetrics({QMargins(padding, profile ? 24 : 28, padding, 20), 20, 1.5});
  m_frame->body()->setContentsMargins(padding, 0, padding, profile ? 24 : 28);
}
}  // namespace uwf::ui
#include "moc_HormUsageDialog.cpp"
