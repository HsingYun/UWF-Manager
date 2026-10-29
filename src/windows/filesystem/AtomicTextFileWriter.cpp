// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "AtomicTextFileWriter.h"

#include <QSaveFile>

namespace uwf::windows {
app::TextFileWriteResult writeUtf8TextFile(const QString& path, const QString& text) {
  using Failure = app::TextFileWriteResult::Failure;
  QSaveFile file(path);
  if (!file.open(QIODevice::WriteOnly)) return {Failure::Open, file.errorString()};
  const auto encoded = text.toUtf8();
  if (file.write(encoded) != encoded.size()) {
    const auto error = file.errorString();
    file.cancelWriting();
    return {Failure::Write, error};
  }
  if (!file.commit()) return {Failure::Write, file.errorString()};
  return {};
}
}  // namespace uwf::windows
