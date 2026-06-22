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
#include "PendingCollect.h"

#include <string>

#include "DiskTab.h"
#include "GlobalStatusPanel.h"

namespace uwf::ui {

core::PendingChanges collectPending(const GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs) {
  core::PendingChanges changes;

  if (auto v = global->pendingFilterEnabled()) changes.setFilterEnabled = *v;
  changes.setOverlay = global->pendingOverlay();

  for (const auto& t : diskTabs) {
    if (!t || !t->supported()) continue;
    const std::string dlStd = t->driveLetter().toStdString();

    if (auto v = t->pendingVolumeProtected()) changes.volumeProtect[dlStd] = *v;
    if (auto v = t->pendingBindByVolumeName()) changes.volumeBindByVolumeName[dlStd] = *v;
    // 注意只在有 pending 时才 access map[dlStd]——map 的 operator[] 会无端
    // 插入空 entry，commit 分支后续 for-each 会因此误以为这个卷有变更并尝试
    // 注册它（"为何改 D: 时连 F: 也被注册"的根因）。
    if (const auto added = t->pendingFileAdded(); !added.isEmpty()) {
      auto& bucket = changes.addFileExclusions[dlStd];
      for (const auto& p : added) bucket.push_back(p.toStdString());
    }
    if (const auto removed = t->pendingFileRemoved(); !removed.isEmpty()) {
      auto& bucket = changes.removeFileExclusions[dlStd];
      for (const auto& p : removed) bucket.push_back(p.toStdString());
    }
    if (const auto regAdded = t->pendingRegAdded(); !regAdded.isEmpty())
      for (const auto& p : regAdded) changes.addRegistryExclusions.push_back(p.toStdString());
    if (const auto regRemoved = t->pendingRegRemoved(); !regRemoved.isEmpty())
      for (const auto& p : regRemoved) changes.removeRegistryExclusions.push_back(p.toStdString());
    if (const auto v = t->pendingPersistDomainSecretKey()) changes.setPersistDomainSecretKey = *v;
    if (const auto v = t->pendingPersistTSCAL()) changes.setPersistTSCAL = *v;
  }

  return changes;
}

}  // namespace uwf::ui
