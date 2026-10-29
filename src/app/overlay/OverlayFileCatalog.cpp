// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayFileCatalog.h"

#include <QDir>
#include <QHash>
#include <algorithm>
#include <limits>
#include <utility>

namespace uwf::app {
namespace {
qulonglong saturatingSizeSum(const qulonglong left, const qulonglong right) {
  const auto maximum = std::numeric_limits<qulonglong>::max();
  return right > maximum - left ? maximum : left + right;
}

// 移除文件名中的 NTFS 流后缀；末段首个冒号为流分隔符，盘符冒号不参与匹配。
// :$INDEX_ALLOCATION 标识目录；任一路径段以 $ 开头时按系统元数据处理，仅供诊断。
void normalizeOverlayEntry(const QString& driveLetter, OverlayFileEntry& e) {
  QString s = e.rawName;

  if (s.contains(QStringLiteral(":$INDEX_ALLOCATION"), Qt::CaseInsensitive)) {
    e.isDirectory = true;
  }

  const qsizetype lastSlash = s.lastIndexOf(QChar('\\'));
  const qsizetype searchFrom = (lastSlash >= 0) ? lastSlash : 0;
  const qsizetype streamColon = s.indexOf(QChar(':'), searchFrom);
  if (streamColon > searchFrom) s = s.left(streamColon);

  if (s.startsWith(QChar('\\'))) s = driveLetter + s;
  e.absolutePath = QDir::toNativeSeparators(s);

  const QStringList segs = e.absolutePath.split(QChar('\\'), Qt::SkipEmptyParts);
  e.isSystemMetadata = false;
  for (qsizetype i = 1; i < segs.size(); ++i) {
    if (segs[i].startsWith(QChar('$'))) {
      e.isSystemMetadata = true;
      break;
    }
  }
}

}  // namespace

QVector<OverlayFileEntry> buildOverlayFileCatalog(const QString& driveLetter, QVector<OverlayFileEntry> entries) {
  for (auto& e : entries) normalizeOverlayEntry(driveLetter, e);

  // 按不区分大小写的绝对路径合并 NTFS 流：累加字节数，合并目录和元数据标记，保留首个原始名称。
  QHash<QString, int> indexByKey;
  indexByKey.reserve(entries.size());
  QVector<OverlayFileEntry> dedup;
  dedup.reserve(entries.size());
  for (auto& e : entries) {
    const QString key = e.absolutePath.toLower();
    if (auto it = indexByKey.find(key); it != indexByKey.end()) {
      auto& dst = dedup[*it];
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
