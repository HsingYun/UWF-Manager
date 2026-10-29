// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowsFileTraversalSource.h"

#include <QDir>
#include <filesystem>
#include <stdexcept>

#include "app/common/ApplicationText.h"
#include "app/ports/VolumePaths.h"
#include "app/ports/WindowsFileSystem.h"
namespace uwf::windows {
using app::ApplicationText;
using app::CommitNode;
using app::CommitNodeVisitor;
using app::CommitTargetKind;
std::string WindowsFileTraversalSource::volumeForPath(const QString& path) const { return drive::fromPath(path.toStdString()); }
std::vector<std::string> WindowsFileTraversalSource::mountedLetters(const std::string& volume) const { return drive::mountedLetters(volume); }
bool WindowsFileTraversalSource::exists(const CommitNode& target) const {
  return winfs::existsForVerification(std::filesystem::path(target.path.toStdWString()));
}
std::optional<CommitNode> WindowsFileTraversalSource::inspect(const CommitNode& target) const {
  auto node = target;
  const auto type = winfs::inspect(std::filesystem::path(target.path.toStdWString()));
  if (type == winfs::EntryType::Missing) return std::nullopt;
  if (type == winfs::EntryType::Directory) node.kind = CommitTargetKind::Folder;
  if (type == winfs::EntryType::File) node.kind = CommitTargetKind::File;
  if (type == winfs::EntryType::ReparsePoint)
    node.skippedReason = ApplicationText::tr("Reparse points are skipped to keep traversal within the selected scope.");
  return node;
}
QString WindowsFileTraversalSource::read(const CommitNode& target, const std::stop_token stop, const CommitNodeVisitor& discover) const {
  if (target.kind != CommitTargetKind::Folder) return {};
  std::error_code error;
  auto iterator = std::filesystem::directory_iterator(winfs::extendedPath(std::filesystem::path(target.path.toStdWString())), error);
  const std::filesystem::directory_iterator end;
  while (!error && iterator != end && !stop.stop_requested()) {
    auto path = QString::fromStdWString(iterator->path().wstring());
    if (path.startsWith(QStringLiteral("\\\\?\\UNC\\")))
      path = QStringLiteral("\\\\") + path.mid(8);
    else if (path.startsWith(QStringLiteral("\\\\?\\")))
      path = path.mid(4);
    CommitNode child{CommitTargetKind::File, QDir::toNativeSeparators(path), {}, {}};
    std::error_code metadataError;
    if (iterator->symlink_status(metadataError).type() == std::filesystem::file_type::directory) child.kind = CommitTargetKind::Folder;
    discover(child);
    iterator.increment(error);
  }
  return error ? QString::fromLocal8Bit(error.message()) : QString();
}
}  // namespace uwf::windows
