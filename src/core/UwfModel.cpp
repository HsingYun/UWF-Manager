/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "UwfModel.h"

#include <algorithm>
#include <ranges>

#include "../util/StringUtil.h"

namespace uwf::core {

void sortExclusions(std::vector<std::string>& items) {
  std::ranges::sort(items, [](const std::string& a, const std::string& b) { return toLowerAscii(a) < toLowerAscii(b); });
  // 必须传与排序一致的大小写不敏感谓词，否则 "C:\Foo" 与 "c:\foo"
  // 会排到一起但 std::ranges::unique 默认 operator== 判不等，留两份。
  const auto dup = std::ranges::unique(items, [](const std::string& a, const std::string& b) { return toLowerAscii(a) == toLowerAscii(b); });
  items.erase(dup.begin(), dup.end());
}

void sortSnapshot(UwfSnapshot& snapshot) {
  for (auto& v : snapshot.current.fileExclusions | std::views::values) sortExclusions(v);
  for (auto& v : snapshot.next.fileExclusions | std::views::values) sortExclusions(v);
  sortExclusions(snapshot.current.registryExclusions);
  sortExclusions(snapshot.next.registryExclusions);
}

bool PendingChanges::empty() const {
  return !setFilterEnabled && setOverlay.empty() && volumeProtect.empty() && volumeBindByVolumeName.empty() && addFileExclusions.empty() &&
         removeFileExclusions.empty() && addRegistryExclusions.empty() && removeRegistryExclusions.empty() && !setPersistDomainSecretKey && !setPersistTSCAL;
}

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
