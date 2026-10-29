// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

namespace uwf::app {

class FileExclusionRules final {
 public:
  [[nodiscard]] static QString rejectionReason(const QString& rawPath, const QString& volumeDl);
};

class RegistryExclusionRules final {
 public:
  [[nodiscard]] static QString rejectionReason(const QString& rawKey);
};

}  // namespace uwf::app
