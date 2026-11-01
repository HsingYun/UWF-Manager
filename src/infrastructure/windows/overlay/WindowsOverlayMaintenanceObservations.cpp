// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/overlay/WindowsOverlayMaintenanceObservations.h"

#include <QStringList>
#include <stdexcept>
#include <utility>

#include "infrastructure/uwf/windows/RuntimeAdmissionFacts.h"
#include "infrastructure/windows/power/WindowsHibernationCapabilities.h"

namespace uwf::windows {
app::OverlayMaintenanceReadOnlyPorts windowsOverlayMaintenanceObservations(std::shared_ptr<const app::VolumeRepository> volumes) {
  if (!volumes) throw std::invalid_argument("Overlay maintenance observations require a volume repository");
  app::OverlayMaintenanceReadOnlyPorts result;
  result.hibernationCapabilities = readWindowsHibernationCapabilities;
  result.hormVolumeProtection = [volumes = std::move(volumes)](const std::stop_token stop) {
    const auto observation = readHormVolumeProtectionDetails({stop});
    if (const auto failure = std::get_if<compat::Failure>(&observation)) throw compat::Error(*failure);
    const auto& observed = std::get<HormVolumeProtectionDetails>(observation);
    app::OverlayHormVolumeProtection value{observed.allProtected, {}, {}};
    for (const auto& id : observed.unprotectedVolumes) {
      if (stop.stop_requested()) throw std::runtime_error("HORM volume observation canceled");
      QStringList names;
      try {
        for (const auto& path : volumes->mountedPaths(compat::volumePath(id))) names.append(QString::fromStdString(path));
      } catch (const std::exception& error) {
        if (!value.detailError.isEmpty()) value.detailError += QLatin1Char('\n');
        value.detailError += QString::fromStdString(id) + QStringLiteral(": ") + QString::fromUtf8(error.what());
      }
      value.blockedVolumes.push_back({QString::fromStdString(compat::normalizeVolumeId(id)), names.join(QStringLiteral(", "))});
    }
    return value;
  };
  return result;
}
}  // namespace uwf::windows
