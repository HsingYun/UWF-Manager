// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <string>
#include <vector>

namespace uwf::app::paths {
[[nodiscard]] QString normalize(const QString& path);
// Resolve volume-rooted file paths (\Data) against the selected drive before normalization.
[[nodiscard]] QString normalize(const QString& path, const QString& driveLetter);
[[nodiscard]] QString identity(const QString& path);
[[nodiscard]] bool contains(const QString& parent, const QString& path);
[[nodiscard]] bool overlap(const QString& left, const QString& right);
[[nodiscard]] bool isDriveRoot(const QString& path);
[[nodiscard]] std::string stripTrailingSeparators(std::string path);
// 与 QString 重载使用相同的 Unicode 大小写匹配，并保留注册表键中的点段。
[[nodiscard]] bool contains(const std::string& parent, const std::string& path);

// 返回首个覆盖目标的原始排除项，保留其书写形式供诊断展示；无匹配时返回空串。
[[nodiscard]] std::string findCoveringExclusion(const std::vector<std::string>& excls, const std::string& target);

}  // namespace uwf::app::paths
