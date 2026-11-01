// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/settings/UserConfig.h"

#include <QDir>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

#include "foundation/logging/Log.h"

namespace uwf::app {
QString userConfigPath(const QString& applicationDirectory) {
  if (!std::filesystem::path(applicationDirectory.toStdWString()).is_absolute() || applicationDirectory.contains(QChar::Null))
    throw std::invalid_argument("user configuration directory must be absolute");
  return QDir(applicationDirectory).filePath(QStringLiteral("UWF.ini"));
}

QString serializeUserConfig(const UserConfig& config) {
  QString theme = QStringLiteral("system");
  if (config.theme) {
    switch (*config.theme) {
      case Theme::Dark:
        theme = QStringLiteral("dark");
        break;
      case Theme::Light:
        theme = QStringLiteral("light");
        break;
      default:
        throw std::invalid_argument("unsupported user configuration theme");
    }
  }
  QString language = QStringLiteral("system");
  if (config.language) {
    const auto supported = languages();
    const auto selected = std::ranges::find_if(supported, [&](const auto& candidate) { return candidate.id == *config.language; });
    if (selected == supported.end()) throw std::invalid_argument("unsupported user configuration language");
    language = QString(selected->code);
  }
  return QStringLiteral("[UI]\nTheme=%1\nLanguage=%2\nHubVisible=%3\n")
      .arg(theme, language, config.hubVisible ? QStringLiteral("true") : QStringLiteral("false"));
}

UserConfig interpretUserConfig(const UserConfigOptions& options, const QString& path) {
  UserConfig config;
  if (const auto theme = options.theme ? std::optional{options.theme->trimmed().toLower()} : std::nullopt) {
    if (*theme == QStringLiteral("dark"))
      config.theme = Theme::Dark;
    else if (*theme == QStringLiteral("light"))
      config.theme = Theme::Light;
    else if (*theme != QStringLiteral("system"))
      UWF_LOG_W("config") << "configuration option ignored: path=" << path.toStdString() << " key=UI/Theme value=" << theme->toStdString()
                          << " expected=system|light|dark";
  }
  if (const auto language = options.language ? std::optional{options.language->trimmed().toLower()} : std::nullopt) {
    if (*language != QStringLiteral("system")) {
      config.language = parseLanguageCode(*language);
      if (!config.language)
        UWF_LOG_W("config") << "configuration option ignored: path=" << path.toStdString() << " key=UI/Language value=" << language->toStdString()
                            << " reason=unsupported-language";
    }
  }
  if (const auto hubVisible = options.hubVisible ? std::optional{options.hubVisible->trimmed().toLower()} : std::nullopt) {
    if (*hubVisible == QStringLiteral("true"))
      config.hubVisible = true;
    else if (*hubVisible == QStringLiteral("false"))
      config.hubVisible = false;
    else
      UWF_LOG_W("config") << "configuration option ignored: path=" << path.toStdString() << " key=UI/HubVisible value=" << hubVisible->toStdString()
                          << " expected=true|false";
  }
  return config;
}

}  // namespace uwf::app
