// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QVector>

namespace uwf::app {

// Keep raw stream names for diagnostics; actions use stream-free absolute paths.
// Merged sizes saturate, and directory/metadata flags are combined.
struct OverlayFileEntry {
  QString rawName;
  QString absolutePath;
  bool isDirectory = false;
  bool isSystemMetadata = false;
  qulonglong fileSize = 0;
};

enum class OverlayFileKind { File, Directory, SystemMetadata };

[[nodiscard]] inline OverlayFileKind overlayFileKind(const OverlayFileEntry& entry) {
  return entry.isSystemMetadata ? OverlayFileKind::SystemMetadata : entry.isDirectory ? OverlayFileKind::Directory : OverlayFileKind::File;
}

// Stable diagnostic codes share the same classification as display and actions.
[[nodiscard]] inline const char* overlayFileKindCode(const OverlayFileKind kind) {
  switch (kind) {
    case OverlayFileKind::File:
      return "file";
    case OverlayFileKind::Directory:
      return "dir";
    case OverlayFileKind::SystemMetadata:
      return "metadata";
  }
  return "unknown";
}

}  // namespace uwf::app
