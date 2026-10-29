// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QSet>
#include <QTemporaryDir>
#include <QTextStream>
#include <stdexcept>
#include <utility>

#include "CatalogCheck.h"
#include "app/settings/UiPreferences.h"

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QCommandLineParser parser;
  parser.setApplicationDescription(QStringLiteral("Validate translation artifacts using Qt source extraction and TS XML."));
  parser.addHelpOption();
  parser.addOption({QStringLiteral("root"), QStringLiteral("Project source directory."), QStringLiteral("directory")});
  parser.addOption({QStringLiteral("lupdate"), QStringLiteral("Qt lupdate executable."), QStringLiteral("executable")});
  parser.addOption({QStringLiteral("source-catalog"), QStringLiteral("Already extracted source TS instead of running lupdate."), QStringLiteral("file")});
  parser.addOption({QStringLiteral("locales"), QStringLiteral("Comma-separated CMake catalog locales."), QStringLiteral("codes")});
  parser.addOption({QStringLiteral("qt-source"), QStringLiteral("Optional Qt source tree for upstream context comparison."), QStringLiteral("directory")});
  parser.process(app);
  QTextStream output(stdout);
  QTextStream errors(stderr);
  try {
    if (!parser.isSet(QStringLiteral("root")) || (!parser.isSet(QStringLiteral("lupdate")) && !parser.isSet(QStringLiteral("source-catalog"))))
      throw std::runtime_error("Specify --root and either --lupdate or --source-catalog.");
    QTemporaryDir temporary;
    if (!temporary.isValid()) throw std::runtime_error("Cannot create extraction directory.");
    const QDir root(parser.value(QStringLiteral("root")));
    QString extracted = parser.value(QStringLiteral("source-catalog"));
    if (parser.isSet(QStringLiteral("lupdate"))) {
      extracted = temporary.filePath(QStringLiteral("source.ts"));
      QProcess process;
      process.start(parser.value(QStringLiteral("lupdate")),
                    {root.filePath(QStringLiteral("main.cpp")), root.filePath(QStringLiteral("src")), QStringLiteral("-no-obsolete"),
                     QStringLiteral("-locations"), QStringLiteral("none"), QStringLiteral("-ts"), extracted});
      if (!process.waitForFinished(120000) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        throw std::runtime_error((QStringLiteral("lupdate failed: ") + QString::fromUtf8(process.readAllStandardError())).toStdString());
    }
    using namespace uwf::test::tools;
    const auto reference = readCatalog(extracted);
    QStringList locales;
    for (const auto& language : uwf::app::languages())
      if (language.id != uwf::app::LanguageId::En) locales.append(QString(language.code));
    if (parser.isSet(QStringLiteral("locales"))) {
      const auto configured = parser.value(QStringLiteral("locales")).split(u',', Qt::SkipEmptyParts);
      if (configured.size() != locales.size() || QSet<QString>(configured.begin(), configured.end()) != QSet<QString>(locales.begin(), locales.end()))
        throw std::runtime_error("Configured catalogs differ from runtime languages.");
    }
    bool passed = true;
    const QDir catalogs(root.filePath(QStringLiteral("resources/i18n")));
    const auto qtReference = readCatalog(catalogs.filePath(QStringLiteral("qt/qtbase_zh_CN.ts")));
    const QString upstream = parser.value(QStringLiteral("qt-source"));
    for (const auto& code : std::as_const(locales)) {
      const auto catalog = readCatalog(catalogs.filePath(code + QStringLiteral(".ts")));
      auto problems = checkCatalog(catalog, reference, code);
      const QString qtName = QStringLiteral("qt/qtbase_") + code + QStringLiteral(".ts");
      const auto qtCatalog = readCatalog(catalogs.filePath(qtName));
      problems.append(checkCatalog(qtCatalog, qtReference, code, true));
      const bool local = code == QLatin1String("id") || code == QLatin1String("vi");
      if (!local && !upstream.isEmpty()) {
        const QDir translations(QDir(upstream).filePath(QStringLiteral("qttranslations/translations")));
        QString sourceName = QStringLiteral("qtbase_") + code + QStringLiteral(".ts");
        if (code == QLatin1String("pt_PT") && !translations.exists(sourceName)) sourceName = QStringLiteral("qtbase_pt_BR.ts");
        const auto upstreamCatalog = readCatalog(translations.filePath(sourceName));
        for (const auto& [key, message] : qtCatalog.messages) {
          const auto found = upstreamCatalog.messages.find(key);
          if (found == upstreamCatalog.messages.end() || found->second.plural != message.plural)
            problems.append(QStringLiteral("Qt message differs from upstream source: ") + std::get<1>(key));
        }
      }
      for (const auto& problem : std::as_const(problems)) errors << code << ": " << problem << '\n';
      passed = passed && problems.isEmpty();
      output << code << ": " << reference.messages.size() << " active messages (" << catalog.messages.size() << " catalog entries)\n";
    }
    const QDir qtDirectory(catalogs.filePath(QStringLiteral("qt")));
    QSet<QString> expectedAppFiles;
    QSet<QString> expectedFiles;
    for (const auto& code : std::as_const(locales)) {
      expectedAppFiles.insert(code + QStringLiteral(".ts"));
      expectedFiles.insert(QStringLiteral("qtbase_") + code + QStringLiteral(".ts"));
    }
    const auto appFiles = catalogs.entryList({QStringLiteral("*.ts")}, QDir::Files);
    if (QSet<QString>(appFiles.begin(), appFiles.end()) != expectedAppFiles) throw std::runtime_error("Application catalogs differ from runtime languages.");
    const auto files = qtDirectory.entryList({QStringLiteral("qtbase_*.ts")}, QDir::Files);
    if (QSet<QString>(files.begin(), files.end()) != expectedFiles) throw std::runtime_error("Qt catalogs differ from runtime languages.");
    return passed ? 0 : 1;
  } catch (const std::exception& error) {
    errors << error.what() << '\n';
    return 2;
  }
}
