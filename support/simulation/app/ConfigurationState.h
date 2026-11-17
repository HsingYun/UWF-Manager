// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "domain/uwf/model/UwfModel.h"

namespace uwf::simulation::application {
// Default volume identities for simulated application state.
inline constexpr char fixtureVolumeCName[] = "Volume{11111111-2222-3333-4444-555555555555}";
inline constexpr char fixtureVolumeDName[] = "Volume{66666666-7777-8888-9999-aaaaaaaaaaaa}";

core::UwfSnapshot editableSnapshot();
}  // namespace uwf::simulation::application
