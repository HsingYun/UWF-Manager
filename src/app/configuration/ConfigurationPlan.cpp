// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "app/api/ConfigurationPlan.h"

#include <algorithm>
#include <utility>

#include "app/paths/PathPolicy.h"
#include "app/ports/FileStagingStore.h"

namespace uwf::app {
std::vector<ConfigurationOperation> ConfigurationPlan::operations() const {
  using Kind = ConfigurationOperationKind;
  std::vector<ConfigurationOperation> result;
  const auto add = [&](Kind kind, std::string target = {}) { result.push_back({kind, std::move(target)}); };
  if (staging) add(Kind::Staging);
  if (changes.setFilterEnabled) add(Kind::Filter);
  if (changes.setOverlay.type) add(Kind::OverlayType);
  if (changes.setOverlay.maximumSizeMb) add(Kind::OverlaySize);
  if (changes.setOverlay.warningThresholdMb) add(Kind::WarningThreshold);
  if (changes.setOverlay.criticalThresholdMb) add(Kind::CriticalThreshold);
  for (const auto& [drive, value] : changes.volumeProtect) {
    (void)value;
    add(Kind::VolumeProtection, drive);
  }
  for (const auto& [drive, value] : changes.volumeBindByVolumeName) {
    (void)value;
    add(Kind::VolumeBinding, drive);
  }
  for (const auto& [drive, paths] : changes.addFileExclusions)
    for (const auto& path : paths) add(Kind::AddFileExclusion, drive + path);
  for (const auto& [drive, paths] : changes.removeFileExclusions)
    for (const auto& path : paths) add(Kind::RemoveFileExclusion, drive + path);
  for (const auto& path : changes.addRegistryExclusions) add(Kind::AddRegistryExclusion, path);
  for (const auto& path : changes.removeRegistryExclusions) add(Kind::RemoveRegistryExclusion, path);
  if (changes.setPersistDomainSecretKey) add(Kind::DomainPersistence);
  if (changes.setPersistTSCAL) add(Kind::LicensePersistence);
  return result;
}
ConfigurationPlan planConfiguration(core::PendingChanges pending, core::UwfSnapshot snapshot, std::optional<FileStagingChanges> staging) {
  std::vector<ConfigurationOperation> stagingCompletion;
  if (staging) {
    const std::string root(kFileStagingRegistryRoot);
    const auto sameKey = [](const auto& a, const auto& b) { return paths::contains(a, b) && paths::contains(b, a); };
    const auto hasRoot = [&](const auto& keys) { return std::ranges::any_of(keys, [&](const auto& key) { return sameKey(key, root); }); };
    if (!staging->after.isEmpty()) std::erase_if(pending.removeRegistryExclusions, [&](const auto& key) { return sameKey(key, root); });
    auto retained = snapshot.next.registryExclusions;
    std::erase_if(retained, [&](const auto& key) {
      return std::ranges::any_of(pending.removeRegistryExclusions, [&](const auto& removed) { return sameKey(key, removed); });
    });
    auto target = retained;
    target.insert(target.end(), pending.addRegistryExclusions.begin(), pending.addRegistryExclusions.end());
    if (staging->after.isEmpty()) {
      if (hasRoot(snapshot.next.registryExclusions) && !hasRoot(pending.addRegistryExclusions) && !hasRoot(pending.removeRegistryExclusions))
        pending.removeRegistryExclusions.push_back(root);
      if (hasRoot(snapshot.next.registryExclusions) && hasRoot(pending.removeRegistryExclusions))
        for (const auto& key : pending.removeRegistryExclusions)
          if (sameKey(key, root)) stagingCompletion.push_back({ConfigurationOperationKind::RemoveRegistryExclusion, key});
    } else if (paths::findCoveringExclusion(target, root).empty()) {
      pending.addRegistryExclusions.push_back(root);
    }
    if (!staging->after.isEmpty() && paths::findCoveringExclusion(retained, root).empty())
      for (const auto& key : pending.addRegistryExclusions)
        if (paths::contains(key, root)) stagingCompletion.push_back({ConfigurationOperationKind::AddRegistryExclusion, key});
  }
  return {std::move(pending), std::move(snapshot), std::move(staging), std::move(stagingCompletion)};
}
}  // namespace uwf::app
