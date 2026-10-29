// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "ui/dialogs/Dialogs.h"

namespace uwf::test {
class FileDialogStub final : public ui::dialogs::FileDialogProvider {
 public:
  QStringList openedFiles;
  QString openedFile;
  QString selectedDirectory;
  QString savedFile;
  QList<ui::dialogs::FileDialogRequest> requests;

  QStringList openFiles(QWidget*, const ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return openedFiles;
  }
  QString openFile(QWidget*, const ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return openedFile;
  }
  QString selectDirectory(QWidget*, const ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return selectedDirectory;
  }
  QString saveFile(QWidget*, const ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return savedFile;
  }
};
}  // namespace uwf::test
