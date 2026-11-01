// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/console/ConsoleSession.h"

#include <algorithm>
#include <array>
#include <mutex>
#include <stdexcept>

#include "foundation/logging/Log.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {
namespace {
constexpr std::array<DWORD, 3> standardStreams{STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
bool usable(const HANDLE handle) noexcept {
  if (!handle || handle == INVALID_HANDLE_VALUE) return false;
  SetLastError(ERROR_SUCCESS);
  return GetFileType(handle) != FILE_TYPE_UNKNOWN || GetLastError() == ERROR_SUCCESS;
}
bool redirected(const HANDLE handle) noexcept {
  if (!usable(handle)) return false;
  DWORD mode = 0;
  return !GetConsoleMode(handle, &mode);
}
struct Attachment {
  std::mutex mutex;
  bool attempted = false;
  bool attached = false;
  // AttachConsole can replace the standard-handle table. Duplicates preserve
  // each inherited redirection independently and live through the CLI host.
  std::array<UniqueHandle, standardStreams.size()> inherited;
};
Attachment& attachment() {
  static Attachment state;
  return state;
}
struct CancellationState {
  std::mutex mutex;
  std::shared_ptr<std::stop_source> source;
};
CancellationState& cancellationState() {
  // Initialize both synchronization and shared ownership before a handler can
  // be registered, rather than during process-wide static initialization.
  static CancellationState state;
  return state;
}
BOOL WINAPI controlHandler(const DWORD event) {
  if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
  const auto source = [] {
    auto& state = cancellationState();
    const std::lock_guard lock(state.mutex);
    return state.source;
  }();
  if (!source) return FALSE;
  source->request_stop();
  return TRUE;
}
}  // namespace

bool ConsoleSession::attach() noexcept {
  try {
    auto& state = attachment();
    const std::lock_guard lock(state.mutex);
    if (state.attempted) return state.attached;
    state.attempted = true;
    DWORD process = 0;
    if (GetConsoleProcessList(&process, 1) != 0) {
      state.attached = true;
      return true;
    }
    for (std::size_t index = 0; index < standardStreams.size(); ++index) {
      const auto handle = GetStdHandle(standardStreams[index]);
      if (!redirected(handle)) continue;
      HANDLE duplicate = nullptr;
      if (!DuplicateHandle(GetCurrentProcess(), handle, GetCurrentProcess(), &duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        // Abort attachment if inherited redirection cannot be preserved.
        return false;
      }
      state.inherited[index].reset(duplicate);
    }
    state.attached = AttachConsole(ATTACH_PARENT_PROCESS) != FALSE;
    for (std::size_t index = 0; index < standardStreams.size(); ++index) {
      if (!state.inherited[index].valid()) continue;
      if (state.attached) {
        if (!SetStdHandle(standardStreams[index], state.inherited[index].get()))
          UWF_LOG_W("console") << "standard handle restoration failed: stream=" << standardStreams[index] << " error=" << GetLastError();
      } else
        state.inherited[index].reset();
    }
    return state.attached;
  } catch (...) {
    return false;
  }
}
bool hasConsoleContext() noexcept {
  for (const auto stream : standardStreams) {
    const auto type = GetFileType(GetStdHandle(stream));
    if (type == FILE_TYPE_PIPE || type == FILE_TYPE_DISK) return true;
  }
  return ConsoleSession::attach();
}
bool hasRedirectedConsoleOutput() noexcept { return redirected(GetStdHandle(STD_OUTPUT_HANDLE)) || redirected(GetStdHandle(STD_ERROR_HANDLE)); }
ConsoleSession::ConsoleSession() {
  static_cast<void>(attach());
  auto& state = attachment();
  const std::lock_guard lock(state.mutex);
  m_output = state.inherited[1].valid() ? state.inherited[1].get() : GetStdHandle(STD_OUTPUT_HANDLE);
  m_error = state.inherited[2].valid() ? state.inherited[2].get() : GetStdHandle(STD_ERROR_HANDLE);
}
void writeConsoleText(const HANDLE handle, const std::string_view utf8) {
  if (!usable(handle)) throw WindowsSystemError(ERROR_INVALID_HANDLE, "write console stream");
  // Validate redirected bytes too. Console chunks stay within the legacy API limit.
  const auto text = utf8ToWide(utf8);
  DWORD mode = 0;
  if (GetConsoleMode(handle, &mode)) {
    constexpr std::size_t consoleChunk = 16384;
    std::size_t offset = 0;
    while (offset < text.size()) {
      auto count = std::min(consoleChunk, text.size() - offset);
      if (offset + count < text.size() && text[offset + count - 1] >= 0xd800 && text[offset + count - 1] <= 0xdbff) --count;
      DWORD written = 0;
      if (!WriteConsoleW(handle, text.data() + offset, static_cast<DWORD>(count), &written, nullptr))
        throw WindowsSystemError(GetLastError(), "write console text");
      if (!written) throw WindowsSystemError(ERROR_WRITE_FAULT, "write console text");
      offset += written;
    }
  } else {
    constexpr std::size_t byteChunk = 65536;
    std::size_t offset = 0;
    while (offset < utf8.size()) {
      const auto count = std::min(byteChunk, utf8.size() - offset);
      DWORD written = 0;
      if (!WriteFile(handle, utf8.data() + offset, static_cast<DWORD>(count), &written, nullptr))
        throw WindowsSystemError(GetLastError(), "write redirected console stream");
      if (!written) throw WindowsSystemError(ERROR_WRITE_FAULT, "write redirected console stream");
      offset += written;
    }
  }
}
void ConsoleSession::writeOutput(const std::string_view utf8) const { writeConsoleText(m_output, utf8); }
void ConsoleSession::writeError(const std::string_view utf8) const { writeConsoleText(m_error, utf8); }
ConsoleCancellation::ConsoleCancellation() : m_stop(std::make_shared<std::stop_source>()) {
  auto& state = cancellationState();
  {
    const std::lock_guard lock(state.mutex);
    if (state.source) throw std::logic_error("Console cancellation is already registered");
    state.source = m_stop;
  }
  DWORD process = 0;
  if (GetConsoleProcessList(&process, 1) == 0) return;
  m_registered = SetConsoleCtrlHandler(controlHandler, TRUE) != FALSE;
  if (!m_registered) {
    const auto error = GetLastError();
    const std::lock_guard lock(state.mutex);
    state.source.reset();
    throw WindowsSystemError(error, "register console cancellation");
  }
}
ConsoleCancellation::~ConsoleCancellation() {
  if (m_registered) SetConsoleCtrlHandler(controlHandler, FALSE);
  auto& state = cancellationState();
  const std::lock_guard lock(state.mutex);
  state.source.reset();
}
std::stop_token ConsoleCancellation::token() const { return m_stop->get_token(); }
}  // namespace uwf::windows
