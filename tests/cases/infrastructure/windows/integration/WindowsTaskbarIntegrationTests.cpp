// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <dwmapi.h>
#include <windows.h>

#include <QAction>
#include <QDebug>
#include <QElapsedTimer>
#include <QEnterEvent>
#include <QEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QLabel>
#include <QMenu>
#include <QScreen>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <QtMath>
#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <vector>
#include <utility>

#include "domain/uwf/model/UwfModel.h"
#include "foundation/logging/Log.h"
#include "presentation/appearance/I18n.h"
#include "presentation/views/overlay/coordination/OverlayHub.h"
#include "presentation/views/overlay/hud/OverlayFloatingWidget.h"
#include "presentation/views/overlay/hud/OverlayHudRenderer.h"
#include "presentation/views/overlay/hud/OverlayTaskbarWidget.h"
#include "presentation/views/overlay/taskbar/Win11TaskbarEnvironment.h"
#include "presentation/views/overlay/taskbar/Win11TaskbarLayoutStrategy.h"
#include "support/windows/Win11TaskbarLayoutStrategyTestAccess.h"

namespace uwf::ui {

class OverlayHubViewTestAccess {
 public:
  static QTimer* healthTimer(OverlayHubView& view) { return view.m_healthTimer; }
};

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

struct NativeTaskbarPlacement {
  bool readable = false;
  RECT actual{}, expected{}, taskbarClient{}, notification{};
  POINT taskbarOrigin{};
  UINT dpi = 0;
  int hostAwareness = -1;
  int windowAwareness = -1;
  DWORD error = ERROR_SUCCESS;

  [[nodiscard]] bool matches() const { return readable && EqualRect(&actual, &expected); }
  [[nodiscard]] bool sameHostGeometry(const NativeTaskbarPlacement& other) const {
    return readable && other.readable && dpi == other.dpi && hostAwareness == other.hostAwareness && EqualRect(&taskbarClient, &other.taskbarClient) &&
           taskbarOrigin.x == other.taskbarOrigin.x && taskbarOrigin.y == other.taskbarOrigin.y && EqualRect(&notification, &other.notification);
  }
  [[nodiscard]] QByteArray diagnostic() const {
    const auto rect = [](const RECT& value) { return QStringLiteral("[%1,%2,%3,%4]").arg(value.left).arg(value.top).arg(value.right).arg(value.bottom); };
    return QStringLiteral(
               "readable=%1 actual=%2 expected=%3 taskbarClient=%4 taskbarOrigin=[%5,%6] notification=%7 dpi=%8 hostAwareness=%9 "
               "windowAwareness=%10 win32Error=%11")
        .arg(readable)
        .arg(rect(actual), rect(expected), rect(taskbarClient))
        .arg(taskbarOrigin.x)
        .arg(taskbarOrigin.y)
        .arg(rect(notification))
        .arg(dpi)
        .arg(hostAwareness)
        .arg(windowAwareness)
        .arg(error)
        .toUtf8();
  }
};

// Independent placement oracle: maximize logical height within native space,
// then round ideal tray/center positions to logical pixels.
std::optional<RECT> expectedTaskbarClientPlacement(const QSize& logicalSize, const QSize& clientSize, const int notificationX, const UINT dpi) {
  if (dpi == 0 || logicalSize.width() <= 0 || logicalSize.height() <= 0) return {};
  const qreal scale = static_cast<qreal>(dpi) / USER_DEFAULT_SCREEN_DPI;
  const auto native = [scale](const int logical) { return qRound(logical * scale); };
  constexpr int insetLogicalPixels = 3;
  const int inset = native(insetLogicalPixels);
  const int availableHeight = clientSize.height() - 2 * inset;
  int height = 0;
  for (int logicalHeight = 1; logicalHeight <= logicalSize.height(); ++logicalHeight) {
    const int candidate = native(logicalHeight);
    if (candidate > availableHeight) break;
    height = candidate;
  }
  const int width = native(logicalSize.width());
  const qreal idealX = notificationX - width - inset;
  const qreal idealY = (clientSize.height() - height) / 2.0;
  const int x = native(qRound(idealX / scale));
  const int y = native(qRound(idealY / scale));
  if (width <= 0 || height <= 0 || x < inset || y < 0 || x + width > notificationX || x + width > clientSize.width() || y + height > clientSize.height())
    return {};
  return RECT{x, y, x + width, y + height};
}

// Read live native geometry under the host's coordinate/DPI context. The oracle
// operates in parent client coordinates before adding the screen origin.
NativeTaskbarPlacement observeTaskbarPlacement(const HWND window, const HWND taskbar, const QSize& logicalSize) {
  NativeTaskbarPlacement result;
  const win11_taskbar::ScopedThreadDpiAwareness dpiScope(taskbar);
  const HWND notification = FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr);
  result.dpi = GetDpiForWindow(taskbar);
  result.hostAwareness = static_cast<int>(GetAwarenessFromDpiAwarenessContext(GetWindowDpiAwarenessContext(taskbar)));
  result.windowAwareness = static_cast<int>(GetAwarenessFromDpiAwarenessContext(GetWindowDpiAwarenessContext(window)));
  if (!window || !IsWindow(window) || !taskbar || !IsWindow(taskbar) || !notification || GetParent(window) != taskbar || result.dpi == 0 ||
      !GetWindowRect(window, &result.actual) || !GetClientRect(taskbar, &result.taskbarClient) || !ClientToScreen(taskbar, &result.taskbarOrigin) ||
      !GetWindowRect(notification, &result.notification)) {
    result.error = GetLastError();
    return result;
  }
  const int clientWidth = static_cast<int>(result.taskbarClient.right - result.taskbarClient.left);
  const int clientHeight = static_cast<int>(result.taskbarClient.bottom - result.taskbarClient.top);
  const int notificationX = static_cast<int>(result.notification.left - result.taskbarOrigin.x);
  const auto expected = expectedTaskbarClientPlacement(logicalSize, QSize(clientWidth, clientHeight), notificationX, result.dpi);
  if (!expected) return result;
  result.expected = {result.taskbarOrigin.x + expected->left, result.taskbarOrigin.y + expected->top, result.taskbarOrigin.x + expected->right,
                     result.taskbarOrigin.y + expected->bottom};
  result.readable = true;
  return result;
}

QByteArray qtPlacementDiagnostic(const QWidget& widget) {
  const auto rect = [](const QRect& value) { return QStringLiteral("[%1,%2,%3,%4]").arg(value.x()).arg(value.y()).arg(value.width()).arg(value.height()); };
  const QWindow* const window = widget.windowHandle();
  RECT native{};
  (void)GetWindowRect(nativeWindow(widget), &native);
  return QStringLiteral("widgetGeometry=%1 windowGeometry=%2 parentGeometry=%3 dpr=%4 callerNative=[%5,%6,%7,%8] callerAwareness=%9")
      .arg(rect(widget.geometry()), window ? rect(window->geometry()) : QStringLiteral("none"),
           window && window->parent() ? rect(window->parent()->geometry()) : QStringLiteral("none"))
      .arg(widget.devicePixelRatioF())
      .arg(native.left)
      .arg(native.top)
      .arg(native.right)
      .arg(native.bottom)
      .arg(static_cast<int>(GetAwarenessFromDpiAwarenessContext(GetThreadDpiAwarenessContext())))
      .toUtf8();
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

bool postWindowRightClick(QWidget& widget) {
  HWND window = nullptr;
  std::optional<POINT> global;
  // Qt geometry may precede native hit-target updates; wait for input readiness.
  if (!waitUntil([&] {
        if (!nativeVisible(widget)) return false;
        window = nativeWindow(widget);
        global = nativeClientPoint(widget);
        if (!global) return false;
        const HWND hit = WindowFromPoint(*global);
        return hit == window || IsChild(window, hit);
      })) {
    qWarning() << "Native right-click target is not hit-testable:" << &widget;
    return false;
  }
  POINT client = *global;
  if (!ScreenToClient(window, &client)) return false;
  const LPARAM position = MAKELPARAM(client.x, client.y);
  // Post native input so a modal QMenu can dispatch the handoff observer.
  return PostMessageW(window, WM_MOUSEMOVE, 0, position) && PostMessageW(window, WM_RBUTTONDOWN, MK_RBUTTON, position) &&
         PostMessageW(window, WM_RBUTTONUP, 0, position);
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
  // Rollback must restore the original child/popup styles.
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

  void enable() { hub.applyUsageState(app::OverlayUsageEnabled{runtime, core::OverlayConfig{}}); }

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

bool selectFloatingMenuAction(QWidget& handle, const QString& text) {
  QElapsedTimer elapsed;
  elapsed.start();
  QTimer poll;
  bool clicked = false;
  bool expired = false;
  QObject::connect(&poll, &QTimer::timeout, &poll, [&] {
    expired = elapsed.elapsed() >= 2000;
    QMenu* const menu = visibleMenu();
    if (!menu) return;
    if (expired) {
      menu->close();
      return;
    }
    if (clicked) return;
    QAction* const action = menuAction(*menu, text);
    // Do not pump events here: receiving the right click can enter QMenu::exec,
    // which must still be able to dispatch the next polling tick.
    if (action) clicked = dispatchWindowLeftClick(*menu, menu->actionGeometry(action).center());
  });
  poll.start(10);
  if (!postWindowRightClick(handle)) return false;
  return waitUntil([&] { return expired || (clicked && !visibleMenu()); }, 2200) && clicked && !expired;
}

QString visibleToolTipText() {
  QStringList content;
  const QWidgetList topLevels = QApplication::topLevelWidgets();
  for (QWidget* widget : topLevels) {
    if (!widget->isVisible() || widget->windowType() != Qt::ToolTip) continue;
    if (const auto* label = qobject_cast<QLabel*>(widget); label && !label->text().isEmpty()) content.append(label->text());
    for (auto labels = widget->findChildren<QLabel*>(); const auto* label : std::as_const(labels))
      if (label->isVisible() && !label->text().isEmpty()) content.append(label->text());
  }
  return content.join(u'\n');
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
  // Shell disturbance is sufficient; Start-menu visibility is not a prerequisite.
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
    if (QGuiApplication::platformName() != QLatin1String("windows") || qEnvironmentVariable("UWF_TEST_ALLOW_DESKTOP") != QLatin1String("1"))
      QSKIP("Native taskbar integration requires the Windows platform and explicit UWF_TEST_ALLOW_DESKTOP=1 opt-in");
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
    // Recovery may exhaust its exclusive retries and yield to the floating view.
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
    // Inject a partial parent commit to exercise rollback independently of Shell timing.
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
#if defined(UWF_DEBUG_LOGGING)
    QVERIFY2(diagnosticLog().contains("qt-parent-mismatch"), diagnosticLog().constData());
#endif
    QVERIFY(!Win11TaskbarLayoutStrategyTestAccess::hasLiveAttachment(strategy));

    if (result == TaskbarLayoutStrategy::AttachResult::TemporarilyUnavailable) {
      QVERIFY2(IsWindow(hwnd), "soft atomic rollback must keep the QWindow/HWND for retry");
      QVERIFY2(!GetPropW(hwnd, kAttachmentCookieProperty), "cookie must be removed on atomic rollback");
      QVERIFY2(GetParent(hwnd) == originalParent, "HWND parent must restore to the pre-commit parent");
      QVERIFY2(stylesMatch(hwnd, originalStyle, originalExStyle), partialInjectionDiagnostic(host, m_explorerTaskbar).constData());
      QVERIFY2(!hasPartialTaskbarInjection(hwnd, m_explorerTaskbar), partialInjectionDiagnostic(host, m_explorerTaskbar).constData());
#if defined(UWF_DEBUG_LOGGING)
      QVERIFY2(diagnosticLog().contains("reason=qt-parent-mismatch action=rollback-and-retry"), diagnosticLog().constData());
#endif
    } else {
      QVERIFY2(!IsWindow(hwnd) || !GetPropW(hwnd, kAttachmentCookieProperty), "hard abort must not leave a live cookie");
      QVERIFY2(attachmentWindows(m_explorerTaskbar).empty(), "hard abort must not leave explorer attachments");
#if defined(UWF_DEBUG_LOGGING)
      QVERIFY2(diagnosticLog().contains("reason=qt-parent-mismatch action=destroy-and-recreate"), diagnosticLog().constData());
#endif
    }
  }

  void parentCommitRollbackDoesNotLeavePartialInjection() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY2(waitUntil([&ui]() { return ui.taskbarConfirmed(); }), diagnosticLog().constData());

    // Rollback must restore the cookie, parent and styles before another acquisition.
    // An exclusive recovery may retain one attachment while it is in progress.
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

    ui.hub.applyUsageState(app::OverlayUsageUnavailable{});
    QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView); }));
    ui.hub.applyUsageState(app::OverlayUsageEnabled{ui.runtime, core::OverlayConfig{}});
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));

    ui.hub.applyUsageState(app::OverlayUsageDisabled{});
    QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView); }));
    ui.hub.applyUsageState(app::OverlayUsageEnabled{ui.runtime, core::OverlayConfig{}});
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

  void placementGeometryUsesRepresentableQtPixels_data() {
    QTest::addColumn<quint32>("dpi");
    for (const quint32 dpi :
         {quint32{0}, quint32{96}, quint32{120}, quint32{144}, quint32{168}, quint32{192}, quint32{216}, quint32{240}, quint32{288}, quint32{384}})
      QTest::newRow(qPrintable(QString::number(dpi))) << dpi;
  }
  void placementGeometryUsesRepresentableQtPixels() {
    QFETCH(quint32, dpi);
    const win11_taskbar::ScopedThreadDpiAwareness dpiScope(m_explorerTaskbar);
    POINT hostOrigin{};
    QVERIFY(ClientToScreen(m_explorerTaskbar, &hostOrigin));
    const qreal scale = static_cast<qreal>(dpi) / USER_DEFAULT_SCREEN_DPI;
    constexpr int clientWidth = 1000;
    const int inset = qRound(3 * scale);
    for (const int logicalWidth : {0, 1, 31, 96, 153, 193, 257})
      for (const int logicalHeight : {0, 1, 3, 16, 32, 57})
        for (const int availableHeight : {-1, 0, 1, 2, 3, 4, 5, 7, 15, 31, 32, 33, 47, 63}) {
          const QSize logicalSize(logicalWidth, logicalHeight);
          const int clientHeight = availableHeight + 2 * inset;
          const int nativeWidth = qRound(logicalWidth * scale);
          const std::array notificationPositions{nativeWidth + 2 * inset - 1, nativeWidth + 2 * inset, nativeWidth + 2 * inset + 1, 500, 777};
          for (const int notificationX : notificationPositions) {
            win11_taskbar::Environment environment;
            environment.taskbar = m_explorerTaskbar;
            environment.taskbarClientRect = {0, 0, clientWidth, clientHeight};
            environment.notifyRect = {hostOrigin.x + notificationX, hostOrigin.y, hostOrigin.x + notificationX + 1, hostOrigin.y + 1};
            environment.dpi = dpi;
            const auto expected = expectedTaskbarClientPlacement(logicalSize, QSize(clientWidth, clientHeight), notificationX, dpi);
            const auto actual = win11_taskbar::calculatePlacement(environment, logicalSize);
            const auto description = QStringLiteral("dpi=%1 logicalSize=%2x%3 availableHeight=%4 notificationX=%5")
                                         .arg(dpi)
                                         .arg(logicalWidth)
                                         .arg(logicalHeight)
                                         .arg(availableHeight)
                                         .arg(notificationX);
            QVERIFY2(actual.has_value() == expected.has_value(), qUtf8Printable(description));
            if (!actual) continue;
            const RECT actualRect{actual->x, actual->y, actual->x + actual->width, actual->y + actual->height};
            QVERIFY2(EqualRect(&actualRect, &*expected), qUtf8Printable(description));
            const auto roundtrip = [scale](const int native) { return qRound(qRound(native / scale) * scale); };
            for (const int value : {actual->x, actual->y, actual->width, actual->height}) QVERIFY2(roundtrip(value) == value, qUtf8Printable(description));
            QVERIFY2(actual->x >= inset && actual->y >= 0 && actual->x + actual->width <= notificationX && actual->x + actual->width <= clientWidth &&
                         actual->y + actual->height <= clientHeight && actual->height <= availableHeight,
                     qUtf8Printable(description));
            const int chosenLogicalHeight = qRound(actual->height / scale);
            QVERIFY2(chosenLogicalHeight <= logicalHeight, qUtf8Printable(description));
            if (chosenLogicalHeight < logicalHeight) QVERIFY2(qRound((chosenLogicalHeight + 1) * scale) > availableHeight, qUtf8Printable(description));
            // One nearest-logical rounding and one nearest-native rounding.
            const qreal quantizationBound = scale / 2 + 0.5;
            QVERIFY2(qAbs(actual->x - (notificationX - actual->width - inset)) <= quantizationBound, qUtf8Printable(description));
            QVERIFY2(qAbs(actual->y - (clientHeight - actual->height) / 2.0) <= quantizationBound, qUtf8Printable(description));
          }
        }
  }
  void placementGeometryRejectsUnrepresentableInputs_data() {
    QTest::addColumn<int>("left");
    QTest::addColumn<int>("right");
    QTest::addColumn<int>("notificationX");
    QTest::addColumn<int>("logicalWidth");
    QTest::addColumn<quint32>("dpi");
    constexpr int minimum = std::numeric_limits<int>::min();
    constexpr int maximum = std::numeric_limits<int>::max();
    QTest::newRow("negative-position-underflow") << 0 << 1920 << minimum << 400 << quint32{96};
    QTest::newRow("client-span-overflow") << minimum << maximum << 1800 << 400 << quint32{96};
    QTest::newRow("scaled-width-overflow") << 0 << maximum << maximum << maximum << quint32{192};
  }
  void placementGeometryRejectsUnrepresentableInputs() {
    QFETCH(int, left);
    QFETCH(int, right);
    QFETCH(int, notificationX);
    QFETCH(int, logicalWidth);
    QFETCH(quint32, dpi);
    const win11_taskbar::ScopedThreadDpiAwareness dpiScope(m_explorerTaskbar);
    win11_taskbar::Environment environment;
    environment.taskbar = m_explorerTaskbar;
    environment.taskbarClientRect = {left, 0, right, 48};
    environment.notifyRect.left = notificationX;
    environment.dpi = dpi;
    QVERIFY(!win11_taskbar::calculatePlacement(environment, QSize(logicalWidth, 28)));
  }
  void placementDriftIsRepairedInPlace() {
    ProductionUi ui;
    ui.registerTaskbarFirst();
    QVERIFY(waitUntil([&ui]() { return ui.taskbarConfirmed(); }));
    const HWND window = nativeWindow(*ui.taskbarView);
    const QSize logicalSize = ui.taskbarView->size();
    NativeTaskbarPlacement initial;
    QVERIFY2(waitUntil([&] {
               initial = observeTaskbarPlacement(window, m_explorerTaskbar, logicalSize);
               return ui.taskbarConfirmed() && initial.matches();
             }),
             (initial.diagnostic() + '\n' + diagnosticLog()).constData());
    ui.visibility->reset();
    constexpr int horizontalDriftPixels = 120;
    {
      const win11_taskbar::ScopedThreadDpiAwareness dpiScope(m_explorerTaskbar);
      POINT displaced{initial.actual.left - horizontalDriftPixels, initial.actual.top};
      QVERIFY(ScreenToClient(m_explorerTaskbar, &displaced));
      QVERIFY(SetWindowPos(window, nullptr, displaced.x, displaced.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER));
    }
    const auto displaced = observeTaskbarPlacement(window, m_explorerTaskbar, logicalSize);
    QVERIFY2(displaced.readable && !displaced.matches(), displaced.diagnostic().constData());
    QCOMPARE(displaced.actual.left, initial.actual.left - horizontalDriftPixels);
    QCOMPARE(displaced.actual.top, initial.actual.top);
    QCOMPARE(displaced.actual.right - displaced.actual.left, initial.actual.right - initial.actual.left);
    QCOMPARE(displaced.actual.bottom - displaced.actual.top, initial.actual.bottom - initial.actual.top);
    NativeTaskbarPlacement repaired;
    QVERIFY2(waitUntil([&] {
               repaired = observeTaskbarPlacement(nativeWindow(*ui.taskbarView), m_explorerTaskbar, logicalSize);
               return nativeWindow(*ui.taskbarView) == window && ui.taskbarConfirmed() && repaired.matches();
             }),
             (QByteArray("initial: ") + initial.diagnostic() + "\ndisplaced: " + displaced.diagnostic() + "\nrepaired: " + repaired.diagnostic() + '\n' +
              diagnosticLog())
                 .constData());
    QCOMPARE(nativeWindow(*ui.taskbarView), window);
    QTimer* const healthTimer = OverlayHubViewTestAccess::healthTimer(*ui.taskbarView);
    QVERIFY(healthTimer && healthTimer->isActive());
    QSignalSpy healthChecks(healthTimer, &QTimer::timeout);
    QSignalSpy stateChanges(ui.taskbarView, &OverlayHubView::displayStateChanged);
    bool hostStayedUnchanged = true;
    bool geometryStayedCorrect = true;
    bool confirmationStayedCorrect = true;
    bool sameWindow = true;
    QByteArray firstBad;
    NativeTaskbarPlacement afterHealth;
    const bool healthConverged = waitUntil([&] {
      const HWND current = nativeWindow(*ui.taskbarView);
      sameWindow = sameWindow && current == window;
      afterHealth = observeTaskbarPlacement(current, m_explorerTaskbar, logicalSize);
      if (afterHealth.readable && !repaired.sameHostGeometry(afterHealth)) hostStayedUnchanged = false;
      const bool confirmed = ui.taskbarConfirmed();
      geometryStayedCorrect = geometryStayedCorrect && afterHealth.matches();
      confirmationStayedCorrect = confirmationStayedCorrect && confirmed;
      if (firstBad.isEmpty() && (!confirmed || !afterHealth.matches())) {
        firstBad = QByteArray("confirmed=") + (confirmed ? "true " : "false ") + afterHealth.diagnostic() + " " + qtPlacementDiagnostic(*ui.taskbarView);
      }
      return !healthChecks.isEmpty() && sameWindow && confirmed && afterHealth.matches();
    });
    qInfo().noquote() << "Placement health witnesses: checks=" << healthChecks.size() << "stateChanges=" << stateChanges.size()
                      << "hostStayedUnchanged=" << hostStayedUnchanged << "geometryStayedCorrect=" << geometryStayedCorrect
                      << "confirmationStayedCorrect=" << confirmationStayedCorrect << "\nfirstBad:" << firstBad;
    QVERIFY2(healthConverged,
             (QByteArray("repair: ") + repaired.diagnostic() + "\nafter health: " + afterHealth.diagnostic() + '\n' + firstBad + '\n' + diagnosticLog())
                 .constData());
    // A stable host needs no further repair; an Explorer reflow must converge without HWND recreation or fallback.
    if (hostStayedUnchanged) {
      QVERIFY2(geometryStayedCorrect, firstBad.constData());
      QVERIFY2(confirmationStayedCorrect, firstBad.constData());
      QCOMPARE(stateChanges.size(), qsizetype{0});
    }
    qInfo().noquote() << "Placement repair witnesses:\ninitial:" << initial.diagnostic() << "\ndisplaced:" << displaced.diagnostic()
                      << "\nrepaired:" << repaired.diagnostic() << "\nafter health:" << afterHealth.diagnostic();
    QVERIFY(!ui.visibility->floatingSeen());
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
    // Simulate restart with TaskbarCreated on the owned HWND; leave Explorer running.
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

    // A restart signal during a toolbar toggle must preserve exclusive taskbar recovery.
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
      // Use an owned HWND so this process's Qt nativeEventFilter receives TaskbarCreated.
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
      ui.hub.applyUsageState(app::OverlayUsageEnabled{runtime, core::OverlayConfig{}});
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
    bool menuOpened = false;
    bool timedOut = false;
    QTimer deadline;
    deadline.setSingleShot(true);
    connect(&deadline, &QTimer::timeout, &deadline, [&] {
      timedOut = true;
      if (auto* menu = visibleMenu()) menu->close();
    });
    deadline.start(kUiTimeoutMs);
    QTimer handoff;
    connect(&handoff, &QTimer::timeout, &handoff, [&] {
      if (!visibleMenu()) return;
      menuOpened = true;
      handoff.stop();
      ui.registerPreparedTaskbar();
    });
    handoff.start(10);
    QVERIFY(postWindowRightClick(*handle));
    QVERIFY2(waitUntil([&]() { return menuOpened && ui.taskbarConfirmed() && visibleMenu() == nullptr; }), diagnosticLog().constData());
    QVERIFY2(!timedOut, diagnosticLog().constData());
    QVERIFY(!nativeVisible(*ui.floatingView));
    QVERIFY(!ui.visibility->overlapObserved());
  }

  void taskbarMouseTooltipAndContextMenuActions() {
    ProductionUi ui;
    ui.hub.setOperationAvailability(true, true);
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
    QVERIFY(visibleToolTipText().isEmpty());

    QVERIFY(QCoreApplication::sendEvent(ui.taskbarView, &enter));
    QVERIFY(waitUntil([]() { return !visibleToolTipText().isEmpty(); }, 1500));
    QCOMPARE(visibleToolTipText(), QStringLiteral("UWF Manager\n20% (1,024/5,120 MB)"));

    QVERIFY(openContextMenu(*ui.taskbarView));
    QVERIFY(waitUntil([]() { return visibleToolTipText().isEmpty(); }, 1000));
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
    QVERIFY(clickVisibleMenuAction(I18n::tr("Hide overlay widget")));
    QVERIFY(waitUntil([&ui]() { return !nativeVisible(*ui.taskbarView) && !nativeVisible(*ui.floatingView); }));
  }

  void floatingWidgetRealInteractionsAndDataBoundaries() {
    ProductionUi ui;
    ui.hub.setOperationAvailability(true, true);
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
    ui.hub.applyUsageState(app::OverlayUsageEnabled{core::OverlayRuntime{.availableSpaceMb = 49152, .currentConsumptionMb = 49151}, core::OverlayConfig{}});
    QVERIFY(waitUntil(
        [&ui, initialWidth, initialRight]() { return ui.floatingView->width() >= initialWidth && ui.floatingView->geometry().right() == initialRight; }));
    const core::OverlayRuntime maximum{.availableSpaceMb = std::numeric_limits<uint32_t>::max(), .currentConsumptionMb = std::numeric_limits<uint32_t>::max()};
    ui.hub.applyUsageState(app::OverlayUsageEnabled{maximum, core::OverlayConfig{}});
    QCOMPARE(usage->text(), overlayUsageText(maximum));
    QVERIFY(ui.floatingView->width() > QFontMetrics(usage->font()).horizontalAdvance(usage->text()));
    const core::OverlayRuntime empty{};
    ui.hub.applyUsageState(app::OverlayUsageEnabled{empty, core::OverlayConfig{}});
    QCOMPARE(usage->text(), overlayUsageText(empty));
    ui.hub.applyUsageState(app::OverlayUsageUnavailable{});
    QCOMPARE(usage->text(), QStringLiteral("—"));
    ui.hub.applyUsageState(app::OverlayUsageDisabled{});
    QCOMPARE(usage->text(), QStringLiteral("—"));
    ui.hub.applyUsageState(app::OverlayUsageEnabled{ui.runtime, core::OverlayConfig{}});
    QVERIFY(usage->text() != QStringLiteral("—"));
    QVERIFY(waitUntil([&ui]() { return ui.floatingConfirmed(); }));
    QVERIFY2(waitUntil([handle]() { return nativeVisible(*handle); }), "floating handle did not recover with its owner window");

    QTest::mouseDClick(handle, Qt::LeftButton, Qt::NoModifier, handle->rect().center());
    QTest::mouseRelease(handle, Qt::LeftButton, Qt::NoModifier, handle->rect().center());
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

    QVERIFY(selectFloatingMenuAction(*handle, I18n::tr("Restore default position")));
    QVERIFY(waitUntil([&ui, clampedPosition]() { return ui.floatingView->pos() != clampedPosition; }));

    QVERIFY(selectFloatingMenuAction(*handle, I18n::tr("Show main window")));
    QCOMPARE(showSpy.count(), 2);

    QVERIFY(selectFloatingMenuAction(*handle, I18n::tr("Safe shutdown")));
    QCOMPARE(shutdownSpy.count(), 1);

    QVERIFY(selectFloatingMenuAction(*handle, I18n::tr("Safe restart")));
    QCOMPARE(restartSpy.count(), 1);

    QVERIFY(selectFloatingMenuAction(*handle, I18n::tr("Exit application")));
    QCOMPARE(exitSpy.count(), 1);

    QVERIFY(selectFloatingMenuAction(*handle, I18n::tr("Hide overlay widget")));
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
