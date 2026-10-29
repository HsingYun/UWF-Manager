// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTextStream>
#include <QUrl>
#include <algorithm>

namespace {
QString identifier(const QString& name) {
  QString result;
  bool capitalize = true;
  for (const auto ch : name) {
    if (ch == QLatin1Char('-')) {
      capitalize = true;
      continue;
    }
    result += capitalize ? ch.toUpper() : ch;
    capitalize = false;
  }
  return result;
}
bool write(const QString& path, const QByteArray& bytes) {
  if (!QDir().mkpath(QFileInfo(path).absolutePath())) return false;
  QFile previous(path);
  if (previous.open(QIODevice::ReadOnly) && previous.readAll() == bytes) return true;
  previous.close();
  QSaveFile output(path);
  return output.open(QIODevice::WriteOnly) && output.write(bytes) == bytes.size() && output.commit();
}
}  // namespace
int main(int argc, char** argv) {
  QCoreApplication application(argc, argv);
  const auto args = application.arguments();
  if (args.size() != 4) {
    QTextStream(stderr) << "Usage: UwfDesignCompiler design.json header.h browser.js\n";
    return 2;
  }
  QFile input(args[1]);
  if (!input.open(QIODevice::ReadOnly)) {
    QTextStream(stderr) << "Cannot read design source\n";
    return 1;
  }
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(input.readAll(), &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) {
    QTextStream(stderr) << "Invalid design JSON\n";
    return 1;
  }
  const auto root = document.object();
  const auto palettes = root["colors"].toObject();
  const auto light = palettes["light"].toObject(), dark = palettes["dark"].toObject();
  const auto names = light.keys();
  if (names.isEmpty() || names != dark.keys()) {
    QTextStream(stderr) << "Theme color roles must match\n";
    return 1;
  }
  QString header = QStringLiteral(
      "// Generated from resources/styles/design.json.\n#pragma once\n#include <array>\n#include <cstdint>\nnamespace uwf::ui::design {\nenum class Color {\n");
  for (const auto& name : names) header += identifier(name) + QStringLiteral(",\n");
  header += QStringLiteral("Count };\ninline constexpr std::array<const char*, %1> colorNames{\n").arg(names.size());
  for (const auto& name : names) header += u'"' + name + QStringLiteral("\",\n");
  header += QStringLiteral("};\n");
  for (const auto& theme : {QStringLiteral("light"), QStringLiteral("dark")}) {
    header += QStringLiteral("inline constexpr std::array<std::uint32_t, %1> %2Colors{\n").arg(names.size()).arg(theme);
    const auto values = palettes[theme].toObject();
    for (const auto& name : names) {
      const auto text = values[name].toString();
      bool valid = false;
      const auto rgb = text.mid(1).toUInt(&valid, 16);
      if (!valid || text.size() != 7 || !text.startsWith(QLatin1Char('#'))) {
        QTextStream(stderr) << "Invalid theme color\n";
        return 1;
      }
      header += QStringLiteral("0xFF%1u,\n").arg(rgb, 6, 16, QLatin1Char('0'));
    }
    header += QStringLiteral("};\n");
  }
  const auto metrics = root["metrics"].toObject();
  for (auto it = metrics.begin(); it != metrics.end(); ++it) {
    if (!it.value().isDouble() || it.value().toDouble() != it.value().toInt()) {
      QTextStream(stderr) << "Design metrics must be integers\n";
      return 1;
    }
    header += QStringLiteral("inline constexpr int %1 = %2;\n").arg(it.key()).arg(it.value().toInt());
  }
  header += QStringLiteral("}\n");
  auto webDesign = root;
  QJsonObject scrollbarImages;
  QDir resources(QFileInfo(input).absoluteDir());
  resources.cdUp();
  for (const auto& theme : {QStringLiteral("light"), QStringLiteral("dark")}) {
    QJsonObject images;
    const auto palette = palettes[theme].toObject();
    for (const auto& direction : {QStringLiteral("up"), QStringLiteral("down"), QStringLiteral("left"), QStringLiteral("right")}) {
      QFile icon(resources.filePath(QStringLiteral("icons/scroll_%1.svg").arg(direction)));
      if (!icon.open(QIODevice::ReadOnly)) {
        QTextStream(stderr) << "Cannot read shared scrollbar SVG\n";
        return 1;
      }
      const auto svg = QString::fromUtf8(icon.readAll());
      for (const bool hover : {false, true}) {
        const auto ink = palette[hover ? "scrollbar-thumb-hover" : "scrollbar-thumb"].toString();
        const auto tinted = QString(svg).replace(QStringLiteral("fill=\"#000\""), QStringLiteral("fill=\"%1\"").arg(ink));
        const QString name = QStringLiteral("scrollbar-%1%2").arg(direction, hover ? QStringLiteral("-hover") : QString{});
        images[name] = QStringLiteral("data:image/svg+xml,") + QString::fromLatin1(QUrl::toPercentEncoding(tinted));
      }
    }
    scrollbarImages[theme] = images;
  }
  webDesign["scrollbarImages"] = scrollbarImages;
  QByteArray browser = "globalThis.UWF_DESIGN = " + QJsonDocument(webDesign).toJson(QJsonDocument::Compact) + ";\n";
  browser +=
      R"JS((() => { const s = document.createElement("style"); const metrics = Object.entries(UWF_DESIGN.metrics).map(([name, value]) => `--${name.replace(/Ms$/, "").replace(/[A-Z]/g, c => `-${c.toLowerCase()}`)}: ${value}${name.endsWith("Ms") ? "ms" : "px"};`).join(" "); s.textContent = `:root { ${metrics} }\n` + Object.entries(UWF_DESIGN.colors).map(([theme, colors]) => `[data-theme="${theme}"] { ${Object.entries(colors).map(([name, value]) => `--${name}: ${value};`).join(" ")} ${Object.entries(UWF_DESIGN.scrollbarImages[theme]).map(([name, value]) => `--${name}: url("${value}");`).join(" ")} }`).join("\n"); document.head.append(s); })();
)JS";
  if (!write(args[2], header.toUtf8()) || !write(args[3], browser)) {
    QTextStream(stderr) << "Cannot write generated design files\n";
    return 1;
  }
  return 0;
}
