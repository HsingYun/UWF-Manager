// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <memory>

#include "app/api/UwfCapability.h"
#include "app/ports/FileStagingStorage.h"
#include "app/ports/WmiOperations.h"
namespace uwf::app {
// Writes the accepted list durably in the current session. Next-session
// changes belong exclusively to ConfigurationPlan / ConfigurationExecutor.
class DurableFileStagingStore final : public FileStagingStore {
 public:
  DurableFileStagingStore(std::shared_ptr<FileStagingStorage> storage, wmi::WmiOperations& session, UwfCapability capability);
  [[nodiscard]] QList<FileStagingEntry> load() const override;
  void replace(const QList<FileStagingEntry>& entries) override;
  void reconcileDurability();

 private:
  void preserveCurrent();
  void clearCurrent();
  std::shared_ptr<FileStagingStorage> m_storage;
  std::function<wmi::WmiOperations&()> m_session;
  UwfCapability m_capability;
};
}  // namespace uwf::app
