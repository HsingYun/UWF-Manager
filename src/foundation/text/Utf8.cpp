// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "foundation/text/Utf8.h"

#include <cstddef>
#include <cstdint>

namespace uwf {
bool isValidUtf16(const std::u16string_view text) noexcept {
  for (std::size_t index = 0; index < text.size(); ++index) {
    const auto unit = text[index];
    if (unit >= 0xd800 && unit <= 0xdbff) {
      if (++index == text.size() || text[index] < 0xdc00 || text[index] > 0xdfff) return false;
    } else if (unit >= 0xdc00 && unit <= 0xdfff) {
      return false;
    }
  }
  return true;
}

std::optional<std::u16string> utf8ToUtf16(const std::string_view text) {
  std::u16string result;
  result.reserve(text.size());
  for (std::size_t offset = 0; offset < text.size();) {
    const auto first = static_cast<unsigned char>(text[offset++]);
    if (first < 0x80) {
      result.push_back(static_cast<char16_t>(first));
      continue;
    }
    std::uint32_t scalar;
    std::uint32_t minimum;
    std::size_t remaining;
    if (first >= 0xc2 && first <= 0xdf) {
      scalar = first & 0x1f;
      minimum = 0x80;
      remaining = 1;
    } else if (first >= 0xe0 && first <= 0xef) {
      scalar = first & 0x0f;
      minimum = 0x800;
      remaining = 2;
    } else if (first >= 0xf0 && first <= 0xf4) {
      scalar = first & 0x07;
      minimum = 0x10000;
      remaining = 3;
    } else {
      return std::nullopt;
    }
    if (text.size() - offset < remaining) return std::nullopt;
    for (std::size_t index = 0; index < remaining; ++index) {
      const auto next = static_cast<unsigned char>(text[offset++]);
      if ((next & 0xc0) != 0x80) return std::nullopt;
      scalar = (scalar << 6) | (next & 0x3f);
    }
    if (scalar < minimum || scalar > 0x10ffff || (scalar >= 0xd800 && scalar <= 0xdfff)) return std::nullopt;
    if (scalar <= 0xffff) {
      result.push_back(static_cast<char16_t>(scalar));
    } else {
      scalar -= 0x10000;
      result.push_back(static_cast<char16_t>(0xd800 + (scalar >> 10)));
      result.push_back(static_cast<char16_t>(0xdc00 + (scalar & 0x3ff)));
    }
  }
  return result;
}

std::string truncateUtf8(const std::string_view text, const std::size_t maxBytes, std::string_view suffix) {
  if (text.size() <= maxBytes) return std::string(text);
  if (suffix.size() > maxBytes) suffix = {};
  auto prefixSize = maxBytes - suffix.size();
  while (prefixSize > 0 && (static_cast<unsigned char>(text[prefixSize]) & 0xc0u) == 0x80u) --prefixSize;
  std::string result(text.substr(0, prefixSize));
  result.append(suffix);
  return result;
}
}  // namespace uwf
