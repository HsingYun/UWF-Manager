// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/settings/IniUserConfig.h"

#include <QFileInfo>
#include <QMetaType>
#include <QSettings>
#include <QVariant>

#include "foundation/logging/Log.h"

namespace uwf::windows {
namespace {
std::optional<QString> readOption(const QSettings& settings, const QString& path, const QString& key) {
  if (!settings.contains(key)) return std::nullopt;
  const auto value = settings.value(key);
  if (value.metaType() != QMetaType::fromType<QString>()) {
    UWF_LOG_W("config") << "configuration option ignored: path=" << path.toStdString() << " key=" << key.toStdString() << " reason=expected-single-string";
    return std::nullopt;
  }
  return value.toString();
}
}  // namespace

app::UserConfig readIniUserConfig(const QString& applicationDirectory) {
  const QString path = app::userConfigPath(applicationDirectory);
  if (!QFileInfo::exists(path)) return {};
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
        key.compare(QStringLiteral("UI/HubVisible"), Qt::CaseInsensitive) != 0)
      UWF_LOG_W("config") << "unknown configuration key ignored: path=" << path.toStdString() << " key=" << key.toStdString();
  }
  return app::interpretUserConfig({readOption(settings, path, QStringLiteral("UI/Theme")), readOption(settings, path, QStringLiteral("UI/Language")),
                                   readOption(settings, path, QStringLiteral("UI/HubVisible"))},
                                  path);
}
}  // namespace uwf::windows
