/*
* Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ByteFormat.h"

#include <format>

namespace uwf {

std::string formatBytes(uint64_t bytes) {
  constexpr double KB = 1024.0;
  constexpr double MB = 1024.0 * 1024.0;
  constexpr double GB = 1024.0 * 1024.0 * 1024.0;
  constexpr double TB = 1024.0 * 1024.0 * 1024.0 * 1024.0;
  const auto b = static_cast<double>(bytes);
  if (b >= TB) return std::format("{:.2f} TB", b / TB);
  if (b >= GB) return std::format("{:.2f} GB", b / GB);
  if (b >= MB) return std::format("{:.1f} MB", b / MB);
  if (b >= KB) return std::format("{:.1f} KB", b / KB);
  return std::format("{} B", bytes);
}

std::string formatMb(uint32_t mb) {
  constexpr uint64_t GB = 1024ULL;
  constexpr uint64_t TB = 1024ULL * 1024;
  constexpr uint64_t PB = 1024ULL * 1024 * 1024;
  if (mb >= PB && mb % PB == 0) return std::format("{} PB", mb / PB);
  if (mb >= TB && mb % TB == 0) return std::format("{} TB", mb / TB);
  if (mb >= GB && mb % GB == 0) return std::format("{} GB", mb / GB);
  if (mb >= PB) return std::format("{:.2f} PB", static_cast<double>(mb) / static_cast<double>(PB));
  if (mb >= TB) return std::format("{:.2f} TB", static_cast<double>(mb) / static_cast<double>(TB));
  if (mb >= GB) return std::format("{:.2f} GB", static_cast<double>(mb) / static_cast<double>(GB));
  return std::format("{} MB", mb);
}

}  // namespace uwf
