// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>

#include <string>

namespace uwf::windows {
enum class FileIdentityFormat { FileIndex64, FileId128 };

[[nodiscard]] std::wstring processImagePath(HANDLE process);
[[nodiscard]] std::wstring currentExecutablePath();
// Compares file identities, not contents. Query failures return false.
[[nodiscard]] bool sameFile(const std::wstring& leftPath, const std::wstring& rightPath, FileIdentityFormat format);
}  // namespace uwf::windows
