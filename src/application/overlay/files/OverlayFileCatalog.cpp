// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/overlay/files/OverlayFileCatalog.h"

#include <QStringView>
#include <algorithm>
#include <array>
#include <limits>
#include <map>
#include <string_view>
#include <utility>

#include "application/identity/contracts/ObjectIdentity.h"
#include "application/paths/PathPolicy.h"

namespace uwf::app {
namespace {
constexpr std::array<std::u16string_view, 11> ntfsRootMetadataNames{u"$MFT",  u"$MFTMirr", u"$LogFile", u"$Volume", u"$AttrDef", u"$Bitmap",
                                                                    u"$Boot", u"$BadClus", u"$Secure",  u"$UpCase", u"$Extend"};
bool ntfsMetadataPath(const QString& path) {
  constexpr qsizetype driveRootPrefixLength = 3;  // Drive letter, colon and separator.
  const QStringView absolute(path);
  if (absolute.size() < driveRootPrefixLength || absolute[1] != u':' || absolute[2] != u'\\') return false;
  const auto relative = absolute.sliced(driveRootPrefixLength);
  const auto separator = relative.indexOf(u'\\');
  const auto rootName = separator < 0 ? relative : relative.first(separator);
  return std::ranges::any_of(ntfsRootMetadataNames,
                             [rootName](const auto name) { return equalObjectNames(rootName, QStringView(name.data(), static_cast<qsizetype>(name.size()))); });
}
qulonglong saturatingSizeSum(const qulonglong left, const qulonglong right) {
  const auto maximum = std::numeric_limits<qulonglong>::max();
  return right > maximum - left ? maximum : left + right;
}

// Remove NTFS stream suffixes from the final path component.
// The index-allocation stream identifies a directory; only known root metadata names are diagnostic-only.
void normalizeOverlayEntry(const QString& driveLetter, OverlayFileEntry& e) {
  QString s = e.rawName;

  if (s.endsWith(QStringLiteral(":$INDEX_ALLOCATION"), Qt::CaseInsensitive)) {
    e.isDirectory = true;
  }

  const qsizetype lastSlash = s.lastIndexOf(QChar('\\'));
  const qsizetype searchFrom = (lastSlash >= 0) ? lastSlash : 0;
  const qsizetype streamColon = s.indexOf(QChar(':'), searchFrom);
  if (streamColon > searchFrom) s = s.left(streamColon);

  if (s.startsWith(QChar('\\'))) s = driveLetter + s;
  e.absolutePath = paths::windowsSeparators(s);

  e.isSystemMetadata = ntfsMetadataPath(e.absolutePath);
}

}  // namespace

QVector<OverlayFileEntry> buildOverlayFileCatalog(const QString& driveLetter, QVector<OverlayFileEntry> entries) {
  for (auto& e : entries) normalizeOverlayEntry(driveLetter, e);

  // Merge streams by object identity, retaining the first raw name and combined size/flags.
  std::map<QString, int, ObjectNameLess> indexByKey;
  QVector<OverlayFileEntry> dedup;
  dedup.reserve(entries.size());
  for (auto& e : entries) {
    const auto& key = e.absolutePath;
    if (auto it = indexByKey.find(key); it != indexByKey.end()) {
      auto& dst = dedup[it->second];
      dst.fileSize = saturatingSizeSum(dst.fileSize, e.fileSize);
      if (e.isDirectory) dst.isDirectory = true;
      if (e.isSystemMetadata) dst.isSystemMetadata = true;
    } else {
      indexByKey[key] = static_cast<int>(dedup.size());
      dedup.append(std::move(e));
    }
  }
  entries = std::move(dedup);

  std::ranges::sort(entries,
                    [](const OverlayFileEntry& a, const OverlayFileEntry& b) { return a.absolutePath.compare(b.absolutePath, Qt::CaseInsensitive) < 0; });

  return entries;
}

qulonglong overlayFileCatalogSize(const QVector<OverlayFileEntry>& entries) {
  qulonglong total = 0;
  for (const auto& entry : entries) total = saturatingSizeSum(total, entry.fileSize);
  return total;
}
}  // namespace uwf::app
