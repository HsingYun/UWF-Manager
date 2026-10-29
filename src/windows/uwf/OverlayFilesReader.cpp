// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayFilesReader.h"

#include <limits>
#include <stdexcept>

#include "app/uwf/UwfOverlay.h"
#include "windows/wmi/WmiRuntime.h"

namespace uwf::windows {

QVector<app::OverlayFileEntry> loadOverlayFilesFromSystem(const QString& driveLetter, const std::stop_token stopToken) {
  if (stopToken.stop_requested()) return {};
  // Background tasks reuse pool threads. Their WMI context belongs to the
  // thread and is released by TLS cleanup, not when this read finishes.
  auto& session = embeddedWmiSession();
  session.ensureConnected();
  app::provider::UwfOverlay overlay(session);
  const auto row = overlay.read();
  const auto files = overlay.getOverlayFiles(row, driveLetter.toStdString(), stopToken);
  if (stopToken.stop_requested()) return {};

  // 后续分页和 Qt 模型索引使用 int。不能把超出表示范围的 provider
  // 结果静默截断成负数；在构造 Qt 容器之前明确拒绝这一不可表示的数据集。
  if (files.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    throw std::length_error("overlay file count exceeds the UI model capacity");
  }

  QVector<app::OverlayFileEntry> entries;
  entries.reserve(static_cast<int>(files.size()));
  for (const auto& file : files) {
    entries.append({QString::fromStdString(file.fileName), {}, false, false, static_cast<qulonglong>(file.fileSize)});
  }
  return entries;
}

}  // namespace uwf::windows
