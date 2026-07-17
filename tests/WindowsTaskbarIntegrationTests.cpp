/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include <dwmapi.h>
#include <windows.h>

#include <QAction>
#include <QElapsedTimer>
#include <QEnterEvent>
#include <QEvent>
#include <QFontMetrics>
#include <QLabel>
#include <QMenu>
#include <QScreen>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

#include "Win11TaskbarLayoutStrategyTestAccess.h"
#include "core/UwfModel.h"
#include "ui/I18n.h"
#include "ui/OverlayFloatingWidget.h"
#include "ui/OverlayHub.h"
#include "ui/OverlayHudRenderer.h"
#include "ui/OverlayTaskbarWidget.h"
#include "ui/Win11TaskbarEnvironment.h"
#include "ui/Win11TaskbarLayoutStrategy.h"
#include "util/Log.h"

namespace uwf::ui {

namespace {

constexpr wchar_t kAttachmentCookieProperty[] = L"UWF.TaskbarHub.AttachmentCookie";
constexpr int kUiTimeoutMs = 6000;

QByteArray diagnosticLog() {
  QByteArray result;
  for (const std::string& line : uwf::recentLogLines()) {
    result += QByteArray::fromStdString(line);
    result += '\n';
  }
  return result;
}

bool waitUntil(const std::function<bool()>& condition, const int timeoutMs = kUiTimeoutMs) {
  QElapsedTimer elapsed;
  elapsed.start();
  while (!condition() && elapsed.elapsed() < timeoutMs) QTest::qWait(20);
  return condition();
}

HWND nativeWindow(const QWidget& widget) { return reinterpret_cast<HWND>(widget.internalWinId()); }

bool nativeVisible(const QWidget& widget) {
  const HWND window = nativeWindow(widget);
  return widget.isVisible() && window && IsWindow(window) && IsWindowVisible(window);
}

bool sendMouseButton(const DWORD down, const DWORD up, const int clicks = 1) {
  for (int click = 0; click < clicks; ++click) {
    INPUT input[2]{};
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = down;
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = up;
    if (SendInput(2, input, sizeof(INPUT)) != 2) return false;
    if (click + 1 < clicks) QTest::qWait(40);
  }
  return true;
}

std::optional<POINT> nativeClientPoint(QWidget& widget, const QPoint& localPosition = {}) {
  const HWND window = nativeWindow(widget);
  RECT client{};
  if (!window || !GetClientRect(window, &client)) return std::nullopt;
  POINT point{};
  if (localPosition.isNull()) {
    point = {(client.left + client.right) / 2, (client.top + client.bottom) / 2};
  } else {
    const qreal scale = widget.devicePixelRatioF();
    point = {qRound(localPosition.x() * scale), qRound(localPosition.y() * scale)};
  }
  if (!ClientToScreen(window, &point)) return std::nullopt;
  return point;
}

bool nativeMouseClick(QWidget& widget, const Qt::MouseButton button, const int clicks = 1, const QPoint localPosition = {}) {
  const auto global = nativeClientPoint(widget, localPosition);
  if (!global || !SetCursorPos(global->x, global->y)) return false;
  QTest::qWait(30);
  if (button == Qt::LeftButton) return sendMouseButton(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, clicks);
  if (button == Qt::RightButton) return sendMouseButton(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP, clicks);
  return false;
}

bool dispatchWindowLeftClick(QWidget& widget, const QPoint localPosition = {}) {
  const HWND window = nativeWindow(widget);
  RECT client{};
  if (!window || !GetClientRect(window, &client)) return false;
  const qreal scale = widget.devicePixelRatioF();
  const int x = localPosition.isNull() ? (client.left + client.right) / 2 : qRound(localPosition.x() * scale);
  const int y = localPosition.isNull() ? (client.top + client.bottom) / 2 : qRound(localPosition.y() * scale);
  const LPARAM clientPosition = MAKELPARAM(x, y);
  SendMessageW(window, WM_LBUTTONDOWN, MK_LBUTTON, clientPosition);
  SendMessageW(window, WM_LBUTTONUP, 0, clientPosition);
  return true;
}

QMenu* visibleMenu();

bool openContextMenu(QWidget& widget) {
  const HWND window = nativeWindow(widget);
  RECT client{};
  if (!window || !GetClientRect(window, &client)) return false;
  const LPARAM position = MAKELPARAM((client.left + client.right) / 2, (client.top + client.bottom) / 2);
  SendMessageW(window, WM_RBUTTONDOWN, MK_RBUTTON, position);
  SendMessageW(window, WM_RBUTTONUP, 0, position);
  return waitUntil([]() { return visibleMenu() != nullptr; }, 1500);
}

void dragWidget(QWidget& widget, const QPoint& delta) {
  const QPoint start = widget.rect().center();
  QTest::mousePress(&widget, Qt::LeftButton, Qt::NoModifier, start);
  QTest::mouseMove(&widget, start + delta, 50);
  QTest::mouseRelease(&widget, Qt::LeftButton, Qt::NoModifier, start + delta);
}

std::vector<HWND> attachmentWindows(const HWND taskbar) {
  std::vector<HWND> result;
  if (!taskbar) return result;
  EnumChildWindows(
      taskbar,
      [](const HWND window, const LPARAM context) -> BOOL {
        if (GetPropW(window, kAttachmentCookieProperty)) reinterpret_cast<std::vector<HWND>*>(context)->push_back(window);
        return TRUE;
      },
      reinterpret_cast<LPARAM>(&result));
  return result;
}

QByteArray partialInjectionDiagnostic(const QWidget& taskbarView, const HWND explorerTaskbar) {
  QByteArray text;
  text += "explorerAttachments=";
  text += QByteArray::number(static_cast<int>(attachmentWindows(explorerTaskbar).size()));
  const HWND window = nativeWindow(taskbarView);
  text += " hwnd=";
  text += QByteArray::number(reinterpret_cast<qulonglong>(window));
  if (window && IsWindow(window)) {
    const LONG_PTR style = GetWindowLongPtrW(window, GWL_STYLE);
    const LONG_PTR exStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
    text += " cookie=";
    text += GetPropW(window, kAttachmentCookieProperty) ? "yes" : "no";
    text += " parentIsTaskbar=";
    text += GetParent(window) == explorerTaskbar ? "yes" : "no";
    text += " wsChild=";
    text += (style & WS_CHILD) ? "yes" : "no";
    text += " wsPopup=";
    text += (style & WS_POPUP) ? "yes" : "no";
    text += " exNoActivate=";
    text += (exStyle & WS_EX_NOACTIVATE) ? "yes" : "no";
    text += " exAppWindow=";
    text += (exStyle & WS_EX_APPWINDOW) ? "yes" : "no";
  }
  text += '\n';
  text += diagnosticLog();
  return text;
}

bool hasPartialTaskbarInjection(const HWND window, const HWND explorerTaskbar) {
  if (!attachmentWindows(explorerTaskbar).empty()) return true;
  if (!window || !IsWindow(window)) return false;
  if (GetPropW(window, kAttachmentCookieProperty)) return true;
  if (GetParent(window) == explorerTaskbar) return true;
  const LONG_PTR style = GetWindowLongPtrW(window, GWL_STYLE);
  // 注入态是 WS_CHILD 且清掉 WS_POPUP；回滚后不应残留这对组合。
  if ((style & WS_CHILD) != 0 && (style & WS_POPUP) == 0) return true;
  return false;
}

bool hasPartialTaskbarInjection(const QWidget& taskbarView, const HWND explorerTaskbar) {
  return hasPartialTaskbarInjection(nativeWindow(taskbarView), explorerTaskbar);
}

bool stylesMatch(const HWND window, const LONG_PTR style, const LONG_PTR exStyle) {
  return window && IsWindow(window) && GetWindowLongPtrW(window, GWL_STYLE) == style && GetWindowLongPtrW(window, GWL_EXSTYLE) == exStyle;
}

class VisibilityInvariantMonitor final : public QObject {
 public:
  VisibilityInvariantMonitor(QWidget& taskbar, QWidget& floating) : m_taskbar(taskbar), m_floating(floating) {
    taskbar.installEventFilter(this);
    floating.installEventFilter(this);
  }

  void reset() {
    m_overlap = false;
    m_floatingSeen = false;
    observe();
  }
  void observe() {
    const bool taskbarVisible = nativeVisible(m_taskbar);
    const bool floatingVisible = nativeVisible(m_floating);
    m_overlap = m_overlap || (taskbarVisible && floatingVisible);
    m_floatingSeen = m_floatingSeen || floatingVisible;
  }
  [[nodiscard]] bool overlapObserved() const { return m_overlap; }
  [[nodiscard]] bool floatingSeen() const { return m_floatingSeen; }

 protected:
  bool eventFilter(QObject*, QEvent* event) override {
    if (event->type() == QEvent::Show || event->type() == QEvent::Hide || event->type() == QEvent::WinIdChange || event->type() == QEvent::ParentChange ||
        event->type() == QEvent::Move || event->type() == QEvent::Resize)
      observe();
    return false;
  }

 private:
  QWidget& m_taskbar;
  QWidget& m_floating;
  bool m_overlap = false;
  bool m_floatingSeen = false;
};

class ProductionUi final {
 public:
  ProductionUi() {
    auto taskbar = std::make_unique<OverlayTaskbarWidget>();
    taskbarView = taskbar.get();
    m_taskbar = std::move(taskbar);
    auto floating = std::make_unique<OverlayFloatingWidget>();
    floatingView = floating.get();
    m_floating = std::move(floating);
    visibility = std::make_unique<VisibilityInvariantMonitor>(*taskbarView, *floatingView);
    QObject::connect(taskbarView, &OverlayHubView::displayStateChanged, visibility.get(), &VisibilityInvariantMonitor::observe);
    QObject::connect(floatingView, &OverlayHubView::displayStateChanged, visibility.get(), &VisibilityInvariantMonitor::observe);
  }

  void registerTaskbarOnly() {
    hub.registerView(std::move(m_taskbar));
    enable();
  }

  void registerTaskbarFirst() {
    hub.registerView(std::move(m_taskbar));
    hub.registerView(std::move(m_floating));
    enable();
  }

  void registerFloatingOnly() {
    hub.registerView(std::move(m_floating));
    enable();
  }

  void registerPreparedTaskbar() { hub.registerView(std::move(m_taskbar)); }

  void enable() {
    hub.applyUsageState(OverlayUsageEnabled{runtime, core::OverlayConfig{}});
  }

  [[nodiscard]] bool taskbarConfirmed() const {
    return taskbarView->presentationConfirmed() && nativeVisible(*taskbarView) && !nativeVisible(*floatingView) && hub.presented();
  }

  [[nodiscard]] bool floatingConfirmed() const {
    return floatingView->presentationConfirmed() && nativeVisible(*floatingView) && !nativeVisible(*taskbarView) && hub.presented();
  }

  OverlayHub hub;
  OverlayTaskbarWidget* taskbarView = nullptr;
  OverlayFloatingWidget* floatingView = nullptr;
  std::unique_ptr<VisibilityInvariantMonitor> visibility;
  core::OverlayRuntime runtime{.availableSpaceMb = 4096, .currentConsumptionMb = 1024};

 private:
  std::unique_ptr<OverlayTaskbarWidget> m_taskbar;
  std::unique_ptr<OverlayFloatingWidget> m_floating;
};

QMenu* visibleMenu() {
  const QWidgetList topLevels = QApplication::topLevelWidgets();
  for (QWidget* widget : topLevels) {
    if (auto* const menu = qobject_cast<QMenu*>(widget); menu && menu->isVisible()) return menu;
  }
  return nullptr;
}

QAction* menuAction(QMenu& menu, const QString& text) {
  const QList<QAction*> actions = menu.actions();
  for (QAction* action : actions) {
    if (action && action->text() == text) return action;
  }
  return nullptr;
}

bool clickVisibleMenuAction(const QString& text) {
  if (!waitUntil([]() { return visibleMenu() != nullptr; }, 1500)) return false;
  QMenu* const menu = visibleMenu();
  QAction* const action = menu ? menuAction(*menu, text) : nullptr;
  if (!menu || !action) return false;
  return dispatchWindowLeftClick(*menu, menu->actionGeometry(action).center());
}

void scheduleFloatingMenuAction(const QString& text, bool& clicked) {
  QTimer::singleShot(50, qApp, [&clicked, text]() { clicked = clickVisibleMenuAction(text); });
}

QLabel* visibleToolTipLabel() {
  const QWidgetList topLevels = QApplication::topLevelWidgets();
  for (QWidget* widget : topLevels) {
    if (auto* const label = qobject_cast<QLabel*>(widget); label && label->isVisible() && label->text() == I18n::applicationTitle()) return label;
  }
  return nullptr;
}

bool postTaskbarCreated(const HWND receiver) {
  const UINT message = RegisterWindowMessageW(L"TaskbarCreated");
  return receiver && message != 0 && PostMessageW(receiver, message, 0, 0);
}

bool isNativeCloaked(const HWND window) {
  DWORD cloaked = 0;
  return window && SUCCEEDED(DwmGetWindowAttribute(window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked != 0;
}

bool sendVirtualKey(const WORD virtualKey, const bool keyUp) {
  INPUT input{};
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = virtualKey;
  input.ki.dwFlags = keyUp ? KEYEVENTF_KEYUP : 0;
  return SendInput(1, &input, sizeof(INPUT)) == 1;
}

bool tapVirtualKey(const WORD virtualKey) { return sendVirtualKey(virtualKey, false) && sendVirtualKey(virtualKey, true); }

bool attachmentDisturbedByStart(const HWND taskbarHub) { return taskbarHub && (isNativeCloaked(taskbarHub) || !IsWindowVisible(taskbarHub)); }

bool openStartMenu(const HWND taskbarHub = nullptr) {
  if (!tapVirtualKey(VK_LWIN)) return false;
  // 测试侧只负责投键制造抖动；不把 Start 是否真正打开做成硬门禁。
  if (taskbarHub) (void)waitUntil([taskbarHub]() { return attachmentDisturbedByStart(taskbarHub); }, 800);
  QTest::qWait(120);
  return true;
}

bool closeStartMenu() {
  if (!tapVirtualKey(VK_ESCAPE)) return false;
  QTest::qWait(80);
  (void)tapVirtualKey(VK_ESCAPE);
  QTest::qWait(120);
  return true;
}

}  // namespace

class WindowsTaskbarIntegrationTests final : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase() {
    const auto probe = win11_taskbar::probeEnvironment();
    QVERIFY2(probe.availability == win11_taskbar::RuntimeAvailability::Available && probe.environment,
             "the Windows UI integration gate requires a live compatible Explorer taskbar");
    m_explorerTaskbar = probe.environment->taskbar;
    OverlayTaskbarWidget compatibilityProbe;
    QVERIFY2(compatibilityProbe.isCompatible(), "production taskbar strategy is incompatible with this Windows version");
  }

  void init() {
    uwf::clearLogLines();
    (void)closeStartMenu();
    QVERIFY2(waitUntil([this]() { return attachmentWindows(m_explorerTaskbar).empty(); }, 2000),
             "another UWF taskbar attachment is active; UI test isolation is not satisfied");
  }

  void cleanup() {
    if (QMenu* menu = visibleMenu()) menu->close();
    (void)closeStartMenu();
    QCoreApplication::processEvents();
    QVERIFY2(waitUntil([this]() { return attachmentWindows(m_explorerTaskbar).empty(); }, 3000),
             "UI test cleanup left an attachment cookie or Explorer child behind");
  }

  void floatingFirstHandoffIsAtomic() {
    ProductionUi ui;
    ui.registerFloatingOnly();
    QVERIFY2(waitUntil([&ui]() { return ui.floatingConfirmed(); }), diagnosticLog().constData());
    ui.visibility->reset();
    ui.registerPreparedTaskbar();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
    ui.visibility->observe();
    QVERIFY(!ui.visibility->overlapObserved());
    QCOMPARE(GetParent(nativeWindow(*ui.taskbarView)), m_explorerTaskbar);
    QCOMPARE(attachmentWindows(m_explorerTaskbar).size(), std::size_t{1});
  }

  void taskbarFirstNeverFlashesFloating() {
    ProductionUi ui;
    ui.visibility->reset();
    ui.registerTaskbarFirst();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
    ui.visibility->observe();
    QVERIFY(!ui.visibility->overlapObserved());
    QVERIFY(!ui.visibility->floatingSeen());
  }

  void layeredChildStateLossHardResetsAndRecovers() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());

    const HWND oldWindow = nativeWindow(*ui.taskbarView);
    LONG_PTR exStyle = GetWindowLongPtrW(oldWindow, GWL_EXSTYLE);
    QVERIFY((exStyle & WS_EX_LAYERED) != 0);
    exStyle &= ~static_cast<LONG_PTR>(WS_EX_LAYERED);
    SetLastError(ERROR_SUCCESS);
    QVERIFY(SetWindowLongPtrW(oldWindow, GWL_EXSTYLE, exStyle) != 0 || GetLastError() == ERROR_SUCCESS);
    QVERIFY(SetWindowPos(oldWindow, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER));

    ui.visibility->reset();
    QVERIFY2(waitUntil(
                 [&ui, oldWindow, this]() {
                   const HWND current = nativeWindow(*ui.taskbarView);
                   return current && current != oldWindow && !IsWindow(oldWindow) && ui.taskbarConfirmed() && GetParent(current) == m_explorerTaskbar &&
                          (GetWindowLongPtrW(current, GWL_EXSTYLE) & WS_EX_LAYERED) != 0;
                 },
                 15000),
             diagnosticLog().constData());
    ui.visibility->observe();
    QVERIFY2(!ui.visibility->overlapObserved(), diagnosticLog().constData());
    QCOMPARE(attachmentWindows(m_explorerTaskbar).size(), std::size_t{1});
  }

  void showTaskbarToolbarBriefly() {
    ProductionUi ui;
    ui.registerTaskbarOnly();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
    QCOMPARE(GetParent(nativeWindow(*ui.taskbarView)), m_explorerTaskbar);
    QCoreApplication::processEvents();
    QVERIFY2(ui.taskbarConfirmed(), diagnosticLog().constData());
  }

  void startMenuJitterThenToolbarToggleKeepsTaskbarExclusive() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
    ui.visibility->reset();

    for (int cycle = 0; cycle < 3; ++cycle) {
      const HWND hubWindow = nativeWindow(*ui.taskbarView);
      QVERIFY2(openStartMenu(hubWindow), "failed to send VK_LWIN for Start jitter");
      QVERIFY2(closeStartMenu(), "failed to send Escape after Start jitter");
      QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
      QVERIFY2(!ui.visibility->floatingSeen(), diagnosticLog().constData());
      QCOMPARE(attachmentWindows(m_explorerTaskbar).size(), std::size_t{1});
    }

    QVERIFY2(openStartMenu(nativeWindow(*ui.taskbarView)), "failed to send VK_LWIN before toolbar toggle");
    ui.hub.setRequestedVisible(false);
    ui.hub.setRequestedVisible(true);
    QVERIFY2(closeStartMenu(), "failed to send Escape after toolbar toggle");

    for (int cycle = 0; cycle < 2; ++cycle) {
      QVERIFY2(openStartMenu(nativeWindow(*ui.taskbarView)), "failed to send VK_LWIN during post-toggle jitter");
      QVERIFY2(closeStartMenu(), "failed to send Escape during post-toggle jitter");
      ui.hub.setRequestedVisible(false);
      ui.hub.setRequestedVisible(true);
    }

    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed() || ui.floatingConfirmed(); }), diagnosticLog().constData());
    ui.visibility->observe();
    QVERIFY2(!ui.visibility->overlapObserved(), diagnosticLog().constData());
    QVERIFY2(attachmentWindows(m_explorerTaskbar).size() <= 1, diagnosticLog().constData());
    if (ui.taskbarConfirmed()) {
      QCOMPARE(GetParent(nativeWindow(*ui.taskbarView)), m_explorerTaskbar);
      QCOMPARE(attachmentWindows(m_explorerTaskbar).size(), std::size_t{1});
    }
  }

  void sustainedStartMenuAndToolbarHubJitterPreservesInvariants() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
    ui.visibility->reset();

    QElapsedTimer window;
    window.start();
    int cycles = 0;
    while (window.elapsed() < 10000) {
      QVERIFY2(tapVirtualKey(VK_LWIN), "failed to open Start during sustained jitter");
      QTest::qWait(50);
      ui.hub.setRequestedVisible(false);
      QTest::qWait(30);
      ui.hub.setRequestedVisible(true);
      QVERIFY2(tapVirtualKey(VK_ESCAPE), "failed to dismiss Start during sustained jitter");
      QTest::qWait(50);
      ui.hub.setRequestedVisible(false);
      ui.hub.setRequestedVisible(true);
      ui.visibility->observe();
      QVERIFY2(!ui.visibility->overlapObserved(), QByteArray("taskbar and floating overlapped during sustained jitter at cycle ")
                                                      .append(QByteArray::number(cycles))
                                                      .append('\n')
                                                      .append(diagnosticLog())
                                                      .constData());
      QVERIFY2(attachmentWindows(m_explorerTaskbar).size() <= 1, QByteArray("Explorer must never host more than one UWF attachment; count=")
                                                                     .append(QByteArray::number(static_cast<int>(attachmentWindows(m_explorerTaskbar).size())))
                                                                     .append(" cycle=")
                                                                     .append(QByteArray::number(cycles))
                                                                     .append('\n')
                                                                     .append(diagnosticLog())
                                                                     .constData());
      ++cycles;
    }

    (void)closeStartMenu();
    (void)closeStartMenu();
    QVERIFY2(cycles >= 5, "sustained jitter window did not exercise enough Start/toolbar cycles");

    ui.hub.setRequestedVisible(false);
    ui.hub.setRequestedVisible(true);
    // Recovering 可有限次排他重试；耗尽后让出 floating。settle 要求唯一 presentation。
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed() || ui.floatingConfirmed(); }, 15000), diagnosticLog().constData());
    ui.visibility->observe();
    QVERIFY2(!ui.visibility->overlapObserved(), diagnosticLog().constData());
    if (ui.taskbarConfirmed()) {
      QCOMPARE(GetParent(nativeWindow(*ui.taskbarView)), m_explorerTaskbar);
      QCOMPARE(attachmentWindows(m_explorerTaskbar).size(), std::size_t{1});
    } else {
      QCOMPARE(attachmentWindows(m_explorerTaskbar).size(), std::size_t{0});
      QVERIFY(ui.floatingConfirmed());
    }
  }

  void parentMismatchAtomicRollbackRestoresNativeInvariants() {
    // 确定性覆盖 abortIncompleteParentCommit（commit 的 qt-parent-mismatch 同一入口），
    // 不依赖 Start 抖动是否碰巧触发该分支。
    QWidget host(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    host.setAttribute(Qt::WA_DontCreateNativeAncestors, true);
    host.resize(80, 24);
    host.show();
    QVERIFY(waitUntil([&host]() { return nativeWindow(host) != nullptr; }));
    const HWND hwnd = nativeWindow(host);
    const LONG_PTR originalStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
    const LONG_PTR originalExStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    const HWND originalParent = GetParent(hwnd);

    const auto probe = win11_taskbar::probeEnvironment();
    QVERIFY(probe.availability == win11_taskbar::RuntimeAvailability::Available && probe.environment);
    Win11TaskbarLayoutStrategy strategy;
    QVERIFY2(Win11TaskbarLayoutStrategyTestAccess::plantPartialParentCommit(strategy, host.windowHandle(), *probe.environment, host.size()),
             "failed to plant a partial parent-commit injection");
    QVERIFY(Win11TaskbarLayoutStrategyTestAccess::hasLiveAttachment(strategy));
    QVERIFY(GetPropW(hwnd, kAttachmentCookieProperty));
    QVERIFY(GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_CHILD);
    QVERIFY((GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_POPUP) == 0);
    QVERIFY(GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE);
    QVERIFY((GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_APPWINDOW) == 0);

    uwf::clearLogLines();
    const auto result = Win11TaskbarLayoutStrategyTestAccess::abortIncompleteParentCommit(strategy);
    QVERIFY2(result == TaskbarLayoutStrategy::AttachResult::TemporarilyUnavailable || result == TaskbarLayoutStrategy::AttachResult::Invalid,
             "parent-mismatch abort must resolve to soft retry or hard invalidate");
    QVERIFY2(diagnosticLog().contains("qt-parent-mismatch"), diagnosticLog().constData());
    QVERIFY(!Win11TaskbarLayoutStrategyTestAccess::hasLiveAttachment(strategy));

    if (result == TaskbarLayoutStrategy::AttachResult::TemporarilyUnavailable) {
      QVERIFY2(IsWindow(hwnd), "soft atomic rollback must keep the QWindow/HWND for retry");
      QVERIFY2(!GetPropW(hwnd, kAttachmentCookieProperty), "cookie must be removed on atomic rollback");
      QVERIFY2(GetParent(hwnd) == originalParent, "HWND parent must restore to the pre-commit parent");
      QVERIFY2(stylesMatch(hwnd, originalStyle, originalExStyle), partialInjectionDiagnostic(host, m_explorerTaskbar).constData());
      QVERIFY2(!hasPartialTaskbarInjection(hwnd, m_explorerTaskbar), partialInjectionDiagnostic(host, m_explorerTaskbar).constData());
      QVERIFY2(diagnosticLog().contains("atomic-rollback-retry"), diagnosticLog().constData());
    } else {
      QVERIFY2(!IsWindow(hwnd) || !GetPropW(hwnd, kAttachmentCookieProperty), "hard abort must not leave a live cookie");
      QVERIFY2(attachmentWindows(m_explorerTaskbar).empty(), "hard abort must not leave explorer attachments");
      QVERIFY2(diagnosticLog().contains("destroy-all-and-reinject"), diagnosticLog().constData());
    }
  }

  void parentCommitRollbackDoesNotLeavePartialInjection() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());

    // Start + 快速显隐容易触发 qt-parent-mismatch。软回滚必须原子清空 cookie/parent/style，
    // 不能留下半注入让下一次 acquire 误判。独占途中允许 in-flight attachment。
    for (int cycle = 0; cycle < 12; ++cycle) {
      QVERIFY2(openStartMenu(nativeWindow(*ui.taskbarView)), "failed to send VK_LWIN");
      ui.hub.setRequestedVisible(false);
      ui.hub.setRequestedVisible(true);
      QVERIFY2(closeStartMenu(), "failed to send Escape");
      QVERIFY2(waitUntil(
                   [&ui, this]() {
                     if (ui.taskbarConfirmed()) return attachmentWindows(m_explorerTaskbar).size() == 1;
                     if (ui.floatingConfirmed()) return !hasPartialTaskbarInjection(*ui.taskbarView, m_explorerTaskbar);
                     if (ui.taskbarView->presentationExclusive()) return attachmentWindows(m_explorerTaskbar).size() <= 1;
                     return !hasPartialTaskbarInjection(*ui.taskbarView, m_explorerTaskbar);
                   },
                   3000),
               partialInjectionDiagnostic(*ui.taskbarView, m_explorerTaskbar).constData());
    }

    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed() || ui.floatingConfirmed(); }, 15000), diagnosticLog().constData());
    if (ui.taskbarConfirmed()) {
      QCOMPARE(attachmentWindows(m_explorerTaskbar).size(), std::size_t{1});
    } else {
      QVERIFY2(!hasPartialTaskbarInjection(*ui.taskbarView, m_explorerTaskbar), partialInjectionDiagnostic(*ui.taskbarView, m_explorerTaskbar).constData());
    }

    ui.hub.setRequestedVisible(false);
    QVERIFY2(waitUntil([&ui, this]() {
               return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView) && !hasPartialTaskbarInjection(*ui.taskbarView, m_explorerTaskbar);
             }),
             partialInjectionDiagnostic(*ui.taskbarView, m_explorerTaskbar).constData());
  }

  void visibilityAvailabilityAndRapidToggleConverge() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));

    for (int iteration = 0; iteration < 8; ++iteration) {
      ui.hub.setRequestedVisible(false);
      QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView); }));
      ui.hub.setRequestedVisible(true);
      QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    }

    ui.hub.hideTemporarily();
    QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView); }));
    ui.hub.restoreAfterTemporaryHide();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));

    ui.hub.applyUsageState(OverlayUsageUnavailable{});
    QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView); }));
    ui.hub.applyUsageState(OverlayUsageEnabled{ui.runtime, core::OverlayConfig{}});
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));

    ui.hub.applyUsageState(OverlayUsageDisabled{});
    QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView); }));
    ui.hub.applyUsageState(OverlayUsageEnabled{ui.runtime, core::OverlayConfig{}});
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    QVERIFY(!ui.visibility->overlapObserved());
  }

  void nativeHideIsRepairedWithoutFallback() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    ui.visibility->reset();
    const HWND original = nativeWindow(*ui.taskbarView);
    QVERIFY(ShowWindow(original, SW_HIDE));
    QVERIFY(!IsWindowVisible(original));
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
    QVERIFY(!ui.visibility->overlapObserved());
    QVERIFY(!ui.visibility->floatingSeen());
  }

  void placementDriftIsRepairedInPlace() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    const HWND window = nativeWindow(*ui.taskbarView);
    RECT original{};
    QVERIFY(GetWindowRect(window, &original));
    QVERIFY(SetWindowPos(window, nullptr, original.left - 120, original.top, original.right - original.left, original.bottom - original.top,
                         SWP_NOACTIVATE | SWP_NOZORDER));
    ui.visibility->reset();
    QVERIFY2(waitUntil([&ui, original]() {
               RECT repaired{};
               const HWND current = nativeWindow(*ui.taskbarView);
               return ui.taskbarConfirmed() && GetWindowRect(current, &repaired) && repaired.left == original.left && repaired.top == original.top;
             }),
             diagnosticLog().constData());
    QVERIFY(!ui.visibility->overlapObserved());
  }

  void corruptedNativeStyleHardResetsAndRecovers() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    const HWND oldWindow = nativeWindow(*ui.taskbarView);
    const LONG_PTR style = GetWindowLongPtrW(oldWindow, GWL_STYLE);
    SetLastError(ERROR_SUCCESS);
    QVERIFY(SetWindowLongPtrW(oldWindow, GWL_STYLE, (style & ~static_cast<LONG_PTR>(WS_CHILD)) | static_cast<LONG_PTR>(WS_POPUP)) != 0 ||
            GetLastError() == ERROR_SUCCESS);
    QVERIFY(SetWindowPos(oldWindow, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER));
    ui.visibility->reset();
    QVERIFY2(waitUntil([&ui, this]() {
               const HWND recovered = nativeWindow(*ui.taskbarView);
               return ui.taskbarConfirmed() && recovered && (GetWindowLongPtrW(recovered, GWL_STYLE) & WS_CHILD) && GetParent(recovered) == m_explorerTaskbar;
             }),
             diagnosticLog().constData());
    QVERIFY(!ui.visibility->overlapObserved());
  }

  void destroyedNativeWindowRecovers() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    const HWND oldWindow = nativeWindow(*ui.taskbarView);
    ui.visibility->reset();
    QVERIFY(DestroyWindow(oldWindow));
    QVERIFY2(waitUntil([&ui, oldWindow]() { return ui.taskbarConfirmed() && nativeWindow(*ui.taskbarView) != oldWindow; }), diagnosticLog().constData());
    QVERIFY(!ui.visibility->overlapObserved());
  }

  void taskbarCreatedAttachedAndDetachedConverge() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    ui.visibility->reset();
    const HWND oldWindow = nativeWindow(*ui.taskbarView);
    QVERIFY(postTaskbarCreated(nativeWindow(*ui.taskbarView)));
    QVERIFY2(waitUntil([&ui, oldWindow]() {
               const HWND current = nativeWindow(*ui.taskbarView);
               return current && current != oldWindow && !IsWindow(oldWindow) && ui.taskbarConfirmed();
             }),
             diagnosticLog().constData());
    QVERIFY(!ui.visibility->overlapObserved());

    ui.hub.setRequestedVisible(false);
    QVERIFY(waitUntil(
        [&ui, this]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView) && attachmentWindows(m_explorerTaskbar).empty(); }));
    QWidget receiver;
    (void)receiver.winId();
    QVERIFY(postTaskbarCreated(nativeWindow(receiver)));
    QTest::qWait(100);
    QVERIFY(!nativeVisible(*ui.taskbarView));
    QVERIFY(!nativeVisible(*ui.floatingView));
    ui.hub.setRequestedVisible(true);
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
  }

  void explorerRestartSignalThenToolbarToggleKeepsTaskbarExclusive() {
    // Simulate Explorer restart the way Shell itself announces it: broadcast-equivalent
    // TaskbarCreated to our HWND. Never TerminateProcess(explorer.exe).
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
    ui.visibility->reset();

    for (int cycle = 0; cycle < 2; ++cycle) {
      const HWND oldWindow = nativeWindow(*ui.taskbarView);
      QVERIFY2(postTaskbarCreated(oldWindow), "failed to deliver the Explorer TaskbarCreated restart signal");
      QVERIFY2(waitUntil([&ui, oldWindow]() {
                 const HWND current = nativeWindow(*ui.taskbarView);
                 return current && current != oldWindow && !IsWindow(oldWindow) && ui.taskbarConfirmed();
               }),
               diagnosticLog().constData());
      QVERIFY2(!ui.visibility->floatingSeen(), diagnosticLog().constData());
      QVERIFY2(!ui.visibility->overlapObserved(), diagnosticLog().constData());
    }

    // Restart signal while the toolbar hub is being toggled — the product failure mode
    // where a recovering taskbar must stay exclusive instead of falling back to floating.
    const HWND beforeToggle = nativeWindow(*ui.taskbarView);
    QVERIFY(postTaskbarCreated(beforeToggle));
    ui.hub.setRequestedVisible(false);
    ui.hub.setRequestedVisible(true);
    QVERIFY2(waitUntil([&ui, beforeToggle]() {
               const HWND current = nativeWindow(*ui.taskbarView);
               return ui.taskbarConfirmed() && current && current != beforeToggle;
             }),
             diagnosticLog().constData());

    ui.hub.setRequestedVisible(false);
    QVERIFY(waitUntil(
        [&ui, this]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView) && attachmentWindows(m_explorerTaskbar).empty(); }));
    {
      // Deliver TaskbarCreated to a live HWND in this process so Qt's nativeEventFilter sees it.
      // Do not post to Explorer's Shell_TrayWnd — that lives in explorer.exe.
      QWidget receiver;
      (void)receiver.winId();
      QVERIFY(postTaskbarCreated(nativeWindow(receiver)));
    }
    // Detached host refresh must not spontaneously show either surface.
    QTest::qWait(150);
    QVERIFY(!nativeVisible(*ui.taskbarView));
    QVERIFY(!nativeVisible(*ui.floatingView));
    ui.hub.setRequestedVisible(true);
    QVERIFY2(waitUntil([&ui, this]() {
               return ui.taskbarConfirmed() && GetParent(nativeWindow(*ui.taskbarView)) == m_explorerTaskbar &&
                      attachmentWindows(m_explorerTaskbar).size() == 1;
             }),
             diagnosticLog().constData());
    ui.visibility->observe();
    QVERIFY2(!ui.visibility->overlapObserved(), diagnosticLog().constData());
    QVERIFY2(!ui.visibility->floatingSeen(), "simulated Explorer restart (TaskbarCreated) plus toolbar toggles must not yield to the floating fallback");
  }

  void taskbarCreatedDuringTemporaryHideThenRestoreKeepsTaskbarExclusive() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
    ui.visibility->reset();

    ui.hub.hideTemporarily();
    QVERIFY2(waitUntil(
                 [&ui, this]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView) && attachmentWindows(m_explorerTaskbar).empty(); }),
             diagnosticLog().constData());

    QWidget receiver;
    (void)receiver.winId();
    QVERIFY2(postTaskbarCreated(nativeWindow(receiver)), "failed to deliver TaskbarCreated while the hub is temporarily hidden");
    QTest::qWait(150);
    QVERIFY2(!nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView),
             "TaskbarCreated while temporarily hidden must not spontaneously show either surface");
    QVERIFY2(!ui.visibility->floatingSeen(), diagnosticLog().constData());

    ui.hub.restoreAfterTemporaryHide();
    QVERIFY2(waitUntil([&ui, this]() { return ui.taskbarConfirmed() && GetParent(nativeWindow(*ui.taskbarView)) == m_explorerTaskbar; }),
             diagnosticLog().constData());
    QVERIFY2(!ui.visibility->floatingSeen(), "restore after TaskbarCreated-during-hide must not fall back to floating");
    QVERIFY2(!ui.visibility->overlapObserved(), diagnosticLog().constData());
    QCOMPARE(attachmentWindows(m_explorerTaskbar).size(), std::size_t{1});
  }

  void taskbarUsageBoundariesRemainReadableAndAttached() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    const HWND originalWindow = nativeWindow(*ui.taskbarView);
    ui.visibility->reset();

    const auto verifyRuntime = [&ui, originalWindow](const core::OverlayRuntime runtime) {
      ui.hub.applyUsageState(OverlayUsageEnabled{runtime, core::OverlayConfig{}});
      const QString text = overlayUsageText(runtime);
      QFont font = ui.taskbarView->font();
      font.setWeight(QFont::DemiBold);
      return waitUntil([&ui, originalWindow, text, font]() {
        return ui.taskbarConfirmed() && nativeWindow(*ui.taskbarView) == originalWindow && ui.taskbarView->width() > QFontMetrics(font).horizontalAdvance(text);
      });
    };

    QVERIFY2(verifyRuntime({.availableSpaceMb = 0, .currentConsumptionMb = 0}), diagnosticLog().constData());
    const int zeroWidth = ui.taskbarView->width();
    QVERIFY2(verifyRuntime({.availableSpaceMb = std::numeric_limits<uint32_t>::max(), .currentConsumptionMb = std::numeric_limits<uint32_t>::max()}),
             diagnosticLog().constData());
    QVERIFY(ui.taskbarView->width() > zeroWidth);
    QVERIFY2(verifyRuntime({.availableSpaceMb = 0, .currentConsumptionMb = std::numeric_limits<uint32_t>::max()}), diagnosticLog().constData());
    QCOMPARE(GetParent(nativeWindow(*ui.taskbarView)), m_explorerTaskbar);
    QVERIFY(!ui.visibility->overlapObserved());
    QVERIFY(!ui.visibility->floatingSeen());
  }

  void disableDuringTaskbarRecoveryCancelsFallbackAndRetry() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    const HWND window = nativeWindow(*ui.taskbarView);
    const LONG_PTR style = GetWindowLongPtrW(window, GWL_STYLE);
    SetLastError(ERROR_SUCCESS);
    QVERIFY(SetWindowLongPtrW(window, GWL_STYLE, (style & ~static_cast<LONG_PTR>(WS_CHILD)) | static_cast<LONG_PTR>(WS_POPUP)) != 0 ||
            GetLastError() == ERROR_SUCCESS);
    ui.hub.setRequestedVisible(false);
    QVERIFY(waitUntil(
        [&ui, this]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView) && attachmentWindows(m_explorerTaskbar).empty(); }));
    QTest::qWait(1200);
    QVERIFY(!nativeVisible(*ui.taskbarView));
    QVERIFY(!nativeVisible(*ui.floatingView));
    ui.hub.setRequestedVisible(true);
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());
  }

  void floatingMenuClosesDuringTaskbarHandoff() {
    ProductionUi ui;
    ui.registerFloatingOnly();
    QVERIFY(waitUntil([&ui]() { return ui.floatingConfirmed(); }));
    QWidget* const handle = ui.floatingView->findChild<QWidget*>(QStringLiteral("overlayFloatingHandle"));
    QVERIFY(handle);
    QTimer::singleShot(50, &ui.hub, [&ui]() { ui.registerPreparedTaskbar(); });
    QVERIFY(nativeMouseClick(*handle, Qt::RightButton));
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed() && visibleMenu() == nullptr; }), diagnosticLog().constData());
    QVERIFY(!nativeVisible(*ui.floatingView));
    QVERIFY(!ui.visibility->overlapObserved());
  }

  void taskbarMouseTooltipAndContextMenuActions() {
    ProductionUi ui;
    QSignalSpy showSpy(&ui.hub, &OverlayHub::showMainWindowRequested);
    QSignalSpy shutdownSpy(&ui.hub, &OverlayHub::safeShutdownRequested);
    QSignalSpy restartSpy(&ui.hub, &OverlayHub::safeRestartRequested);
    QSignalSpy exitSpy(&ui.hub, &OverlayHub::exitApplicationRequested);
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));

    const auto taskbarHitPoint = nativeClientPoint(*ui.taskbarView);
    QVERIFY(taskbarHitPoint.has_value());
    const HWND hitWindow = WindowFromPoint(*taskbarHitPoint);
    const QByteArray hitDiagnostic = QStringLiteral("visible taskbar HWND is not hit-testable: widget=0x%1 hit=0x%2")
                                         .arg(reinterpret_cast<quintptr>(nativeWindow(*ui.taskbarView)), 0, 16)
                                         .arg(reinterpret_cast<quintptr>(hitWindow), 0, 16)
                                         .toLocal8Bit();
    QVERIFY2(hitWindow == nativeWindow(*ui.taskbarView) || IsChild(nativeWindow(*ui.taskbarView), hitWindow), hitDiagnostic.constData());
    QVERIFY(dispatchWindowLeftClick(*ui.taskbarView));
    QVERIFY(waitUntil([&showSpy]() { return showSpy.count() == 1; }));
    QCOMPARE(showSpy.count(), 1);

    const QPointF localCenter(ui.taskbarView->rect().center());
    const QPointF globalCenter(ui.taskbarView->mapToGlobal(ui.taskbarView->rect().center()));
    QEnterEvent enter(localCenter, localCenter, globalCenter);
    QVERIFY(QCoreApplication::sendEvent(ui.taskbarView, &enter));
    QTest::qWait(100);
    QEvent earlyLeave(QEvent::Leave);
    QVERIFY(QCoreApplication::sendEvent(ui.taskbarView, &earlyLeave));
    QTest::qWait(850);
    QVERIFY(visibleToolTipLabel() == nullptr);

    QVERIFY(QCoreApplication::sendEvent(ui.taskbarView, &enter));
    QVERIFY(waitUntil([]() { return visibleToolTipLabel() != nullptr; }, 1500));

    QVERIFY(openContextMenu(*ui.taskbarView));
    QVERIFY(waitUntil([]() { return visibleToolTipLabel() == nullptr; }, 1000));
    QVERIFY(clickVisibleMenuAction(I18n::tr("Show main window")));
    QVERIFY(waitUntil([&showSpy]() { return showSpy.count() == 2; }));
    QCOMPARE(showSpy.count(), 2);
    QVERIFY(waitUntil([]() { return visibleMenu() == nullptr; }));

    QVERIFY(openContextMenu(*ui.taskbarView));
    QVERIFY(clickVisibleMenuAction(I18n::tr("Safe shutdown")));
    QVERIFY(waitUntil([&shutdownSpy]() { return shutdownSpy.count() == 1; }));
    QCOMPARE(shutdownSpy.count(), 1);

    QVERIFY(openContextMenu(*ui.taskbarView));
    QVERIFY(clickVisibleMenuAction(I18n::tr("Safe restart")));
    QVERIFY(waitUntil([&restartSpy]() { return restartSpy.count() == 1; }));
    QCOMPARE(restartSpy.count(), 1);

    QVERIFY(openContextMenu(*ui.taskbarView));
    QVERIFY(clickVisibleMenuAction(I18n::tr("Exit application")));
    QVERIFY(waitUntil([&exitSpy]() { return exitSpy.count() == 1; }));
    QCOMPARE(exitSpy.count(), 1);
    QVERIFY(waitUntil([]() { return visibleMenu() == nullptr; }));

    QVERIFY(openContextMenu(*ui.taskbarView));
    QVERIFY(clickVisibleMenuAction(I18n::tr("Hide overlay hub")));
    QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView); }));
  }

  void floatingWidgetRealInteractionsAndDataBoundaries() {
    ProductionUi ui;
    QSignalSpy showSpy(&ui.hub, &OverlayHub::showMainWindowRequested);
    QSignalSpy shutdownSpy(&ui.hub, &OverlayHub::safeShutdownRequested);
    QSignalSpy restartSpy(&ui.hub, &OverlayHub::safeRestartRequested);
    QSignalSpy exitSpy(&ui.hub, &OverlayHub::exitApplicationRequested);
    ui.registerFloatingOnly();
    QVERIFY(waitUntil([&ui]() { return ui.floatingConfirmed(); }));
    QWidget* const handle = ui.floatingView->findChild<QWidget*>(QStringLiteral("overlayFloatingHandle"));
    QLabel* const usage = ui.floatingView->findChild<QLabel*>(QStringLiteral("overlayFloatUsage"));
    QVERIFY(handle && usage && nativeVisible(*handle));
    QVERIFY(usage->text() != QStringLiteral("—"));

    const int initialWidth = ui.floatingView->width();
    const int initialRight = ui.floatingView->geometry().right();
    ui.hub.applyUsageState(
        OverlayUsageEnabled{core::OverlayRuntime{.availableSpaceMb = 49152, .currentConsumptionMb = 49151}, core::OverlayConfig{}});
    QVERIFY(waitUntil(
        [&ui, initialWidth, initialRight]() { return ui.floatingView->width() >= initialWidth && ui.floatingView->geometry().right() == initialRight; }));
    const core::OverlayRuntime maximum{.availableSpaceMb = std::numeric_limits<uint32_t>::max(), .currentConsumptionMb = std::numeric_limits<uint32_t>::max()};
    ui.hub.applyUsageState(OverlayUsageEnabled{maximum, core::OverlayConfig{}});
    QCOMPARE(usage->text(), overlayUsageText(maximum));
    QVERIFY(ui.floatingView->width() > QFontMetrics(usage->font()).horizontalAdvance(usage->text()));
    const core::OverlayRuntime empty{};
    ui.hub.applyUsageState(OverlayUsageEnabled{empty, core::OverlayConfig{}});
    QCOMPARE(usage->text(), overlayUsageText(empty));
    ui.hub.applyUsageState(OverlayUsageUnavailable{});
    QCOMPARE(usage->text(), QStringLiteral("—"));
    ui.hub.applyUsageState(OverlayUsageDisabled{});
    QCOMPARE(usage->text(), QStringLiteral("—"));
    ui.hub.applyUsageState(OverlayUsageEnabled{ui.runtime, core::OverlayConfig{}});
    QVERIFY(usage->text() != QStringLiteral("—"));
    QVERIFY(waitUntil([&ui]() { return ui.floatingConfirmed(); }));
    QVERIFY2(waitUntil([handle]() { return nativeVisible(*handle); }), "floating handle did not recover with its owner window");

    QTest::mouseDClick(handle, Qt::LeftButton, Qt::NoModifier, handle->rect().center());
    QVERIFY(waitUntil([&showSpy]() { return showSpy.count() == 1; }));
    QCOMPARE(showSpy.count(), 1);

    const QPoint originalPosition = ui.floatingView->pos();
    dragWidget(*handle, QPoint(-80, -40));
    QVERIFY(waitUntil([&ui, originalPosition]() { return ui.floatingView->pos() != originalPosition; }));
    const QPoint draggedPosition = ui.floatingView->pos();
    dragWidget(*handle, QPoint(-10000, -10000));
    QVERIFY(waitUntil([&ui, draggedPosition]() { return ui.floatingView->pos() != draggedPosition; }));
    const QPoint clampedPosition = ui.floatingView->pos();
    QScreen* const clampedScreen = QGuiApplication::screenAt(ui.floatingView->frameGeometry().center());
    QVERIFY(clampedScreen);
    QVERIFY(clampedScreen->availableGeometry().contains(ui.floatingView->frameGeometry()));

    bool clicked = false;
    scheduleFloatingMenuAction(I18n::tr("Restore default position"), clicked);
    QVERIFY(nativeMouseClick(*handle, Qt::RightButton));
    QVERIFY(waitUntil([&clicked]() { return clicked; }, 2000));
    QVERIFY(waitUntil([&ui, clampedPosition]() { return ui.floatingView->pos() != clampedPosition; }));

    clicked = false;
    scheduleFloatingMenuAction(I18n::tr("Show main window"), clicked);
    QVERIFY(nativeMouseClick(*handle, Qt::RightButton));
    QVERIFY(waitUntil([&clicked]() { return clicked; }, 2000));
    QCOMPARE(showSpy.count(), 2);

    clicked = false;
    scheduleFloatingMenuAction(I18n::tr("Safe shutdown"), clicked);
    QVERIFY(nativeMouseClick(*handle, Qt::RightButton));
    QVERIFY(waitUntil([&clicked]() { return clicked; }, 2000));
    QCOMPARE(shutdownSpy.count(), 1);

    clicked = false;
    scheduleFloatingMenuAction(I18n::tr("Safe restart"), clicked);
    QVERIFY(nativeMouseClick(*handle, Qt::RightButton));
    QVERIFY(waitUntil([&clicked]() { return clicked; }, 2000));
    QCOMPARE(restartSpy.count(), 1);

    clicked = false;
    scheduleFloatingMenuAction(I18n::tr("Exit application"), clicked);
    QVERIFY(nativeMouseClick(*handle, Qt::RightButton));
    QVERIFY(waitUntil([&clicked]() { return clicked; }, 2000));
    QCOMPARE(exitSpy.count(), 1);

    clicked = false;
    scheduleFloatingMenuAction(I18n::tr("Hide overlay hub"), clicked);
    QVERIFY(nativeMouseClick(*handle, Qt::RightButton));
    QVERIFY(waitUntil([&clicked]() { return clicked; }, 2000));
    QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.floatingView) && !nativeVisible(*ui.taskbarView); }));
  }

 private:
  HWND m_explorerTaskbar = nullptr;
};

}  // namespace uwf::ui

int main(int argc, char* argv[]) {
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication app(argc, argv);
  QTEST_SET_MAIN_SOURCE_PATH
  uwf::ui::WindowsTaskbarIntegrationTests tc;
  return QTest::qExec(&tc, argc, argv);
}

#include "WindowsTaskbarIntegrationTests.moc"
