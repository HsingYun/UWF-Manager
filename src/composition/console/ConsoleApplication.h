// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QStringList>

namespace uwf::composition {
int runConsoleApplication(int argc, char* argv[], const QStringList& arguments);
int reportConsoleStartupFailure(const QString& message) noexcept;
}  // namespace uwf::composition
