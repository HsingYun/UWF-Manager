// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
class QWidget;
namespace uwf::presentation { class ApplyPlanViewModel; }
namespace uwf::ui {
class RecyclingListView;
RecyclingListView* createApplyPlanList(presentation::ApplyPlanViewModel& model, QWidget* commands, QWidget* parent);
}
