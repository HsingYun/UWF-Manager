// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// clang-format off
#include <Windows.h>
#include <OleAuto.h>
// clang-format on

#include <cstddef>
#include <cstdint>

namespace uwf::windows::backends::native {
// Native x64 ATL output ABI, assumed stable across DLL versions.
// Items are BSTRs; the buffer requires the producing module's bound CRT free.
struct StringArray {
  BSTR* items = nullptr;
  std::int32_t count = 0;
  std::int32_t capacity = 0;
};
static_assert(sizeof(void*) != 8 || sizeof(StringArray) == 16);
static_assert(offsetof(StringArray, count) == sizeof(void*));

using Deallocate = void(__cdecl*)(void*);
using GetStrings = HRESULT(__cdecl*)(bool current, StringArray* output);
using GetVolumeStrings = HRESULT(__cdecl*)(bool current, PCWSTR volume, StringArray* output);
}  // namespace uwf::windows::backends::native
