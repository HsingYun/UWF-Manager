// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QString>
#include <exception>

#include "application/operations/contracts/OperationError.h"
namespace uwf::app {
// The read may still be executing in the provider. This is independent of
// mutation effects: enumeration remains read-only.
class OverlayFilesQueryUncertain final : public OperationError {
 public:
  explicit OverlayFilesQueryUncertain(std::string detail) : OperationError(std::make_error_code(std::errc::state_not_recoverable), std::move(detail)) {}
};
struct OverlayFilesFailure {
  enum class Kind { General, Unavailable, ResourcesOrUnsupported, Unexpected, Canceled, CompletionUnknown };
  Kind kind = Kind::General;
  QString detail;
};
[[nodiscard]] OverlayFilesFailure classifyOverlayFilesFailure(std::exception_ptr failure);
}  // namespace uwf::app
