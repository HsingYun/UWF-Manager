// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <memory>

#include "domain/uwf/contracts/Backend.h"
#include "infrastructure/windows/management/wmi/WmiOperations.h"

namespace uwf::windows::backends {
struct WmiCurrentIdentities;

// Maps one raw capability to WMI. The accessor resolves the current thread's
// transport; state prechecks and multi-capability workflows belong to UWF.
class WmiUwfBackend final : public compat::Backend {
 public:
  using Accessor = std::function<wmi::WmiOperations&()>;
  explicit WmiUwfBackend(Accessor operations);
  explicit WmiUwfBackend(wmi::WmiOperations& operations);

  [[nodiscard]] std::string name() const override;
  [[nodiscard]] compat::Capability probe(const compat::Request& request) const override;
  [[nodiscard]] compat::Response execute(const compat::Request& request, const compat::Context& context = {}) const override;

 private:
  [[nodiscard]] compat::Capability probeRaw(const compat::Request& request) const;
  [[nodiscard]] compat::Response executeRaw(const compat::Request& request, const compat::Context& context) const;
  Accessor m_operations;
  std::shared_ptr<WmiCurrentIdentities> m_currentIdentities;
};

}  // namespace uwf::windows::backends
