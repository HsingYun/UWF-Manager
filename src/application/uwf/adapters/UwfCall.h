// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/operations/contracts/OperationError.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
[[nodiscard]] WriteEffect uwfWriteEffect(compat::Effect effect);
[[nodiscard]] OperationFailureKind uwfFailureKind(compat::ErrorKind kind);
[[nodiscard]] std::string uwfDiagnostics(const compat::Error& error);
[[noreturn]] void throwUwfOperationError(const compat::Error& error);
[[nodiscard]] WriteEffect callUwfWrite(const compat::Uwf& system, const compat::Request& request, const compat::Context& context = {});

// Translate compatibility failures while preserving typed cancellation and write effects.
template <class R>
[[nodiscard]] typename R::Result callUwf(const compat::Uwf& system, const R& request, const compat::Context& context = {}) {
  try {
    return compat::call(system, request, context);
  } catch (const compat::Error& error) {
    throwUwfOperationError(error);
  }
}
}  // namespace uwf::app
