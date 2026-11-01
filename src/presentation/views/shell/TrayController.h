// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Presents supplied usage state and forwards user intents without reading UWF.
// The alert icon denotes disabled, unavailable or stale state.
// No tray objects are created when the system tray is unavailable.

#include <QAbstractNativeEventFilter>
#include <QIcon>
#include <QObject>
#include <QObjectCleanupHandler>
#include <QPoint>
#include <QPointer>

#include "application/overlay/contracts/OverlayUsage.h"

class QAction;
class QEvent;
class QFrame;
class QLabel;
class QProgressBar;
class QSystemTrayIcon;
class QWidget;
class QTimer;

namespace uwf::ui {

class OverlayContextMenu;

class TrayController : public QObject, public QAbstractNativeEventFilter {
  Q_OBJECT
 public:
  // ownerWindow parents menu widgets; controller lifetime is managed separately.
  explicit TrayController(QWidget* ownerWindow, QObject* parent = nullptr);
  ~TrayController() override;

  // Includes retained observations marked stale after a read failure.
  void applyUsageState(app::OverlayUsageState state);
  void setHubState(bool available, bool enabled);
  void setOperationAvailability(bool powerAllowed, bool exitAllowed);
 signals:

  void refreshRequested();

  void activateWindowRequested();
  void toggleHubRequested();
  void safeShutdownRequested();
  void safeRestartRequested();

  void exitApplicationRequested();

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;
  bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

 private:
  void renderCommittedState();
  void suppressNativeToolTip();
  void showHoverToolTip();

  QIcon m_iconNormal;
  QIcon m_iconAlert;
  QSystemTrayIcon* m_tray = nullptr;  // Null when the system tray is unavailable.
  OverlayContextMenu* m_menu = nullptr;
  QAction* m_usageAction = nullptr;
  QWidget* m_usagePane = nullptr;
  QWidget* m_usageCard = nullptr;
  QProgressBar* m_usageProgress = nullptr;
  QLabel* m_usageTitle = nullptr;
  QLabel* m_usagePercent = nullptr;
  QLabel* m_usageLabel = nullptr;
  QPointer<QWidget> m_hoverSurface;
  QObjectCleanupHandler m_ownedWidgets;
  QLabel* m_hoverText = nullptr;
  QTimer* m_hoverDelay = nullptr;
  QTimer* m_hoverDismiss = nullptr;
  QPoint m_hoverAnchor;
  quintptr m_callbackWindow = 0;
  unsigned int m_callbackIcon = 0;

  // One supplied state drives retranslation and repaint, including stale observations.
  app::OverlayUsageState m_usageState{app::OverlayUsageUnavailable{}};
};

}  // namespace uwf::ui
