// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QStringView>
#include <compare>
#include <utility>

#include "foundation/text/ObjectNameComparison.h"

namespace uwf::app {
[[nodiscard]] inline int compareObjectNames(const QStringView left, const QStringView right) {
  return uwf::compareObjectNames(std::u16string_view(left.utf16(), static_cast<std::size_t>(left.size())),
                                 std::u16string_view(right.utf16(), static_cast<std::size_t>(right.size())));
}
[[nodiscard]] inline bool equalObjectNames(const QStringView left, const QStringView right) { return compareObjectNames(left, right) == 0; }

// Retain the original spelling: Windows ordinal identity has no Qt casefold
// canonical key. Ordered containers use the same host comparison as equality.
class ObjectIdentity final {
 public:
  ObjectIdentity() = default;
  explicit ObjectIdentity(QString name) : m_name(std::move(name)) {}
  [[nodiscard]] bool operator==(const ObjectIdentity& other) const { return equalObjectNames(m_name, other.m_name); }
  [[nodiscard]] std::strong_ordering operator<=>(const ObjectIdentity& other) const {
    const auto order = compareObjectNames(m_name, other.m_name);
    return order < 0 ? std::strong_ordering::less : order > 0 ? std::strong_ordering::greater : std::strong_ordering::equal;
  }

 private:
  QString m_name;
};

struct ObjectNameLess {
  using is_transparent = void;
  [[nodiscard]] bool operator()(const QStringView left, const QStringView right) const { return compareObjectNames(left, right) < 0; }
};
}  // namespace uwf::app
