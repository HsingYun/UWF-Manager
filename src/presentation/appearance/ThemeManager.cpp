// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/appearance/ThemeManager.h"

#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QFile>
#include <QHash>
#include <QPalette>
#include <QStyleHints>
#include <utility>

#include "foundation/logging/Log.h"
#include "presentation/appearance/AppearanceTransition.h"
#include "presentation/controls/navigation/ContextMenus.h"
#include "presentation/design/painting/IconPainting.h"
#include "presentation/design/theme/DesignStyle.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/platform/DesktopPlatform.h"

namespace uwf::ui {

ThemeManager& ThemeManager::instance() {
  static ThemeManager mgr;
  return mgr;
}

ThemeManager::ThemeManager() : m_systemTheme(detectSystemShellTheme()) {
  // Structural QSS must not freeze inherited widget palettes. Let Qt propagate
  // the application palette while retaining explicit dialog stylesheet colors.
  QCoreApplication::setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);
  QApplication::setStyle(new DesignStyle());
  installContextMenuPresentation();
  QApplication::setFont(DesignSystem::defaultFont());
  if (auto* dispatcher = QAbstractEventDispatcher::instance()) dispatcher->installNativeEventFilter(this);
}

void ThemeManager::apply(app::Theme t) {
  if (m_applied && m_theme == t) return;
  AppearanceTransition::run(
      [&] {
        m_theme = t;
        m_applied = true;
        QElapsedTimer elapsed;
        elapsed.start();

        // Native controls use the application color scheme.
        QGuiApplication::styleHints()->setColorScheme(t == app::Theme::Light ? Qt::ColorScheme::Light : Qt::ColorScheme::Dark);
        const auto schemeMs = elapsed.elapsed();

        auto& designSystem = DesignSystem::instance();
        designSystem.setLight(t == app::Theme::Light);
        qApp->setPalette(designSystem.palette());
        const auto paletteMs = elapsed.elapsed();
        const auto style = styleSheet(QStringLiteral(":/styles/surfaces.qss"));
        if (qApp->styleSheet() != style) qApp->setStyleSheet(style);
        const auto styleMs = elapsed.elapsed();
        // Qt queues palette changes to QWindow and resets frameless HWNDs to light.
        // Deliver them before consumers restore the application's native appearance.
        QCoreApplication::sendPostedEvents(nullptr, QEvent::ApplicationPaletteChange);
        const auto deliveryMs = elapsed.elapsed();
        emit themeChanged(t);
        UWF_LOG_D("appearance") << "theme updated: schemeMs=" << schemeMs << " paletteMs=" << paletteMs - schemeMs << " styleMs=" << styleMs - paletteMs
                                << " deliveryMs=" << deliveryMs - styleMs << " consumersMs=" << elapsed.elapsed() - deliveryMs;
      },
      false);
}

void ThemeManager::applyStartupTheme(const std::optional<app::Theme> overrideTheme) { apply(overrideTheme ? *overrideTheme : detectSystemTheme()); }

void ThemeManager::toggle() { apply(m_theme == app::Theme::Dark ? app::Theme::Light : app::Theme::Dark); }

void ThemeManager::requestToggle() {
  m_requestedTheme = m_requestedTheme.value_or(m_theme) == app::Theme::Dark ? app::Theme::Light : app::Theme::Dark;
  AppearanceTransition::request(
      this,
      [this] {
        const auto theme = std::exchange(m_requestedTheme, std::nullopt);
        if (theme) apply(*theme);
      },
      [this] { return m_requestedTheme && *m_requestedTheme != m_theme; });
}

app::Theme ThemeManager::detectSystemTheme() {
  // A missing AppsUseLightTheme value uses the dark fallback.
  return platform::systemTheme(false);
}

app::Theme ThemeManager::detectSystemShellTheme() { return platform::systemTheme(true); }

bool ThemeManager::nativeEventFilter(const QByteArray& eventType, void* message, qintptr*) {
  if (platform::isThemeChangeEvent(eventType, message)) refreshSystemTheme();
  return false;
}

void ThemeManager::refreshSystemTheme() {
  const app::Theme next = detectSystemShellTheme();
  if (next == m_systemTheme) return;
  m_systemTheme = next;
  emit systemThemeChanged(next);
}

QColor ThemeManager::color(Sem s) const {
  using enum design::Color;
  design::Color role = Text;
  switch (s) {
    case Sem::Fg:
      role = Text;
      break;
    case Sem::FgMuted:
      role = Muted;
      break;
    case Sem::Accent:
      role = Accent;
      break;
    case Sem::Warn:
      role = Warning;
      break;
    case Sem::Danger:
      role = Danger;
      break;
    case Sem::AddOk:
      role = Success;
      break;
    case Sem::Surface:
      role = Surface;
      break;
    case Sem::Bg:
      role = Window;
      break;
    case Sem::Border:
      role = Line;
      break;
    case Sem::BarBg:
      role = Line;
      break;
  }
  return DesignSystem::instance().color(role);
}

QIcon ThemeManager::icon(const QString& resourcePath, Qt::Alignment align) const {
  const QColor fg = color(Sem::Fg);
  return iconWithColor(resourcePath, fg, align);
}

QString ThemeManager::styleSheet(const QString& resourcePath) const {
  static QHash<QString, QString> styles;
  const auto& design = DesignSystem::instance();
  const auto key = resourcePath + u'|' + QString::number(design.isLight());
  if (const auto cached = styles.constFind(key); cached != styles.cend()) return *cached;
  QFile file(resourcePath);
  if (!file.open(QIODevice::ReadOnly)) return {};
  const auto resolved = design.resolveStyle(QString::fromUtf8(file.readAll()));
  styles.insert(key, resolved);
  return resolved;
}

QIcon ThemeManager::iconWithColor(const QString& resourcePath, const QColor& fg, Qt::Alignment align) { return coloredSvgIcon(resourcePath, fg, align); }

}  // namespace uwf::ui

#include "moc_ThemeManager.cpp"
