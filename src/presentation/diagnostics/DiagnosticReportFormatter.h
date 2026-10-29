// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/DiagnosticReportTypes.h"
namespace uwf::presentation {
class DiagnosticReportFormatter final {
 public:
  [[nodiscard]] static QString format(const app::DiagnosticReportData& report);
};
}  // namespace uwf::presentation
