// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>

#include <chrono>
#include <cstddef>
#include <optional>
#include <span>

namespace uwf::windows {

enum class NamedPipeIoResult {
  Completed,
  Disconnected,
  Stopped,
  TimedOut,
};

struct NamedPipeIoContext {
  HANDLE stopEvent = nullptr;
  // One absolute deadline is shared by all chunks and both directions of an
  // exchange. Absence is intentional for a long-lived listener's idle wait.
  std::optional<std::chrono::steady_clock::time_point> deadline{};
};

// All handles must support overlapped I/O. Timeout and stop cancel and drain
// pending kernel work before returning, keeping OVERLAPPED and buffers alive.
[[nodiscard]] NamedPipeIoResult connectNamedPipe(HANDLE pipe, const NamedPipeIoContext& context);
[[nodiscard]] NamedPipeIoResult readNamedPipe(HANDLE pipe, std::span<std::byte> bytes, const NamedPipeIoContext& context);
[[nodiscard]] NamedPipeIoResult writeNamedPipe(HANDLE pipe, std::span<const std::byte> bytes, const NamedPipeIoContext& context);

struct NamedPipeFrameResult {
  NamedPipeIoResult result;
  NamedPipeIoContext exchange;
};
// A persistent channel can idle before a frame starts. Once its first byte
// arrives, the complete remaining body and response share a finite deadline.
[[nodiscard]] NamedPipeFrameResult readNamedPipeFrame(HANDLE pipe, std::span<std::byte> bytes, HANDLE stopEvent, std::chrono::milliseconds exchangeTimeout);

}  // namespace uwf::windows
