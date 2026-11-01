// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/uwf/SystemCapability.h"

#include "application/uwf/adapters/UwfReadState.h"
#include "composition/uwf/SystemUwf.h"

namespace uwf::composition {

app::UwfCapability probeUwfCapability() { return app::probeUwfCapability(*systemUwf()); }

}  // namespace uwf::composition
