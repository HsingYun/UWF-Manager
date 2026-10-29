// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ExclusionDraft.h"

#include <algorithm>

namespace uwf::app {

bool containsExclusion(const QSet<QString>& set, const QString& key) {
  return std::ranges::any_of(set, [&](const QString& s) { return QString::compare(s, key, Qt::CaseInsensitive) == 0; });
}

void removeExclusion(QSet<QString>& set, const QString& key) {
  for (auto it = set.begin(); it != set.end();) {
    if (QString::compare(*it, key, Qt::CaseInsensitive) == 0)
      it = set.erase(it);
    else
      ++it;
  }
}

bool ExclusionDraft::add(const QString& p) {
  // 撤销待删除项，或将下一会话尚不存在的条目加入待添加集合。
  bool changed = false;
  if (containsExclusion(removed, p)) {
    removeExclusion(removed, p);
    changed = true;
  } else if (!next.contains(p, Qt::CaseInsensitive) && !containsExclusion(added, p)) {
    added.insert(p);
    changed = true;
  }
  return changed;
}

bool ExclusionDraft::remove(const QString& p) {
  // 撤销待添加项，或将下一会话基线中的条目标为待删除；其余为无操作。
  bool changed = false;
  if (containsExclusion(added, p)) {
    removeExclusion(added, p);
    changed = true;
  } else if (next.contains(p, Qt::CaseInsensitive) && !containsExclusion(removed, p)) {
    removed.insert(p);
    changed = true;
  }
  return changed;
}

void ExclusionDraft::reset() {
  added.clear();
  removed.clear();
  domainSecretKey.pendingNext.reset();
  tscal.pendingNext.reset();
}

void ExclusionDraft::togglePersistence(PersistenceFlagDraft& flag) {
  if (flag.pendingNext == true)
    flag.pendingNext.reset();
  else if (flag.baseNext)
    flag.pendingNext = false;
}

void ExclusionDraft::enablePersistence(PersistenceFlagDraft& flag) {
  if (flag.pendingNext == false)
    flag.pendingNext.reset();
  else if (!flag.baseNext)
    flag.pendingNext = true;
}

}  // namespace uwf::app
