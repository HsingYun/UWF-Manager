// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractNativeEventFilter>
#include <QIcon>
#include <QObject>
#include <QSize>
#include <QString>
#include <optional>

#include "application/settings/UiPreferences.h"

namespace uwf::ui {

// Semantic color roles for custom painting and rich text.
enum class Sem {
  Fg,       // Primary text.
  FgMuted,  // Secondary text.
  Accent,   // Accent and selection.
  Warn,     // Warnings.
  Danger,   // Errors and removals.
  AddOk,    // Success and additions.
  Surface,
  Bg,
  Border,
  BarBg,
};

// GUI-thread owner of the application palette, style and themed assets.
class ThemeManager : public QObject, private QAbstractNativeEventFilter {
  Q_OBJECT
 public:
  static ThemeManager& instance();

  [[nodiscard]] app::Theme current() const { return m_theme; }
  [[nodiscard]] bool isLight() const { return m_theme == app::Theme::Light; }
  [[nodiscard]] app::Theme systemTheme() const { return m_systemTheme; }

  void apply(app::Theme t);
  // Uses the override or Windows app preference. Runtime changes do not persist
  // this preference or alter the independently observed shell theme.
  void applyStartupTheme(std::optional<app::Theme> overrideTheme);
  void toggle();
  void requestToggle();

  // Reads AppsUseLightTheme; missing or unreadable values use Dark.
  static app::Theme detectSystemTheme();
  // Reads the independent SystemUsesLightTheme preference for shell surfaces.
  static app::Theme detectSystemShellTheme();

  // Recolors the SVG foreground marker (#E8EAED); align positions it within the icon slot.
  [[nodiscard]] QIcon icon(const QString& resourcePath, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter) const;

  static QIcon iconWithColor(const QString& resourcePath, const QColor& fg, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter);

  [[nodiscard]] QColor color(Sem s) const;
  [[nodiscard]] QString styleSheet(const QString& resourcePath) const;

 signals:
  // Fully qualified parameter names preserve type identity in queued connections.
  void themeChanged(uwf::app::Theme t);
  void systemThemeChanged(uwf::app::Theme t);

 private:
  ThemeManager();
  bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
  void refreshSystemTheme();

  app::Theme m_theme = app::Theme::Dark;
  app::Theme m_systemTheme = app::Theme::Dark;
  std::optional<app::Theme> m_requestedTheme;
  bool m_applied = false;
};

}  // namespace uwf::ui
