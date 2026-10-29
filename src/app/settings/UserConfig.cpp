// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UserConfig.h"

#include <QDir>
#include <QMetaType>
#include <QSettings>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

#include "shared/Log.h"

namespace uwf::app {

namespace {

std::optional<QString> readOption(const QSettings& settings, const QString& key) {
  if (!settings.contains(key)) return std::nullopt;
  const auto value = settings.value(key);
  if (value.metaType() != QMetaType::fromType<QString>()) {
    UWF_LOG_W("config") << "configuration option ignored: path=" << settings.fileName().toStdString() << " key=" << key.toStdString()
                        << " reason=expected-single-string";
    return std::nullopt;
  }
  return value.toString().trimmed().toLower();
}

}  // namespace

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

UserConfig readUserConfig(const QString& applicationDirectory) {
  const QString path = userConfigPath(applicationDirectory);

  // 显式指定 INI 文件，禁止平台默认存储和组织级回退。这里只调用读取接口，
  // 不产生待保存的变更，因此析构也不会创建配置、锁文件或回写用户文件。
  QSettings settings(path, QSettings::IniFormat);
  settings.setFallbacksEnabled(false);
  const auto keys = settings.allKeys();
  if (settings.status() != QSettings::NoError) {
    UWF_LOG_W("config") << "user configuration ignored: path=" << path.toStdString()
                        << " reason=" << (settings.status() == QSettings::FormatError ? "invalid-ini" : "read-failed");
    return {};
  }

  for (const auto& key : keys) {
    if (key.compare(QStringLiteral("UI/Theme"), Qt::CaseInsensitive) != 0 && key.compare(QStringLiteral("UI/Language"), Qt::CaseInsensitive) != 0 &&
        key.compare(QStringLiteral("UI/HubVisible"), Qt::CaseInsensitive) != 0) {
      UWF_LOG_W("config") << "unknown configuration key ignored: path=" << path.toStdString() << " key=" << key.toStdString();
    }
  }
  UserConfig config;
  if (const auto theme = readOption(settings, QStringLiteral("UI/Theme"))) {
    if (*theme == QStringLiteral("dark"))
      config.theme = Theme::Dark;
    else if (*theme == QStringLiteral("light"))
      config.theme = Theme::Light;
    else if (*theme != QStringLiteral("system"))
      UWF_LOG_W("config") << "configuration option ignored: path=" << path.toStdString() << " key=UI/Theme value=" << theme->toStdString()
                          << " expected=system|light|dark";
  }
  if (const auto language = readOption(settings, QStringLiteral("UI/Language"))) {
    if (*language != QStringLiteral("system")) {
      config.language = parseLanguageCode(*language);
      if (!config.language)
        UWF_LOG_W("config") << "configuration option ignored: path=" << path.toStdString() << " key=UI/Language value=" << language->toStdString()
                            << " reason=unsupported-language";
    }
  }
  if (const auto hubVisible = readOption(settings, QStringLiteral("UI/HubVisible"))) {
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
