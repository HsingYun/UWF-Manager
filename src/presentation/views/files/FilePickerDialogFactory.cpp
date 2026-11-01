// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/files/FilePickerDialogFactory.h"

#include <utility>

#include "presentation/viewmodels/files/FilePickerViewModel.h"
#include "presentation/views/files/FilePickerDialog.h"

namespace uwf::ui {
namespace {
using dialogs::FileDialogProvider;
using dialogs::FileDialogRequest;
class FilePickerDialogProvider final : public FileDialogProvider {
  std::shared_ptr<const app::FileBrowser> m_browser;
  QStringList pick(QWidget* parent, const FileDialogRequest& request, const presentation::FilePickerViewModel::Mode mode) {
    auto model = std::make_shared<presentation::FilePickerViewModel>(mode, m_browser);
    model->setFilters(request.filter);
    model->setAllowedRoots(request.allowedRoots);
    FilePickerDialog dialog(model, request.title, parent);
    model->start(request.initialPath);
    return dialog.exec() == QDialog::Accepted ? dialog.selectedPaths() : QStringList{};
  }

 public:
  explicit FilePickerDialogProvider(std::shared_ptr<const app::FileBrowser> browser) : m_browser(std::move(browser)) {}
  QStringList openFiles(QWidget* parent, const FileDialogRequest& request) override {
    return pick(parent, request, presentation::FilePickerViewModel::Mode::OpenFiles);
  }
  QString openFile(QWidget* parent, const FileDialogRequest& request) override {
    return pick(parent, request, presentation::FilePickerViewModel::Mode::OpenFile).value(0);
  }
  QString selectDirectory(QWidget* parent, const FileDialogRequest& request) override {
    return pick(parent, request, presentation::FilePickerViewModel::Mode::Directory).value(0);
  }
  QString saveFile(QWidget* parent, const FileDialogRequest& request) override {
    return pick(parent, request, presentation::FilePickerViewModel::Mode::SaveFile).value(0);
  }
  QVector<app::FileBrowserEntry> openItems(QWidget* parent, const FileDialogRequest& request) override {
    auto model = std::make_shared<presentation::FilePickerViewModel>(presentation::FilePickerViewModel::Mode::Mixed, m_browser);
    model->setFilters(request.filter);
    model->setAllowedRoots(request.allowedRoots);
    FilePickerDialog dialog(model, request.title, parent);
    model->start(request.initialPath);
    return dialog.exec() == QDialog::Accepted ? dialog.selectedItems() : QVector<app::FileBrowserEntry>{};
  }
};
}  // namespace
std::unique_ptr<dialogs::FileDialogProvider> createFilePickerDialogs(std::shared_ptr<const app::FileBrowser> browser) {
  return std::make_unique<FilePickerDialogProvider>(std::move(browser));
}
}  // namespace uwf::ui
