// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstdint>
#include <optional>

namespace uwf::app {

enum class CommitOperation { Persist, DeleteAndPersist };

struct CommitTargetResult {
  std::optional<bool> existedBefore;
  std::optional<bool> existsAfter;
  bool targetAlreadyAbsent = false;
  int32_t hresult = 0;
  uint32_t returnValue = 0;
  QString failureDetail = {};
  QString authoritativeFailure = {};
  bool succeeded = false;
};

}  // namespace uwf::app
