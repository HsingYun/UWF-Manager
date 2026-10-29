// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QString>
#include <exception>
namespace uwf::app {
struct OverlayFilesFailure {
  enum class Kind { General, ProviderCrashed, ResourcesOrUnsupported, Unexpected, Canceled };
  Kind kind = Kind::General;
  QString detail;
};
[[nodiscard]] OverlayFilesFailure classifyOverlayFilesFailure(std::exception_ptr failure);
}  // namespace uwf::app
