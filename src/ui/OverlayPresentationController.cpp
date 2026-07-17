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
#include "OverlayPresentationController.h"

#include <QAction>
#include <QMainWindow>
#include <QSignalBlocker>
#include <QTimer>
#include <exception>

#include "../util/Log.h"
#include "GlobalStatusPanel.h"
#include "OverlayHub.h"
#include "OverlayHubFactory.h"
#include "ThemeManager.h"
#include "TrayController.h"

namespace uwf::ui {

OverlayPresentationController::OverlayPresentationController(WmiSession& session, QMainWindow& ownerWindow, TrayController& tray, QObject* parent)
    : QObject(parent),
      m_ownerWindow(ownerWindow),
      m_tray(tray),
      m_hub(createDefaultOverlayHub()),
      m_usageTimer(new QTimer(this)),
      m_filter(session),
      m_overlay(session),
      m_overlayConfig(session) {
  connect(m_hub.get(), &OverlayHub::showMainWindowRequested, this, &OverlayPresentationController::activateMainWindowRequested);
  connect(m_hub.get(), &OverlayHub::safeShutdownRequested, this, &OverlayPresentationController::safeShutdownRequested);
  connect(m_hub.get(), &OverlayHub::safeRestartRequested, this, &OverlayPresentationController::safeRestartRequested);
  connect(m_hub.get(), &OverlayHub::exitApplicationRequested, this, &OverlayPresentationController::exitApplicationRequested);
  connect(m_hub.get(), &OverlayHub::stateChanged, this, &OverlayPresentationController::syncAvailability);
  connect(&m_tray, &TrayController::refreshRequested, this, &OverlayPresentationController::refreshUsage);

  m_usageTimer->setInterval(5000);
  connect(m_usageTimer, &QTimer::timeout, this, &OverlayPresentationController::refreshUsage);
  syncAvailability();
}

OverlayPresentationController::~OverlayPresentationController() = default;

void OverlayPresentationController::bindUi(GlobalStatusPanel* global, QAction* displaysAction) {
  m_global = global;
  m_action = displaysAction;
  syncAvailability();
}

void OverlayPresentationController::unbindUi() {
  m_global = nullptr;
  m_action = nullptr;
}

void OverlayPresentationController::applySnapshot(const core::UwfSnapshot& snapshot) {
  m_usageRefreshFailed = false;
  if (snapshot.uwfAvailable) {
    m_hasCommittedSnapshot = true;
    if (snapshot.current.filter.enabled)
      publishUsageState(OverlayUsageEnabled{snapshot.runtime, snapshot.current.overlay});
    else
      publishUsageState(OverlayUsageDisabled{});
    m_usageTimer->start();
  } else {
    m_hasCommittedSnapshot = false;
    publishUsageState(OverlayUsageUnavailable{});
    m_usageTimer->stop();
  }
  syncAvailability();
}

bool OverlayPresentationController::hubEnabled() const { return m_hub->enabled(); }

bool OverlayPresentationController::hubPresented() const { return m_hub->presented(); }

void OverlayPresentationController::setHubEnabled(const bool enabled) { m_hub->setRequestedVisible(enabled); }

void OverlayPresentationController::hideHubTemporarily() { m_hub->hideTemporarily(); }

void OverlayPresentationController::restoreHub() { m_hub->restoreAfterTemporaryHide(); }

void OverlayPresentationController::refreshActionIcon() {
  if (!m_action) return;
  constexpr Qt::Alignment kCenter = Qt::AlignHCenter | Qt::AlignVCenter;
  m_action->setIcon(ThemeManager::instance().icon(m_hub->enabled() ? ":/icons/overlay_float_on.svg" : ":/icons/overlay_float_off.svg", kCenter));
}

void OverlayPresentationController::refreshUsage() {
  // 增量刷新必须建立在一份完整快照上。构造期或首次读取失败时，禁止只凭
  // Filter/Overlay 三个运行时字段提前改变 Hub/托盘，避免不同 UI 消费者对
  // “是否已有可信状态”得出相反结论。
  if (!m_hasCommittedSnapshot) return;

  OverlayUsageState candidate{OverlayUsageDisabled{}};
  try {
    const api::FilterRow filter = m_filter.read();
    if (filter.currentEnabled) {
      const auto overlay = m_overlay.read();
      const auto config = m_overlayConfig.read(api::Session::Current);
      core::OverlayRuntime runtime;
      runtime.currentConsumptionMb = overlay.overlayConsumption;
      runtime.availableSpaceMb = overlay.availableSpace;
      core::OverlayConfig presentationConfig;
      presentationConfig.maximumSizeMb = config.maximumSize;
      presentationConfig.type = config.type == api::OverlayType::RAM ? core::OverlayType::RAM : core::OverlayType::Disk;
      presentationConfig.warningThresholdMb = overlay.warningOverlayThreshold;
      presentationConfig.criticalThresholdMb = overlay.criticalOverlayThreshold;
      candidate = OverlayUsageEnabled{runtime, presentationConfig};
    }
  } catch (const std::exception& error) {
    // 周期刷新失败可能持续多个 tick；同一故障周期只发一条 Warning，后续重复
    // 仅保留在 Debug，避免 Release 日志每五秒刷屏。
    if (!m_usageRefreshFailed)
      UWF_LOG_W("hub") << "usage refresh failed: committedState=retained error=" << error.what();
    else
      UWF_LOG_D("hub") << "usage refresh still failing: committedState=retained error=" << error.what();
    m_usageRefreshFailed = true;
    return;
  }

  if (m_usageRefreshFailed) UWF_LOG_I("hub") << "usage refresh recovered";
  m_usageRefreshFailed = false;

  // 候选读取全部成功后一次性提交到所有消费者，Hub、主面板与托盘不会看见
  // 来自不同轮次的混合状态。
  if (const auto* const enabled = std::get_if<OverlayUsageEnabled>(&candidate); enabled && m_ownerWindow.isVisible() && m_global)
    m_global->updateUsage(enabled->runtime);
  publishUsageState(candidate);
  syncAvailability();
}

void OverlayPresentationController::publishUsageState(const OverlayUsageState& state) {
  m_hub->applyUsageState(state);
  m_tray.applyUsageState(state);
}

void OverlayPresentationController::syncAvailability() {
  if (m_action) {
    m_action->setVisible(m_hub->available());
    if (m_action->isChecked() != m_hub->enabled()) {
      const QSignalBlocker blocker(m_action);
      m_action->setChecked(m_hub->enabled());
    }
  }
  refreshActionIcon();
}

}  // namespace uwf::ui
