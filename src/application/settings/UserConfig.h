// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <optional>

#include "application/settings/UiPreferences.h"

namespace uwf::app {

struct UserConfig {
  // Unspecified overrides follow the system at startup; these are not resolved values.
  std::optional<Theme> theme;
  std::optional<LanguageId> language;
  bool hubVisible = true;
};

// Require an absolute directory so shortcuts and services cannot change the configuration location.
[[nodiscard]] QString userConfigPath(const QString& applicationDirectory);
// Scalar options decoded by a storage adapter. Interpretation and independent
// fallback of invalid preferences belong to the application.
struct UserConfigOptions {
  std::optional<QString> theme;
  std::optional<QString> language;
  std::optional<QString> hubVisible;
};
[[nodiscard]] UserConfig interpretUserConfig(const UserConfigOptions& options, const QString& source = {});
// Serialize without writing storage; unspecified overrides remain "system".
[[nodiscard]] QString serializeUserConfig(const UserConfig& config);

}  // namespace uwf::app
