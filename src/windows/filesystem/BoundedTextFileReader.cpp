// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "BoundedTextFileReader.h"

#include <QFile>
#include <limits>
#include <stdexcept>

namespace uwf::windows {
app::TextFileReadResult readBoundedTextFile(const QString& path, const qint64 maximumBytes) {
  if (maximumBytes < 0 || maximumBytes == std::numeric_limits<qint64>::max()) throw std::invalid_argument("invalid text read limit");
  using Failure = app::TextFileReadResult::Failure;
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return {Failure::Open, file.errorString(), {}};
  auto bytes = file.read(maximumBytes + 1);
  if (file.error() != QFileDevice::NoError) return {Failure::Read, file.errorString(), {}};
  if (bytes.size() > maximumBytes || !file.atEnd()) return {Failure::TooLarge, {}, {}};
  return {Failure::None, {}, std::move(bytes)};
}
}  // namespace uwf::windows
