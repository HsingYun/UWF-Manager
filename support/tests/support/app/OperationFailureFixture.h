// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/operations/contracts/OperationError.h"

namespace uwf::test::application {
class DetailedOperationFailure final : public app::OperationError {
 public:
  DetailedOperationFailure(app::OperationFailureKind kind, app::WriteEffect effect)
      : OperationError(std::make_error_code(std::errc::io_error), "Original provider failure\nExact diagnostic retained", effect), m_kind(kind) {}
  app::OperationFailureKind failureKind() const noexcept override { return m_kind; }
  app::OperationDiagnostic diagnostic() const noexcept override { return {1234, 0x80070005u}; }

 private:
  app::OperationFailureKind m_kind;
};
}  // namespace uwf::test::application
