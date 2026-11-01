// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QString>
#include <functional>

namespace uwf::app {
struct TextFileWriteResult {
  enum class Failure { None, Open, Write };
  Failure failure = Failure::None;
  QString error;
};
using TextFileWriter = std::function<TextFileWriteResult(const QString& path, const QString& text)>;
}  // namespace uwf::app
