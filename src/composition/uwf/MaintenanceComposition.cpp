// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/uwf/MaintenanceComposition.h"

#include <stdexcept>

#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "composition/uwf/SystemUwf.h"
#include "infrastructure/windows/registry/RegistryKey.h"

namespace uwf::composition {
std::uint32_t systemCurrentControlSet() {
  const auto current = regkey::readDword("HKEY_LOCAL_MACHINE\\SYSTEM\\Select", "Current");
  if (!current) throw std::runtime_error("The current registry control set is unavailable");
  return *current;
}
std::shared_ptr<app::UwfMaintenanceRepository> systemMaintenanceRepository() { return app::uwfMaintenanceRepository(systemUwf()); }
}  // namespace uwf::composition
