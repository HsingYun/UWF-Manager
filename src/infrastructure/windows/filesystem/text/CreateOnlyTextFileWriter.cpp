// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/filesystem/text/CreateOnlyTextFileWriter.h"

#include <filesystem>
#include <span>
#include <stdexcept>
#include <system_error>

#include "infrastructure/windows/errors/WindowsErrorText.h"
#include "infrastructure/windows/filesystem/WindowsFileSystem.h"

namespace uwf::windows {
app::TextFileCreationResult createUtf8TextFile(const QString& path, const QString& text) {
  using Outcome = app::TextFileCreationResult::Outcome;
  const std::filesystem::path target(path.toStdWString());
  if (!target.is_absolute() || path.contains(QChar::Null)) return {Outcome::Failed, QStringLiteral("Target path must be absolute and valid.")};
  const auto encoded = text.toUtf8();
  try {
    const auto result =
        winfs::writeFile(target, std::as_bytes(std::span(encoded.constData(), static_cast<std::size_t>(encoded.size()))), winfs::FilePublication::CreateNew);
    if (result.failure == winfs::FileIoFailure::AlreadyExists) return {Outcome::AlreadyExists, {}};
    if (result.failure != winfs::FileIoFailure::None) return {Outcome::Failed, windowsErrorText(static_cast<std::uint32_t>(result.error.value()))};
  } catch (const std::system_error& error) {
    return {Outcome::Failed, QString::fromUtf8(error.what())};
  } catch (const std::invalid_argument& error) {
    return {Outcome::Failed, QString::fromUtf8(error.what())};
  }
  return {};
}
}  // namespace uwf::windows
