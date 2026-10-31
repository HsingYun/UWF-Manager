// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayContextMenu.h"

#include <QAction>
#include <QFont>
#include <QWidgetAction>

#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"

namespace uwf::ui {

OverlayContextMenu::OverlayContextMenu(QWidget* parent) : CommandMenu(parent) {
  setObjectName(QStringLiteral("overlayContextMenu"));
  setMinimumWidth(268);
  m_title = addAction(QString());
  m_title->setObjectName(QStringLiteral("overlayMenuTitle"));
  QFont titleFont = m_title->font();
  titleFont.setWeight(QFont::Medium);
  m_title->setFont(titleFont);
  m_title->setEnabled(false);
  m_title->setProperty("designMenuHeading", true);
  addSeparator();
  m_showMain = addAction(QString());
  m_showMain->setObjectName(QStringLiteral("overlayMenuShowMain"));
  m_restorePosition = addAction(QString());
  m_restorePosition->setObjectName(QStringLiteral("overlayMenuRestorePosition"));
  m_restorePosition->setVisible(false);
  m_toggleHub = addAction(QString());
  m_toggleHub->setObjectName(QStringLiteral("overlayMenuToggleHub"));
  addSeparator();
  m_shutdown = addAction(QString());
  m_shutdown->setObjectName(QStringLiteral("overlayMenuSafeShutdown"));
  m_restart = addAction(QString());
  m_restart->setObjectName(QStringLiteral("overlayMenuSafeRestart"));
  addSeparator();
  m_exit = addAction(QString());
  m_exit->setObjectName(QStringLiteral("overlayMenuExit"));
  for (auto* action : {m_showMain, m_restorePosition, m_toggleHub, m_shutdown, m_restart, m_exit}) {
    connect(action, &QAction::triggered, this, [this, action] {
      if (!m_deferSignals) dispatch(action);
    });
  }
  connect(this, &QMenu::aboutToShow, this, &OverlayContextMenu::retranslate);
  retranslate();
  updateActionAvailability();
  I18n::bind(this, [this] { retranslate(); });
}

void OverlayContextMenu::setRestorePositionAvailable(const bool available) { m_restorePosition->setVisible(available); }

void OverlayContextMenu::setHubState(const bool available, const bool enabled) {
  m_hubEnabled = enabled;
  m_toggleHub->setEnabled(available);
  retranslate();
}

void OverlayContextMenu::setOperationAvailability(const bool powerAllowed, const bool exitAllowed) {
  m_powerAllowed = powerAllowed;
  m_exitAllowed = exitAllowed;
  updateActionAvailability();
}
void OverlayContextMenu::updateActionAvailability() {
  m_shutdown->setEnabled(m_powerAllowed);
  m_restart->setEnabled(m_powerAllowed);
  m_exit->setEnabled(m_exitAllowed);
}

QWidgetAction* OverlayContextMenu::addUsageWidget(QWidget* widget) {
  auto* action = new QWidgetAction(this);
  action->setObjectName(QStringLiteral("overlayMenuUsage"));
  action->setDefaultWidget(widget);
  insertAction(m_showMain, action);
  insertSeparator(m_showMain);
  connect(action, &QAction::triggered, this, &OverlayContextMenu::showMainWindowRequested);
  return action;
}

void OverlayContextMenu::execAt(const QPoint& position) {
  // 浮窗菜单保持原有顺序：阻塞菜单退出后再执行可能隐藏 HUB 的操作。
  m_deferSignals = true;
  QAction* selected = QMenu::exec(position);
  m_deferSignals = false;
  dispatch(selected);
}

void OverlayContextMenu::dispatch(QAction* action) {
  if (!action || !action->isEnabled()) return;
  if (action == m_showMain)
    emit showMainWindowRequested();
  else if (action == m_restorePosition)
    emit restorePositionRequested();
  else if (action == m_toggleHub)
    emit toggleHubRequested();
  else if (action == m_shutdown)
    emit safeShutdownRequested();
  else if (action == m_restart)
    emit safeRestartRequested();
  else if (action == m_exit)
    emit exitApplicationRequested();
}

void OverlayContextMenu::retranslate() {
  m_title->setText(QStringLiteral("UWF Manager"));
  m_showMain->setText(I18n::tr("Show main window"));
  m_restorePosition->setText(I18n::tr("Restore default position"));
  m_toggleHub->setText(m_hubEnabled ? I18n::tr("Hide overlay hub") : I18n::tr("Show overlay hub"));
  m_shutdown->setText(I18n::tr("Safe shutdown"));
  m_restart->setText(I18n::tr("Safe restart"));
  m_exit->setText(I18n::tr("Exit application"));
  const auto& theme = ThemeManager::instance();
  m_showMain->setIcon(theme.icon(QStringLiteral(":/icons/disk.svg")));
  m_restorePosition->setIcon(theme.icon(QStringLiteral(":/icons/refresh.svg")));
  m_toggleHub->setIcon(theme.icon(QStringLiteral(":/icons/overlay_float_on.svg")));
  m_shutdown->setIcon(theme.icon(QStringLiteral(":/icons/shutdown.svg")));
  m_restart->setIcon(theme.icon(QStringLiteral(":/icons/restart.svg")));
  m_exit->setIcon(theme.icon(QStringLiteral(":/icons/close.svg")));
}

}  // namespace uwf::ui

#include "moc_OverlayContextMenu.cpp"
