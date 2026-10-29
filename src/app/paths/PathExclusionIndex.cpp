// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PathExclusionIndex.h"

#include <QDir>
namespace uwf::app::paths {
void PathExclusionIndex::add(QString path) {
  path = QDir::toNativeSeparators(QDir::fromNativeSeparators(path));
  while (path.endsWith(u'\\')) path.chop(1);
  if (!path.isEmpty()) m_roots.insert(std::move(path));
}
PathExclusionIndex::PathExclusionIndex(const QStringList& exclusions) {
  for (const auto& path : exclusions) add(path);
}
PathExclusionIndex::PathExclusionIndex(const std::vector<std::string>& exclusions) {
  for (const auto& path : exclusions) add(QString::fromStdString(path));
}
QString PathExclusionIndex::covering(const QString& path) const {
  if (m_roots.empty()) return {};
  const auto normalized = QDir::toNativeSeparators(QDir::fromNativeSeparators(path));
  QStringView ancestor(normalized);
  while (true) {
    const auto found = m_roots.find(ancestor);
    if (found != m_roots.end()) return *found;
    const auto separator = ancestor.lastIndexOf(u'\\');
    if (separator < 0) return {};
    ancestor = ancestor.first(separator);
  }
}
}  // namespace uwf::app::paths
