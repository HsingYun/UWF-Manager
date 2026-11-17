// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>

namespace uwf::ui {
class CommitResultSummary final : public QWidget {
 public:
  CommitResultSummary(int succeeded, int failed, QWidget* parent = nullptr);
};
}  // namespace uwf::ui
