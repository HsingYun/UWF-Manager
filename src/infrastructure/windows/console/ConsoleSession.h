// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>

#include <memory>
#include <stop_token>
#include <string_view>

namespace uwf::windows {
// Called only after excluding --quiet/--service. A redirected standard stream
// is a console use case even when the parent has no console to attach to.
[[nodiscard]] bool hasConsoleContext() noexcept;

// Inspect stdout/stderr only. An inherited pipe, file or non-console device
// selects standard-stream output without attaching or changing any handle.
[[nodiscard]] bool hasRedirectedConsoleOutput() noexcept;

// Writes Unicode to a real console, UTF-8 bytes to files/pipes. The handle is
// borrowed; no CRT stream, code page or process-global redirection is changed.
void writeConsoleText(HANDLE handle, std::string_view utf8);

class ConsoleSession final {
 public:
  ConsoleSession();
  [[nodiscard]] static bool attach() noexcept;
  void writeOutput(std::string_view utf8) const;
  void writeError(std::string_view utf8) const;

 private:
  HANDLE m_output = nullptr;
  HANDLE m_error = nullptr;
};

// Ctrl+C/Break requests cooperative cancellation. An in-flight provider call
// may finish before it observes the token; its real effect must still be kept.
class ConsoleCancellation final {
 public:
  ConsoleCancellation();
  ~ConsoleCancellation();
  ConsoleCancellation(const ConsoleCancellation&) = delete;
  ConsoleCancellation& operator=(const ConsoleCancellation&) = delete;
  [[nodiscard]] std::stop_token token() const;

 private:
  std::shared_ptr<std::stop_source> m_stop;
  bool m_registered = false;
};
}  // namespace uwf::windows
