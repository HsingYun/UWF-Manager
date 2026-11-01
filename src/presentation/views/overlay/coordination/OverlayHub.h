// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <memory>
#include <vector>

#include "application/overlay/contracts/OverlayUsage.h"

namespace uwf::ui {

class OverlayHubView;

// Selects one HUD endpoint; callers manage visibility and data independently of its host.
class OverlayHub final : public QObject {
  Q_OBJECT
 public:
  explicit OverlayHub(QObject* parent = nullptr);
  ~OverlayHub() override;

  void registerView(std::unique_ptr<OverlayHubView> view);
  void applyUsageState(const app::OverlayUsageState& state);
  void retranslate();
  void setRequestedVisible(bool visible);
  void setOperationAvailability(bool powerAllowed, bool exitAllowed);
  void hideTemporarily();
  void restoreAfterTemporaryHide();

  [[nodiscard]] bool available() const;
  [[nodiscard]] bool requestedVisible() const { return m_requestedVisible; }
  [[nodiscard]] bool enabled() const;
  [[nodiscard]] bool presented() const;

 signals:
  void showMainWindowRequested();
  void safeShutdownRequested();
  void safeRestartRequested();
  void exitApplicationRequested();
  void stateChanged();

 private:
  void installView(std::unique_ptr<OverlayHubView> view);
  void sortViewsByPriority();
  void schedulePendingViewFlush();
  void flushPendingViews();
  void reconcilePresentation();
  void reconcilePresentationOnce();

  std::vector<std::unique_ptr<OverlayHubView>> m_views;
  std::vector<std::unique_ptr<OverlayHubView>> m_pendingViews;
  app::OverlayUsageState m_usageState{app::OverlayUsageUnavailable{}};
  OverlayHubView* m_presentedView = nullptr;
  bool m_requestedVisible = true;
  bool m_powerAllowed = false;
  bool m_exitAllowed = true;
  bool m_temporarilyHidden = false;
  bool m_reconciling = false;
  bool m_reconcilePending = false;
  bool m_pendingViewFlushScheduled = false;
  bool m_shuttingDown = false;
};

}  // namespace uwf::ui
