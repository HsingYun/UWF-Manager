// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "domain/uwf/contracts/Backend.h"
#include "infrastructure/windows/management/csp/DmCspTransport.h"

namespace uwf::windows::backends {
// Maps one raw capability to WAP. State prechecks and multi-capability workflows
// belong to the UWF abstraction; this adapter does not acquire their facts.
class CspUwfBackend final : public compat::Backend {
 public:
  explicit CspUwfBackend(std::shared_ptr<DmCspTransport> transport = createSystemDmCspTransport());
  [[nodiscard]] std::string name() const override;
  [[nodiscard]] compat::Capability probe(const compat::Request& request) const override;
  [[nodiscard]] compat::Response execute(const compat::Request& request, const compat::Context& context = {}) const override;

 private:
  [[nodiscard]] compat::Capability probeRaw(const compat::Request& request) const;
  [[nodiscard]] compat::Response executeRaw(const compat::Request& request, const compat::Context& context) const;
  [[nodiscard]] compat::Response writeSwapfile(const std::string& volume, std::uint32_t value, const compat::Context& context) const;
  std::shared_ptr<DmCspTransport> m_transport;
};
}  // namespace uwf::windows::backends
