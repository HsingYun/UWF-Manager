// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/paths/PathPolicy.h"

#include <QDir>
#include <QStringView>

#include "application/paths/contracts/WindowsPathOperations.h"
#include "domain/uwf/contracts/Types.h"
#include "foundation/paths/DriveLetter.h"
#include "foundation/paths/RegistryPath.h"
#include "foundation/text/Utf8.h"

namespace uwf::app::paths {
namespace {
bool hasWindowsNamespace(const QString& native) { return native.startsWith(QLatin1String(R"(\\?\)")) || native.startsWith(QLatin1String(R"(\\.\)")); }
qsizetype namespaceRootLength(const QString& native) {
  if (const auto address = splitVolumeFilePath(native)) {
    if (address->relativePath.startsWith(u'\\')) return native.size() - address->relativePath.size() + 1;
  } else if (native.startsWith(QLatin1String(R"(\\?\UNC\)"), Qt::CaseInsensitive)) {
    const auto serverEnd = native.indexOf(u'\\', 8);
    if (serverEnd >= 0) {
      const auto shareEnd = native.indexOf(u'\\', serverEnd + 1);
      return shareEnd >= 0 ? shareEnd + 1 : native.size();
    }
  }
  // Unknown device namespaces are not a navigable parent hierarchy.
  return native.size();
}
}  // namespace
std::optional<VolumeFilePath> splitVolumeFilePath(const QString& path) {
  auto body = QStringView(path);
  if (body.startsWith(QLatin1String(R"(\\?\)")) || body.startsWith(QLatin1String(R"(\\.\)"))) body = body.sliced(4);
  if (body.size() >= 2 && body[1] == u':' && ((body[0] >= u'A' && body[0] <= u'Z') || (body[0] >= u'a' && body[0] <= u'z')))
    return VolumeFilePath{drive::normalize(body.first(2).toString().toStdString()), body.sliced(2).toString()};
  // ParsePathSpec identifies exactly Volume{GUID} before copying the remaining
  // UTF-16 units unchanged. It does not resolve or normalize the pattern.
  constexpr qsizetype guidAddressUnits = 44;
  if (body.size() >= guidAddressUnits)
    if (const auto identity = compat::parseVolumeId(body.first(guidAddressUnits).toString().toStdString()))
      return VolumeFilePath{*identity, body.sliced(guidAddressUnits).toString()};
  return std::nullopt;
}
std::optional<VolumeFilePath> parseLocalFilePath(const QString& path) {
  if (path.isEmpty() || path.contains(QChar::Null) || !isValidUtf16(QStringView(path)) || !platform::fullyQualified(path) ||
      path.startsWith(QLatin1String(R"(\\.\)")))
    return std::nullopt;
  const auto address = splitVolumeFilePath(path);
  if (!address) return std::nullopt;
  if (path.startsWith(QLatin1String(R"(\\?\)")) && (!address->relativePath.startsWith(u'\\') || address->relativePath.startsWith(QLatin1String("\\\\"))))
    return std::nullopt;
  return address;
}
std::optional<VolumeFilePath> resolveVolumeFilePath(const QString& path, const VolumeIdentities& volumes, const std::string_view relativeVolume) {
  if (const auto address = splitVolumeFilePath(path)) {
    if (const auto id = compat::parseVolumeId(address->volume)) return VolumeFilePath{*id, address->relativePath};
    const auto found = volumes.find(QString::fromStdString(address->volume));
    if (found != volumes.end())
      if (const auto id = compat::parseVolumeId(found->second)) return VolumeFilePath{*id, address->relativePath};
    return std::nullopt;
  }
  if (path.startsWith(u'\\') && !path.startsWith(QLatin1String("\\\\")))
    if (const auto id = compat::parseVolumeId(std::string(relativeVolume))) return VolumeFilePath{*id, path};
  return std::nullopt;
}
QString volumeFilePath(const VolumeFilePath& path) {
  auto root = compat::volumePath(path.volume);
  if (!root.empty() && root.back() == '\\') root.pop_back();
  return QString::fromStdString(root) + path.relativePath;
}
QString windowsSeparators(QString path) { return platform::nativeSeparators(path); }
QString normalize(const QString& path) {
  if (path.isEmpty()) return {};
  const auto native = platform::nativeSeparators(path);
  // Qt's fromNativeSeparators removes the extended namespace prefix. Preserve
  // explicitly addressed Windows objects, including their literal suffix, so
  // every caller keeps the same volume address through staging and execution.
  if (hasWindowsNamespace(native)) return native;
  return platform::nativeSeparators(platform::clean(platform::portableSeparators(path)));
}
QString normalize(const QString& path, const QString& driveLetter) {
  const auto normalized = normalize(path);
  if (!driveLetter.isEmpty() && normalized.startsWith(u'\\') && !normalized.startsWith(QLatin1String("\\\\"))) return normalize(driveLetter + normalized);
  return normalized;
}
QString fileSystemPath(const QString& path) {
  if (path.isEmpty()) return {};
  const auto native = platform::nativeSeparators(path);
  return hasWindowsNamespace(native) ? native : platform::clean(platform::portableSeparators(path));
}
QString parentDirectory(const QString& path) {
  auto address = fileSystemPath(path);
  if (!hasWindowsNamespace(address)) return platform::clean(platform::parent(address));
  const auto rootLength = namespaceRootLength(address);
  while (address.size() > rootLength && address.endsWith(u'\\')) address.chop(1);
  if (address.size() <= rootLength) return address;
  const auto separator = address.lastIndexOf(u'\\');
  return address.left(separator < rootLength ? rootLength : separator);
}
QString fileName(const QString& path) {
  const auto address = fileSystemPath(path);
  return hasWindowsNamespace(address) ? address.mid(address.lastIndexOf(u'\\') + 1) : platform::fileName(address);
}
QString joinFilePath(const QString& directory, const QString& name) {
  const auto child = fileSystemPath(name);
  const auto parent = fileSystemPath(directory);
  const auto nativeChild = platform::nativeSeparators(name);
  if (hasWindowsNamespace(parent) && nativeChild.startsWith(u'\\') && !nativeChild.startsWith(QLatin1String("\\\\")))
    return parent.left(namespaceRootLength(parent)) + nativeChild.mid(1);
  if (hasWindowsNamespace(child) || platform::absolute(child)) return child;
  if (!hasWindowsNamespace(parent)) return fileSystemPath(platform::join(parent, name));
  return parent + (parent.endsWith(u'\\') ? QString{} : QStringLiteral("\\")) + nativeChild;
}
QString absoluteFilePath(const QString& path, const QString& directory) {
  const auto address = fileSystemPath(path);
  if (hasWindowsNamespace(address)) return address;
  const auto parent = fileSystemPath(directory.isEmpty() ? QDir::currentPath() : directory);
  if (hasWindowsNamespace(parent) && !address.startsWith(QLatin1String("//")) && !(address.size() >= 2 && address[1] == u':')) {
    auto result = parent;
    const auto relative = platform::nativeSeparators(path);
    if (relative.startsWith(u'\\')) result = parent.left(namespaceRootLength(parent));
    // Interpret navigation components only in a relative user request. Explicit
    // namespace addresses and enumerated object names retain their literal tail.
    const auto components = relative.split(u'\\', Qt::SkipEmptyParts);
    for (const auto& component : components) {
      if (component == QLatin1String(".."))
        result = parentDirectory(result);
      else if (component != QLatin1String("."))
        result = joinFilePath(result, component);
    }
    return result;
  }
  return fileSystemPath(platform::absolute(address) ? address : platform::resolve(parent, address));
}
ObjectIdentity identity(const QString& path) { return ObjectIdentity(normalize(path)); }
QString expandRegistryHive(const QString& path) { return QString::fromStdU16String(regkey::expandHive(QStringView(path))); }
bool contains(const QString& parent, const QString& path) {
  QString root = expandRegistryHive(platform::nativeSeparators(parent));
  const QString target = expandRegistryHive(platform::nativeSeparators(path));
  while (root.endsWith(u'\\') || root.endsWith(u'/')) root.chop(1);
  if (root.isEmpty()) return false;
  if (equalObjectNames(target, root)) return true;
  return target.size() > root.size() && equalObjectNames(QStringView(target).first(root.size()), root) &&
         (target.at(root.size()) == u'\\' || target.at(root.size()) == u'/');
}
bool isDriveRoot(const QString& path) {
  const drive::PathSplit split = drive::split(path.toStdString());
  return !split.letter.empty() && stripTrailingSeparators(split.rest).empty();
}

std::string stripTrailingSeparators(std::string path) {
  while (!path.empty() && (path.back() == '\\' || path.back() == '/')) path.pop_back();
  return path;
}

}  // namespace uwf::app::paths
