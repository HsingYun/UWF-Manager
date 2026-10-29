// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QStringList>
#include <QStringView>
#include <set>
#include <string>
#include <vector>
namespace uwf::app::paths {
class PathExclusionIndex final {
 public:
  explicit PathExclusionIndex(const QStringList& exclusions);
  explicit PathExclusionIndex(const std::vector<std::string>& exclusions);
  [[nodiscard]] QString covering(const QString& path) const;
  [[nodiscard]] bool contains(const QString& path) const { return !covering(path).isEmpty(); }

 private:
  struct Less {
    using is_transparent = void;
    bool operator()(QStringView left, QStringView right) const { return left.compare(right, Qt::CaseInsensitive) < 0; }
  };
  void add(QString path);
  std::set<QString, Less> m_roots;
};
}  // namespace uwf::app::paths
