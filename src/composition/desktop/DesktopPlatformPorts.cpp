// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/desktop/DesktopPlatformPorts.h"

#include <stdexcept>

#include "application/settings/UserConfig.h"

namespace uwf::composition {
void validateDesktopPlatformPorts(const DesktopPlatformPorts& ports) {
  if (!ports.volumes || !ports.files || !ports.traversal || !ports.registry || !ports.systemInformation || !ports.log || !ports.fileDialogs ||
      !ports.readText || !ports.writeText || !ports.createText)
    throw std::invalid_argument("desktop platform ports must be supplied explicitly");
  static_cast<void>(app::userConfigPath(ports.applicationDirectory));
}
}  // namespace uwf::composition
