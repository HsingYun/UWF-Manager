// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::app {

[[nodiscard]] FileStagingCommandResult toFileStagingCommandResult(const FileStagingCommitResult& result, QString failureDetail);

}  // namespace uwf::app
