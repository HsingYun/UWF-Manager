// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QStringList>
#include <map>
#include <set>
#include <stdexcept>

#include "application/files/contracts/FileBrowser.h"
#include "application/identity/contracts/ObjectIdentity.h"

namespace uwf::test::application {
// A read-only metadata fixture. Entries identify literal object names; an
// unexpected browse or mutation fails instead of touching the host filesystem.
class ReadOnlyFileBrowser final : public app::FileBrowser {
 public:
  std::map<QString, app::FileBrowserEntry, app::ObjectNameLess> entries;
  std::set<QString, app::ObjectNameLess> readFailures;
  mutable QStringList inspections;

  app::FileBrowserEntry inspect(const QString& path) const override {
    inspections.append(path);
    if (readFailures.contains(path)) throw std::runtime_error("File metadata is unavailable");
    const auto found = entries.find(path);
    return found == entries.end() ? app::FileBrowserEntry{.path = path, .name = {}, .directory = false, .size = 0, .modified = {}, .exists = false}
                                  : found->second;
  }
  QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override { throw std::logic_error("Unexpected directory enumeration"); }
  QVector<app::FileBrowserPlace> places() const override { throw std::logic_error("Unexpected file browser places read"); }
  void createDirectory(const QString&) const override { throw std::logic_error("A read-only file browser cannot create directories"); }
  void renameEntry(const QString&, const QString&) const override { throw std::logic_error("A read-only file browser cannot rename entries"); }
};
}  // namespace uwf::test::application
