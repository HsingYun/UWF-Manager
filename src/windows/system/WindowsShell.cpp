// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowsShell.h"

// clang-format off
#include <windows.h>
#include <shlobj.h>
// clang-format on

#include <QDir>
#include <QFileInfo>
#include <string>

#include "app/ports/VolumePaths.h"

namespace uwf::windows::shell {

namespace {

std::wstring explorerPath() {
  std::wstring windowsDir(MAX_PATH, L'\0');
  UINT length = GetWindowsDirectoryW(windowsDir.data(), static_cast<UINT>(windowsDir.size()));
  if (length == 0) return {};
  if (length >= windowsDir.size()) {
    windowsDir.resize(static_cast<size_t>(length) + 1);
    length = GetWindowsDirectoryW(windowsDir.data(), static_cast<UINT>(windowsDir.size()));
    if (length == 0 || length >= windowsDir.size()) return {};
  }
  windowsDir.resize(length);
  if (!windowsDir.ends_with(L'\\')) windowsDir.push_back(L'\\');
  windowsDir += L"explorer.exe";
  return windowsDir;
}

}  // namespace

void revealInExplorer(const QString& path) {
  const QString abs = QDir::toNativeSeparators(path);

  // 使用 Shell API 定位目标，避免 explorer.exe 命令行的路径引用差异。
  if (QFileInfo::exists(abs)) {
    const std::wstring wide = abs.toStdWString();
    if (PIDLIST_ABSOLUTE pidl = ILCreateFromPathW(wide.c_str())) {
      // cidl=0 + apidl=NULL：选中 pidl 自身（文件选文件、目录选目录，落在各自父目录里）。
      (void)SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
      ILFree(pidl);
      return;
    }
  }

  // 路径不存在或拿不到 PIDL：沿父目录回溯到第一个真实存在的文件夹直接打开（不选中）。
  QString folder = QFileInfo(abs).absolutePath();
  if (folder.isEmpty()) folder = abs;
  while (!folder.isEmpty() && !QFileInfo::exists(folder)) {
    const QString up = QFileInfo(folder).absolutePath();
    if (up == folder) break;
    folder = up;
  }
  if (!folder.isEmpty() && QFileInfo::exists(folder)) {
    const std::wstring wf = QDir::toNativeSeparators(folder).toStdWString();
    // 高权限进程不能按裸文件名搜索 explorer.exe：当前目录 / PATH 若可写会形成
    // binary planting。GetWindowsDirectoryW 来自内核，不信任可伪造的 SystemRoot
    // 环境变量；完整路径也避免落到默认文件管理器替身。
    const std::wstring explorer = explorerPath();
    if (!explorer.empty()) ShellExecuteW(nullptr, L"open", explorer.c_str(), wf.c_str(), nullptr, SW_SHOWDEFAULT);
  }
}

QString extractDriveLetter(const QString& path) { return QString::fromStdString(drive::fromPath(path.toStdString())); }

}  // namespace uwf::windows::shell
