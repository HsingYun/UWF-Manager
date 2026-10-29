// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PathPolicy.h"

#include <QDir>

#include "shared/DriveLetter.h"

namespace uwf::app::paths {
QString normalize(const QString& path) { return path.isEmpty() ? QString{} : QDir::toNativeSeparators(QDir::cleanPath(QDir::fromNativeSeparators(path))); }
QString normalize(const QString& path, const QString& driveLetter) {
  const auto normalized = normalize(path);
  if (!driveLetter.isEmpty() && normalized.startsWith(u'\\') && !normalized.startsWith(QLatin1String("\\\\"))) return normalize(driveLetter + normalized);
  return normalized;
}
QString identity(const QString& path) { return normalize(path).toCaseFolded(); }
bool contains(const QString& parent, const QString& path) {
  QString root = QDir::toNativeSeparators(QDir::fromNativeSeparators(parent));
  const QString target = QDir::toNativeSeparators(QDir::fromNativeSeparators(path));
  while (root.endsWith(u'\\') || root.endsWith(u'/')) root.chop(1);
  if (root.isEmpty()) return false;
  if (target.compare(root, Qt::CaseInsensitive) == 0) return true;
  return target.size() > root.size() && target.startsWith(root, Qt::CaseInsensitive) && (target.at(root.size()) == u'\\' || target.at(root.size()) == u'/');
}
bool overlap(const QString& left, const QString& right) { return contains(left, right) || contains(right, left); }
bool isDriveRoot(const QString& path) {
  const drive::PathSplit split = drive::split(path.toStdString());
  return !split.letter.empty() && stripTrailingSeparators(split.rest).empty();
}

std::string stripTrailingSeparators(std::string path) {
  while (!path.empty() && (path.back() == '\\' || path.back() == '/')) path.pop_back();
  return path;
}

bool contains(const std::string& parent, const std::string& path) { return contains(QString::fromStdString(parent), QString::fromStdString(path)); }

std::string findCoveringExclusion(const std::vector<std::string>& excls, const std::string& target) {
  const std::string t = stripTrailingSeparators(target);
  for (const auto& e : excls) {
    const std::string p = stripTrailingSeparators(e);
    if (contains(p, t)) return e;
  }
  return {};
}

}  // namespace uwf::app::paths
