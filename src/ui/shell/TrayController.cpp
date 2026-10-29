// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TrayController.h"

// Windows types must precede the Shell API declarations.
// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on

#include <QAction>
#include <QByteArray>
#include <QCoreApplication>
#include <QCursor>
#include <QEvent>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QProgressBar>
#include <QScreen>
#include <QStyle>
#include <QSvgRenderer>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QWidgetAction>
#include <algorithm>
#include <cmath>
#include <cstdint>

#include "presentation/overlay/OverlayViewModel.h"
#include "shared/Log.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/overlay/OverlayContextMenu.h"
#include "ui/overlay/OverlayHudRenderer.h"
#include "ui/widgets/HoverToolTip.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {

namespace {

// 禁用或不可用时将应用图标渐变改为红色；资源读取失败时保留原图标。
QIcon makeAlertIcon() {
  QFile f(QStringLiteral(":/icons/app.svg"));
  if (!f.open(QIODevice::ReadOnly)) return QIcon(QStringLiteral(":/icons/app.svg"));
  QByteArray svg = f.readAll();
  svg.replace("#4C8BF5", "#E15A4C");  // 渐变亮端：蓝 → 红
  svg.replace("#1A5DC7", "#B4271C");  // 渐变暗端：蓝 → 红
  QSvgRenderer renderer(svg);
  if (!renderer.isValid()) return QIcon(QStringLiteral(":/icons/app.svg"));
  QIcon icon;
  for (const int sz : {16, 20, 24, 32, 48, 64}) {
    QPixmap pm(sz, sz);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    p.end();
    icon.addPixmap(pm);
  }
  return icon;
}

}  // namespace

TrayController::TrayController(QWidget* ownerWindow, QObject* parent)
    : QObject(parent), m_iconNormal(QStringLiteral(":/icons/app.svg")), m_iconAlert(makeAlertIcon()) {
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    UWF_LOG_W("tray") << "system tray unavailable: action=tray-disabled";
    return;
  }

  m_menu = new OverlayContextMenu(ownerWindow);
  m_menu->setHubState(false, false);
  m_menu->setPowerActionsEnabled(false);
  connect(m_menu, &OverlayContextMenu::showMainWindowRequested, this, &TrayController::activateWindowRequested);
  connect(m_menu, &OverlayContextMenu::toggleHubRequested, this, &TrayController::toggleHubRequested);
  connect(m_menu, &OverlayContextMenu::safeShutdownRequested, this, &TrayController::safeShutdownRequested);
  connect(m_menu, &OverlayContextMenu::safeRestartRequested, this, &TrayController::safeRestartRequested);
  connect(m_menu, &OverlayContextMenu::exitApplicationRequested, this, &TrayController::exitApplicationRequested);

  m_usagePane = new QWidget(m_menu);
  m_usagePane->setObjectName(QStringLiteral("trayUsageItem"));
  m_usagePane->setAttribute(Qt::WA_Hover, true);
  m_usagePane->setAttribute(Qt::WA_StyledBackground, true);
  m_usagePane->setCursor(Qt::PointingHandCursor);
  m_usagePane->setMinimumWidth(240);
  m_usagePane->installEventFilter(this);
  auto* outer = new QVBoxLayout(m_usagePane);
  outer->setContentsMargins(11, 3, 11, 11);
  m_usageCard = new Surface(design::Color::Surface, m_usagePane);
  m_usageCard->setObjectName(QStringLiteral("trayUsageCard"));
  m_usageCard->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  auto* card = new QVBoxLayout(m_usageCard);
  card->setContentsMargins(0, 0, 0, 0);
  card->setSpacing(11);
  auto* heading = new QHBoxLayout;
  heading->setSpacing(12);
  m_usageTitle = new TextLabel({}, TextStyle::Caption, m_usageCard);
  m_usageTitle->setObjectName(QStringLiteral("trayUsageTitle"));
  m_usagePercent = new TextLabel({}, TextStyle::Section, m_usageCard);
  m_usagePercent->setObjectName(QStringLiteral("trayUsagePercent"));
  m_usagePercent->setLayoutDirection(Qt::LeftToRight);
  QFont percentFont = m_usagePercent->font();
  percentFont.setPixelSize(17);
  percentFont.setWeight(QFont::DemiBold);
  m_usagePercent->setFont(percentFont);
  heading->addWidget(m_usageTitle);
  heading->addWidget(m_usagePercent);
  heading->addStretch();
  m_usageLabel = new TextLabel({}, TextStyle::Eyebrow, m_usageCard);
  m_usageLabel->setObjectName(QStringLiteral("trayUsageNumeric"));
  m_usageLabel->setLayoutDirection(Qt::LeftToRight);
  auto numericFont = m_usageLabel->font();
  numericFont.setPixelSize(10);
  m_usageLabel->setFont(numericFont);
  static_cast<TextLabel*>(m_usageLabel)->setColorRole(design::Color::Muted);
  heading->addWidget(m_usageLabel, 0, Qt::AlignBottom);
  card->addLayout(heading);
  m_usageProgress = new QProgressBar(m_usageCard);
  m_usageProgress->setObjectName(QStringLiteral("trayUsageProgress"));
  m_usageProgress->setRange(0, 10000);
  m_usageProgress->setTextVisible(false);
  m_usageProgress->setFixedHeight(5);
  card->addWidget(m_usageProgress);
  outer->addWidget(m_usageCard);
  m_usageAction = m_menu->addUsageWidget(m_usagePane);
  connect(m_menu, &QMenu::hovered, this, [this](QAction* action) {
    const bool active = action == m_usageAction;
    if (m_usagePane->property("menuActive").toBool() == active) return;
    m_usagePane->setProperty("menuActive", active);
    m_usagePane->style()->unpolish(m_usagePane);
    m_usagePane->style()->polish(m_usagePane);
    m_usagePane->update();
  });
  connect(m_menu, &QMenu::aboutToShow, this, [this] {
    renderCommittedState();
    emit refreshRequested();
  });
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { renderCommittedState(); });
  connect(&I18n::events(), &LanguageEvents::changed, this, &TrayController::renderCommittedState);

  m_tray = new QSystemTrayIcon(m_iconAlert, this);
  auto* toolTip = new HoverToolTip(ownerWindow);
  m_ownedWidgets.add(toolTip);
  toolTip->setObjectName(QStringLiteral("trayHoverToolTip"));
  m_hoverText = toolTip->textLabel();
  m_hoverSurface = toolTip;
  m_hoverDelay = new QTimer(this);
  m_hoverDelay->setSingleShot(true);
  m_hoverDelay->setInterval(HoverToolTip::delayMs);
  connect(m_hoverDelay, &QTimer::timeout, this, &TrayController::showHoverToolTip);
  m_hoverDismiss = new QTimer(this);
  m_hoverDismiss->setInterval(design::hoverPollIntervalMs);
  connect(m_hoverDismiss, &QTimer::timeout, this, [this] {
    const QRect icon = m_tray->geometry();
    if ((icon.isValid() && icon.contains(QCursor::pos())) || (!icon.isValid() && (QCursor::pos() - m_hoverAnchor).manhattanLength() < 20)) return;
    if (m_hoverSurface) m_hoverSurface->hide();
    m_hoverDismiss->stop();
  });
  QCoreApplication::instance()->installNativeEventFilter(this);
  // 按光标位置弹出；Windows 托盘图标 geometry 在折叠区域中并不可靠。
  connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
    m_hoverDelay->stop();
    if (m_hoverSurface) m_hoverSurface->hide();
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
      emit activateWindowRequested();
    else if (reason == QSystemTrayIcon::Context)
      m_menu->popup(QCursor::pos());
  });
  m_tray->show();
  renderCommittedState();
}

TrayController::~TrayController() {
  if (QCoreApplication::instance()) QCoreApplication::instance()->removeNativeEventFilter(this);
}

void TrayController::suppressNativeToolTip() {
  if (!m_callbackWindow) return;
  NOTIFYICONDATAW icon{};
  icon.cbSize = sizeof(icon);
  icon.hWnd = reinterpret_cast<HWND>(m_callbackWindow);
  icon.uID = m_callbackIcon;
  // Clear the shell tooltip and NIF_SHOWTIP; retain Qt's accessible text and
  // activation/context-menu handling on the original QSystemTrayIcon.
  icon.uFlags = NIF_TIP;
  Shell_NotifyIconW(NIM_MODIFY, &icon);
}

bool TrayController::nativeEventFilter(const QByteArray&, void* message, qintptr*) {
  const auto* native = static_cast<const MSG*>(message);
  // Qt 6's Windows tray backend forwards its version-4 callback through native
  // event filters before activation. See qwindowssystemtrayicon.cpp.
  if (!m_tray || native->message != WM_APP + 101) return false;
  if (!m_callbackWindow) {
    wchar_t className[256]{};
    DWORD process = 0;
    GetWindowThreadProcessId(native->hwnd, &process);
    GetClassNameW(native->hwnd, className, 256);
    if (process != GetCurrentProcessId() || !QString::fromWCharArray(className).contains(QStringLiteral("TrayIconMessageWindow"))) return false;
    m_callbackWindow = reinterpret_cast<quintptr>(native->hwnd);
    m_callbackIcon = HIWORD(native->lParam);
    suppressNativeToolTip();
  }
  if (reinterpret_cast<quintptr>(native->hwnd) != m_callbackWindow || HIWORD(native->lParam) != m_callbackIcon) return false;
  switch (LOWORD(native->lParam)) {
    case WM_MOUSEMOVE:
    case NIN_POPUPOPEN:
      m_hoverAnchor = QCursor::pos();
      if (!m_hoverDelay->isActive() && m_hoverSurface && !m_hoverSurface->isVisible()) m_hoverDelay->start();
      break;
    case NIN_POPUPCLOSE:
      m_hoverDelay->stop();
      if (m_hoverSurface) m_hoverSurface->hide();
      break;
    default:
      break;
  }
  return false;
}

void TrayController::showHoverToolTip() {
  if (!m_hoverSurface || m_menu->isVisible()) return;
  const QRect icon = m_tray->geometry();
  if (icon.isValid() && !icon.contains(QCursor::pos())) return;
  m_hoverText->setText(m_tray->toolTip());
  m_hoverSurface->adjustSize();
  auto* screen = QGuiApplication::screenAt(QCursor::pos());
  if (!screen) screen = QGuiApplication::primaryScreen();
  const QRect area = screen->availableGeometry();
  const QSize size = m_hoverSurface->size();
  const QPoint anchor = QCursor::pos();
  const int x = std::clamp(anchor.x() + 12, area.left(), std::max(area.left(), area.right() - size.width() + 1));
  const int y = anchor.y() + size.height() + 22 <= area.bottom() ? anchor.y() + 22 : std::max(area.top(), anchor.y() - size.height() - 12);
  m_hoverSurface->move(x, y);
  m_hoverSurface->show();
  m_hoverDismiss->start();
}

void TrayController::renderCommittedState() {
  if (!m_tray) return;
  m_menu->retranslate();
  auto& theme = ThemeManager::instance();
  QColor progressColor = theme.color(Sem::Accent);
  const auto* const enabled = std::get_if<app::OverlayUsageEnabled>(&m_usageState);
  m_usageProgress->setVisible(enabled != nullptr);
  m_usagePercent->setVisible(enabled != nullptr);
  m_usageLabel->setVisible(enabled != nullptr);
  m_usageTitle->setVisible(enabled == nullptr);
  if (enabled) {
    m_tray->setIcon(m_iconNormal);
    const std::uint32_t used = enabled->runtime.currentConsumptionMb;
    const std::uint64_t total = core::overlayTotalMb(enabled->runtime);
    const double percent = total == 0 ? 0.0 : static_cast<double>(used) * 100.0 / static_cast<double>(total);
    const QString percentage = QString::number(percent, 'f', 0) + u'%';
    m_usageTitle->setText(I18n::tr("Overlay"));
    m_usagePercent->setText(percentage);
    const QLocale numbers(QLocale::English, QLocale::UnitedStates);
    m_usageLabel->setText(QStringLiteral("%1 / %2 MB").arg(numbers.toString(used), numbers.toString(static_cast<qulonglong>(total))));
    m_usageProgress->setValue(static_cast<int>(std::lround(percent * 100.0)));
    m_usageAction->setText(I18n::tr("Used %1 MB / Total %2 MB").arg(used).arg(static_cast<qulonglong>(total)));
#if QT_CONFIG(accessibility)
    m_usagePane->setAccessibleName(I18n::tr("UWF: Enabled"));
    m_usagePane->setAccessibleDescription(m_usageAction->text());
#endif
    m_tray->setToolTip(presentation::overlayUsageToolTip(m_usageState));
    const auto severity = core::overlayUsageSeverity(used, enabled->config.warningThresholdMb, enabled->config.criticalThresholdMb);
    if (severity == core::OverlayUsageSeverity::Critical)
      progressColor = theme.color(Sem::Danger);
    else if (severity == core::OverlayUsageSeverity::Warning)
      progressColor = theme.color(Sem::Warn);
  } else {
    m_tray->setIcon(m_iconAlert);
    const QString status = std::holds_alternative<app::OverlayUsageDisabled>(m_usageState) ? I18n::tr("UWF: Disabled") : I18n::tr("UWF status unavailable");
    m_usageTitle->setText(status);
    m_usageAction->setText(status);
#if QT_CONFIG(accessibility)
    m_usagePane->setAccessibleName(status);
    m_usagePane->setAccessibleDescription(QString());
#endif
    m_tray->setToolTip(presentation::overlayUsageToolTip(m_usageState));
  }
  m_usageProgress->setProperty("designProgressColor", progressColor);
  m_usageProgress->update();
  if (m_hoverText) m_hoverText->setText(m_tray->toolTip());
  suppressNativeToolTip();
}

void TrayController::applyUsageState(app::OverlayUsageState state) {
  if (!m_tray) return;
  m_usageState = std::move(state);
  renderCommittedState();
}

void TrayController::setHubState(const bool available, const bool enabled) {
  if (m_menu) m_menu->setHubState(available, enabled);
}

void TrayController::setPowerActionsEnabled(const bool enabled) {
  if (m_menu) m_menu->setPowerActionsEnabled(enabled);
}
void TrayController::setOperationAvailability(const bool powerAllowed, const bool exitAllowed) {
  if (m_menu) m_menu->setOperationAvailability(powerAllowed, exitAllowed);
}

bool TrayController::eventFilter(QObject* obj, QEvent* ev) {
  if (obj == m_usagePane && ev->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent*>(ev)->button() == Qt::LeftButton) {
    m_menu->close();
    emit activateWindowRequested();
    return true;
  }
  return QObject::eventFilter(obj, ev);
}

}  // namespace uwf::ui

#include "moc_TrayController.cpp"
