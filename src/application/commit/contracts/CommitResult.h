// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <optional>

#include "application/operations/contracts/OperationError.h"

namespace uwf::app {

enum class CommitOperation { Persist, DeleteAndPersist };
enum class CommitTargetKind { File, Folder, RegistryKey, RegistryValue };

struct CommitTargetResult {
  std::optional<bool> existedBefore;
  std::optional<bool> existsAfter;
  OperationDiagnostic diagnostic{};
  QString failureDetail = {};
  QString authoritativeFailure = {};
  bool succeeded = false;
  OperationFailureKind failureKind = OperationFailureKind::Other;
  WriteEffect failureEffect = WriteEffect::None;
};

}  // namespace uwf::app
