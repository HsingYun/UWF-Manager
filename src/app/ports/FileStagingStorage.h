// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/RegistryCommitTypes.h"
#include "app/ports/FileStagingStore.h"
namespace uwf::app {
// Raw persistence, without UWF policy or provider calls.
class FileStagingStorage : public FileStagingStore {
 public:
  [[nodiscard]] virtual bool exists() const = 0;
  [[nodiscard]] virtual std::vector<RegistryCommitTarget> deletionPlan() const = 0;
};
}  // namespace uwf::app
