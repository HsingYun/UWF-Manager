// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/commit/WindowsFileTraversalSource.h"

#include <Windows.h>

#include <array>
#include <filesystem>
#include <system_error>

#include "application/localization/ApplicationText.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/contracts/Types.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/filesystem/WindowsFileSystem.h"
#include "infrastructure/windows/storage/WindowsVolumeRepository.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {
using app::ApplicationText;
using app::CommitNode;
using app::CommitNodeVisitor;
using app::CommitTargetKind;
std::string WindowsFileTraversalSource::driveLetterForPath(const QString& path) const {
  return driveLetterForPath(path, resolveFileTarget(path));
}
std::string WindowsFileTraversalSource::driveLetterForPath(const QString& path, const app::FileCommitTarget& target) const {
  app::validateCommitNodeText({CommitTargetKind::File, path, {}, {}});
  app::validateFileCommitTarget(target);
  const auto letters = systemVolumeRepository()->mountedLetters(compat::volumePath(target.volumeId));
  return letters.empty() ? std::string{} : letters.front();
}
app::FileCommitTarget WindowsFileTraversalSource::resolveFileTarget(const QString& path) const {
  app::validateCommitNodeText({CommitTargetKind::File, path, {}, {}});
  const auto input = path.toStdWString();
  if (input.empty() || input.find(L'\0') != std::wstring::npos) throw std::invalid_argument("Invalid file path");
  const auto required = GetFullPathNameW(input.c_str(), 0, nullptr, nullptr);
  if (!required) throw WindowsSystemError(GetLastError(), "resolve absolute file path");
  std::wstring absolute(required, L'\0');
  const auto written = GetFullPathNameW(input.c_str(), required, absolute.data(), nullptr);
  if (!written) throw WindowsSystemError(GetLastError(), "resolve absolute file path");
  if (written >= required) throw std::runtime_error("The absolute file path changed while being resolved");
  absolute.resize(written);
  constexpr std::size_t maximumWindowsPath = 32768;
  std::array<wchar_t, maximumWindowsPath> mount{};
  if (!GetVolumePathNameW(absolute.c_str(), mount.data(), static_cast<DWORD>(mount.size())))
    throw WindowsSystemError(GetLastError(), "resolve file volume mount");
  std::array<wchar_t, MAX_PATH + 1> volume{};
  if (!GetVolumeNameForVolumeMountPointW(mount.data(), volume.data(), static_cast<DWORD>(volume.size())))
    throw WindowsSystemError(GetLastError(), "resolve file volume identity");
  const auto id = compat::parseVolumeId(wideToUtf8(volume.data()));
  if (!id) throw std::runtime_error("The mounted volume returned an invalid GUID");
  const std::wstring_view root(mount.data());
  if (absolute.size() + 1 == root.size() && root.back() == L'\\' &&
      CompareStringOrdinal(absolute.data(), static_cast<int>(absolute.size()), root.data(), static_cast<int>(absolute.size()), TRUE) == CSTR_EQUAL)
    return {*id, QStringLiteral("\\")};
  if (absolute.size() < root.size() ||
      CompareStringOrdinal(absolute.data(), static_cast<int>(root.size()), root.data(), static_cast<int>(root.size()), TRUE) != CSTR_EQUAL)
    throw std::runtime_error("The file path does not belong to its resolved mount");
  return {*id, u'\\' + QString::fromStdWString(absolute.substr(root.size()))};
}
namespace {
std::filesystem::path selectedLeaf(const QString& displayPath) {
  auto path = std::filesystem::path(displayPath.toStdWString());
  while (path.has_relative_path() && path.filename().empty()) path = path.parent_path();
  return path;
}
std::filesystem::path executionPath(const app::CommitNode& target) {
  return std::filesystem::path(app::paths::volumeFilePath({target.fileTarget->volumeId, target.fileTarget->relativePath}).toStdWString());
}
}  // namespace
bool WindowsFileTraversalSource::exists(const CommitNode& target) const {
  return winfs::existsForVerification(executionPath(app::bindFileCommitTarget(*this, target)));
}
std::optional<CommitNode> WindowsFileTraversalSource::inspect(const CommitNode& target) const {
  app::validateCommitNodeText(target);
  auto node = target;
  // A mount point's own attributes disappear when its path is converted to the
  // mounted volume root. Check the selected leaf before following that mapping.
  const auto selectedType = winfs::inspect(selectedLeaf(target.path));
  if (node.fileTarget)
    app::validateFileCommitTarget(*node.fileTarget);
  else
    node = app::bindFileCommitTarget(*this, std::move(node));
  if (selectedType == winfs::EntryType::ReparsePoint) {
    node.skippedReason = ApplicationText::tr("Reparse points are skipped to keep traversal within the selected scope.");
    return node;
  }
  const auto type = winfs::inspect(executionPath(node));
  if (type == winfs::EntryType::Missing) return std::nullopt;
  if (type == winfs::EntryType::Directory) node.kind = CommitTargetKind::Folder;
  if (type == winfs::EntryType::File) node.kind = CommitTargetKind::File;
  if (type == winfs::EntryType::ReparsePoint)
    node.skippedReason = ApplicationText::tr("Reparse points are skipped to keep traversal within the selected scope.");
  return node;
}
QString WindowsFileTraversalSource::enumerateChildren(const CommitNode& target, const std::stop_token stop, const CommitNodeVisitor& discover) const {
  if (target.kind != CommitTargetKind::Folder) return {};
  try {
    const auto observedTarget = inspect(target);
    if (!observedTarget) throw WindowsSystemError(ERROR_PATH_NOT_FOUND, "enumerate selected directory");
    if (!observedTarget->skippedReason.isEmpty()) return observedTarget->skippedReason;
    if (observedTarget->kind != CommitTargetKind::Folder) throw WindowsSystemError(ERROR_DIRECTORY, "enumerate selected directory");
    const auto& bound = *observedTarget;
    winfs::enumerate(executionPath(bound), stop, [&](const auto& path, const auto& observed) {
      const auto name = QString::fromStdWString(path.filename().native());
      auto relative = bound.fileTarget->relativePath;
      if (!relative.endsWith(u'\\')) relative += u'\\';
      relative += name;
      CommitNode child{observed.directory ? CommitTargetKind::Folder : CommitTargetKind::File,
                       QString::fromStdWString((std::filesystem::path(bound.path.toStdWString()) / path.filename()).wstring()),
                       {},
                       {},
                       app::FileCommitTarget{bound.fileTarget->volumeId, relative}};
      if (observed.reparsePoint) child.skippedReason = ApplicationText::tr("Reparse points are skipped to keep traversal within the selected scope.");
      discover(child);
    });
  } catch (const std::system_error& error) {
    return QString::fromUtf8(error.what());
  }
  return {};
}
}  // namespace uwf::windows
