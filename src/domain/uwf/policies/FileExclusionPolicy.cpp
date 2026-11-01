// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/policies/FileExclusionPolicy.h"

#include <cstddef>

namespace uwf::core {
FileExclusionError fileExclusionError(std::u16string_view path) {
  if (path.find(u'\0') != std::u16string_view::npos) return FileExclusionError::EmbeddedNull;
  constexpr std::u16string_view extendedPrefix = uR"(\\?\)";
  constexpr std::size_t ordinaryLimit = 260;
  constexpr std::size_t extendedLimit = 32767;
  const bool extended = path.starts_with(extendedPrefix);
  if (extended) path.remove_prefix(extendedPrefix.size());
  if (path.empty()) return FileExclusionError::Empty;
  constexpr std::u16string_view forbidden = u"<>:\"/|?*";
  for (std::size_t index = 0; index < path.size(); ++index) {
    if (path[index] < u'\x1f' || forbidden.find(path[index]) != std::u16string_view::npos) return FileExclusionError::InvalidCharacter;
    if (index >= (extended ? extendedLimit : ordinaryLimit)) return FileExclusionError::TooLong;
  }
  return FileExclusionError::None;
}
}  // namespace uwf::core
