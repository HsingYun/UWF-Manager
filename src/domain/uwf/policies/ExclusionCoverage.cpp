// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/policies/ExclusionCoverage.h"

#include <limits>
#include <stdexcept>

#include "foundation/paths/RegistryPath.h"
#include "foundation/text/ObjectNameComparison.h"
#include "foundation/text/StringUtil.h"

namespace uwf::core {
namespace {
constexpr std::u16string_view systemNamespace = u"HKEY_LOCAL_MACHINE\\SYSTEM";
constexpr std::u16string_view currentControlSetMarker = u"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet";
constexpr std::u16string_view numberedControlSetPrefix = u"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet";
constexpr std::size_t minimumControlSetDigits = 3;
bool ordinalStartsWith(const std::u16string_view value, const std::u16string_view prefix) {
  return value.size() >= prefix.size() && compareObjectNames(value.substr(0, prefix.size()), prefix) == 0;
}
std::u16string controlSetNumber(const std::uint32_t value) {
  auto number = std::to_string(value);
  if (number.size() < minimumControlSetDigits) number.insert(0, minimumControlSetDigits - number.size(), '0');
  return {number.begin(), number.end()};
}
std::u16string translateCurrentControlSet(const std::u16string_view key, const std::uint32_t currentControlSet, const bool pattern) {
  auto name = regkey::expandHive(key);
  if (!ordinalStartsWith(name, currentControlSetMarker) ||
      (!pattern && name.size() > currentControlSetMarker.size() && name[currentControlSetMarker.size()] != u'\\'))
    return name;
  std::u16string translated(numberedControlSetPrefix);
  translated += controlSetNumber(currentControlSet);
  translated.append(name.substr(currentControlSetMarker.size()));
  return translated;
}
std::u16string_view withoutOneTrailingSeparator(std::u16string_view path) {
  if (path.ends_with(u'\\')) path.remove_suffix(1);
  return path;
}
bool driverPrefix(const std::u16string_view pattern, const std::u16string_view target) {
  return !pattern.empty() && target.size() >= pattern.size() && compareObjectNames(target.substr(0, pattern.size()), pattern) == 0 &&
         (target.size() == pattern.size() || pattern == u"\\" || target[pattern.size()] == u'\\');
}
bool asciiPrefix(const std::u16string_view pattern, const std::u16string_view target) {
  return target.size() >= pattern.size() && equalAsciiInsensitive(target.substr(0, pattern.size()), pattern) &&
         (target.size() == pattern.size() || target[pattern.size()] == u'\\');
}
bool registryManagementPrefix(const std::u16string_view stored, const std::u16string_view actual) {
  const auto patternSeparator = stored.find(u'\\'), targetSeparator = actual.find(u'\\');
  if (!equalAsciiInsensitive(stored.substr(0, patternSeparator), actual.substr(0, targetSeparator))) return false;
  const auto relativePattern = patternSeparator == std::u16string_view::npos ? std::u16string_view{} : stored.substr(patternSeparator + 1);
  const auto relativeTarget = targetSeparator == std::u16string_view::npos ? std::u16string_view{} : actual.substr(targetSeparator + 1);
  return asciiPrefix(withoutOneTrailingSeparator(relativePattern), withoutOneTrailingSeparator(relativeTarget));
}
bool possibleNumberedControlSetMatch(const std::u16string_view stored, const std::u16string_view actual, const bool patternAlias) {
  const auto untranslated = patternAlias ? actual : stored;
  if (!ordinalStartsWith(untranslated, numberedControlSetPrefix)) return false;
  const auto numberAndSuffix = untranslated.substr(numberedControlSetPrefix.size());
  constexpr std::size_t maximumDigits = std::numeric_limits<std::uint32_t>::digits10 + 1;
  std::uint64_t value = 0;
  for (std::size_t index = 0; index < numberAndSuffix.size() && index < maximumDigits; ++index) {
    const auto code = numberAndSuffix[index];
    if (code < u'0' || code > u'9') break;
    value = value * 10 + static_cast<std::uint32_t>(code - u'0');
    if (value > std::numeric_limits<std::uint32_t>::max()) break;
    const auto candidate = static_cast<std::uint32_t>(value);
    if (index + 1 < minimumControlSetDigits || controlSetNumber(candidate) != numberAndSuffix.substr(0, index + 1)) continue;
    if (driverPrefix(translateCurrentControlSet(stored, candidate, true), translateCurrentControlSet(actual, candidate, false))) return true;
  }
  return false;
}
std::optional<bool> registryCoverageWithoutControlSet(const std::u16string_view stored, const std::u16string_view actual) {
  const bool patternAlias = ordinalStartsWith(stored, currentControlSetMarker);
  const bool targetAlias = driverPrefix(currentControlSetMarker, actual);
  // Replacing the same prefix on both sides cannot change their suffix
  // comparison or component boundary. Ordinary names need no translation.
  if (patternAlias == targetAlias) return driverPrefix(stored, actual);
  if (targetAlias && driverPrefix(stored, systemNamespace)) return true;
  // The fixed decimal format admits only bounded numeric prefixes. Preserve
  // arbitrary original suffixes: CSS1 can translate to ControlSet0011, whereas
  // ControlSetOther, short digits or a mismatching suffix cannot overlap.
  if (!possibleNumberedControlSetMatch(stored, actual, patternAlias)) return false;
  return {};
}
}  // namespace
ExclusionEffects fileExclusionEffects(const std::u16string_view pattern, const std::u16string_view target) {
  if (pattern.empty() || target.empty()) return {};
  const auto parent = withoutOneTrailingSeparator(pattern);
  const bool descendant = target.size() > parent.size() && target[parent.size()] == u'\\' && equalAsciiInsensitive(target.substr(0, parent.size()), parent);
  return {equalAsciiInsensitive(pattern, target) || descendant, driverPrefix(pattern, target)};
}
bool registryExclusionNeedsCurrentControlSet(const std::u16string_view pattern, const std::u16string_view target) {
  const auto stored = regkey::expandHive(pattern), actual = regkey::expandHive(target);
  return !registryCoverageWithoutControlSet(stored, actual).has_value();
}
bool registryManagementSkipsCommit(const std::u16string_view pattern, const std::u16string_view target) {
  if (pattern.empty() || target.empty()) return false;
  return registryManagementPrefix(regkey::expandHive(pattern), regkey::expandHive(target));
}
std::u16string registryDriverObjectName(const std::u16string_view key, const std::uint32_t currentControlSet) {
  return translateCurrentControlSet(key, currentControlSet, false);
}
ExclusionEffects registryExclusionEffects(const std::u16string_view pattern, const std::u16string_view target,
                                          const std::optional<std::uint32_t> currentControlSet) {
  if (pattern.empty() || target.empty()) return {};
  const auto resolvedPattern = regkey::expandHive(pattern);
  const auto resolvedTarget = regkey::expandHive(target);
  const std::u16string_view stored(resolvedPattern), actual(resolvedTarget);
  const bool management = registryManagementPrefix(stored, actual);
  if (const auto known = registryCoverageWithoutControlSet(stored, actual)) return {management, *known};
  if (!currentControlSet) throw std::invalid_argument("The current registry control set is required to determine UWF exclusion coverage");
  return {management, driverPrefix(translateCurrentControlSet(stored, *currentControlSet, true), registryDriverObjectName(actual, *currentControlSet))};
}
}  // namespace uwf::core
