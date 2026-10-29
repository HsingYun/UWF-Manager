// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ImageCompare.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonArray>
#include <QSet>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace uwf::test::tools {
namespace {

QSet<QString> imageNames(const QDir& directory) {
  QSet<QString> names;
  QDirIterator files(directory.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
  while (files.hasNext()) {
    const auto path = files.next();
    if (QFileInfo(path).suffix().compare(QLatin1String("png"), Qt::CaseInsensitive) == 0) names.insert(directory.relativeFilePath(path));
  }
  return names;
}

QJsonArray sortedNames(const QSet<QString>& names) {
  auto sorted = names.values();
  sorted.sort();
  return QJsonArray::fromStringList(sorted);
}

}  // namespace

PixelDifference comparePixels(const QImage& reference, const QImage& actual) {
  if (reference.isNull() || actual.isNull()) throw std::runtime_error("Cannot compare an empty or unreadable image.");
  PixelDifference result;
  result.sizeMismatch = reference.size() != actual.size();
  if (result.sizeMismatch) return result;
  const QImage first = reference.convertToFormat(QImage::Format_RGBA8888);
  const QImage second = actual.convertToFormat(QImage::Format_RGBA8888);
  if (first.isNull() || second.isNull()) throw std::runtime_error("Cannot normalize image pixels.");
  int left = first.width();
  int top = first.height();
  int right = -1;
  int bottom = -1;
  for (int y = 0; y < first.height(); ++y) {
    const auto* a = first.constScanLine(y);
    const auto* b = second.constScanLine(y);
    if (std::memcmp(a, b, static_cast<std::size_t>(first.width()) * 4) == 0) continue;
    for (int x = 0; x < first.width(); ++x) {
      const auto offset = static_cast<qsizetype>(x) * 4;
      if (std::equal(a + offset, a + offset + 4, b + offset)) continue;
      ++result.changedPixels;
      left = std::min(left, x);
      top = std::min(top, y);
      right = std::max(right, x);
      bottom = std::max(bottom, y);
    }
  }
  if (result.changedPixels != 0) result.bounds = QRect(QPoint(left, top), QPoint(right, bottom));
  return result;
}

QJsonObject compareImageDirectories(const QString& reference, const QString& actual, const qsizetype expectedCount) {
  const QDir referenceDirectory(reference);
  const QDir actualDirectory(actual);
  if (!referenceDirectory.exists() || !actualDirectory.exists()) throw std::runtime_error("Both capture directories must exist.");
  if (QFileInfo(reference).canonicalFilePath().compare(QFileInfo(actual).canonicalFilePath(), Qt::CaseInsensitive) == 0)
    throw std::runtime_error("Reference and actual directories must differ.");
  if (expectedCount < 0) throw std::runtime_error("Expected image count cannot be negative.");
  const auto referenceNames = imageNames(referenceDirectory);
  const auto actualNames = imageNames(actualDirectory);
  if (referenceNames.isEmpty()) throw std::runtime_error("Reference capture has no PNG images.");
  QJsonArray differences;
  qsizetype identical = 0;
  auto names = referenceNames.values();
  names.sort();
  for (const auto& name : std::as_const(names)) {
    if (!actualNames.contains(name)) continue;
    const QImage first(referenceDirectory.filePath(name));
    const QImage second(actualDirectory.filePath(name));
    if (first.isNull() || second.isNull()) {
      differences.append(QJsonObject{{QStringLiteral("name"), name}, {QStringLiteral("error"), QStringLiteral("unreadable PNG")}});
      continue;
    }
    const auto difference = comparePixels(first, second);
    if (!difference.sizeMismatch && difference.changedPixels == 0) {
      ++identical;
      continue;
    }
    const auto& bounds = difference.bounds;
    differences.append(QJsonObject{{QStringLiteral("name"), name},
                                   {QStringLiteral("changedPixels"), difference.changedPixels},
                                   {QStringLiteral("sizeMismatch"), difference.sizeMismatch},
                                   {QStringLiteral("referenceSize"), QJsonArray{first.width(), first.height()}},
                                   {QStringLiteral("actualSize"), QJsonArray{second.width(), second.height()}},
                                   {QStringLiteral("bounds"), QJsonArray{bounds.x(), bounds.y(), bounds.width(), bounds.height()}}});
  }
  return {{QStringLiteral("passed"),
           referenceNames == actualNames && identical == referenceNames.size() && (expectedCount == 0 || referenceNames.size() == expectedCount)},
          {QStringLiteral("expected"), expectedCount == 0 ? referenceNames.size() : expectedCount},
          {QStringLiteral("referenceCount"), referenceNames.size()},
          {QStringLiteral("actualCount"), actualNames.size()},
          {QStringLiteral("identical"), identical},
          {QStringLiteral("tolerance"), 0},
          {QStringLiteral("maskedRegions"), 0},
          {QStringLiteral("missingActual"), sortedNames(referenceNames - actualNames)},
          {QStringLiteral("unexpectedActual"), sortedNames(actualNames - referenceNames)},
          {QStringLiteral("differences"), differences}};
}

}  // namespace uwf::test::tools
