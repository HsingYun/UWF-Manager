// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
// clang-format off
#include <Windows.h>
// clang-format on
#include "WidgetInput.h"

#include <QtTest/qtestspontaneevent.h>

#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QEnterEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPointer>
#include <QScopeGuard>
#include <QScreen>
#include <QStringList>
#include <QStyleHints>
#include <QTest>
#include <QWheelEvent>
#include <QWidget>
#include <QWindow>
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

#include "PointerPressHistory.h"

namespace uwf::test {
namespace {
ULONG_PTR nextInputTag() {
  static ULONG_PTR tag = 0x55574654;
  return ++tag;
}
struct ButtonInput {
  Qt::MouseButton button;
  WORD key;
  DWORD down, up, data;
};
constexpr std::array buttonInputs{ButtonInput{Qt::LeftButton, VK_LBUTTON, MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, 0},
                                  ButtonInput{Qt::RightButton, VK_RBUTTON, MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP, 0},
                                  ButtonInput{Qt::MiddleButton, VK_MBUTTON, MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP, 0},
                                  ButtonInput{Qt::BackButton, VK_XBUTTON1, MOUSEEVENTF_XDOWN, MOUSEEVENTF_XUP, XBUTTON1},
                                  ButtonInput{Qt::ForwardButton, VK_XBUTTON2, MOUSEEVENTF_XDOWN, MOUSEEVENTF_XUP, XBUTTON2}};
struct ModifierInput {
  Qt::KeyboardModifier modifier;
  WORD key;
};
constexpr std::array modifierInputs{ModifierInput{Qt::ShiftModifier, VK_SHIFT}, ModifierInput{Qt::ControlModifier, VK_CONTROL},
                                    ModifierInput{Qt::AltModifier, VK_MENU}, ModifierInput{Qt::MetaModifier, VK_LWIN}};
bool check(const bool condition, const char* message) {
  if (!condition) QTest::qFail(message, __FILE__, __LINE__);
  return condition;
}
bool nativePointerInputAllowed() {
  if (QTest::currentTestFailed()) return false;
  if (!check(QGuiApplication::platformName() == QLatin1String("windows"),
             "Native pointer gestures require the Windows platform; offscreen input is not supported"))
    return false;
  return check(qEnvironmentVariable("UWF_TEST_ALLOW_DESKTOP") == QStringLiteral("1"),
               "Native pointer gestures are disabled; explicit desktop tests require UWF_TEST_ALLOW_DESKTOP=1");
}
bool submit(INPUT input) {
  const auto count = SendInput(1, &input, sizeof(INPUT));
  if (count == 1) return true;
  const auto error = GetLastError();
  return check(false, qUtf8Printable(QStringLiteral("SendInput failed: count=%1 error=%2").arg(count).arg(error)));
}
Qt::MouseButtons nativeButtons() {
  Qt::MouseButtons result;
  for (const auto& button : buttonInputs)
    if (GetAsyncKeyState(button.key) & 0x8000) result |= button.button;
  return result;
}
INPUT buttonInput(const ButtonInput& button, const bool down, const ULONG_PTR tag = 0) {
  INPUT input{};
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = down ? button.down : button.up;
  input.mi.mouseData = button.data;
  input.mi.dwExtraInfo = tag;
  return input;
}
INPUT keyInput(const WORD key, const bool down) {
  INPUT input{};
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = key;
  input.ki.dwFlags = (down ? 0 : KEYEVENTF_KEYUP) | (key == VK_LWIN ? KEYEVENTF_EXTENDEDKEY : 0);
  input.ki.dwExtraInfo = 0;
  return input;
}
struct PointerGeometry {
  QPointer<QWindow> window;
  WId nativeId;
  QRect geometry;
  QPointer<QScreen> screen;
  qreal devicePixelRatio;
  QPoint local, global, nativeGlobal;
  QRect nativeClient;
  QPoint nativeOrigin;
  bool operator==(const PointerGeometry&) const = default;
};
struct HeldInput {
  Qt::MouseButtons buttons;
  Qt::KeyboardModifiers modifiers;
  QMetaObject::Connection lifetime;
  std::optional<PointerGeometry> pointer;
};
HeldInput& heldInput() {
  static HeldInput held;
  return held;
}
void releaseModifiers() {
  auto& held = heldInput();
  for (const auto& modifier : modifierInputs)
    if (held.modifiers.testFlag(modifier.modifier)) (void)submit(keyInput(modifier.key, false));
  held.modifiers = Qt::NoModifier;
}
void releaseHeldInput() {
  auto& held = heldInput();
  for (const auto& button : buttonInputs)
    if (held.buttons.testFlag(button.button)) (void)submit(buttonInput(button, false));
  held.buttons = Qt::NoButton;
  held.pointer.reset();
  releaseModifiers();
  QObject::disconnect(held.lifetime);
  held.lifetime = {};
}
bool pressModifiers(const Qt::KeyboardModifiers requested) {
  constexpr auto supported = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
  if (!check((requested & ~supported) == Qt::NoModifier, "The native pointer gesture has unsupported modifiers")) return false;
  for (const auto& modifier : modifierInputs)
    if (requested.testFlag(modifier.modifier) && !(GetAsyncKeyState(modifier.key) & 0x8000)) {
      if (!submit(keyInput(modifier.key, true))) return false;
      heldInput().modifiers |= modifier.modifier;
    }
  return true;
}
class PointerTarget {
 public:
  PointerTarget(QWindow* window, const QPoint position) : m_window(window), m_position(position) {}
  PointerTarget(QWidget* widget, const QPoint position) : m_widget(widget), m_position(position), m_isWidget(true) {}
  QWindow* window() const { return m_isWidget ? m_widget ? m_widget->window()->windowHandle() : nullptr : m_window.data(); }
  bool alive() const { return m_isWidget ? m_widget && m_widget->isVisible() : m_window && m_window->isVisible(); }
  std::optional<PointerGeometry> capture() const {
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    auto* current = window();
    if (!alive() || !current || !current->isVisible()) return {};
    const auto local = m_isWidget            ? m_widget->mapTo(m_widget->window(), m_position.isNull() ? m_widget->rect().center() : m_position)
                       : m_position.isNull() ? QPoint(current->width() / 2, current->height() / 2)
                                             : m_position;
    const auto id = current->winId();
    RECT client{};
    POINT origin{};
    if (!check(GetClientRect(reinterpret_cast<HWND>(id), &client) && ClientToScreen(reinterpret_cast<HWND>(id), &origin),
               "The visible pointer target has no native client geometry"))
      return {};
    const auto dpr = current->devicePixelRatio();
    // ClientToScreen uses the Qt process's per-monitor-aware native coordinates.
    // The native client size is part of the barrier, including resize/DPI changes.
    return PointerGeometry{current,
                           id,
                           current->geometry(),
                           current->screen(),
                           dpr,
                           local,
                           current->mapToGlobal(local),
                           QPoint(origin.x + qRound(local.x() * dpr), origin.y + qRound(local.y() * dpr)),
                           QRect(client.left, client.top, client.right - client.left, client.bottom - client.top),
                           QPoint(origin.x, origin.y)};
  }

 private:
  QPointer<QWidget> m_widget;
  QPointer<QWindow> m_window;
  QPoint m_position;
  bool m_isWidget = false;
};
enum class PointerAction { Move, Press, Release };
// Temporary read-only receipts. No Enter/Leave or Qt button state is fabricated.
class PointerReceipt final : public QObject, public QAbstractNativeEventFilter {
 public:
  PointerReceipt() {
    qApp->installNativeEventFilter(this);
    qApp->installEventFilter(this);
  }
  ~PointerReceipt() override {
    qApp->removeEventFilter(this);
    qApp->removeNativeEventFilter(this);
  }
  ULONG_PTR tag() const { return m_tag; }
  void clear() {
    m_tag = nextInputTag();
    m_moves = m_ncMoves = m_downs = m_ups = 0;
    m_qt.clear();
    m_nativeMoves.clear();
  }
  bool received(const PointerAction action, const PointerGeometry& geometry, const Qt::MouseButtons buttons, const Qt::MouseButton button = Qt::NoButton,
                const std::optional<Qt::KeyboardModifiers> modifiers = std::nullopt) const {
    const int tolerance = std::abs(geometry.devicePixelRatio - std::round(geometry.devicePixelRatio)) > .001 ? 1 : 0;
    for (const auto& event : m_qt) {
      const bool matchesType = action == PointerAction::Move    ? event.type == QEvent::MouseMove || event.type == QEvent::Enter || event.type == QEvent::Leave
                               : action == PointerAction::Press ? event.type == QEvent::MouseButtonPress
                                                                : event.type == QEvent::MouseButtonRelease;
      if (matchesType && event.buttons == buttons && event.button == button && (!modifiers || event.modifiers == *modifiers) &&
          std::abs(event.global.x() - geometry.global.x()) <= tolerance && std::abs(event.global.y() - geometry.global.y()) <= tolerance &&
          (event.nativeId == geometry.nativeId ||
           ((event.popup || event.captured) && std::ranges::find(event.owners, geometry.nativeId) != event.owners.end())))
        return true;
    }
    return false;
  }
  bool nativeMoved(const PointerGeometry& geometry, const bool nonClient) const {
    return std::ranges::any_of(m_nativeMoves, [&](const auto& event) {
      return event.nonClient == nonClient && event.window == geometry.nativeId && std::abs(event.position.x() - geometry.nativeGlobal.x()) <= 1 &&
             std::abs(event.position.y() - geometry.nativeGlobal.y()) <= 1;
    });
  }
  QString diagnostic(const PointerGeometry* expected = nullptr) const {
    int moves = 0, boundaries = 0, downs = 0, doubles = 0, ups = 0;
    for (const auto& event : m_qt) {
      if (event.type == QEvent::MouseMove)
        ++moves;
      else if (event.type == QEvent::MouseButtonRelease)
        ++ups;
      else if (event.type == QEvent::MouseButtonPress)
        ++downs;
      else if (event.type == QEvent::MouseButtonDblClick)
        ++doubles;
      else
        ++boundaries;
    }
    QString result = QStringLiteral("native(move/ncmove/down/up)=%1/%2/%3/%4 Qt(move/boundary/down/double/up)=%5/%6/%7/%8/%9 observedNativeMoves=%10")
                         .arg(m_moves)
                         .arg(m_ncMoves)
                         .arg(m_downs)
                         .arg(m_ups)
                         .arg(moves)
                         .arg(boundaries)
                         .arg(downs)
                         .arg(doubles)
                         .arg(ups)
                         .arg(m_nativeMoves.size());
    if (expected) {
      auto* hit = QApplication::widgetAt(expected->global);
      result += QStringLiteral(" expected(global=%1,%2 win=%3 local=%4,%5 hit=%6#%7)")
                    .arg(expected->global.x())
                    .arg(expected->global.y())
                    .arg(expected->nativeId)
                    .arg(expected->local.x())
                    .arg(expected->local.y())
                    .arg(hit ? QString::fromLatin1(hit->metaObject()->className()) : QStringLiteral("none"), hit ? hit->objectName() : QString{});
    }
    for (const auto& event : m_qt) {
      QStringList owners;
      for (const auto owner : event.owners) owners.append(QString::number(owner));
      result += QStringLiteral(" Qt(type=%1 global=%2,%3 button=%4 buttons=%5 win=%6 receiver=%7 popup=%8 capture=%9 owners=[%10] modifiers=%11)")
                    .arg(int(event.type))
                    .arg(event.global.x())
                    .arg(event.global.y())
                    .arg(int(event.button))
                    .arg(event.buttons.toInt())
                    .arg(event.nativeId)
                    .arg(event.receiver)
                    .arg(event.popup)
                    .arg(event.captured)
                    .arg(owners.join(u','))
                    .arg(event.modifiers.toInt());
    }
    return result;
  }

 protected:
  bool eventFilter(QObject* receiver, QEvent* event) override {
    const bool boundary = event->type() == QEvent::Enter || event->type() == QEvent::Leave;
    if (!boundary && !event->spontaneous()) return false;
    if (!boundary && event->type() != QEvent::MouseMove && event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonDblClick &&
        event->type() != QEvent::MouseButtonRelease)
      return false;
    auto* window = qobject_cast<QWindow*>(receiver);
    if (auto* widget = qobject_cast<QWidget*>(receiver)) window = widget->window()->windowHandle();
    if (window) {
      const auto* mouse = boundary ? nullptr : static_cast<QMouseEvent*>(event);
      const auto global = mouse                            ? mouse->globalPosition().toPoint()
                          : event->type() == QEvent::Enter ? static_cast<QEnterEvent*>(event)->globalPosition().toPoint()
                                                           : QCursor::pos();
      const auto id = window->winId();
      std::vector<WId> owners;
      // Snapshot ownership during delivery; the popup may be destroyed before the waiting predicate runs.
      auto* current = window;
      for (int depth = 0; current && depth < 32; ++depth) {
        owners.push_back(current->winId());
        current = current->parent(QWindow::IncludeTransients);
      }
      auto* ownerWidget = qobject_cast<QWidget*>(receiver);
      if (!ownerWidget) ownerWidget = QWidget::find(id);
      const bool popup = window->type() == Qt::Popup || (ownerWidget && ownerWidget->windowType() == Qt::Popup);
      for (int depth = 0; ownerWidget && depth < 32; ++depth) {
        if (auto* ownerWindow = ownerWidget->window()->windowHandle()) owners.push_back(ownerWindow->winId());
        ownerWidget = ownerWidget->parentWidget();
      }
      auto owner = reinterpret_cast<HWND>(id);
      for (int depth = 0; owner && depth < 32; ++depth) {
        owners.push_back(reinterpret_cast<WId>(owner));
        owner = GetWindow(owner, GW_OWNER);
      }
      const bool captured = GetCapture() == reinterpret_cast<HWND>(id);
      m_qt.push_back({event->type(), global, mouse ? mouse->buttons() : QApplication::mouseButtons(), mouse ? mouse->button() : Qt::NoButton,
                      mouse ? mouse->modifiers() : QApplication::keyboardModifiers(), id, popup, captured,
                      QString::fromLatin1(receiver->metaObject()->className()) + u'#' + receiver->objectName(), std::move(owners)});
    }
    return false;
  }
  bool nativeEventFilter(const QByteArray& type, void* message, qintptr*) override {
    if (type != "windows_generic_MSG" && type != "windows_dispatcher_MSG") return false;
    const auto& msg = *static_cast<MSG*>(message);
    // Coalesced moves may lose extraInfo; validate their HWND and screen point separately.
    if (msg.message == WM_MOUSEMOVE || msg.message == WM_NCMOUSEMOVE)
      m_nativeMoves.push_back({reinterpret_cast<WId>(msg.hwnd), QPoint(msg.pt.x, msg.pt.y), msg.message == WM_NCMOUSEMOVE});
    if (static_cast<ULONG_PTR>(GetMessageExtraInfo()) != m_tag) return false;
    switch (msg.message) {
      case WM_MOUSEMOVE:
        ++m_moves;
        break;
      case WM_NCMOUSEMOVE:
        ++m_ncMoves;
        break;
      case WM_LBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_XBUTTONDOWN:
      case WM_LBUTTONDBLCLK:
      case WM_RBUTTONDBLCLK:
      case WM_MBUTTONDBLCLK:
      case WM_XBUTTONDBLCLK:
        ++m_downs;
        break;
      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP:
      case WM_XBUTTONUP:
        ++m_ups;
        break;
      default:
        break;
    }
    return false;
  }

 private:
  struct QtReceipt {
    QEvent::Type type;
    QPoint global;
    Qt::MouseButtons buttons;
    Qt::MouseButton button;
    Qt::KeyboardModifiers modifiers;
    WId nativeId;
    bool popup, captured;
    QString receiver;
    std::vector<WId> owners;
  };
  struct NativeMove {
    WId window;
    QPoint position;
    bool nonClient;
  };
  std::vector<NativeMove> m_nativeMoves;
  std::vector<QtReceipt> m_qt;
  ULONG_PTR m_tag = nextInputTag();
  int m_moves = 0, m_ncMoves = 0, m_downs = 0, m_ups = 0;
};
bool sendMove(const PointerGeometry& geometry, const ULONG_PTR tag) {
  const int left = GetSystemMetrics(SM_XVIRTUALSCREEN), top = GetSystemMetrics(SM_YVIRTUALSCREEN);
  const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN), height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  const auto point = geometry.nativeGlobal;
  if (!check(width > 0 && height > 0 && QRect(left, top, width, height).contains(point), "The requested pointer point is outside the virtual desktop"))
    return false;
  INPUT input{};
  input.type = INPUT_MOUSE;
  // Address pixel centers in normalized virtual-desktop coordinates, including
  // negative monitor origins; do not mix Qt logical positions with SendInput.
  input.mi.dx = static_cast<LONG>(((2LL * (point.x() - left) + 1) * 65536) / (2LL * width));
  input.mi.dy = static_cast<LONG>(((2LL * (point.y() - top) + 1) * 65536) / (2LL * height));
  input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE_NOCOALESCE;
  input.mi.dwExtraInfo = tag;
  return submit(input);
}
// Input-transparent decoration windows are not native pointer targets. Resolve
// the window below them before consulting its QWidget child hit test.
HWND nativeHit(const QPoint position) {
  const POINT point{position.x(), position.y()};
  auto hit = WindowFromPoint(point);
  auto root = hit ? GetAncestor(hit, GA_ROOT) : nullptr;
  const auto transparent = [](const HWND window) {
    auto* widget = QWidget::find(reinterpret_cast<WId>(window));
    return (GetWindowLongPtrW(window, GWL_EXSTYLE) & WS_EX_TRANSPARENT) != 0 || (widget && widget->windowFlags().testFlag(Qt::WindowTransparentForInput));
  };
  if (root && !transparent(root)) return hit;
  for (auto current = root ? GetWindow(root, GW_HWNDNEXT) : GetTopWindow(nullptr); current; current = GetWindow(current, GW_HWNDNEXT)) {
    RECT rectangle{};
    if (IsWindowVisible(current) && !transparent(current) && GetWindowRect(current, &rectangle) && PtInRect(&rectangle, point)) return current;
  }
  return nullptr;
}
struct PointerHit {
  bool belongs = false;
  bool nonClient = false;
  QWidget* widget = nullptr;
};
PointerHit pointerHit(const PointerGeometry& geometry) {
  const auto expected = reinterpret_cast<HWND>(geometry.nativeId);
  const auto hit = nativeHit(geometry.nativeGlobal);
  const bool belongs = hit && (hit == expected || IsChild(expected, hit));
  DWORD_PTR area = HTNOWHERE;
  const bool inspected = belongs && SendMessageTimeoutW(expected, WM_NCHITTEST, 0, MAKELPARAM(geometry.nativeGlobal.x(), geometry.nativeGlobal.y()),
                                                        SMTO_ABORTIFHUNG | SMTO_BLOCK, 50, &area);
  auto* widget = QWidget::find(geometry.nativeId);
  if (widget) {
    const auto local = widget->mapFromGlobal(geometry.global);
    if (auto* child = widget->childAt(local)) widget = child;
  }
  return {belongs,
          inspected && area != HTCLIENT && area != HTNOWHERE && area != static_cast<DWORD_PTR>(HTTRANSPARENT) && area != static_cast<DWORD_PTR>(HTERROR),
          widget};
}
std::optional<QPoint> outsideFrame(const PointerGeometry& geometry) {
  RECT rectangle{};
  if (!GetWindowRect(reinterpret_cast<HWND>(geometry.nativeId), &rectangle)) return {};
  const int x = (rectangle.left + rectangle.right) / 2, y = (rectangle.top + rectangle.bottom) / 2;
  for (const auto point : {QPoint(rectangle.left - 2, y), QPoint(rectangle.right + 2, y), QPoint(x, rectangle.top - 2), QPoint(x, rectangle.bottom + 2)})
    if (MonitorFromPoint(POINT{point.x(), point.y()}, MONITOR_DEFAULTTONULL)) return point;
  return {};
}
bool canReenterPointerTarget(const PointerGeometry& geometry, const Qt::MouseButtons expectedButtons) {
  if (expectedButtons != Qt::NoButton || nativeButtons() != Qt::NoButton || QApplication::mouseButtons() != Qt::NoButton || heldInput().buttons != Qt::NoButton)
    return false;
  const auto capture = GetCapture();
  if (!capture) return true;
  // Idle popup capture permits re-entry only into the exact active target popup.
  const auto* popup = QApplication::activePopupWidget();
  return popup && popup->isVisible() && popup->windowHandle() == geometry.window && capture == reinterpret_cast<HWND>(geometry.nativeId);
}
std::optional<PointerGeometry> movePointer(const PointerTarget& target, const int delay) {
  if (!nativePointerInputAllowed()) return {};
  bool completed = false;
  const auto cleanup = qScopeGuard([&] {
    if (!completed) releaseHeldInput();
  });
  auto* window = target.window();
  if (!check(window && target.alive(), "The pointer target is unavailable before movement")) return {};
  if (!window->isActive() && (window->type() == Qt::Window || window->type() == Qt::Dialog) && !window->flags().testFlag(Qt::WindowDoesNotAcceptFocus)) {
    window->requestActivate();
    if (!check(QTest::qWaitForWindowActive(window), "The pointer target did not become active")) return {};
  }
  // Wait for queued releases from retired popups; mixed synthetic/native button state cannot authorize movement.
  if (!check(QTest::qWaitFor([] { return QApplication::mouseButtons() == nativeButtons(); }),
             "The delivered Qt buttons and physical buttons differ; use complete native pointer gestures"))
    return {};
  if (delay > 0) QTest::qWait(delay);
  auto commanded = target.capture();
  if (!commanded) return {};
  const auto buttons = nativeButtons();
  const auto initialGeometry = *commanded;
  POINT initialNative{};
  const auto initialLogical = QCursor::pos();
  const int initialTolerance = std::abs(commanded->devicePixelRatio - std::round(commanded->devicePixelRatio)) > .001 ? 1 : 0;
  const bool initiallyAtTarget = GetPhysicalCursorPos(&initialNative) && std::abs(initialNative.x - commanded->nativeGlobal.x()) <= 1 &&
                                 std::abs(initialNative.y - commanded->nativeGlobal.y()) <= 1 &&
                                 std::abs(initialLogical.x() - commanded->global.x()) <= initialTolerance &&
                                 std::abs(initialLogical.y() - commanded->global.y()) <= initialTolerance;
  const auto initialHit = pointerHit(*commanded);
  const bool initiallyHovered = initiallyAtTarget && initialHit.belongs && !initialHit.nonClient && initialHit.widget && initialHit.widget->underMouse() &&
                                buttons == Qt::NoButton && QApplication::mouseButtons() == buttons;
  PointerReceipt receipt;
  // Input precedes arbitrary timer processing, especially for transient popups.
  if (!sendMove(*commanded, receipt.tag())) return {};
  int stable = 0, changed = 0, displaced = 0, unresolvedClient = 0, outsideStable = 0;
  bool retired = false, inputFailed = false, reentered = false;
  std::optional<QPoint> outside;
  const bool aligned = QTest::qWaitFor([&] {
    if (!target.alive()) {
      retired = true;
      return true;
    }
    const auto next = target.capture();
    if (!next) return false;
    POINT native{};
    if (!GetPhysicalCursorPos(&native)) return false;
    if (outside) {
      if (std::abs(native.x - outside->x()) > 1 || std::abs(native.y - outside->y()) > 1 || ++outsideStable < 2) return false;
      // Revalidate capture and buttons after leave before returning to the target.
      if (!canReenterPointerTarget(*next, buttons)) return false;
      // One idle leave/re-entry repairs a same-point move suppressed by the platform cache.
      outside.reset();
      commanded = next;
      receipt.clear();
      if (!sendMove(*next, receipt.tag())) {
        inputFailed = true;
        return true;
      }
      return false;
    }
    if (*next != *commanded) {
      commanded = next;
      ++changed;
      stable = 0;
      receipt.clear();
      if (!sendMove(*next, receipt.tag())) {
        inputFailed = true;
        return true;
      }
      return false;
    }
    const auto logical = QCursor::pos();
    const int tolerance = std::abs(next->devicePixelRatio - std::round(next->devicePixelRatio)) > .001 ? 1 : 0;
    if (std::abs(next->nativeClient.width() - qRound(next->geometry.width() * next->devicePixelRatio)) > 1 ||
        std::abs(next->nativeClient.height() - qRound(next->geometry.height() * next->devicePixelRatio)) > 1) {
      stable = 0;
      return false;
    }
    if (std::abs(native.x - next->nativeGlobal.x()) > 1 || std::abs(native.y - next->nativeGlobal.y()) > 1 ||
        std::abs(logical.x() - next->global.x()) > tolerance || std::abs(logical.y() - next->global.y()) > tolerance) {
      ++displaced;
      stable = 0;
      return false;
    }
    const auto hit = pointerHit(*next);
    const bool sameConfirmed = heldInput().pointer && *heldInput().pointer == *next;
    const bool captured = GetCapture() == reinterpret_cast<HWND>(next->nativeId);
    const bool qtReceipt = receipt.received(PointerAction::Move, *next, buttons);
    // Untracked client moves and non-client areas may produce no QMouseEvent.
    // A native move receipt is still required; stale underMouse alone is insufficient.
    const bool clientReceipt = hit.belongs && receipt.nativeMoved(*next, false) && hit.widget && hit.widget->underMouse() && !hit.widget->hasMouseTracking();
    const bool ncReceipt = hit.belongs && hit.nonClient && receipt.nativeMoved(*next, true);
    // An unchanged, already-hovered native/Qt target needs no new move receipt.
    const bool initialNoOp = initiallyHovered && initialGeometry == *next && hit.belongs && hit.widget && hit.widget->underMouse();
    const bool delivered = qtReceipt || clientReceipt || ncReceipt || initialNoOp || (sameConfirmed && (hit.belongs || captured));
    if (!delivered && hit.belongs && !hit.nonClient && !reentered && canReenterPointerTarget(*next, buttons) && ++unresolvedClient >= 3) {
      outside = outsideFrame(*next);
      if (outside) {
        reentered = true;
        auto exit = *next;
        exit.nativeGlobal = *outside;
        if (!sendMove(exit, receipt.tag())) {
          inputFailed = true;
          return true;
        }
      }
      return false;
    }
    if (!delivered || QApplication::mouseButtons() != buttons || nativeButtons() != buttons) {
      stable = 0;
      return false;
    }
    return ++stable >= 2;
  });
  if (retired || inputFailed) return {};
  const auto hit = pointerHit(*commanded);
  const auto diagnostic = QStringLiteral(
                              "Native pointer receipt/geometry mismatch: requested=%1,%2 actual=%3,%4 changed=%5 displaced=%6 stable=%7 buttons=%8/%9 hit=%10 "
                              "nc=%11 reentry=%12 initialAtTarget=%13 initialHover=%14")
                              .arg(commanded->global.x())
                              .arg(commanded->global.y())
                              .arg(QCursor::pos().x())
                              .arg(QCursor::pos().y())
                              .arg(changed)
                              .arg(displaced)
                              .arg(stable)
                              .arg(QApplication::mouseButtons().toInt())
                              .arg(buttons.toInt())
                              .arg(hit.belongs)
                              .arg(hit.nonClient)
                              .arg(reentered)
                              .arg(initiallyAtTarget)
                              .arg(initiallyHovered) +
                          u' ' + receipt.diagnostic(&*commanded);
  if (!check(aligned, qUtf8Printable(diagnostic))) return {};
  heldInput().pointer = *commanded;
  completed = true;
  return commanded;
}
struct WindowPressHistory {
  QPointer<QWindow> window;
  WId nativeId = 0;
  QMetaObject::Connection lifetime;
  PointerPressHistory presses;
};
WindowPressHistory& windowPressHistory() {
  static WindowPressHistory history;
  return history;
}
PointerWindowIdentity pressWindow(const PointerGeometry& location) {
  return {reinterpret_cast<std::uintptr_t>(location.window.data()), static_cast<std::uintptr_t>(location.nativeId)};
}
void recordPress(const PointerGeometry& location, const Qt::MouseButton button) {
  auto& history = windowPressHistory();
  const auto identity = pressWindow(location);
  if (history.window != location.window || history.nativeId != location.nativeId) {
    if (history.window) history.presses.retire({reinterpret_cast<std::uintptr_t>(history.window.data()), static_cast<std::uintptr_t>(history.nativeId)});
    QObject::disconnect(history.lifetime);
    history.window = location.window;
    history.nativeId = location.nativeId;
    history.lifetime = QObject::connect(location.window.data(), &QObject::destroyed, qApp, [identity] {
      auto& current = windowPressHistory();
      current.presses.retire(identity);
    });
  }
  history.presses.record(identity, static_cast<std::uint32_t>(button), {location.nativeGlobal.x(), location.nativeGlobal.y()}, GetTickCount64());
}
bool separateSinglePress(const PointerTarget& target, const PointerGeometry& location, const Qt::MouseButton button) {
  const auto interval = std::max<ULONGLONG>(GetDoubleClickTime(), static_cast<ULONGLONG>(QGuiApplication::styleHints()->mouseDoubleClickInterval()));
  const SinglePressPolicy policy{interval, GetSystemMetrics(SM_CXDOUBLECLK) / 2, GetSystemMetrics(SM_CYDOUBLECLK) / 2};
  const auto identity = pressWindow(location);
  const auto ready = [&] {
    return windowPressHistory().presses.remainingDelay(identity, static_cast<std::uint32_t>(button), {location.nativeGlobal.x(), location.nativeGlobal.y()},
                                                       GetTickCount64(), policy) == 0;
  };
  if (ready()) return target.alive();
  // Preserve QTest's independent-click semantics by waiting beyond the native double-click interval.
  return QTest::qWaitFor([&] { return !target.alive() || ready(); }, static_cast<int>(interval + 1000)) && target.alive();
}
bool changeButton(const PointerTarget& target, const Qt::MouseButton button, const Qt::KeyboardModifiers modifiers, const int delay, const bool down) {
  if (!nativePointerInputAllowed()) return false;
  const ButtonInput* mapping = nullptr;
  for (const auto& item : buttonInputs)
    if (item.button == button) mapping = &item;
  if (!check(mapping, "Unsupported native mouse button")) return false;
  auto location = target.alive() ? movePointer(target, delay) : std::nullopt;
  if (down && !location) return false;
  if (!down && !target.alive() && !nativeButtons().testFlag(button)) return true;
  if (down) {
    if (!separateSinglePress(target, *location, button)) return false;
    const auto current = target.capture();
    if (!current) return false;
    if (*current != *location) location = movePointer(target, -1);
    if (!location) return false;
  }
  if (!pressModifiers(modifiers)) {
    releaseHeldInput();
    return false;
  }
  auto& held = heldInput();
  auto expected = nativeButtons();
  expected.setFlag(button, down);
  if (down) {
    if (held.buttons == Qt::NoButton) held.lifetime = QObject::connect(location->window.data(), &QObject::destroyed, qApp, [] { releaseHeldInput(); });
    held.buttons |= button;
  }
  PointerReceipt receipt;
  if (down) recordPress(*location, button);
  if (!submit(buttonInput(*mapping, down, receipt.tag()))) {
    releaseHeldInput();
    return false;
  }
  bool retired = false;
  const bool delivered = QTest::qWaitFor([&] {
    if (!target.alive()) {
      retired = true;
      return true;
    }
    const bool physical = nativeButtons() == expected;
    // Always release the physical button, even if the press hid or destroyed its target.
    return physical && location &&
           receipt.received(down ? PointerAction::Press : PointerAction::Release, *location, expected, button, modifiers | held.modifiers) &&
           QApplication::mouseButtons() == expected;
  });
  if (retired) {
    releaseHeldInput();
    return true;
  }
  if (!down) {
    held.buttons.setFlag(button, false);
    if (held.buttons == Qt::NoButton) {
      releaseModifiers();
      QObject::disconnect(held.lifetime);
      held.lifetime = {};
    }
  }
  if (!check(delivered, qUtf8Printable(QStringLiteral("The native pointer button event was not delivered by Qt; ") +
                                       receipt.diagnostic(location ? &*location : nullptr)))) {
    releaseHeldInput();
    return false;
  }
  if (location) heldInput().pointer = *location;
  return true;
}
void clickPointer(const PointerTarget& target, const Qt::MouseButton button, const Qt::KeyboardModifiers modifiers, const int delay) {
  if (!nativePointerInputAllowed()) return;
  const ButtonInput* mapping = nullptr;
  for (const auto& item : buttonInputs)
    if (item.button == button) mapping = &item;
  if (!check(mapping, "Unsupported native mouse button")) return;
  auto location = movePointer(target, delay);
  if (!location || !separateSinglePress(target, *location, button)) return;
  const auto current = target.capture();
  if (!current) return;
  if (*current != *location) location = movePointer(target, -1);
  if (!location) return;
  const auto expected = nativeButtons();
  if (!check(!expected.testFlag(button), "A complete click cannot start with its native button already pressed")) return;
  constexpr auto supported = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
  if (!check((modifiers & ~supported) == Qt::NoModifier, "The native pointer click has unsupported modifiers")) return;
  PointerReceipt receipt;
  std::vector<INPUT> inputs;
  std::vector<WORD> addedModifiers;
  Qt::KeyboardModifiers addedMask;
  for (const auto& modifier : modifierInputs)
    if (modifiers.testFlag(modifier.modifier) && !(GetAsyncKeyState(modifier.key) & 0x8000)) {
      inputs.push_back(keyInput(modifier.key, true));
      addedModifiers.push_back(modifier.key);
      addedMask |= modifier.modifier;
    }
  inputs.push_back(buttonInput(*mapping, true, receipt.tag()));
  inputs.push_back(buttonInput(*mapping, false, receipt.tag()));
  for (auto it = addedModifiers.rbegin(); it != addedModifiers.rend(); ++it) inputs.push_back(keyInput(*it, false));
  // Queue the complete click before pumping Qt. A press may enter QMenu::exec;
  // its release and modifier cleanup must already be in the native input queue.
  recordPress(*location, button);
  const auto count = static_cast<UINT>(inputs.size());
  if (!check(SendInput(count, inputs.data(), sizeof(INPUT)) == count, "The complete native click was not submitted")) {
    // Partial SendInput submission requires releasing only this sequence's button and modifiers.
    (void)submit(buttonInput(*mapping, false));
    for (const auto key : addedModifiers) (void)submit(keyInput(key, false));
    return;
  }
  const bool delivered = QTest::qWaitFor([&] {
    for (const auto key : addedModifiers)
      if (GetAsyncKeyState(key) & 0x8000) return false;
    // keyboardModifiers caches delivered events; query platform state after key-up.
    return nativeButtons() == expected &&
           (!target.alive() || (receipt.received(PointerAction::Press, *location, expected | button, button, modifiers | heldInput().modifiers) &&
                                receipt.received(PointerAction::Release, *location, expected, button, modifiers | heldInput().modifiers) &&
                                QApplication::mouseButtons() == expected && (QApplication::queryKeyboardModifiers() & addedMask) == Qt::NoModifier));
  });
  if (check(delivered,
            qUtf8Printable(QStringLiteral("The complete native click had no delivered normal Qt press/release; ") + receipt.diagnostic(&*location))) &&
      target.alive())
    heldInput().pointer = *location;
}
}  // namespace
bool prepareClipboardText(const QString& text) {
  auto* clipboard = QApplication::clipboard();
  return QTest::qWaitFor(
      [&] {
        clipboard->setText(text);
        return clipboard->text() == text;
      },
      5000);
}
bool copySelectionToClipboard(QWidget& target, const QString& expectedText) {
  const QPointer owner(&target);
  auto initial = QStringLiteral("UWF clipboard selection transfer pending");
  if (initial == expectedText) initial += u'\u2060';
  if (!prepareClipboardText(initial) || !owner) return false;
  auto* clipboard = QApplication::clipboard();
  // Retry transient clipboard contention until the shortcut produces the expected text.
  return QTest::qWaitFor(
      [&] {
        if (!owner) return false;
        QTest::keyClick(owner.data(), Qt::Key_C, Qt::ControlModifier);
        return clipboard->text() == expectedText;
      },
      5000);
}

void moveMouse(QWindow* window, const QPoint position, const int delay) { (void)movePointer(PointerTarget(window, position), delay); }
void moveMouse(QWidget& target, const QPoint position, const int delay) { (void)movePointer(PointerTarget(&target, position), delay); }
void moveMouse(QWidget* target, const QPoint position, const int delay) { (void)movePointer(PointerTarget(target, position), delay); }
void clickMouse(QWindow* target, const Qt::MouseButton button, const Qt::KeyboardModifiers modifiers, const QPoint position, const int delay) {
  clickPointer(PointerTarget(target, position), button, modifiers, delay);
}
void clickMouse(QWidget* target, const Qt::MouseButton button, const Qt::KeyboardModifiers modifiers, const QPoint position, const int delay) {
  clickPointer(PointerTarget(target, position), button, modifiers, delay);
}
void pressMouse(QWindow* target, const Qt::MouseButton button, const Qt::KeyboardModifiers modifiers, const QPoint position, const int delay) {
  (void)changeButton(PointerTarget(target, position), button, modifiers, delay, true);
}
void pressMouse(QWidget* target, const Qt::MouseButton button, const Qt::KeyboardModifiers modifiers, const QPoint position, const int delay) {
  (void)changeButton(PointerTarget(target, position), button, modifiers, delay, true);
}
void releaseMouse(QWindow* target, const Qt::MouseButton button, const Qt::KeyboardModifiers modifiers, const QPoint position, const int delay) {
  (void)changeButton(PointerTarget(target, position), button, modifiers, delay, false);
}
void releaseMouse(QWidget* target, const Qt::MouseButton button, const Qt::KeyboardModifiers modifiers, const QPoint position, const int delay) {
  (void)changeButton(PointerTarget(target, position), button, modifiers, delay, false);
}

// Deliver spontaneous logical coordinates through the native window so high-DPI
// hit testing and parent propagation follow the same path as real wheel input.
void sendWheel(QWidget& target, const QPoint angleDelta) {
  auto* window = target.window();
  QVERIFY(window->isVisible() && window->windowHandle());
  const QPoint position = target.mapTo(window, target.rect().center());
  QWheelEvent event(position, window->mapToGlobal(position), QPoint(), angleDelta, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QSpontaneKeyEvent::setSpontaneous(&event);
  qApp->notify(window->windowHandle(), &event);
  QCoreApplication::processEvents();
}

}  // namespace uwf::test
