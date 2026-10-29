// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayFilesFailure.h"

#include <optional>

#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
namespace uwf::app {
OverlayFilesFailure classifyOverlayFilesFailure(std::exception_ptr failure) {
  OverlayFilesFailure result;
  std::optional<int32_t> code;
  try {
    std::rethrow_exception(failure);
  } catch (const wmi::WmiCancelled&) {
    result.kind = OverlayFilesFailure::Kind::Canceled;
    return result;
  } catch (const wmi::WmiException& error) {
    result.detail = QString::fromUtf8(error.what());
    if (error.code().category() == wmi::wmiErrorCategory()) code = static_cast<int32_t>(error.code().value());
  } catch (const std::exception& error) {
    result.detail = QString::fromUtf8(error.what());
  } catch (...) {
    result.kind = OverlayFilesFailure::Kind::Unexpected;
    return result;
  }
  const wmi::WmiError error(code.value_or(0));
  if (error == wmi::WmiErrorCode::RpcServerFault)
    result.kind = OverlayFilesFailure::Kind::ProviderCrashed;
  else if (error == wmi::WmiErrorCode::OutOfMemory || error == wmi::WmiErrorCode::NotSupported)
    result.kind = OverlayFilesFailure::Kind::ResourcesOrUnsupported;
  return result;
}
}  // namespace uwf::app
