// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QObject>
#include <QTimer>
#include <memory>

#include "app/api/OverlayUsageReader.h"
#include "presentation/common/AsyncTask.h"
namespace uwf::presentation {
[[nodiscard]] QString overlayUsageToolTip(const app::OverlayUsageState& state);
class OverlayViewModel final : public QObject {
  Q_OBJECT
 public:
  explicit OverlayViewModel(std::shared_ptr<app::OverlayUsageReader> reader);
  void applySnapshot(const core::UwfSnapshot& snapshot);
  void refreshUsage();
  [[nodiscard]] bool refreshing() const { return m_read.isRunning(); }
  [[nodiscard]] const app::OverlayUsageState& usage() const { return m_usage; }
  [[nodiscard]] QTimer* usageTimer() { return &m_usageTimer; }
 signals:
  void usageChanged(const uwf::app::OverlayUsageState& state);
  void availabilityChanged();

 private:
  void publishUsageState(const app::OverlayUsageState& state);
  std::shared_ptr<app::OverlayUsageReader> m_reader;
  AsyncTask m_read;
  app::OverlayUsageState m_usage{app::OverlayUsageUnavailable{}};
  QTimer m_usageTimer;
  bool m_hasCommittedSnapshot = false;
  bool m_usageRefreshFailed = false;
};
}  // namespace uwf::presentation
