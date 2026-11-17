// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QString>

// Windows address primitives supplied at link composition. A non-Windows host
// simulates the target syntax rather than interpreting it as a host filesystem.
namespace uwf::app::paths::platform {
QString nativeSeparators(const QString& path);
QString portableSeparators(const QString& path);
QString clean(const QString& path);
bool fullyQualified(const QString& path);
bool absolute(const QString& path);
QString fileName(const QString& path);
QString parent(const QString& path);
QString join(const QString& directory, const QString& path);
QString resolve(const QString& directory, const QString& path);
}  // namespace uwf::app::paths::platform
