// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QScopeGuard>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>
#include <array>
#include <optional>
#include <stdexcept>
#include <string>

#include "application/settings/UserConfig.h"
#include "foundation/logging/Log.h"
#include "infrastructure/windows/settings/IniUserConfig.h"

namespace {
class IniUserConfigTests final : public QObject {
  Q_OBJECT
 private slots:
  void userConfigReadsOnlyRecognizedValues_data();
  void userConfigReadsOnlyRecognizedValues();
  void userConfigMissingFileDoesNotCreateStorage();
  void userConfigIgnoresWorkingDirectoryAndRejectsRelativeLocations();
  void userConfigMalformedFileFallsBackWithoutChangingTheInput();
  void userConfigInvalidOptionsAreLoggedIndependently();
  void userConfigSerializationRoundTripsSupportedPreferences();
};

void IniUserConfigTests::userConfigReadsOnlyRecognizedValues_data() {
  using namespace uwf::app;
  QTest::addColumn<QByteArray>("contents");
  QTest::addColumn<std::optional<Theme>>("expectedTheme");
  QTest::addColumn<std::optional<LanguageId>>("expectedLanguage");
  QTest::addColumn<bool>("expectedHubVisible");
  struct Case {
    const char* name;
    QByteArray contents;
    std::optional<Theme> theme;
    std::optional<LanguageId> language;
    bool hubVisible = true;
  };
  const std::array cases{Case{"empty", {}, {}, {}},
                         Case{"system-defaults", "[UI]\nTheme=system\nLanguage=system\n", {}, {}},
                         Case{"dark-english-hidden", "[UI]\nTheme=dark\nLanguage=en\nHubVisible=false\n", Theme::Dark, LanguageId::En, false},
                         Case{"light-chinese-visible", "[UI]\nTheme=light\nLanguage=zh_CN\nHubVisible=true\n", Theme::Light, LanguageId::Zh_CN},
                         Case{"quoted-case-and-whitespace", "; preferences\r\n[ui]\r\ntHeMe = \" LIGHT \"\r\nlAnGuAgE = \" ZH-cn \"\r\nhUbViSiBlE=FaLsE\r\n",
                              Theme::Light, LanguageId::Zh_CN, false},
                         Case{"utf8-bom", QByteArray::fromHex("efbbbf") + "[UI]\nTheme=dark\nLanguage=ja\n", Theme::Dark, LanguageId::Ja},
                         Case{"invalid-values", "[UI]\nTheme=automatic\nLanguage=unsupported\nHubVisible=off\n", {}, {}},
                         Case{"invalid-theme-is-independent", "[UI]\nTheme=invalid\nLanguage=en\nHubVisible=false\n", {}, LanguageId::En, false},
                         Case{"invalid-language-is-independent", "[UI]\nTheme=dark\nLanguage=unsupported\nHubVisible=false\n", Theme::Dark, {}, false},
                         Case{"invalid-hub-is-independent", "[UI]\nTheme=light\nLanguage=en\nHubVisible=system\n", Theme::Light, LanguageId::En},
                         Case{"theme-list", "[UI]\nTheme=dark,light\nLanguage=en\nHubVisible=false\n", {}, LanguageId::En, false},
                         Case{"language-byte-array", "[UI]\nTheme=dark\nLanguage=@ByteArray(en)\n", Theme::Dark, {}},
                         Case{"hub-invalid-variant", "[UI]\nTheme=light\nLanguage=en\nHubVisible=@Invalid()\n", Theme::Light, LanguageId::En},
                         Case{"empty-values", "[UI]\nTheme=\nLanguage=\nHubVisible=\n", {}, {}},
                         Case{"unknown-keys", "[UI]\nTheme=dark\nLanguage=en\nOther=yes\n[Network]\nEnabled=true\n", Theme::Dark, LanguageId::En},
                         Case{"wrong-section", "[Other]\nTheme=dark\nLanguage=en\nHubVisible=false\n", {}, {}},
                         Case{"unscoped-keys", "Theme=dark\nLanguage=en\nHubVisible=false\n", {}, {}}};
  for (const auto& entry : cases) QTest::newRow(entry.name) << entry.contents << entry.theme << entry.language << entry.hubVisible;
  for (const auto& language : languages()) {
    const auto code = QString(language.code).toUtf8();
    QTest::newRow(("language-" + code).constData()) << QByteArray("[UI]\nLanguage=") + code + '\n'
                                                    << std::optional<Theme>{} << std::optional{language.id} << true;
  }
}

void IniUserConfigTests::userConfigReadsOnlyRecognizedValues() {
  QFETCH(QByteArray, contents);
  QFETCH(std::optional<uwf::app::Theme>, expectedTheme);
  QFETCH(std::optional<uwf::app::LanguageId>, expectedLanguage);
  QFETCH(bool, expectedHubVisible);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(uwf::app::userConfigPath(directory.path()));
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  const auto config = uwf::windows::readIniUserConfig(directory.path());
  QCOMPARE(config.theme, expectedTheme);
  QCOMPARE(config.language, expectedLanguage);
  QCOMPARE(config.hubVisible, expectedHubVisible);
  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
  QCOMPARE(QDir(directory.path()).entryList(QDir::Files), QStringList{QStringLiteral("UWF.ini")});
}

void IniUserConfigTests::userConfigMissingFileDoesNotCreateStorage() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const auto decoyPath = directory.filePath(QStringLiteral("UWF.ini.txt"));
  QFile decoy(decoyPath);
  QVERIFY(decoy.open(QIODevice::WriteOnly));
  const QByteArray contents("[UI]\nTheme=dark\n");
  QCOMPARE(decoy.write(contents), contents.size());
  decoy.close();
  uwf::clearLogLines();
  const auto config = uwf::windows::readIniUserConfig(directory.path());
  QVERIFY(!config.theme && !config.language);
  QVERIFY(config.hubVisible);
  QCOMPARE(QDir(directory.path()).entryList(QDir::Files), QStringList{QStringLiteral("UWF.ini.txt")});
  const auto missingDirectory = directory.filePath(QStringLiteral("absent/application"));
  const auto missing = uwf::windows::readIniUserConfig(missingDirectory);
  QVERIFY(!missing.theme && !missing.language);
  QVERIFY(missing.hubVisible);
  QVERIFY(!QFileInfo::exists(missingDirectory));
  QVERIFY(uwf::recentLogLines().empty());
  QVERIFY(decoy.open(QIODevice::ReadOnly));
  QCOMPARE(decoy.readAll(), contents);
}

void IniUserConfigTests::userConfigIgnoresWorkingDirectoryAndRejectsRelativeLocations() {
  QTemporaryDir applicationDirectory;
  QTemporaryDir workingDirectory;
  QVERIFY(applicationDirectory.isValid() && workingDirectory.isValid());
  QFile decoy(workingDirectory.filePath(QStringLiteral("UWF.ini")));
  QVERIFY(decoy.open(QIODevice::WriteOnly));
  const QByteArray contents("[UI]\nTheme=dark\nLanguage=en\n");
  QCOMPARE(decoy.write(contents), contents.size());
  decoy.close();
  const auto originalDirectory = QDir::currentPath();
  const auto restoreDirectory = qScopeGuard([&] { QDir::setCurrent(originalDirectory); });
  QVERIFY(QDir::setCurrent(workingDirectory.path()));
  const auto config = uwf::windows::readIniUserConfig(applicationDirectory.path());
  QVERIFY(!config.theme && !config.language);
  QVERIFY(config.hubVisible);
  QVERIFY(QDir(applicationDirectory.path()).isEmpty());
  for (const auto& relative : {QString{}, QStringLiteral("."), QStringLiteral("relative/path"), QStringLiteral("C:relative/path")}) {
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::windows::readIniUserConfig(relative)));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::userConfigPath(relative)));
  }
  QVERIFY(decoy.open(QIODevice::ReadOnly));
  QCOMPARE(decoy.readAll(), contents);
}

void IniUserConfigTests::userConfigMalformedFileFallsBackWithoutChangingTheInput() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(uwf::app::userConfigPath(directory.path()));
  QVERIFY(file.open(QIODevice::WriteOnly));
  const QByteArray contents("[UI]\nTheme=dark\nLanguage=zh_CN\nHubVisible=false\n[unterminated\n");
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  uwf::clearLogLines();
  const auto config = uwf::windows::readIniUserConfig(directory.path());
  QVERIFY(!config.theme && !config.language);
  QVERIFY(config.hubVisible);
  const auto lines = uwf::recentLogLines();
  QVERIFY(std::ranges::any_of(lines, [](const auto& line) { return line.find("invalid-ini") != std::string::npos; }));
  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
}

void IniUserConfigTests::userConfigInvalidOptionsAreLoggedIndependently() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(uwf::app::userConfigPath(directory.path()));
  QVERIFY(file.open(QIODevice::WriteOnly));
  const QByteArray contents("[UI]\nTheme=night\nLanguage=zh_CN\nHubVisible=system\nOther=ignored\n");
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  uwf::clearLogLines();
  const auto config = uwf::windows::readIniUserConfig(directory.path());
  QVERIFY(!config.theme);
  QCOMPARE(config.language, std::optional{uwf::app::LanguageId::Zh_CN});
  QVERIFY(config.hubVisible);
  const auto lines = uwf::recentLogLines();
  for (const auto* key : {"key=UI/Theme", "key=UI/HubVisible", "key=UI/Other"})
    QVERIFY(std::ranges::any_of(lines, [key](const auto& line) { return line.find(key) != std::string::npos; }));
}

void IniUserConfigTests::userConfigSerializationRoundTripsSupportedPreferences() {
  using namespace uwf::app;
  QCOMPARE(serializeUserConfig({}), QStringLiteral("[UI]\nTheme=system\nLanguage=system\nHubVisible=true\n"));
  const std::array configs{UserConfig{}, UserConfig{Theme::Dark, LanguageId::En, false}, UserConfig{Theme::Light, LanguageId::Zh_CN, true},
                           UserConfig{{}, LanguageId::Ja, false}};
  for (const auto& expected : configs) {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QFile file(userConfigPath(directory.path()));
    QVERIFY(file.open(QIODevice::WriteOnly));
    const auto text = serializeUserConfig(expected).toUtf8();
    QCOMPARE(file.write(text), text.size());
    file.close();
    const auto actual = uwf::windows::readIniUserConfig(directory.path());
    QCOMPARE(actual.theme, expected.theme);
    QCOMPARE(actual.language, expected.language);
    QCOMPARE(actual.hubVisible, expected.hubVisible);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), text);
  }
}
}  // namespace
QTEST_APPLESS_MAIN(IniUserConfigTests)
#include "IniUserConfigTests.moc"
