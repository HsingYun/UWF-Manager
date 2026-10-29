// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <memory>

#include "app/ports/WmiOperations.h"
#include "presentation/common/ViewModelFactory.h"
#include "windows/registry/WindowsRegistryBrowser.h"
#include "windows/system/WindowsSystemInformationSource.h"
#include "windows/uwf/OverlayFilesReader.h"
namespace uwf::bootstrap {
std::shared_ptr<presentation::ViewModelFactory> createViewModelFactory(
    app::wmi::WmiOperations& session, const app::RegistryBrowser& registry = windows::systemRegistryBrowser(),
    app::OverlayFileLoader overlayFiles = windows::loadOverlayFilesFromSystem,
    const app::SystemInformationSource& systemInformation = windows::systemInformationSource());
std::shared_ptr<presentation::ViewModelFactory> createViewModelFactory();
}  // namespace uwf::bootstrap
