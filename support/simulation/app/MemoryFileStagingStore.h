// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdexcept>

#include "application/staging/contracts/FileStagingStore.h"

namespace uwf::simulation::application {
class MemoryFileStagingStore final : public app::FileStagingStore {
 public:
  QList<app::FileStagingEntry> entries;
  QString readFailure;
  QString writeFailure;
  int writes = 0;
  mutable int loads = 0;

  [[nodiscard]] QList<app::FileStagingEntry> load() const override {
    ++loads;
    if (!readFailure.isEmpty()) throw std::runtime_error(readFailure.toStdString());
    return entries;
  }

  void replace(const QList<app::FileStagingEntry>& replacement) override {
    if (!writeFailure.isEmpty()) throw std::runtime_error(writeFailure.toStdString());
    entries = replacement;
    ++writes;
  }
};

}  // namespace uwf::simulation::application
