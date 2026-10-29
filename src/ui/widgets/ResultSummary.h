// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QWidget>
namespace uwf::ui {
class ResultSummary final : public QWidget {
 public:
  ResultSummary(int succeeded, int failed, int skipped, QWidget* parent = nullptr);
};
}  // namespace uwf::ui
