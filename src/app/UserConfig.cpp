/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "UserConfig.h"

#include <QDir>
#include <QMetaType>
#include <QSettings>
#include <stdexcept>

#include "../util/Log.h"

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
  const QString normalized = value.toString().trimmed().toLower();
  if (normalized == QStringLiteral("system")) return std::nullopt;
  return normalized;
}

}  // namespace

UserConfig readUserConfig(const QString& applicationDirectory) {
  if (!QDir::isAbsolutePath(applicationDirectory)) throw std::invalid_argument("user configuration directory must be absolute");
  const QString path = QDir(applicationDirectory).filePath(QStringLiteral("UWF.ini"));

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
    if (key.compare(QStringLiteral("UI/Theme"), Qt::CaseInsensitive) != 0 && key.compare(QStringLiteral("UI/Language"), Qt::CaseInsensitive) != 0) {
      UWF_LOG_W("config") << "unknown configuration key ignored: path=" << path.toStdString() << " key=" << key.toStdString();
    }
  }
  UserConfig config;
  if (const auto theme = readOption(settings, QStringLiteral("UI/Theme"))) {
    if (*theme == QStringLiteral("dark"))
      config.theme = ui::Theme::Dark;
    else if (*theme == QStringLiteral("light"))
      config.theme = ui::Theme::Light;
    else
      UWF_LOG_W("config") << "configuration option ignored: path=" << path.toStdString() << " key=UI/Theme value=" << theme->toStdString()
                          << " expected=system|light|dark";
  }
  if (const auto language = readOption(settings, QStringLiteral("UI/Language"))) {
    if (*language == QStringLiteral("en"))
      config.language = ui::I18n::Lang::En;
    else if (*language == QStringLiteral("zh_cn"))
      config.language = ui::I18n::Lang::Zh_CN;
    else
      UWF_LOG_W("config") << "configuration option ignored: path=" << path.toStdString() << " key=UI/Language value=" << language->toStdString()
                          << " expected=system|en|zh_CN";
  }
  return config;
}

}  // namespace uwf::app
