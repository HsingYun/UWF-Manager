// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <exception>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "application/commit/contracts/CommitResult.h"
#include "application/localization/ApplicationText.h"
#include "application/operations/contracts/OperationError.h"
#include "foundation/logging/Log.h"

namespace uwf::app {

namespace detail {

struct ExistenceObservation {
  std::optional<bool> value;
  QString failure;
};

template <typename ExistsFn, typename Target>
[[nodiscard]] ExistenceObservation observeExistence(ExistsFn& existsFn, const Target& target) {
  try {
    return {.value = existsFn(target), .failure = {}};
  } catch (const std::exception& error) {
    return {.value = std::nullopt, .failure = QString::fromUtf8(error.what())};
  } catch (...) {
    return {.value = std::nullopt, .failure = ApplicationText::tr("Unknown error while reading the target state.")};
  }
}

}  // namespace detail

template <typename Target, typename CommitFn, typename ExistsFn = decltype(nullptr)>
[[nodiscard]] CommitTargetResult executeCommitTarget(const Target& target, const QString& display, const CommitOperation operation, CommitFn& commitOne,
                                                     ExistsFn& existsFn) {
  constexpr bool kHasExists = !std::is_same_v<ExistsFn, decltype(nullptr)>;
  if constexpr (!kHasExists) {
    if (operation == CommitOperation::DeleteAndPersist) throw std::invalid_argument("deletion verification requires a target existence observation");
  }
  std::optional<bool> existedBefore;
  std::optional<bool> existsAfter;
  if constexpr (kHasExists) {
    const auto observation = detail::observeExistence(existsFn, target);
    existedBefore = observation.value;
    if (!observation.value) {
      UWF_LOG_W("commit") << "target state unavailable before operation; continuing with provider call: target=" << display.toStdString()
                          << " error=" << observation.failure.toStdString();
    }
  }
  OperationDiagnostic diagnostic;
  QString failureDetail = {};
  QString authoritativeFailure = {};
  bool succeeded = false;
  WriteEffect failureEffect = WriteEffect::None;
  auto failureKind = OperationFailureKind::Other;
  try {
    commitOne(target);
    succeeded = true;
  } catch (const OperationError& error) {
    diagnostic = error.diagnostic();
    failureKind = error.failureKind();
    failureEffect = error.writeEffect();
    failureDetail = QString::fromUtf8(error.what());
  } catch (const std::exception& error) {
    failureEffect = WriteEffect::Uncertain;
    failureDetail = QString::fromUtf8(error.what());
  } catch (...) {
    failureEffect = WriteEffect::Uncertain;
    failureDetail = ApplicationText::tr("The operation failed with an unknown error.");
  }
  if constexpr (kHasExists) {
    const auto observation = detail::observeExistence(existsFn, target);
    existsAfter = observation.value;
    if (!observation.value) {
      // Current-view observations do not establish durable deletion.
      if (operation == CommitOperation::DeleteAndPersist && (succeeded || failureEffect != WriteEffect::None)) {
        if (succeeded) failureEffect = WriteEffect::Uncertain;
        succeeded = false;
        authoritativeFailure = ApplicationText::tr("The target state could not be verified, so the operation result is unknown: %1").arg(observation.failure);
      }
    } else if (operation == CommitOperation::DeleteAndPersist && succeeded && *observation.value) {
      succeeded = false;
      failureEffect = WriteEffect::Uncertain;
      authoritativeFailure = ApplicationText::tr("The system accepted the deletion request, but the target still exists after verification.");
    }
  }

  return {.existedBefore = existedBefore,
          .existsAfter = existsAfter,
          .diagnostic = diagnostic,
          .failureDetail = std::move(failureDetail),
          .authoritativeFailure = std::move(authoritativeFailure),
          .succeeded = succeeded,
          .failureKind = failureKind,
          .failureEffect = failureEffect};
}

}  // namespace uwf::app
