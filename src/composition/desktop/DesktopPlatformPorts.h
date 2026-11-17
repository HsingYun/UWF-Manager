// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/diagnostics/contracts/LogSource.h"
#include "application/diagnostics/contracts/SystemInformationSource.h"
#include "application/files/contracts/FileBrowser.h"
#include "application/files/contracts/TextFileCreator.h"
#include "application/files/contracts/TextFileReader.h"
#include "application/files/contracts/TextFileWriter.h"
#include "application/overlay/contracts/OverlayMaintenance.h"
#include "application/registry/contracts/RegistryBrowser.h"
#include "application/volumes/contracts/VolumeRepository.h"
#include "presentation/dialogs/Dialogs.h"

namespace uwf::composition {
// The host resolves these injected services once; factories retain them without service lookup.
struct DesktopPlatformPorts {
  std::shared_ptr<const app::VolumeRepository> volumes;
  std::shared_ptr<const app::FileBrowser> files;
  std::shared_ptr<const app::CommitTraversalSource> traversal;
  std::shared_ptr<const app::RegistryBrowser> registry;
  std::shared_ptr<const app::SystemInformationSource> systemInformation;
  std::shared_ptr<app::LogSource> log;
  std::shared_ptr<ui::dialogs::FileDialogProvider> fileDialogs;
  app::TextFileReader readText;
  app::TextFileWriter writeText;
  app::TextFileCreator createText;
  QString applicationDirectory;
  app::OverlayMaintenanceReadOnlyPorts maintenanceObservations{};
};
void validateDesktopPlatformPorts(const DesktopPlatformPorts& ports);
}  // namespace uwf::composition
