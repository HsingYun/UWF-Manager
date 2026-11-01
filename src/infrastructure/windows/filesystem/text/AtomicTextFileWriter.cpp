// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/filesystem/text/AtomicTextFileWriter.h"

#include <filesystem>
#include <span>
#include <stdexcept>
#include <system_error>

#include "infrastructure/windows/errors/WindowsErrorText.h"
#include "infrastructure/windows/filesystem/WindowsFileSystem.h"

namespace uwf::windows {
app::TextFileWriteResult writeUtf8TextFile(const QString& path, const QString& text) {
  using Failure = app::TextFileWriteResult::Failure;
  const auto encoded = text.toUtf8();
  try {
    const auto result =
        winfs::writeFile(std::filesystem::path(path.toStdWString()), std::as_bytes(std::span(encoded.constData(), static_cast<std::size_t>(encoded.size()))),
                         winfs::FilePublication::Replace);
    if (result.failure != winfs::FileIoFailure::None)
      return {result.failure == winfs::FileIoFailure::Open ? Failure::Open : Failure::Write,
              windowsErrorText(static_cast<std::uint32_t>(result.error.value()))};
  } catch (const std::system_error& error) {
    return {Failure::Open, QString::fromUtf8(error.what())};
  } catch (const std::invalid_argument& error) {
    return {Failure::Open, QString::fromUtf8(error.what())};
  }
  return {};
}
}  // namespace uwf::windows
