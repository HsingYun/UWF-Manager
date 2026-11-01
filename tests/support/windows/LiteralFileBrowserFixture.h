// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>

#include <QByteArray>
#include <QDir>
#include <QTemporaryDir>
#include <array>
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "infrastructure/windows/filesystem/browsing/LocalFileBrowser.h"
#include "infrastructure/windows/resources/UniqueHandle.h"

namespace uwf::test {
// Only observes the real browser: no simulated mutation or path resolver can
// make an unintended rename appear to preserve an independently seeded object.
class FileBrowserWriteObserver final : public app::FileBrowser {
 public:
  mutable std::atomic_int creates{0};
  mutable std::atomic_int renames{0};
  QVector<app::FileBrowserEntry> list(const QString& path, const std::stop_token stop) const override { return m_browser->list(path, stop); }
  app::FileBrowserEntry inspect(const QString& path) const override { return m_browser->inspect(path); }
  QVector<app::FileBrowserPlace> places() const override { return m_browser->places(); }
  void createDirectory(const QString& path) const override {
    ++creates;
    m_browser->createDirectory(path);
  }
  void renameEntry(const QString& path, const QString& destination) const override {
    ++renames;
    m_browser->renameEntry(path, destination);
  }

 private:
  std::shared_ptr<const app::FileBrowser> m_browser = windows::systemFileBrowser();
};

class LiteralFileBrowserFixture final {
 public:
  QTemporaryDir root;
  QString error;
  const std::shared_ptr<FileBrowserWriteObserver> browser = std::make_shared<FileBrowserWriteObserver>();
  ~LiteralFileBrowserFixture() {
    for (const auto& file : m_files) DeleteFileW(file.c_str());
  }
  QString path(const QString& name) const { return root.filePath(name); }
  bool exists(const QString& name) const { return GetFileAttributesW(exact(path(name)).c_str()) != INVALID_FILE_ATTRIBUTES; }
  bool seed(const QString& name, const QByteArray& contents) {
    const auto native = exact(path(name));
    const windows::UniqueHandle file(CreateFileW(native.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, CREATE_NEW,
                                                 FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!file.valid()) {
      error = QStringLiteral("CreateFileW [%1] failed: %2").arg(name).arg(GetLastError());
      return false;
    }
    m_files.push_back(native);
    DWORD written = 0;
    if (!WriteFile(file.get(), contents.constData(), static_cast<DWORD>(contents.size()), &written, nullptr) ||
        written != static_cast<DWORD>(contents.size())) {
      error = QStringLiteral("WriteFile [%1] failed: %2").arg(name).arg(GetLastError());
      return false;
    }
    return true;
  }
  std::optional<QByteArray> read(const QString& name) const {
    const windows::UniqueHandle file(CreateFileW(exact(path(name)).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!file.valid()) return std::nullopt;
    QByteArray contents;
    std::array<char, 256> buffer{};
    for (;;) {
      DWORD count = 0;
      if (!ReadFile(file.get(), buffer.data(), static_cast<DWORD>(buffer.size()), &count, nullptr)) return std::nullopt;
      if (!count) return contents;
      contents.append(buffer.data(), static_cast<qsizetype>(count));
    }
  }

 private:
  // Keep the oracle independent of the production filesystem path adapter.
  static std::wstring exact(const QString& path) {
    const auto native = QDir::toNativeSeparators(path);
    return (native.startsWith(QStringLiteral("\\\\")) ? QStringLiteral("\\\\?\\UNC\\") + native.mid(2) : QStringLiteral("\\\\?\\") + native).toStdWString();
  }
  std::vector<std::wstring> m_files;
};
}  // namespace uwf::test
