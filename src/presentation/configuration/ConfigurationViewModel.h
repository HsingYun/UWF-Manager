// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "GlobalConfigurationViewModel.h"
#include "VolumeConfigurationViewModel.h"
#include "app/configuration/ConfigurationEditSession.h"

namespace uwf::presentation {

class ConfigurationViewModel final : public QObject {
  Q_OBJECT
 public:
  explicit ConfigurationViewModel(uint32_t totalRamMb, std::function<bool()> stagingData = {}, QObject* parent = nullptr);
  [[nodiscard]] const std::shared_ptr<GlobalConfigurationViewModel>& global() const { return m_global; }
  std::shared_ptr<VolumeConfigurationViewModel> addVolume(const core::VolumeInfo& volume, bool hostsRegistry);
  void clearVolumes();
  void resetSnapshot(const core::UwfSnapshot& snapshot);
  void refreshStagingConstraints();
  [[nodiscard]] std::shared_ptr<VolumeConfigurationViewModel> volume(const QString& driveLetter) const;
  [[nodiscard]] core::PendingChanges pendingChanges() const;
  [[nodiscard]] QList<app::ImportResult> importCommands(const QList<app::UwfmgrCommand>& commands);

 signals:
  void pendingChanged();

 private:
  app::ConfigurationEditSession m_edit;
  std::shared_ptr<GlobalConfigurationViewModel> m_global;
  std::vector<std::shared_ptr<VolumeConfigurationViewModel>> m_volumes;
};

}  // namespace uwf::presentation
