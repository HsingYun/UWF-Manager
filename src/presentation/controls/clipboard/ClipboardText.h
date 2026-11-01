// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <functional>

namespace uwf::ui {

// A receipt for this call's ownership/text observation. Confirmation does not
// promise that another application cannot subsequently replace the clipboard.
enum class ClipboardWriteResult { Confirmed, Failed };
using ClipboardTextWriter = std::function<ClipboardWriteResult(const QString&)>;

// Call on the GUI thread. Submit the Qt write once and observe its receipt;
// never retry a user command. Public Qt APIs do not expose the native HRESULT.
[[nodiscard]] ClipboardWriteResult writeClipboardText(const QString& text);

}  // namespace uwf::ui
