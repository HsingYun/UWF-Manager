// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "app/api/ConfigurationActions.h"
#include "app/ports/VolumePaths.h"
#include "app/ports/WmiOperations.h"
#include "app/staging/FileStagingDraft.h"

namespace uwf::app {

class ConfigurationExecutor final : public ConfigurationActions {
 public:
  explicit ConfigurationExecutor(wmi::WmiOperations& session, drive::MountLookup mounts, std::shared_ptr<FileStagingDraft> staging = {},
                                 std::optional<FileStagingChanges> stagingChanges = {});

  [[nodiscard]] ApplyOperation prepareApply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) override;
  void completeApply(const ConfigurationApplyResult& result) override;

 private:
  std::function<wmi::WmiOperations&()> m_session;
  drive::MountLookup m_mounts;
  std::shared_ptr<FileStagingDraft> m_staging;
  std::optional<FileStagingChanges> m_stagingChanges;
};

}  // namespace uwf::app
