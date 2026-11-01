// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/process/UiElevation.h"

// Shell API declarations require the Win32 types first.
// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on

#include <cstdlib>
#include <system_error>

#include "foundation/logging/Log.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/security/ExecutableIdentity.h"

namespace uwf::windows {
bool currentProcessElevatedForUi() {
  HANDLE rawToken = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &rawToken))
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Read UI process elevation token");
  const UniqueHandle token(rawToken);
  TOKEN_ELEVATION elevation{};
  DWORD bytes = 0;
  if (!GetTokenInformation(token.get(), TokenElevation, &elevation, sizeof(elevation), &bytes))
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Read UI process elevation state");
  return elevation.TokenIsElevated != 0;
}

int relaunchUiElevated(const bool quiet) {
  const auto image = currentExecutablePath();
  SHELLEXECUTEINFOW launch{};
  launch.cbSize = sizeof(launch);
  launch.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
  launch.lpVerb = L"runas";
  launch.lpFile = image.c_str();
  launch.lpParameters = quiet ? L"--quiet" : nullptr;
  launch.nShow = SW_SHOWNORMAL;
  if (!ShellExecuteExW(&launch)) {
    const auto error = GetLastError();
    if (error == ERROR_CANCELLED) {
      UWF_LOG_I("main") << "UI elevation canceled";
      return EXIT_FAILURE;
    }
    throw std::system_error(static_cast<int>(error), std::system_category(), "Launch elevated UI process");
  }
  const UniqueHandle process(launch.hProcess);
  UWF_LOG_I("main") << "elevated UI process launched: mode=" << (quiet ? "quiet" : "interactive");
  return EXIT_SUCCESS;
}
}  // namespace uwf::windows
