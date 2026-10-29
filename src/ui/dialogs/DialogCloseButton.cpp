// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DialogCloseButton.h"

#include <QEvent>
#include <QKeyEvent>

#include "DialogAppearance.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/DesignSystem.h"

namespace uwf::ui {
DialogCloseButton::DialogCloseButton(QWidget* parent) : QPushButton(parent) {
  setObjectName(QStringLiteral("dialogCloseButton"));
#if QT_CONFIG(accessibility)
  I18n::bind(this, [this] { setAccessibleName(I18n::tr("Close")); });
#endif
  setIconSize(QSize(design::closeIconSize, design::closeIconSize));
  setFixedSize(design::closeSize, design::closeSize);
  setAutoDefault(false);
  setDialogAction(this, DialogAction::Close);
  ensurePolished();
  setMouseTracking(false);
  updateIcon();
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] { updateIcon(); });
}

bool DialogCloseButton::event(QEvent* event) {
  const bool handled = QPushButton::event(event);
  switch (event->type()) {
    case QEvent::Enter:
    case QEvent::Leave:
    case QEvent::EnabledChange:
    case QEvent::Polish:
    case QEvent::StyleChange:
      // The whole square is interactive, including the area outside the native bevel.
      setMouseTracking(false);
      updateIcon();
      break;
    default:
      break;
  }
  return handled;
}

void DialogCloseButton::keyPressEvent(QKeyEvent* event) {
  if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && event->modifiers() == Qt::NoModifier) {
    click();
    event->accept();
    return;
  }
  QPushButton::keyPressEvent(event);
}

void DialogCloseButton::updateIcon() {
  setIcon(ThemeManager::iconWithColor(QStringLiteral(":/icons/close.svg"), DesignSystem::instance().color(design::Color::Muted), Qt::AlignCenter));
}
}  // namespace uwf::ui
