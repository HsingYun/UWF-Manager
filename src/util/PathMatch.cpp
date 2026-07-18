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

namespace {

bool isPathSeparator(const unsigned char ch) { return ch == '\\' || ch == '/'; }

unsigned char foldedPathCharacter(const unsigned char ch) {
  if (isPathSeparator(ch)) return '\\';
  if (ch >= 'A' && ch <= 'Z') return static_cast<unsigned char>(ch - 'A' + 'a');
  return ch;
}

}  // namespace

std::string stripTrailingSep(std::string s) {
  while (!s.empty() && isPathSeparator(static_cast<unsigned char>(s.back()))) s.pop_back();
  return s;
}

bool pathIsExcludedBy(const std::string& target, const std::string& prefix) {
  if (prefix.empty() || target.size() < prefix.size()) return false;
  for (size_t i = 0; i < prefix.size(); ++i) {
    const auto targetCharacter = foldedPathCharacter(static_cast<unsigned char>(target[i]));
    const auto prefixCharacter = foldedPathCharacter(static_cast<unsigned char>(prefix[i]));
    if (targetCharacter != prefixCharacter) return false;
  }
  if (target.size() == prefix.size()) return true;
  return isPathSeparator(static_cast<unsigned char>(target[prefix.size()]));
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
