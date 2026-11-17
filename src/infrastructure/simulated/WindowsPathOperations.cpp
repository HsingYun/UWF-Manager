// SPDX-License-Identifier: GPL-3.0-or-later
#include "application/paths/contracts/WindowsPathOperations.h"

#include <QDir>
#include <QFileInfo>

namespace uwf::app::paths::platform {
namespace {
bool driveAbsolute(const QString& path) {
  return path.size() >= 3 && path[1] == u':' && (path[2] == u'/' || path[2] == u'\\') &&
         ((path[0] >= u'A' && path[0] <= u'Z') || (path[0] >= u'a' && path[0] <= u'z'));
}
}  // namespace
QString nativeSeparators(const QString& path) {
  auto result = path;
  return result.replace(u'/', u'\\');
}
QString portableSeparators(const QString& path) {
  auto result = path;
  return result.replace(u'\\', u'/');
}
QString clean(const QString& path) {
  const auto portable = portableSeparators(path);
  if (driveAbsolute(portable)) return portable.left(2) + QDir::cleanPath(portable.mid(2));
  auto result = QDir::cleanPath(portable);
  if (portable.startsWith(QLatin1String("//")) && !result.startsWith(QLatin1String("//"))) result.prepend(u'/');
  return result;
}
bool fullyQualified(const QString& path) {
  const auto native = nativeSeparators(path);
  return driveAbsolute(native) || native.startsWith(QLatin1String(R"(\\?\)")) || native.startsWith(QLatin1String(R"(\\.\)")) ||
         native.startsWith(QLatin1String("\\\\"));
}
bool absolute(const QString& path) { return driveAbsolute(path) || path.startsWith(u'/') || path.startsWith(u'\\'); }
QString fileName(const QString& path) {
  const auto address = portableSeparators(path);
  return address.mid(address.lastIndexOf(u'/') + 1);
}
QString parent(const QString& path) {
  auto address = clean(path);
  if (driveAbsolute(address)) {
    while (address.size() > 3 && address.endsWith(u'/')) address.chop(1);
    return address.left(qMax<qsizetype>(3, address.lastIndexOf(u'/')));
  }
  return QFileInfo(address).absolutePath();
}
QString join(const QString& directory, const QString& path) { return QDir(directory).filePath(portableSeparators(path)); }
QString resolve(const QString& directory, const QString& path) {
  if (driveAbsolute(path) || path.startsWith(QLatin1String("//"))) return clean(path);
  if (driveAbsolute(directory)) return clean(path.startsWith(u'/') ? directory.left(2) + path : directory + u'/' + path);
  return QDir(directory).absoluteFilePath(path);
}
}  // namespace uwf::app::paths::platform
