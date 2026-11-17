// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <stdexcept>

#include "application/staging/contracts/FileStagingStore.h"

namespace uwf::test::viewmodels {
class MemoryStagingStore final : public app::FileStagingStore {
 public:
  QList<app::FileStagingEntry> entries;
  bool failRead = false;
  bool failAfterReplace = false;
  bool failReadAfterReplace = false;
  int writes = 0;
  QList<app::FileStagingEntry> load() const override {
    if (failRead) throw std::runtime_error("read failed");
    return entries;
  }
  void replace(const QList<app::FileStagingEntry>& value) override {
    ++writes;
    entries = value;
    if (failReadAfterReplace) failRead = true;
    if (failAfterReplace) throw std::runtime_error("persistence incomplete");
  }
};
}  // namespace uwf::test::viewmodels
