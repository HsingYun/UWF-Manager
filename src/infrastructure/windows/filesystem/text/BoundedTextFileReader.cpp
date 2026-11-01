// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/filesystem/text/BoundedTextFileReader.h"

#include <algorithm>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "infrastructure/windows/errors/WindowsErrorText.h"
#include "infrastructure/windows/filesystem/WindowsFileSystem.h"

namespace uwf::windows {
app::TextFileReadResult readBoundedTextFile(const QString& path, const qint64 maximumBytes) {
  if (maximumBytes < 0 || maximumBytes == std::numeric_limits<qint64>::max()) throw std::invalid_argument("invalid text read limit");
  using Failure = app::TextFileReadResult::Failure;
  QByteArray bytes;
  const auto limit = maximumBytes + 1;
  try {
    const auto result = winfs::readChunks(std::filesystem::path(path.toStdWString()), [&](const std::span<const std::byte> chunk) {
      const auto count = std::min(static_cast<qsizetype>(chunk.size()), limit - bytes.size());
      bytes.append(reinterpret_cast<const char*>(chunk.data()), count);
      return bytes.size() < limit;
    });
    if (result.failure != winfs::FileIoFailure::None) {
      const auto failure = result.failure == winfs::FileIoFailure::Open ? Failure::Open : Failure::Read;
      return {failure, windowsErrorText(static_cast<std::uint32_t>(result.error.value())), {}};
    }
  } catch (const std::system_error& error) {
    return {Failure::Open, QString::fromUtf8(error.what()), {}};
  } catch (const std::invalid_argument& error) {
    return {Failure::Open, QString::fromUtf8(error.what()), {}};
  }
  if (bytes.size() > maximumBytes) return {Failure::TooLarge, {}, {}};
  return {Failure::None, {}, std::move(bytes)};
}
}  // namespace uwf::windows
