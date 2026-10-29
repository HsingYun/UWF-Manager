// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ApplicationStateSession.h"

#include <utility>

namespace uwf::app {

void ApplicationStateSession::commit(ApplicationState state) {
  m_state.volumes = std::move(state.volumes);
  m_state.snapshot = std::move(state.snapshot);
  m_committed = true;
  m_reconciliationRequired = false;
}

void ApplicationStateSession::requireReconciliation() { m_reconciliationRequired = true; }

bool ApplicationStateSession::configurationWritesAllowed() const {
  return m_committed && m_state.snapshot.uwfAvailable && m_state.snapshot.elevated && !m_reconciliationRequired;
}

}  // namespace uwf::app
