// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/overlay/OverlayViewModel.h"

#include <QLocale>
#include <QPointer>
#include <exception>
#include <utility>

#include "foundation/logging/Log.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/overlay/OverlayUsagePresentation.h"

namespace uwf::presentation {
using namespace app;
QString overlayUsageToolTip(const app::OverlayUsageState& state) {
  const auto freshness = overlayUsageFreshnessText(state);
  const auto appendFreshness = [&freshness](QString text) { return freshness.isEmpty() ? text : text + u'\n' + freshness; };
  if (const auto* enabled = std::get_if<OverlayUsageEnabled>(&state)) {
    const QLocale numbers(QLocale::English, QLocale::UnitedStates);
    const auto used = enabled->runtime.currentConsumptionMb;
    const auto total = static_cast<qulonglong>(core::overlayTotalMb(enabled->runtime));
    const auto percentage = total == 0 ? 0.0 : static_cast<double>(used) * 100.0 / static_cast<double>(total);
    return appendFreshness(
        QStringLiteral("UWF Manager\n%1% (%2/%3 MB)").arg(QString::number(percentage, 'f', 0), numbers.toString(used), numbers.toString(total)));
  }
  return appendFreshness(QStringLiteral("UWF Manager\n") + (std::holds_alternative<OverlayUsageDisabled>(state)
                                                                ? PresentationText::tr("UWF: Disabled")
                                                                : PresentationText::tr("UWF status unavailable")));
}
OverlayViewModel::OverlayViewModel(std::shared_ptr<OverlayUsageReader> reader, const std::chrono::milliseconds refreshInterval) : m_reader(std::move(reader)) {
  m_usageTimer.setInterval(refreshInterval);
  connect(&m_usageTimer, &QTimer::timeout, this, &OverlayViewModel::refreshUsage);
}
void OverlayViewModel::applySnapshot(const core::UwfSnapshot& snapshot, const bool usageSampled) {
  if (usageSampled || !snapshot.uwfAvailable || !snapshot.current.filter.enabled) m_read.cancel();
  const QPointer guard(this);
  if (usageSampled) m_usageRefreshFailed = false;
  m_currentConfig = snapshot.current.overlay;
  const auto observation = overlayUsageObservation(m_usage);
  const auto retainObservation = [&](auto value) -> OverlayUsageState {
    static_cast<OverlayUsageObservation&>(value) = observation;
    return value;
  };
  if (snapshot.uwfAvailable) {
    m_hasCommittedSnapshot = true;
    if (!observationPaused() && (usageSampled || !m_usageTimer.isActive())) m_usageTimer.start();
    if (snapshot.current.filter.enabled) {
      if (usageSampled) {
        publishUsageState(OverlayUsageEnabled{snapshot.runtime, m_currentConfig});
      } else if (const auto* previous = std::get_if<OverlayUsageEnabled>(&m_usage)) {
        auto projected = *previous;
        projected.config = m_currentConfig;
        publishUsageState(projected, false);
      } else {
        // Configuration can establish that UWF is active, but cannot create a
        // complete usage sample. The independent poll will supply that sample.
        publishUsageState(retainObservation(OverlayUsageUnavailable{}), false);
      }
    } else {
      publishUsageState(usageSampled ? OverlayUsageState{OverlayUsageDisabled{}} : retainObservation(OverlayUsageDisabled{}), usageSampled);
    }
  } else {
    m_hasCommittedSnapshot = false;
    m_usageTimer.stop();
    publishUsageState(usageSampled ? OverlayUsageState{OverlayUsageUnavailable{}} : retainObservation(OverlayUsageUnavailable{}), usageSampled);
  }
  if (guard) emit availabilityChanged();
}

void OverlayViewModel::refreshUsage() {
  // Usage polling requires a committed application snapshot as its baseline.
  if (!m_hasCommittedSnapshot || observationPaused() || refreshing()) return;
  m_read.start([reader = m_reader](std::stop_token) { return reader->read(); },
               [this](AsyncResult<OverlayUsageState> result) {
                 if (const auto* failure = std::get_if<std::exception_ptr>(&result)) {
                   std::string detail;
                   try {
                     std::rethrow_exception(*failure);
                   } catch (const std::exception& error) {
                     detail = error.what();
                   } catch (...) {
                     detail = "unknown overlay read failure";
                   }
                   // Repeated failures use debug logging until a successful observation resets the warning.
                   if (!m_usageRefreshFailed)
                     UWF_LOG_W("hub") << "usage refresh failed: committedState=retained error=" << detail;
                   else
                     UWF_LOG_D("hub") << "usage refresh still failing: committedState=retained error=" << detail;
                   m_usageRefreshFailed = true;
                   auto retained = m_usage;
                   observeOverlayUsageFailure(retained, std::move(detail));
                   const QPointer guard(this);
                   publishUsageState(retained);
                   if (guard) emit availabilityChanged();
                   return;
                 }

                 if (m_usageRefreshFailed) UWF_LOG_I("hub") << "usage refresh recovered";
                 m_usageRefreshFailed = false;

                 // Consumers receive one complete observation rather than independently updated fields.
                 const QPointer guard(this);
                 auto observed = std::get<OverlayUsageState>(std::move(result));
                 if (auto* enabled = std::get_if<OverlayUsageEnabled>(&observed)) enabled->config = m_currentConfig;
                 observeOverlayUsageSuccess(observed, std::chrono::system_clock::now());
                 publishUsageState(observed);
                 if (guard) emit availabilityChanged();
               });
}
ObservationPause OverlayViewModel::pauseObservation() {
  if (++m_observationPauses == 1) {
    m_usageTimer.stop();
    m_read.cancel();
  }
  return ObservationPause{[owner = QPointer(this)] {
    if (owner) owner->resumeObservation();
  }};
}
void OverlayViewModel::resumeObservation() {
  if (m_observationPauses == 0) return;
  if (--m_observationPauses == 0 && m_hasCommittedSnapshot) m_usageTimer.start();
}
void OverlayViewModel::publishUsageState(const OverlayUsageState& state, const bool usageSampled) {
  m_usage = state;
  if (usageSampled && !std::holds_alternative<OverlayUsageUnavailable>(m_usage) && !overlayUsageObservation(m_usage).lastSuccess &&
      !overlayUsageObservation(m_usage).error)
    observeOverlayUsageSuccess(m_usage, std::chrono::system_clock::now());
  emit usageChanged(m_usage);
}
}  // namespace uwf::presentation

#include "moc_OverlayViewModel.cpp"
