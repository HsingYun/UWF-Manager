// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/windows/execution/MtaUwf.h"

#include <stdexcept>
#include <utility>

#include "infrastructure/windows/execution/MtaExecutor.h"

namespace uwf::windows::backends {
MtaUwf::MtaUwf(Factory factory, std::shared_ptr<MtaExecutor> executor) : m_executor(std::move(executor)) {
  if (!m_executor || !factory) throw std::invalid_argument("MTA UWF execution requires an executor and a backend factory");
  m_system = m_executor->invoke([factory = std::move(factory)] {
    auto system = factory();
    if (!system) throw std::invalid_argument("The UWF backend factory returned no system");
    return system;
  });
}
MtaUwf::~MtaUwf() {
  // Release backend resources in the MTA before releasing the executor lease.
  m_executor->finalize([this]() noexcept { m_system.reset(); });
}
compat::Capability MtaUwf::probe(const compat::Request& request) const {
  return m_executor->invoke([system = m_system, request] { return system->probe(request); });
}
compat::Response MtaUwf::execute(const compat::Request& request, const compat::Context& context) const {
  // Captures outlive queueing; the operation retains actual write effects on cancellation.
  return m_executor->invoke([system = m_system, request, context] { return system->execute(request, context); });
}
compat::UwfStateSnapshot MtaUwf::stateSnapshot() const { return m_system->stateSnapshot(); }
void MtaUwf::updateState(compat::UwfStateFacts state) const { m_system->updateState(std::move(state)); }
void MtaUwf::synchronize(const std::function<void()>& work) const {
  m_executor->invoke([system = m_system, &work] { system->synchronize(work); });
}
compat::Response MtaUwf::refreshState(const compat::Context& context) const {
  return m_executor->invoke([system = m_system, context] { return system->refreshState(context); });
}
}  // namespace uwf::windows::backends
