// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"

#include "application/commit/contracts/CommitTraversalSource.h"
#include "infrastructure/windows/commit/WindowsFileTraversalSource.h"
#include "infrastructure/windows/registry/RegistryKey.h"

namespace uwf::windows {
namespace {
class SystemCommitTraversalSource final : public app::CommitTraversalSource {
 public:
  std::string driveLetterForPath(const QString& path) const override { return m_files.driveLetterForPath(path); }
  std::string driveLetterForPath(const QString& path, const app::FileCommitTarget& target) const override {
    return m_files.driveLetterForPath(path, target);
  }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override { return m_files.resolveFileTarget(path); }
  bool exists(const app::CommitNode& target) const override {
    app::validateCommitNodeText(target);
    if (target.kind == app::CommitTargetKind::RegistryKey) return regkey::keyExists(target.path.toStdString());
    if (target.kind == app::CommitTargetKind::RegistryValue) return regkey::valueExists(target.path.toStdString(), target.valueName.toStdString());
    return m_files.exists(target);
  }
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    app::validateCommitNodeText(target);
    if (target.kind != app::CommitTargetKind::RegistryKey && target.kind != app::CommitTargetKind::RegistryValue) return m_files.inspect(target);
    const bool exists = target.kind == app::CommitTargetKind::RegistryKey ? regkey::keyExists(target.path.toStdString())
                                                                          : regkey::valueExists(target.path.toStdString(), target.valueName.toStdString());
    if (!exists) return std::nullopt;
    return target;
  }
  QString enumerateChildren(const app::CommitNode& target, const std::stop_token stop, const app::CommitNodeVisitor& discover) const override {
    app::validateCommitNodeText(target);
    if (target.kind == app::CommitTargetKind::File || target.kind == app::CommitTargetKind::Folder) return m_files.enumerateChildren(target, stop, discover);
    QString result;
    if (target.kind != app::CommitTargetKind::RegistryKey) return result;
    const auto key = target.path.toStdString();
    try {
      regkey::visitValueNames(
          key, [&](const std::string& name) { discover({app::CommitTargetKind::RegistryValue, target.path, QString::fromStdString(name), {}}); }, stop);
    } catch (const std::exception& error) {
      result = QString::fromUtf8(error.what());
    }
    try {
      regkey::visitSubkeyNames(
          key, [&](const std::string& name) { discover({app::CommitTargetKind::RegistryKey, target.path + u'\\' + QString::fromStdString(name), {}, {}}); },
          stop);
    } catch (const std::exception& error) {
      if (!result.isEmpty()) result += u'\n';
      result += QString::fromUtf8(error.what());
    }
    return result;
  }

 private:
  WindowsFileTraversalSource m_files;
};
}  // namespace
std::shared_ptr<const app::CommitTraversalSource> systemCommitTraversalSource() {
  static const auto source = std::make_shared<SystemCommitTraversalSource>();
  return source;
}
}  // namespace uwf::windows
