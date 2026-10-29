// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <functional>

namespace uwf::app {
struct TextFileCreationResult {
  enum class Outcome { Created, AlreadyExists, Failed };
  Outcome outcome = Outcome::Created;
  QString error;
};
// Publishes complete UTF-8 text only when no file-system entry already occupies the absolute target path.
using TextFileCreator = std::function<TextFileCreationResult(const QString& path, const QString& text)>;
}  // namespace uwf::app
