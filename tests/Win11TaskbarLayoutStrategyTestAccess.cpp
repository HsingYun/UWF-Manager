/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "Win11TaskbarLayoutStrategyTestAccess.h"

#include <windows.h>

#include <atomic>
#include <memory>

#include "ui/Win11TaskbarLayoutStrategyImpl.h"

namespace uwf::ui {
namespace {

bool setWindowLongChecked(const HWND window, const int index, const LONG_PTR value) {
  SetLastError(ERROR_SUCCESS);
  const LONG_PTR previous = SetWindowLongPtrW(window, index, value);
  return previous != 0 || GetLastError() == ERROR_SUCCESS;
}

HANDLE nextAttachmentCookie() {
  static std::atomic<ULONG_PTR> nextCookie{1};
  ULONG_PTR cookie = nextCookie.fetch_add(1, std::memory_order_relaxed);
  if (cookie == 0) cookie = nextCookie.fetch_add(1, std::memory_order_relaxed);
  return reinterpret_cast<HANDLE>(cookie);
}

bool ownsNativeWindow(const HWND window) {
  DWORD processId = 0;
  if (!IsWindow(window)) return false;
  GetWindowThreadProcessId(window, &processId);
  return processId == GetCurrentProcessId();
}

}  // namespace

bool Win11TaskbarLayoutStrategyTestAccess::plantPartialParentCommit(Win11TaskbarLayoutStrategy& strategy, QWindow* const window,
                                                                    const win11_taskbar::Environment& environment, const QSize& logicalSize) {
  if (!window || strategy.m_impl->attachment || !IsWindow(environment.taskbar)) return false;
  const HWND hwnd = reinterpret_cast<HWND>(window->winId());
  if (!ownsNativeWindow(hwnd) || GetPropW(hwnd, kWin11TaskbarAttachmentCookieProperty)) return false;

  std::unique_ptr<QWindow> taskbarWindow(QWindow::fromWinId(reinterpret_cast<WId>(environment.taskbar)));
  if (!taskbarWindow) return false;
  const HANDLE cookie = nextAttachmentCookie();
  if (!SetPropW(hwnd, kWin11TaskbarAttachmentCookieProperty, cookie)) return false;

  strategy.m_impl->attachment.emplace(Win11TaskbarLayoutStrategy::Impl::Attachment{
      environment, window, std::move(taskbarWindow), window->winId(), window->parent(), window->QObject::parent(), GetParent(hwnd),
      GetWindowLongPtrW(hwnd, GWL_STYLE), GetWindowLongPtrW(hwnd, GWL_EXSTYLE), cookie, logicalSize});
  auto& attached = *strategy.m_impl->attachment;
  LONG_PTR style = attached.originalStyle;
  style &= ~static_cast<LONG_PTR>(WS_POPUP);
  style |= static_cast<LONG_PTR>(WS_CHILD);
  LONG_PTR exStyle = attached.originalExStyle;
  exStyle &= ~static_cast<LONG_PTR>(WS_EX_APPWINDOW);
  exStyle |= static_cast<LONG_PTR>(WS_EX_NOACTIVATE);
  if (!setWindowLongChecked(hwnd, GWL_STYLE, style) || !setWindowLongChecked(hwnd, GWL_EXSTYLE, exStyle)) {
    (void)strategy.detach();
    return false;
  }
  window->setParent(attached.taskbarWindow.get());
  return GetPropW(hwnd, kWin11TaskbarAttachmentCookieProperty) == cookie && (GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_CHILD) != 0;
}

TaskbarLayoutStrategy::AttachResult Win11TaskbarLayoutStrategyTestAccess::abortIncompleteParentCommit(Win11TaskbarLayoutStrategy& strategy) {
  return strategy.abortIncompleteParentCommit();
}

bool Win11TaskbarLayoutStrategyTestAccess::hasLiveAttachment(const Win11TaskbarLayoutStrategy& strategy) { return strategy.m_impl->attachment.has_value(); }

}  // namespace uwf::ui
