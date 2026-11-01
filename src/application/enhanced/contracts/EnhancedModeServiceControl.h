// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <vector>

#include "application/enhanced/contracts/EnhancedModeStatus.h"
#include "application/registry/contracts/RegistryCommitTypes.h"

namespace uwf::app {

// Service operations and their registry artifacts share this injected boundary.
class EnhancedModeServiceControl {
 public:
  virtual ~EnhancedModeServiceControl() = default;
  [[nodiscard]] virtual EnhancedModeStatus query() const = 0;
  virtual void installAndStart(const QString& description) = 0;
  virtual void start() = 0;
  virtual void disable() = 0;
  [[nodiscard]] virtual std::vector<RegistryCommitTarget> planRegistryCommit() const = 0;
  [[nodiscard]] virtual std::vector<RegistryCommitTarget> planRegistryDeletion() const = 0;
  virtual void deleteRegistryRemnants() = 0;
  // SCM removes a stopped service after its last handle closes. Keep the returned
  // barrier alive while persisting the planned registry deletions, then release it.
  class DeletionBarrier {
   public:
    virtual ~DeletionBarrier() = default;
  };
  enum class RemovalState {
    Absent,
    AlreadyMarkedForDeletion,
    MarkedForDeletion,
  };
  struct Removal {
    RemovalState state = RemovalState::Absent;
    std::unique_ptr<DeletionBarrier> barrier;
  };
  [[nodiscard]] virtual Removal stopAndMarkForDeletion() = 0;
};

}  // namespace uwf::app
