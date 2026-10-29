// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "app/api/FileStagingTypes.h"
#include "app/startup/ApplicationCommand.h"

namespace uwf::app {

[[nodiscard]] ApplicationCommandResult toApplicationCommandResult(const FileStagingCommitResult& result, QString failureDetail);

}  // namespace uwf::app
