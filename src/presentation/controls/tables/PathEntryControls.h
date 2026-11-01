// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

class QAbstractItemModel;
class QLineEdit;
class QPushButton;
class QWidget;

namespace uwf::ui {
class PaginationBar;
class PathEntryListView;

// Shared list presentation only; callers supply translated labels and decide
// whether their model permits an add, removal or restoration.
struct PathEntryControls {
  QWidget* commands = nullptr;
  QLineEdit* filter = nullptr;
  PathEntryListView* list = nullptr;
  PaginationBar* pagination = nullptr;
  QPushButton* add = nullptr;
  QPushButton* remove = nullptr;

  void renderRemoval(bool restoring, const QString& label) const;
};

[[nodiscard]] PathEntryControls createPathEntryControls(QWidget& host, QAbstractItemModel& model, const QString& heading, const QString& statusHeading);
}  // namespace uwf::ui
