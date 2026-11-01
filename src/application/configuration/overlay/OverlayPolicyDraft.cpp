// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/overlay/OverlayPolicyDraft.h"

#include <stdexcept>
#include <utility>

namespace uwf::app {
void OverlayPolicyDraft::reset(OverlayPolicySnapshot baseline) {
  m_baseline = std::move(baseline);
  discard();
}
void OverlayPolicyDraft::edit(const OverlayPolicyDelta& changes) {
  if (changes.reset == PersistentReset::UseSavedMode) throw std::invalid_argument("Reset selector 255 is read-only in application configuration");
  const auto merge = [](auto& pending, const auto& requested) {
    if (requested) pending = requested;
  };
  merge(m_intent.passthrough, changes.passthrough);
  merge(m_intent.persistent, changes.persistent);
  merge(m_intent.readOnlyMedia, changes.readOnlyMedia);
  merge(m_intent.reset, changes.reset);
}
void OverlayPolicyDraft::replace(OverlayPolicyDelta intent) {
  if (intent.reset == PersistentReset::UseSavedMode) throw std::invalid_argument("Reset selector 255 is read-only in application configuration");
  m_intent = std::move(intent);
}
}  // namespace uwf::app
