// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowsErrorText.h"

#include <windows.h>

#include <QScopeGuard>

namespace uwf::windows {
QString windowsErrorText(const uint32_t code) {
  wchar_t* raw = nullptr;
  const DWORD count = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, 0,
                                     reinterpret_cast<wchar_t*>(&raw), 0, nullptr);
  const auto releaseMessage = qScopeGuard([raw] {
    if (raw) LocalFree(raw);
  });
  return count != 0 && raw ? QString::fromWCharArray(raw, static_cast<qsizetype>(count)).trimmed() : QStringLiteral("Windows error %1").arg(code);
}
}  // namespace uwf::windows
