// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfModel.h"

#include <algorithm>
#include <ranges>

namespace uwf::core {

namespace {

unsigned char foldAscii(const char value) {
  const auto byte = static_cast<unsigned char>(value);
  return byte >= 'A' && byte <= 'Z' ? static_cast<unsigned char>(byte - 'A' + 'a') : byte;
}

bool caseInsensitiveLess(const std::string& left, const std::string& right) {
  return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end(),
                                      [](const char a, const char b) { return foldAscii(a) < foldAscii(b); });
}

bool caseInsensitiveEqual(const std::string& left, const std::string& right) {
  return left.size() == right.size() &&
         std::equal(left.begin(), left.end(), right.begin(), [](const char a, const char b) { return foldAscii(a) == foldAscii(b); });
}

}  // namespace

void sortExclusions(std::vector<std::string>& items) {
  std::stable_sort(items.begin(), items.end(), caseInsensitiveLess);
  // 去重必须沿用排序的大小写不敏感比较，避免同一路径保留不同大小写的副本。
  const auto dup = std::ranges::unique(items, caseInsensitiveEqual);
  items.erase(dup.begin(), dup.end());
}

void sortSnapshot(UwfSnapshot& snapshot) {
  for (auto& v : snapshot.current.fileExclusions | std::views::values) sortExclusions(v);
  for (auto& v : snapshot.next.fileExclusions | std::views::values) sortExclusions(v);
  sortExclusions(snapshot.current.registryExclusions);
  sortExclusions(snapshot.next.registryExclusions);
}

bool PendingChanges::empty() const { return count() == 0; }

std::size_t PendingChanges::count() const {
  std::size_t n = 0;
  if (setFilterEnabled) ++n;
  if (setOverlay.type) ++n;
  if (setOverlay.maximumSizeMb) ++n;
  if (setOverlay.warningThresholdMb) ++n;
  if (setOverlay.criticalThresholdMb) ++n;
  n += volumeProtect.size();
  n += volumeBindByVolumeName.size();
  for (const auto& kv : addFileExclusions) n += kv.second.size();
  for (const auto& kv : removeFileExclusions) n += kv.second.size();
  n += addRegistryExclusions.size();
  n += removeRegistryExclusions.size();
  if (setPersistDomainSecretKey) ++n;
  if (setPersistTSCAL) ++n;
  return n;
}

void PendingChanges::clear() {
  setFilterEnabled.reset();
  setOverlay = {};
  volumeProtect.clear();
  volumeBindByVolumeName.clear();
  addFileExclusions.clear();
  removeFileExclusions.clear();
  addRegistryExclusions.clear();
  removeRegistryExclusions.clear();
  setPersistDomainSecretKey.reset();
  setPersistTSCAL.reset();
}

}  // namespace uwf::core
