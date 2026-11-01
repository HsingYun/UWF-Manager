// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/shell/WindowsShell.h"

// clang-format off
#include <windows.h>
#include <shlobj.h>
// clang-format on

#include <QDir>
#include <QScopeGuard>
#include <array>
#include <filesystem>
#include <optional>
#include <string_view>
#include <system_error>

#include "foundation/text/ObjectNameComparison.h"
#include "infrastructure/windows/filesystem/WindowsFileSystem.h"

namespace uwf::windows::shell {
namespace {
// Includes the terminating NUL within Windows' extended-length path capacity.
constexpr std::size_t kShellPathCapacity = 32768;

std::optional<winfs::EntryMetadata> tryMetadata(const std::filesystem::path& path) {
  try {
    return winfs::metadata(path);
  } catch (const std::system_error&) {
    // An unreadable target may still have an accessible parent.
    return std::nullopt;
  }
}

bool openVerifiedTarget(const std::filesystem::path& path) {
  const auto pidl = ILCreateFromPathW(path.c_str());
  if (!pidl) return false;
  const auto release = qScopeGuard([pidl] { ILFree(pidl); });
  std::array<wchar_t, kShellPathCapacity> resolved{};
  if (!SHGetPathFromIDListEx(pidl, resolved.data(), static_cast<DWORD>(resolved.size()), GPFIDL_DEFAULT)) return false;
  // Shell parsing can collapse trailing spaces/dots; verify the resolved
  // identity before opening or selecting it.
  const auto comparisonPath = [](const std::filesystem::path& item) {
    auto value = winfs::extendedPath(item).native();
    // Ignore trailing separators while preserving literal component spelling.
    while (value.size() > 1 && value.back() == L'\\') value.pop_back();
    return value;
  };
  const auto expected = comparisonPath(path);
  const auto actual = comparisonPath(std::filesystem::path(resolved.data()));
  const auto name = [](const std::wstring_view value) { return std::u16string_view(reinterpret_cast<const char16_t*>(value.data()), value.size()); };
  if (compareObjectNames(name(expected), name(actual)) != 0) return false;
  return SUCCEEDED(SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0));
}
}  // namespace

void revealInExplorer(const QString& path) {
  const std::filesystem::path target(QDir::toNativeSeparators(path).toStdWString());
  if (!target.is_absolute() || path.contains(QChar::Null)) return;
  if (tryMetadata(target) && openVerifiedTarget(target)) return;

  // Fall back to the nearest existing parent with a verified Shell identity.
  auto folder = target.parent_path();
  if (folder == target) return;
  while (!folder.empty()) {
    const auto metadata = tryMetadata(folder);
    if (metadata && metadata->directory && openVerifiedTarget(folder)) return;
    const auto parent = folder.parent_path();
    if (parent == folder) break;
    folder = parent;
  }
}
}  // namespace uwf::windows::shell
