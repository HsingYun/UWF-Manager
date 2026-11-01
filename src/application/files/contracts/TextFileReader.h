// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QByteArray>
#include <QString>
#include <functional>

namespace uwf::app {
struct TextFileReadResult {
  enum class Failure { None, Open, Read, TooLarge };
  Failure failure = Failure::None;
  QString error;
  QByteArray bytes;
};
using TextFileReader = std::function<TextFileReadResult(const QString& path, qint64 maximumBytes)>;
}  // namespace uwf::app
