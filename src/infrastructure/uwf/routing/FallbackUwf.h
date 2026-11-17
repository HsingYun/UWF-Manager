// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <memory>

#include "domain/uwf/contracts/Backend.h"

namespace uwf::compat {
// Tries backends in constructor order; only operations with no effects may retry.
class FallbackUwf final : public Uwf {
 public:
  explicit FallbackUwf(std::vector<std::shared_ptr<Backend>> backends);
  [[nodiscard]] Capability probe(const Request& request) const override;
  [[nodiscard]] Response execute(const Request& request, const Context& context = {}) const override;

 private:
  [[nodiscard]] Response executeRouted(const Request& request, const Context& context, std::uint64_t parentRequestId) const;
  std::vector<std::shared_ptr<Backend>> m_backends;
  std::uint64_t m_logId;
  std::vector<std::string> m_logBackendNames;
};
}  // namespace uwf::compat
