// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QListView>

#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/views/staging/FileStagingWidget.h"

namespace uwf::test {
// Resolve only the staging widget's named shared list, preserving a failure
// when either its semantic identity or concrete component type is wrong.
[[nodiscard]] inline ui::PathEntryListView* stagingEntryList(const ui::FileStagingWidget& widget) {
  return dynamic_cast<ui::PathEntryListView*>(widget.findChild<QListView*>(QStringLiteral("stagingEntries")));
}
}  // namespace uwf::test
