// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/overlay/files/OverlayFilesFailure.h"

#include "application/operations/contracts/OperationError.h"
namespace uwf::app {
OverlayFilesFailure classifyOverlayFilesFailure(std::exception_ptr failure) {
  OverlayFilesFailure result;
  try {
    std::rethrow_exception(failure);
  } catch (const OverlayFilesQueryUncertain& error) {
    result.detail = QString::fromUtf8(error.what());
    result.kind = OverlayFilesFailure::Kind::CompletionUnknown;
  } catch (const OperationError& error) {
    result.detail = QString::fromUtf8(error.what());
    switch (error.failureKind()) {
      case OperationFailureKind::Canceled:
        result.kind = OverlayFilesFailure::Kind::Canceled;
        break;
      case OperationFailureKind::Unavailable:
        result.kind = OverlayFilesFailure::Kind::Unavailable;
        break;
      case OperationFailureKind::Unsupported:
      case OperationFailureKind::ResourceExhausted:
        result.kind = OverlayFilesFailure::Kind::ResourcesOrUnsupported;
        break;
      default:
        break;
    }
  } catch (const std::exception& error) {
    result.detail = QString::fromUtf8(error.what());
  } catch (...) {
    result.kind = OverlayFilesFailure::Kind::Unexpected;
  }
  return result;
}
}  // namespace uwf::app
