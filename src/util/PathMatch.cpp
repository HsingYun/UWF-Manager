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
#include "PathMatch.h"

namespace uwf {

std::string stripTrailingSep(std::string s) {
  while (!s.empty() && (s.back() == '\\' || s.back() == '/')) s.pop_back();
  return s;
}

bool pathIsExcludedBy(const std::string& target, const std::string& prefix) {
  if (prefix.empty() || target.size() < prefix.size()) return false;
  for (size_t i = 0; i < prefix.size(); ++i) {
    auto a = static_cast<unsigned char>(target[i]);
    auto b = static_cast<unsigned char>(prefix[i]);
    if (a >= 'A' && a <= 'Z') a = a - 'A' + 'a';
    if (b >= 'A' && b <= 'Z') b = b - 'A' + 'a';
    if (a != b) return false;
  }
  if (target.size() == prefix.size()) return true;
  const char next = target[prefix.size()];
  return next == '\\' || next == '/';
}

std::string findCoveringExclusion(const std::vector<std::string>& excls, const std::string& target) {
  const std::string t = stripTrailingSep(target);
  for (const auto& e : excls) {
    const std::string p = stripTrailingSep(e);
    if (pathIsExcludedBy(t, p)) return e;
  }
  return {};
}

}  // namespace uwf
