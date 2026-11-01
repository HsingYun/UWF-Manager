// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

namespace uwf::app {

class FileExclusionRules final {
 public:
  [[nodiscard]] static QString rejectionReason(const QString& rawPath, const QString& volumeDriveLetter, const QString& systemDrive);
};

class RegistryExclusionRules final {
 public:
  // Validate the supplied spelling without cleaning literal registry key names.
  // Picker selections and imported configuration patterns keep their names.
  [[nodiscard]] static QString rejectionReason(const QString& rawKey);
};

}  // namespace uwf::app
