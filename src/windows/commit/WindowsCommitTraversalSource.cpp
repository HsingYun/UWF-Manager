// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowsCommitTraversalSource.h"

#include <stdexcept>

#include "WindowsFileTraversalSource.h"
#include "app/common/ApplicationText.h"
#include "app/ports/RegistryKey.h"

namespace uwf::windows {
namespace {
class SystemCommitTraversalSource final : public app::CommitTraversalSource {
 public:
  std::string volumeForPath(const QString& path) const override { return m_files.volumeForPath(path); }
  std::vector<std::string> mountedLetters(const std::string& volume) const override { return m_files.mountedLetters(volume); }
  bool exists(const app::CommitNode& target) const override {
    if (target.kind == app::CommitTargetKind::RegistryKey) return regkey::keyExists(target.path.toStdString());
    if (target.kind == app::CommitTargetKind::RegistryValue) return regkey::valueExists(target.path.toStdString(), target.valueName.toStdString());
    return m_files.exists(target);
  }
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    if (target.kind != app::CommitTargetKind::RegistryKey && target.kind != app::CommitTargetKind::RegistryValue) return m_files.inspect(target);
    const bool exists = target.kind == app::CommitTargetKind::RegistryKey ? regkey::keyExists(target.path.toStdString())
                                                                          : regkey::valueExists(target.path.toStdString(), target.valueName.toStdString());
    if (!exists) return std::nullopt;
    return target;
  }
  QString read(const app::CommitNode& target, const std::stop_token stop, const app::CommitNodeVisitor& discover) const override {
    if (target.kind == app::CommitTargetKind::File || target.kind == app::CommitTargetKind::Folder) return m_files.read(target, stop, discover);
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
