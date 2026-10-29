// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "EnhancedModePipeIo.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

#include "windows/resources/UniqueHandle.h"
#include "windows/system/WindowsSystemError.h"

namespace uwf::windows {

namespace {

[[noreturn]] void throwSystemError(const char* operation, const DWORD error = GetLastError()) { throw WindowsSystemError(error, operation); }

[[nodiscard]] bool disconnectedError(const DWORD error) { return error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA || error == ERROR_PIPE_NOT_CONNECTED; }

struct IoCompletion {
  EnhancedPipeIoResult result = EnhancedPipeIoResult::Completed;
  DWORD transferred = 0;
};

[[nodiscard]] IoCompletion awaitPendingIo(const HANDLE pipe, OVERLAPPED& overlapped, const HANDLE stopEvent) {
  const HANDLE events[] = {stopEvent, overlapped.hEvent};
  const DWORD wait = WaitForMultipleObjects(2, events, FALSE, INFINITE);
  if (wait == WAIT_OBJECT_0) {
    if (!CancelIoEx(pipe, &overlapped)) {
      const DWORD error = GetLastError();
      if (error != ERROR_NOT_FOUND) throwSystemError("cancel enhanced mode pipe operation", error);
    }
    // CancelIoEx 只发出取消请求。OVERLAPPED 在完成包到达前仍归内核使用，
    // 必须等事件并取得最终状态后才能让栈上的结构离开作用域。
    if (WaitForSingleObject(overlapped.hEvent, INFINITE) != WAIT_OBJECT_0) {
      throwSystemError("wait for enhanced mode pipe cancellation");
    }
    DWORD ignored = 0;
    if (!GetOverlappedResult(pipe, &overlapped, &ignored, FALSE)) {
      const DWORD error = GetLastError();
      if (error != ERROR_OPERATION_ABORTED && !disconnectedError(error)) {
        throwSystemError("finish enhanced mode pipe cancellation", error);
      }
    }
    return {EnhancedPipeIoResult::Stopped, 0};
  }
  if (wait != WAIT_OBJECT_0 + 1) throwSystemError("wait for enhanced mode pipe operation");

  DWORD transferred = 0;
  if (!GetOverlappedResult(pipe, &overlapped, &transferred, FALSE)) {
    const DWORD error = GetLastError();
    if (disconnectedError(error)) return {EnhancedPipeIoResult::Disconnected, 0};
    if (error == ERROR_OPERATION_ABORTED) {
      return {WaitForSingleObject(stopEvent, 0) == WAIT_OBJECT_0 ? EnhancedPipeIoResult::Stopped : EnhancedPipeIoResult::Disconnected, 0};
    }
    throwSystemError("complete enhanced mode pipe operation", error);
  }
  return {EnhancedPipeIoResult::Completed, transferred};
}

template <typename Start>
[[nodiscard]] IoCompletion runIo(const HANDLE pipe, const HANDLE stopEvent, Start&& start) {
  if (WaitForSingleObject(stopEvent, 0) == WAIT_OBJECT_0) return {EnhancedPipeIoResult::Stopped, 0};

  UniqueHandle event(CreateEventW(nullptr, TRUE, FALSE, nullptr));
  if (!event.get()) throwSystemError("create enhanced mode pipe operation event");
  OVERLAPPED overlapped{};
  overlapped.hEvent = event.get();
  DWORD transferred = 0;
  if (start(overlapped, transferred)) return {EnhancedPipeIoResult::Completed, transferred};

  const DWORD error = GetLastError();
  if (error == ERROR_IO_PENDING) return awaitPendingIo(pipe, overlapped, stopEvent);
  if (disconnectedError(error)) return {EnhancedPipeIoResult::Disconnected, 0};
  throwSystemError("start enhanced mode pipe operation", error);
}

}  // namespace

EnhancedPipeIoResult connectEnhancedPipe(const HANDLE pipe, const HANDLE stopEvent) {
  const auto completion = runIo(pipe, stopEvent, [&](OVERLAPPED& overlapped, DWORD&) {
    if (ConnectNamedPipe(pipe, &overlapped)) return true;
    if (GetLastError() == ERROR_PIPE_CONNECTED) return true;
    return false;
  });
  return completion.result;
}

EnhancedPipeIoResult readEnhancedPipe(const HANDLE pipe, std::span<std::byte> bytes, const HANDLE stopEvent) {
  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const DWORD requested = static_cast<DWORD>(std::min(bytes.size() - offset, static_cast<std::size_t>(std::numeric_limits<DWORD>::max())));
    const auto completion = runIo(pipe, stopEvent, [&](OVERLAPPED& overlapped, DWORD& transferred) {
      return ReadFile(pipe, bytes.data() + offset, requested, &transferred, &overlapped) != FALSE;
    });
    if (completion.result != EnhancedPipeIoResult::Completed) return completion.result;
    if (completion.transferred == 0) return EnhancedPipeIoResult::Disconnected;
    offset += static_cast<std::size_t>(completion.transferred);
  }
  return EnhancedPipeIoResult::Completed;
}

EnhancedPipeIoResult writeEnhancedPipe(const HANDLE pipe, std::span<const std::byte> bytes, const HANDLE stopEvent) {
  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const DWORD requested = static_cast<DWORD>(std::min(bytes.size() - offset, static_cast<std::size_t>(std::numeric_limits<DWORD>::max())));
    const auto completion = runIo(pipe, stopEvent, [&](OVERLAPPED& overlapped, DWORD& transferred) {
      return WriteFile(pipe, bytes.data() + offset, requested, &transferred, &overlapped) != FALSE;
    });
    if (completion.result != EnhancedPipeIoResult::Completed) return completion.result;
    if (completion.transferred == 0) return EnhancedPipeIoResult::Disconnected;
    offset += static_cast<std::size_t>(completion.transferred);
  }
  return EnhancedPipeIoResult::Completed;
}

}  // namespace uwf::windows
