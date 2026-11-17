// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QTimer>
#include <cstddef>
#include <memory>

#include "application/overlay/contracts/OverlayUsageReader.h"
#include "presentation/viewmodels/common/AsyncTask.h"
#include "presentation/viewmodels/common/ObservationPause.h"
#include "presentation/viewmodels/common/PresentationTiming.h"

namespace uwf::presentation {
[[nodiscard]] QString overlayUsageToolTip(const app::OverlayUsageState& state);
class OverlayViewModel final : public QObject {
  Q_OBJECT
 public:
  explicit OverlayViewModel(std::shared_ptr<app::OverlayUsageReader> reader, std::chrono::milliseconds refreshInterval = timing::kOverlayUsageRefreshInterval);
  void applySnapshot(const core::UwfSnapshot& snapshot, bool usageSampled = true);
  void refreshUsage();
  [[nodiscard]] bool refreshing() const { return m_read.isRunning(); }
  [[nodiscard]] const app::OverlayUsageState& usage() const { return m_usage; }
  // Pauses automatic and manual reads. An existing worker receives a stop
  // request and cannot publish its result; it is never joined or forcibly
  // interrupted. The final lease restarts automatic polling if a snapshot exists.
  [[nodiscard]] ObservationPause pauseObservation();
  [[nodiscard]] bool observationPaused() const { return m_observationPauses != 0; }
 signals:
  void usageChanged(const uwf::app::OverlayUsageState& state);
  void availabilityChanged();

 private:
  void publishUsageState(const app::OverlayUsageState& state, bool usageSampled = true);
  void resumeObservation();
  std::shared_ptr<app::OverlayUsageReader> m_reader;
  AsyncTask m_read;
  app::OverlayUsageState m_usage{app::OverlayUsageUnavailable{}};
  QTimer m_usageTimer;
  std::size_t m_observationPauses = 0;
  bool m_hasCommittedSnapshot = false;
  bool m_usageRefreshFailed = false;
  core::OverlayConfig m_currentConfig;
};
}  // namespace uwf::presentation
