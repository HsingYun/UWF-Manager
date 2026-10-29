// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QModelIndex>
#include <Qt>
#include <algorithm>

namespace uwf::presentation {
enum class ItemStatus { Muted, Information, Warning, Error, Success, Normal, Added, Removed, Unsupported, Failed, Faint };
enum ItemRole {
  PathRole = Qt::UserRole,
  DirectoryRole,
  SystemMetadataRole,
  StatusRole = Qt::UserRole + 16,
  StatusIconRole,
  IconResourceRole,
  BadgeRole,
  SeparatorRole,
  RequiredRole,
  PersistenceRole,
  FullTextRole,
  RowStatusRole,
  MonospaceRole,
  SecondaryTextRole,
  StatusBadgeRole,
};
enum class ItemBadge { None, Add, Remove };

[[nodiscard]] inline bool isRemovedSelection(const QModelIndexList& selection) {
  return !selection.isEmpty() &&
         std::ranges::all_of(selection, [](const QModelIndex& index) { return index.data(StatusRole).toInt() == static_cast<int>(ItemStatus::Removed); });
}
}  // namespace uwf::presentation
