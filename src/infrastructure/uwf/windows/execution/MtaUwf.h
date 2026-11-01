// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <memory>

#include "domain/uwf/contracts/Backend.h"

namespace uwf::windows {
class MtaExecutor;
}

namespace uwf::windows::backends {
// Runs shared-state prechecks, capability composition and routed native calls
// in the MTA pool while preserving a synchronous Uwf port. State access forwards
// to the shared in-memory snapshot without querying providers.
class MtaUwf final : public compat::Uwf {
 public:
  using Factory = std::function<std::shared_ptr<compat::Uwf>()>;
  MtaUwf(Factory factory, std::shared_ptr<MtaExecutor> executor);
  ~MtaUwf() override;
  MtaUwf(const MtaUwf&) = delete;
  MtaUwf& operator=(const MtaUwf&) = delete;

  [[nodiscard]] compat::Capability probe(const compat::Request& request) const override;
  [[nodiscard]] compat::Response execute(const compat::Request& request, const compat::Context& context = {}) const override;
  [[nodiscard]] compat::UwfStateSnapshot stateSnapshot() const override;
  void updateState(compat::UwfStateFacts state) const override;
  void synchronize(const std::function<void()>& work) const override;
  [[nodiscard]] compat::Response refreshState(const compat::Context& context = {}) const override;

 private:
  std::shared_ptr<MtaExecutor> m_executor;
  std::shared_ptr<compat::Uwf> m_system;
};
}  // namespace uwf::windows::backends
