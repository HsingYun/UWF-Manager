// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitFailure.h"

#include "app/wmi/WmiError.h"
namespace uwf::app {
CommitFailureKind classifyCommitFailure(const int32_t hresult, const uint32_t returnValue) {
  const int32_t code = hresult != 0 ? hresult : static_cast<int32_t>(returnValue);
  if (code == 0) return CommitFailureKind::None;
  switch (wmi::WmiError(code).code()) {
    case wmi::WmiErrorCode::Failed:
      return CommitFailureKind::GeneralFailure;
    case wmi::WmiErrorCode::NotFound:
      return CommitFailureKind::NotFound;
    case wmi::WmiErrorCode::InvalidParameter:
      return CommitFailureKind::InvalidParameter;
    default:
      return CommitFailureKind::Other;
  }
}
}  // namespace uwf::app
