// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "application/identity/contracts/ObjectIdentity.h"

namespace uwf::app::paths {
struct VolumeFilePath {
  std::string volume;  // A normalized GUID or literal drive prefix, never a resolved descendant mount.
  QString relativePath;
};
using VolumeIdentities = std::map<QString, std::string, ObjectNameLess>;
// Split the management volume address from its literal pattern. Do not clean
// separators, dot segments, whitespace or an absent path body.
[[nodiscard]] std::optional<VolumeFilePath> splitVolumeFilePath(const QString& path);
// Complete local file syntax for conventional drive and extended drive/GUID
// addresses, excluding network/device namespaces. Never queries mounts or
// existence; successful syntax parsing does not establish a physical identity.
[[nodiscard]] std::optional<VolumeFilePath> parseLocalFilePath(const QString& path);
// Explicit application addresses only. Provider exclusion strings are literal
// patterns owned by a separate volume; do not infer an address from their text.
[[nodiscard]] std::optional<VolumeFilePath> resolveVolumeFilePath(const QString& path, const VolumeIdentities& volumes = {},
                                                                  std::string_view relativeVolume = {});
// Add an unambiguous address shell while retaining the complete literal suffix.
[[nodiscard]] QString volumeFilePath(const VolumeFilePath& path);
// These addresses describe Windows objects, independently of the development host.
[[nodiscard]] QString windowsSeparators(QString path);
[[nodiscard]] QString normalize(const QString& path);
// Resolve volume-rooted file paths (\Data) against the selected drive before normalization.
[[nodiscard]] QString normalize(const QString& path, const QString& driveLetter);
// File-system addresses used for browsing: ordinary paths keep Qt's forward
// separators; explicit Windows namespaces retain their native literal spelling.
// These helpers never reinterpret a provider exclusion pattern as an address.
[[nodiscard]] QString fileSystemPath(const QString& path);
[[nodiscard]] QString absoluteFilePath(const QString& path, const QString& directory = {});
[[nodiscard]] QString parentDirectory(const QString& path);
[[nodiscard]] QString fileName(const QString& path);
[[nodiscard]] QString joinFilePath(const QString& directory, const QString& name);
[[nodiscard]] ObjectIdentity identity(const QString& path);
// Resolve registry hive aliases for object coverage without cleaning key suffixes.
// An absolute file path is unchanged.
[[nodiscard]] QString expandRegistryHive(const QString& path);
[[nodiscard]] bool contains(const QString& parent, const QString& path);
[[nodiscard]] bool isDriveRoot(const QString& path);
[[nodiscard]] std::string stripTrailingSeparators(std::string path);

}  // namespace uwf::app::paths
