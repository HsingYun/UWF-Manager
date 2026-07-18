/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "PowerController.h"

#include <QWidget>
#include <exception>

#include "../util/Log.h"
#include "Dialogs.h"
#include "I18n.h"
#include "PowerConfirmDialog.h"

namespace uwf::ui {

using dialogs::warning;

PowerController::PowerController(WmiOperations& session, QWidget* dialogParent, QObject* parent)
    : QObject(parent), m_dialogParent(dialogParent), m_filter(session) {}

void PowerController::safeShutdown() {
  if (!confirmPowerAction(m_dialogParent, PowerAction::Shutdown)) return;
  try {
    const auto row = m_filter.read();
    m_filter.shutdownSystem(row);
  } catch (const std::exception& error) {
    UWF_LOG_E("power") << "safe shutdown failed: error=" << error.what();
    warning(m_dialogParent, I18n::tr("Safe shutdown failed"), I18n::tr("Shutdown failed: %1").arg(QString::fromUtf8(error.what())));
  }
}

void PowerController::safeRestart() {
  if (!confirmPowerAction(m_dialogParent, PowerAction::Restart)) return;

  try {
    const auto row = m_filter.read();
    m_filter.restartSystem(row);
  } catch (const std::exception& error) {
    UWF_LOG_E("power") << "safe restart failed: error=" << error.what();
    warning(m_dialogParent, I18n::tr("Safe restart failed"), I18n::tr("Restart failed: %1").arg(QString::fromUtf8(error.what())));
  }
}

}  // namespace uwf::ui
