// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingCodec.h"

#include <QByteArray>
#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "app/ports/TextEncoding.h"
#include "shared/StringUtil.h"

namespace uwf::app {

namespace {

constexpr std::string_view kFilePrefix = "F|";
constexpr std::string_view kDirectoryPrefix = "D|";

std::string_view prefixFor(const FileStagingKind kind) {
  switch (kind) {
    case FileStagingKind::File:
      return kFilePrefix;
    case FileStagingKind::Directory:
      return kDirectoryPrefix;
  }
  throw std::invalid_argument("file staging entry has an unknown type");
}

}  // namespace

std::vector<std::string> encodeFileStagingEntries(const QList<FileStagingEntry>& entries) {
  std::vector<std::string> records;
  records.reserve(static_cast<std::size_t>(entries.size()));
  for (const auto& entry : entries) {
    if (entry.path.contains(QChar(u'\0'))) throw std::invalid_argument("file staging paths cannot contain NUL characters");
    if (!isAbsoluteLocalFileStagingPath(entry.path)) {
      throw std::invalid_argument("file staging entries require an absolute path on a local drive");
    }
    const QByteArray encodedPath = entry.path.toUtf8();
    std::string record(prefixFor(entry.kind));
    record.append(encodedPath.constData(), static_cast<std::size_t>(encodedPath.size()));
    records.push_back(std::move(record));
  }
  return records;
}

QList<FileStagingEntry> decodeFileStagingEntries(const std::vector<std::string>& records) {
  QList<FileStagingEntry> entries;
  entries.reserve(static_cast<qsizetype>(records.size()));
  for (const auto& record : records) {
    FileStagingKind kind;
    std::size_t prefixLength = 0;
    if (record.starts_with(kFilePrefix)) {
      kind = FileStagingKind::File;
      prefixLength = kFilePrefix.size();
    } else if (record.starts_with(kDirectoryPrefix)) {
      kind = FileStagingKind::Directory;
      prefixLength = kDirectoryPrefix.size();
    } else {
      throw std::runtime_error("file staging registry data contains an unknown entry type");
    }

    const QString path = decodeUtf8Text(std::string_view(record).substr(prefixLength));
    if (path.contains(QChar(u'\0'))) throw std::runtime_error("file staging registry data contains a NUL character");
    if (!isAbsoluteLocalFileStagingPath(path)) {
      throw std::runtime_error("file staging registry data contains a path outside a local drive");
    }
    entries.append({kind, path});
  }
  return entries;
}

}  // namespace uwf::app
