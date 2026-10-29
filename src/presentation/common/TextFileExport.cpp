// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TextFileExport.h"

#include <exception>

#include "PresentationText.h"

namespace uwf::presentation {
QString exportTextFile(const QString& path, const QString& text, const app::TextFileWriter& writer) {
  if (!writer) return PresentationText::tr("Export failed");
  try {
    const auto result = writer(path, text);
    using Failure = app::TextFileWriteResult::Failure;
    switch (result.failure) {
      case Failure::None:
        return {};
      case Failure::Open:
        return PresentationText::tr("Could not open file for writing: %1").arg(result.error);
      case Failure::Write:
        return PresentationText::tr("Could not write file: %1").arg(result.error);
    }
  } catch (const std::exception& error) {
    return PresentationText::tr("Could not write file: %1").arg(QString::fromUtf8(error.what()));
  }
  return PresentationText::tr("Export failed");
}
}  // namespace uwf::presentation
