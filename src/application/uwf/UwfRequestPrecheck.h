// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <memory>
#include <mutex>

#include "domain/uwf/contracts/Backend.h"
#include "foundation/concurrency/AtomicSnapshot.h"

namespace uwf::app {
using UwfFactsSource = std::function<compat::UwfStateFacts(const compat::Request&, const compat::Context&)>;
using UwfStateRefresh = std::function<compat::Response(const compat::Uwf&, const compat::Context&)>;
// Shared state and cross-capability execution, outside backends and routing.
// Disabling optional inference retains state publication and operation composition.
class PrecheckedUwf final : public compat::Uwf {
 public:
  explicit PrecheckedUwf(std::shared_ptr<compat::Uwf> downstream, UwfFactsSource source = {}, bool enabled = true, UwfStateRefresh refresh = {});
  [[nodiscard]] compat::Capability probe(const compat::Request& request) const override;
  [[nodiscard]] compat::Response execute(const compat::Request& request, const compat::Context& context = {}) const override;
  [[nodiscard]] compat::UwfStateSnapshot stateSnapshot() const override;
  void updateState(compat::UwfStateFacts state) const override;
  void synchronize(const std::function<void()>& work) const override;
  [[nodiscard]] compat::Response refreshState(const compat::Context& context = {}) const override;

 private:
  class Session;
  [[nodiscard]] Session* activeSession() const noexcept;
  static thread_local Session* s_session;
  std::shared_ptr<compat::Uwf> m_downstream;
  UwfFactsSource m_source;
  bool m_enabled;
  UwfStateRefresh m_refresh;
  mutable std::recursive_mutex m_mutex;
  // Immutable publication is independent of the mutex held during native I/O.
  mutable foundation::AtomicSnapshot<compat::UwfStateFacts> m_published;
};

}  // namespace uwf::app
