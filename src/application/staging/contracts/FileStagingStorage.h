// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/registry/contracts/RegistryCommitTypes.h"
#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::app {
// Raw persistence, without UWF policy or provider calls. Writing an empty list
// must retain a real record; deleting its backing metadata is a separate step.
class FileStagingStorage {
 public:
  virtual ~FileStagingStorage() = default;
  [[nodiscard]] virtual QList<FileStagingEntry> load() const = 0;
  virtual void write(const QList<FileStagingEntry>& entries) = 0;
  virtual void erase() = 0;
  [[nodiscard]] virtual std::vector<RegistryCommitTarget> deletionPlan() const = 0;
};
}  // namespace uwf::app
