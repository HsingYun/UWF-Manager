// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/ipc/NamedPipeIo.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/resources/UniqueHandle.h"

namespace uwf::windows {
namespace {
[[noreturn]] void throwSystemError(const char* operation, const DWORD error = GetLastError()) { throw WindowsSystemError(error, operation); }
bool disconnectedError(const DWORD error) { return error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA || error == ERROR_PIPE_NOT_CONNECTED; }

bool stopped(const NamedPipeIoContext& context) {
  if (!context.stopEvent) return false;
  const auto wait = WaitForSingleObject(context.stopEvent, 0);
  if (wait == WAIT_FAILED) throwSystemError("observe named pipe stop event");
  return wait == WAIT_OBJECT_0;
}

DWORD remainingWait(const NamedPipeIoContext& context) {
  if (!context.deadline) return INFINITE;
  const auto now = std::chrono::steady_clock::now();
  if (now >= *context.deadline) return 0;
  const auto milliseconds = std::chrono::ceil<std::chrono::milliseconds>(*context.deadline - now).count();
  return static_cast<DWORD>(std::min<std::int64_t>(milliseconds, std::numeric_limits<DWORD>::max() - 1));
}

void cancelAndDrain(const HANDLE pipe, OVERLAPPED& overlapped) {
  // Cancellation is a request. The kernel still owns OVERLAPPED and buffers
  // until the final completion is collected, including a completion racing us.
  const bool canceled = CancelIoEx(pipe, &overlapped) != FALSE;
  const auto cancelError = canceled ? ERROR_SUCCESS : GetLastError();
  DWORD ignored = 0;
  const bool completed = GetOverlappedResult(pipe, &overlapped, &ignored, TRUE) != FALSE;
  const auto completionError = completed ? ERROR_SUCCESS : GetLastError();
  if (!canceled && cancelError != ERROR_NOT_FOUND) throwSystemError("cancel named pipe operation", cancelError);
  if (!completed && completionError != ERROR_OPERATION_ABORTED && !disconnectedError(completionError))
    throwSystemError("finish named pipe cancellation", completionError);
}

struct IoCompletion {
  NamedPipeIoResult result = NamedPipeIoResult::Completed;
  DWORD transferred = 0;
};

IoCompletion awaitPendingIo(const HANDLE pipe, OVERLAPPED& overlapped, const NamedPipeIoContext& context) {
  const std::array events{context.stopEvent, overlapped.hEvent};
  for (;;) {
    const DWORD wait = WaitForMultipleObjects(context.stopEvent ? 2 : 1, events.data() + (context.stopEvent ? 0 : 1), FALSE, remainingWait(context));
    if (context.stopEvent && wait == WAIT_OBJECT_0) {
      cancelAndDrain(pipe, overlapped);
      return {NamedPipeIoResult::Stopped, 0};
    }
    if (wait == WAIT_TIMEOUT) {
      // A very distant deadline can require more than one DWORD-sized wait.
      if (remainingWait(context) != 0) continue;
      cancelAndDrain(pipe, overlapped);
      return {stopped(context) ? NamedPipeIoResult::Stopped : NamedPipeIoResult::TimedOut, 0};
    }
    if (wait != WAIT_OBJECT_0 + (context.stopEvent ? 1 : 0)) {
      const auto error = GetLastError();
      cancelAndDrain(pipe, overlapped);
      throwSystemError("wait for named pipe operation", error);
    }
    DWORD transferred = 0;
    if (!GetOverlappedResult(pipe, &overlapped, &transferred, FALSE)) {
      const DWORD error = GetLastError();
      if (disconnectedError(error)) return {NamedPipeIoResult::Disconnected, 0};
      if (error == ERROR_OPERATION_ABORTED) return {stopped(context) ? NamedPipeIoResult::Stopped : NamedPipeIoResult::Disconnected, 0};
      throwSystemError("complete named pipe operation", error);
    }
    return {NamedPipeIoResult::Completed, transferred};
  }
}

template <typename Start>
IoCompletion runIo(const HANDLE pipe, const NamedPipeIoContext& context, Start&& start) {
  if (stopped(context)) return {NamedPipeIoResult::Stopped, 0};
  if (remainingWait(context) == 0) return {NamedPipeIoResult::TimedOut, 0};
  UniqueHandle event(CreateEventW(nullptr, TRUE, FALSE, nullptr));
  if (!event.valid()) throwSystemError("create named pipe operation event");
  OVERLAPPED overlapped{};
  overlapped.hEvent = event.get();
  DWORD transferred = 0;
  if (start(overlapped, transferred)) return {NamedPipeIoResult::Completed, transferred};
  const DWORD error = GetLastError();
  if (error == ERROR_IO_PENDING) return awaitPendingIo(pipe, overlapped, context);
  if (disconnectedError(error)) return {NamedPipeIoResult::Disconnected, 0};
  throwSystemError("start named pipe operation", error);
}
}  // namespace

NamedPipeIoResult connectNamedPipe(const HANDLE pipe, const NamedPipeIoContext& context) {
  return runIo(pipe, context,
               [&](OVERLAPPED& overlapped, DWORD&) {
                 if (ConnectNamedPipe(pipe, &overlapped)) return true;
                 return GetLastError() == ERROR_PIPE_CONNECTED;
               })
      .result;
}
NamedPipeIoResult readNamedPipe(const HANDLE pipe, std::span<std::byte> bytes, const NamedPipeIoContext& context) {
  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const DWORD requested = static_cast<DWORD>(std::min(bytes.size() - offset, static_cast<std::size_t>(std::numeric_limits<DWORD>::max())));
    const auto completion = runIo(pipe, context, [&](OVERLAPPED& overlapped, DWORD& transferred) {
      return ReadFile(pipe, bytes.data() + offset, requested, &transferred, &overlapped) != FALSE;
    });
    if (completion.result != NamedPipeIoResult::Completed) return completion.result;
    if (completion.transferred == 0) return NamedPipeIoResult::Disconnected;
    offset += completion.transferred;
  }
  return NamedPipeIoResult::Completed;
}
NamedPipeIoResult writeNamedPipe(const HANDLE pipe, std::span<const std::byte> bytes, const NamedPipeIoContext& context) {
  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const DWORD requested = static_cast<DWORD>(std::min(bytes.size() - offset, static_cast<std::size_t>(std::numeric_limits<DWORD>::max())));
    const auto completion = runIo(pipe, context, [&](OVERLAPPED& overlapped, DWORD& transferred) {
      return WriteFile(pipe, bytes.data() + offset, requested, &transferred, &overlapped) != FALSE;
    });
    if (completion.result != NamedPipeIoResult::Completed) return completion.result;
    if (completion.transferred == 0) return NamedPipeIoResult::Disconnected;
    offset += completion.transferred;
  }
  return NamedPipeIoResult::Completed;
}
NamedPipeFrameResult readNamedPipeFrame(const HANDLE pipe, std::span<std::byte> bytes, const HANDLE stopEvent,
                                        const std::chrono::milliseconds exchangeTimeout) {
  if (bytes.empty() || exchangeTimeout <= std::chrono::milliseconds::zero())
    throw std::invalid_argument("a named pipe frame requires bytes and a positive timeout");
  NamedPipeIoContext context{stopEvent};
  const auto first = readNamedPipe(pipe, bytes.first(1), context);
  if (first != NamedPipeIoResult::Completed) return {first, context};
  context.deadline = std::chrono::steady_clock::now() + exchangeTimeout;
  return {readNamedPipe(pipe, bytes.subspan(1), context), context};
}
}  // namespace uwf::windows
