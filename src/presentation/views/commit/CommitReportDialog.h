// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QString>
#include <cstdint>
#include <optional>

#include "presentation/viewmodels/commit/CommitReportViewModel.h"

class QWidget;

namespace uwf::ui {

// Modal report with cancellation and interruption tracked separately from write failures.
void showCommitReport(QWidget* parent, const QList<presentation::CommitReportRow>& rows, app::CommitOperation operation,
                      presentation::CommitCompletion completion = {});

}  // namespace uwf::ui
