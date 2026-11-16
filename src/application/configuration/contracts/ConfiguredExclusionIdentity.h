// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QStringView>

#include "foundation/text/StringUtil.h"

namespace uwf::app {
// Configuration-list operations use the captured management CRT C-locale
// comparison. Runtime file/key/value identity is a different contract.
[[nodiscard]] inline bool equalConfiguredExclusions(const QStringView left, const QStringView right) {
  return equalAsciiInsensitive(std::u16string_view(left.utf16(), static_cast<std::size_t>(left.size())),
                               std::u16string_view(right.utf16(), static_cast<std::size_t>(right.size())));
}
[[nodiscard]] inline QString configuredExclusionKey(const QStringView path) {
  return QString::fromStdU16String(toLowerAscii(path.toString().toStdU16String()));
}
}  // namespace uwf::app
