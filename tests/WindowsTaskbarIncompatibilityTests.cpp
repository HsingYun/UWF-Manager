/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include <windows.h>

#include <QApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QTest>
#include <QWidget>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "core/UwfModel.h"
#include "ui/OverlayFloatingWidget.h"
#include "ui/OverlayHub.h"
#include "ui/OverlayTaskbarWidget.h"
#include "ui/Win11TaskbarEnvironment.h"
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

int logCount(const std::string_view text) {
  int result = 0;
  for (const std::string& line : uwf::recentLogLines()) {
    if (line.find(text) != std::string::npos) ++result;
  }
  return result;
}

bool postTaskbarCreated(const HWND receiver) {
  const UINT message = RegisterWindowMessageW(L"TaskbarCreated");
  return receiver && message != 0 && PostMessageW(receiver, message, 0, 0);
}

class VisibilityMonitor final : public QObject {
 public:
  VisibilityMonitor(QWidget& taskbar, QWidget& floating) : m_taskbar(taskbar), m_floating(floating) {
    taskbar.installEventFilter(this);
    floating.installEventFilter(this);
  }

  void observe() { m_overlap = m_overlap || (nativeVisible(m_taskbar) && nativeVisible(m_floating)); }
  [[nodiscard]] bool overlapObserved() const { return m_overlap; }

 protected:
  bool eventFilter(QObject*, QEvent* event) override {
    if (event->type() == QEvent::Show || event->type() == QEvent::Hide || event->type() == QEvent::WinIdChange || event->type() == QEvent::ParentChange)
      observe();
    return false;
  }

 private:
  QWidget& m_taskbar;
  QWidget& m_floating;
  bool m_overlap = false;
};

}  // namespace

class WindowsTaskbarIncompatibilityTests final : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase() {
    const auto probe = win11_taskbar::probeEnvironment();
    QVERIFY2(probe.availability == win11_taskbar::RuntimeAvailability::Available && probe.environment,
             "the Windows UI incompatibility gate requires a live Explorer taskbar");
    m_explorerTaskbar = probe.environment->taskbar;
    QVERIFY2(attachmentWindows(m_explorerTaskbar).empty(), "another UWF taskbar attachment is active; UI test isolation is not satisfied");
  }

  void incompatibleLayeredChildFallsBackOnceAndStaysTerminal() {
    uwf::clearLogLines();
    {
      OverlayHub hub;
      auto taskbar = std::make_unique<OverlayTaskbarWidget>();
      OverlayTaskbarWidget* const taskbarView = taskbar.get();
      auto floating = std::make_unique<OverlayFloatingWidget>();
      OverlayFloatingWidget* const floatingView = floating.get();
      VisibilityMonitor visibility(*taskbarView, *floatingView);
      QObject::connect(taskbarView, &OverlayHubView::displayStateChanged, &visibility, &VisibilityMonitor::observe);
      QObject::connect(floatingView, &OverlayHubView::displayStateChanged, &visibility, &VisibilityMonitor::observe);

      hub.registerView(std::move(taskbar));
      hub.registerView(std::move(floating));
      hub.setFilterEnabled(true);
      hub.updateUsage(core::OverlayRuntime{.availableSpaceMb = 4096, .currentConsumptionMb = 1024});

      QVERIFY2(waitUntil([&]() {
                 return taskbarView->displayState() == OverlayHubView::DisplayState::Incompatible && floatingView->presentationConfirmed() &&
                        nativeVisible(*floatingView) && hub.presented();
               }),
               diagnosticLog().constData());
      visibility.observe();
      QVERIFY2(!visibility.overlapObserved(), diagnosticLog().constData());
      QVERIFY2(!nativeVisible(*taskbarView), diagnosticLog().constData());
      QVERIFY2(attachmentWindows(m_explorerTaskbar).empty(), diagnosticLog().constData());
      QCOMPARE(logCount("taskbar endpoint incompatible: reason=layered-child-unsupported"), 1);

      const HWND stableFloatingWindow = nativeWindow(*floatingView);
      const HWND stableTaskbarWindow = nativeWindow(*taskbarView);
      QTest::qWait(1500);
      QCOMPARE(taskbarView->displayState(), OverlayHubView::DisplayState::Incompatible);
      QCOMPARE(nativeWindow(*floatingView), stableFloatingWindow);
      QCOMPARE(nativeWindow(*taskbarView), stableTaskbarWindow);
      QCOMPARE(logCount("taskbar endpoint incompatible: reason=layered-child-unsupported"), 1);
      QVERIFY2(attachmentWindows(m_explorerTaskbar).empty(), diagnosticLog().constData());

      hub.setRequestedVisible(false);
      QVERIFY2(waitUntil([&]() { return !nativeVisible(*taskbarView) && !nativeVisible(*floatingView); }), diagnosticLog().constData());
      QWidget receiver;
      (void)receiver.winId();
      QVERIFY2(postTaskbarCreated(nativeWindow(receiver)), "failed to deliver TaskbarCreated while the incompatible endpoint was disabled");
      hub.setRequestedVisible(true);
      QVERIFY2(waitUntil([&]() { return floatingView->presentationConfirmed() && nativeVisible(*floatingView) && hub.presented(); }),
               diagnosticLog().constData());
      QTest::qWait(1200);

      visibility.observe();
      QCOMPARE(taskbarView->displayState(), OverlayHubView::DisplayState::Incompatible);
      QCOMPARE(nativeWindow(*floatingView), stableFloatingWindow);
      QCOMPARE(logCount("taskbar endpoint incompatible: reason=layered-child-unsupported"), 1);
      QVERIFY2(!visibility.overlapObserved(), diagnosticLog().constData());
      QVERIFY2(!nativeVisible(*taskbarView), diagnosticLog().constData());
      QVERIFY2(attachmentWindows(m_explorerTaskbar).empty(), diagnosticLog().constData());
    }
    QVERIFY2(waitUntil([&]() { return attachmentWindows(m_explorerTaskbar).empty(); }), "taskbar attachment leaked during test cleanup");
  }

 private:
  HWND m_explorerTaskbar = nullptr;
};

}  // namespace uwf::ui

int main(int argc, char* argv[]) {
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication app(argc, argv);
  QTEST_SET_MAIN_SOURCE_PATH
  uwf::ui::WindowsTaskbarIncompatibilityTests tc;
  return QTest::qExec(&tc, argc, argv);
}

#include "WindowsTaskbarIncompatibilityTests.moc"
