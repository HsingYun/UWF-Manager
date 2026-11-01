// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "domain/uwf/contracts/Backend.h"

namespace uwf::windows::backends {
using NativeUwfProcedure = void (*)();

// Symbol presence alone does not establish output semantics. Production matches
// the captured CodeView identity; injected bindings may assert complete outputs
// only when their implementation guarantees them.
enum class NativeObservationProfile { Unverified, Captured20261101, CompleteInjectedOutputs };

// Production resolvers retain the System32 module lease; injected bindings need
// no installed UWF feature.
struct NativeUwfBindings {
  std::function<NativeUwfProcedure(std::string_view)> resolveExport;
  std::function<NativeUwfProcedure(std::string_view)> resolveImport;
  bool stringArrayAbiVerified = false;
  NativeObservationProfile observationProfile = NativeObservationProfile::Unverified;
  std::string unavailableReason;
  std::int64_t unavailableCode = 0;
};

// Maps one raw capability to its DLL entry point. State prechecks and
// multi-capability workflows belong to the UWF abstraction.
class NativeUwfBackend final : public compat::Backend {
 public:
  NativeUwfBackend();
  explicit NativeUwfBackend(NativeUwfBindings bindings);
  [[nodiscard]] std::string name() const override;
  [[nodiscard]] compat::Capability probe(const compat::Request& request) const override;
  [[nodiscard]] compat::Response execute(const compat::Request& request, const compat::Context& context = {}) const override;

 private:
  [[nodiscard]] compat::Capability probeRaw(const compat::Request& request) const;
  [[nodiscard]] compat::Response executeRaw(const compat::Request& request, const compat::Context& context) const;
  [[nodiscard]] compat::Response writeSwapfile(const std::string& volume, std::uint32_t value, const compat::Context& context) const;
  NativeUwfBindings m_bindings;
  std::unordered_map<std::string_view, NativeUwfProcedure> m_exports;
  NativeUwfProcedure m_free = nullptr;
  NativeUwfProcedure m_deleteArray = nullptr;
};

[[nodiscard]] std::shared_ptr<compat::Backend> createNativeUwfBackend();
}  // namespace uwf::windows::backends
