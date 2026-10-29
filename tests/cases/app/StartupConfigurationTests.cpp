// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <windows.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>

#include "app/settings/UserConfig.h"
#include "app/startup/StartupOptions.h"
#include "shared/Log.h"

namespace {

class StartupConfigurationTests final : public QObject {
  Q_OBJECT

 private slots:
  void startupOptionsClassifyEveryExclusiveMode();
  void userConfigReadsOnlyRecognizedValues_data();
  void userConfigReadsOnlyRecognizedValues();
  void userConfigMissingFileDoesNotCreateStorage();
  void userConfigIgnoresWorkingDirectoryAndRejectsRelativeLocations();
  void userConfigMalformedFileFallsBackWithoutDiscardingTheUserFile();
  void userConfigInvalidOptionsAreLoggedIndependently();
  void userConfigReadFailuresFallBackWithoutChangingPermissions();
  void userConfigSerializationRoundTripsSupportedPreferences();
  void userConfigSerializationRejectsInvalidPreferenceValues();
  void languageMatchingRespectsPreferenceOrderScriptAndRegion_data();
  void languageMatchingRespectsPreferenceOrderScriptAndRegion();
};

void StartupConfigurationTests::startupOptionsClassifyEveryExclusiveMode() {
  using uwf::app::StartupMode;

  QCOMPARE(uwf::app::parseStartupOptions({}).mode, StartupMode::Interactive);
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe")}).mode, StartupMode::Interactive);
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--quiet")}).mode, StartupMode::Quiet);
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--commit-stage")}).mode, StartupMode::CommitStage);
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--service")}).mode, StartupMode::Service);
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--install")}).mode, StartupMode::InstallService);
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--uninstall")}).mode, StartupMode::UninstallService);

  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--quiet")}).initialCommand(),
           uwf::app::ApplicationCommandKind::EnsureRunning);
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--commit-stage")}).initialCommand(),
           uwf::app::ApplicationCommandKind::CommitStage);
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe")}).initialCommand(), uwf::app::ApplicationCommandKind::Activate);
  for (const StartupMode mode : {StartupMode::Service, StartupMode::InstallService, StartupMode::UninstallService}) {
    QVERIFY_THROWS_EXCEPTION(uwf::app::StartupOptionsError, static_cast<void>(uwf::app::StartupOptions{mode}.initialCommand()));
  }

  for (const QString& unsupported :
       {QStringLiteral("--quite"), QStringLiteral("--Quiet"), QStringLiteral("--quiet=true"), QStringLiteral(" --quiet"), QStringLiteral("--other")}) {
    QVERIFY_THROWS_EXCEPTION(uwf::app::StartupOptionsError, static_cast<void>(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), unsupported})));
  }
  QVERIFY_THROWS_EXCEPTION(
      uwf::app::StartupOptionsError,
      static_cast<void>(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--quiet"), QStringLiteral("--commit-stage")})));
  QVERIFY_THROWS_EXCEPTION(
      uwf::app::StartupOptionsError,
      static_cast<void>(uwf::app::parseStartupOptions({QStringLiteral("UWF.exe"), QStringLiteral("--service"), QStringLiteral("--service")})));

  // 首项是可执行文件名，不得被当作用户参数。
  QCOMPARE(uwf::app::parseStartupOptions({QStringLiteral("--quiet")}).mode, StartupMode::Interactive);
}

void StartupConfigurationTests::userConfigReadsOnlyRecognizedValues_data() {
  namespace preferences = uwf::app;
  using uwf::app::Theme;
  QTest::addColumn<QByteArray>("contents");
  QTest::addColumn<std::optional<Theme>>("expectedTheme");
  QTest::addColumn<std::optional<uwf::app::LanguageId>>("expectedLanguage");
  QTest::addColumn<bool>("expectedHubVisible");

  struct Case {
    const char* name;
    QByteArray contents;
    std::optional<Theme> theme;
    std::optional<uwf::app::LanguageId> language;
    bool hubVisible = true;
  };
  const std::array cases{
      Case{"empty", {}, {}, {}},
      Case{"comments", "; user preferences\n[UI]\n; startup override\nTheme=dark\n", Theme::Dark, {}},
      Case{"empty-section", "[UI]\n", {}, {}},
      Case{"system", "[UI]\nTheme=system\nLanguage=system\n", {}, {}},
      Case{"dark", "[UI]\nTheme=dark\n", Theme::Dark, {}},
      Case{"light", "[UI]\nTheme=light\n", Theme::Light, {}},
      Case{"english", "[UI]\nLanguage=en\n", {}, uwf::app::LanguageId::En},
      Case{"chinese", "[UI]\nLanguage=zh_CN\n", {}, uwf::app::LanguageId::Zh_CN},
      Case{"independent-overrides", "[UI]\nTheme=light\nLanguage=zh_CN\n", Theme::Light, uwf::app::LanguageId::Zh_CN},
      Case{"mixed-case", "[UI]\nTheme=dArK\nLanguage=ZH_cn\n", Theme::Dark, uwf::app::LanguageId::Zh_CN},
      Case{"key-case", "[ui]\ntHeMe=light\nlAnGuAgE=en\n", Theme::Light, uwf::app::LanguageId::En},
      Case{"spaces-crlf", "[UI]\r\nTheme = light  \r\nLanguage = en \r\n", Theme::Light, uwf::app::LanguageId::En},
      Case{"quoted-values", "[UI]\nTheme=\" dark \"\nLanguage=\" zh_CN \"\n", Theme::Dark, uwf::app::LanguageId::Zh_CN},
      Case{"utf8-bom", QByteArray::fromHex("efbbbf") + "[UI]\r\nTheme=dark\r\nLanguage=en\r\n", Theme::Dark, uwf::app::LanguageId::En},
      Case{"empty-values", "[UI]\nTheme=\nLanguage=\n", {}, {}},
      Case{"unknown-values", "[UI]\nTheme=automatic\nLanguage=unsupported\n", {}, {}},
      Case{"no-substring-matching", "[UI]\nTheme=dark-mode\nLanguage=english\n", {}, {}},
      Case{"invalid-theme-keeps-language", "[UI]\nTheme=invalid\nLanguage=en\n", {}, uwf::app::LanguageId::En},
      Case{"invalid-language-keeps-theme", "[UI]\nTheme=dark\nLanguage=unsupported\n", Theme::Dark, {}},
      Case{"typed-theme-keeps-language", "[UI]\nTheme=dark,light\nLanguage=en\n", {}, uwf::app::LanguageId::En},
      Case{"typed-language-keeps-theme", "[UI]\nTheme=dark\nLanguage=@ByteArray(en)\n", Theme::Dark, {}},
      Case{"empty-theme-keeps-language", "[UI]\nTheme=\nLanguage=zh_CN\n", {}, uwf::app::LanguageId::Zh_CN},
      Case{"whitespace-values", "[UI]\nTheme=\"   \"\nLanguage=\"   \"\n", {}, {}},
      Case{"unknown-keys", "[UI]\nTheme=dark\nLanguage=en\nOther=yes\n[Network]\nEnabled=true\n", Theme::Dark, uwf::app::LanguageId::En},
      Case{"wrong-section", "[Other]\nTheme=dark\nLanguage=en\n", {}, {}},
      Case{"unscoped-keys", "Theme=dark\nLanguage=zh_CN\n", {}, {}},
      Case{"invalid-variant", "[UI]\nTheme=@Invalid()\nLanguage=@Invalid()\n", {}, {}},
      Case{"list-values", "[UI]\nTheme=dark,light\nLanguage=en,zh_CN\n", {}, {}},
      Case{"byte-array-values", "[UI]\nTheme=@ByteArray(dark)\nLanguage=@ByteArray(en)\n", {}, {}},
      Case{"numeric-values", "[UI]\nTheme=1\nLanguage=0\n", {}, {}},
      Case{"non-ascii-values", "[UI]\nTheme=深色\nLanguage=简体中文\n", {}, {}},
      Case{"malformed-utf8-values", QByteArray("[UI]\nTheme=") + char(0xFF) + "\nLanguage=en\n", {}, uwf::app::LanguageId::En},
      Case{"hub-hidden", "[UI]\nHubVisible=false\n", {}, {}, false},
      Case{"hub-visible", "[UI]\nHubVisible=true\n", {}, {}, true},
      Case{"hub-mixed-case", "[ui]\nhUbViSiBlE=FaLsE\n", {}, {}, false},
      Case{"hub-spaces-crlf", "[UI]\r\nHubVisible = \" FALSE \"\r\n", {}, {}, false},
      Case{"hub-with-preferences", "[UI]\nTheme=dark\nLanguage=en\nHubVisible=false\n", Theme::Dark, uwf::app::LanguageId::En, false},
      Case{"hub-keeps-invalid-preferences-independent", "[UI]\nTheme=invalid\nLanguage=invalid\nHubVisible=false\n", {}, {}, false},
      Case{"invalid-hub-keeps-preferences", "[UI]\nTheme=light\nLanguage=zh_CN\nHubVisible=off\n", Theme::Light, uwf::app::LanguageId::Zh_CN},
      Case{"hub-empty", "[UI]\nHubVisible=\n", {}, {}},
      Case{"hub-whitespace", "[UI]\nHubVisible=\"   \"\n", {}, {}},
      Case{"hub-system-is-not-a-boolean", "[UI]\nHubVisible=system\n", {}, {}},
      Case{"hub-zero-is-not-a-boolean", "[UI]\nHubVisible=0\n", {}, {}},
      Case{"hub-one-is-not-a-boolean", "[UI]\nHubVisible=1\n", {}, {}},
      Case{"hub-no-substring-matching", "[UI]\nHubVisible=false-value\n", {}, {}},
      Case{"hub-list", "[UI]\nHubVisible=false,true\n", {}, {}},
      Case{"hub-byte-array", "[UI]\nHubVisible=@ByteArray(false)\n", {}, {}},
      Case{"hub-invalid-variant", "[UI]\nHubVisible=@Invalid()\n", {}, {}},
      Case{"hub-wrong-section", "[Overlay]\nHubVisible=false\n", {}, {}},
      Case{"hub-unscoped", "HubVisible=false\n", {}, {}},
      Case{"hub-malformed-utf8", QByteArray("[UI]\nHubVisible=") + char(0xFF) + "\n", {}, {}},
  };
  for (const auto& entry : cases) QTest::newRow(entry.name) << entry.contents << entry.theme << entry.language << entry.hubVisible;
  for (const auto& language : uwf::app::languages()) {
    const QByteArray code = QString(language.code).toUtf8();
    const QByteArray canonical = "[UI]\nLanguage=" + code + '\n';
    QTest::newRow(("language-" + code).constData()) << canonical << std::optional<Theme>{} << std::optional{language.id} << true;
    const QByteArray normalized = "[UI]\nLanguage=\" " + QString(language.code).replace(u'_', u'-').toUpper().toUtf8() + " \"\n";
    QTest::newRow(("language-normalized-" + code).constData()) << normalized << std::optional<Theme>{} << std::optional{language.id} << true;
  }
}

void StartupConfigurationTests::languageMatchingRespectsPreferenceOrderScriptAndRegion_data() {
  using Lang = uwf::app::LanguageId;
  QTest::addColumn<QStringList>("preferred");
  QTest::addColumn<Lang>("expected");
  QTest::newRow("no-preference") << QStringList{} << Lang::En;
  QTest::newRow("unsupported") << QStringList{QStringLiteral("sv-SE"), QStringLiteral("fi-FI")} << Lang::En;
  QTest::newRow("invalid") << QStringList{QStringLiteral("not-a-language"), QStringLiteral("C")} << Lang::En;
  QTest::newRow("later-supported") << QStringList{QStringLiteral("sv-SE"), QStringLiteral("fr-CA"), QStringLiteral("de-DE")} << Lang::Fr;
  QTest::newRow("english-first") << QStringList{QStringLiteral("en-GB"), QStringLiteral("zh-CN")} << Lang::En;
  QTest::newRow("simplified-cn") << QStringList{QStringLiteral("zh-CN")} << Lang::Zh_CN;
  QTest::newRow("simplified-sg") << QStringList{QStringLiteral("zh-SG")} << Lang::Zh_CN;
  QTest::newRow("simplified-script") << QStringList{QStringLiteral("zh-Hans-HK")} << Lang::Zh_CN;
  QTest::newRow("traditional-tw") << QStringList{QStringLiteral("zh-TW")} << Lang::Zh_TW;
  QTest::newRow("traditional-hk") << QStringList{QStringLiteral("zh-HK")} << Lang::Zh_TW;
  QTest::newRow("traditional-mo") << QStringList{QStringLiteral("zh-MO")} << Lang::Zh_TW;
  QTest::newRow("traditional-script") << QStringList{QStringLiteral("zh-Hant-CN")} << Lang::Zh_TW;
  QTest::newRow("portuguese-br") << QStringList{QStringLiteral("pt-BR")} << Lang::Pt_BR;
  QTest::newRow("portuguese-pt") << QStringList{QStringLiteral("pt-PT")} << Lang::Pt_PT;
  QTest::newRow("portuguese-ao") << QStringList{QStringLiteral("pt-AO")} << Lang::Pt_PT;
  QTest::newRow("spanish-mx") << QStringList{QStringLiteral("es-MX")} << Lang::Es;
  QTest::newRow("german-ch") << QStringList{QStringLiteral("de-CH")} << Lang::De;
  QTest::newRow("arabic-eg") << QStringList{QStringLiteral("ar-EG")} << Lang::Ar;
  for (const auto& language : uwf::app::languages())
    QTest::newRow(QString(language.code).toUtf8().constData()) << QStringList{QString(language.code)} << language.id;
}

void StartupConfigurationTests::languageMatchingRespectsPreferenceOrderScriptAndRegion() {
  QFETCH(QStringList, preferred);
  QFETCH(uwf::app::LanguageId, expected);
  QCOMPARE(uwf::app::matchLanguage(preferred), expected);
}

void StartupConfigurationTests::userConfigReadsOnlyRecognizedValues() {
  QFETCH(QByteArray, contents);
  QFETCH(std::optional<uwf::app::Theme>, expectedTheme);
  QFETCH(std::optional<uwf::app::LanguageId>, expectedLanguage);
  QFETCH(bool, expectedHubVisible);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(directory.filePath(QStringLiteral("UWF.ini")));
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  const auto modifiedBefore = QFileInfo(file).lastModified();

  const auto config = uwf::app::readUserConfig(directory.path());
  QCOMPARE(config.theme, expectedTheme);
  QCOMPARE(config.language, expectedLanguage);
  QCOMPARE(config.hubVisible, expectedHubVisible);
  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
  file.close();
  QCOMPARE(QFileInfo(file).lastModified(), modifiedBefore);
  QCOMPARE(QDir(directory.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System), QStringList{QStringLiteral("UWF.ini")});
}

void StartupConfigurationTests::userConfigMissingFileDoesNotCreateStorage() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  uwf::clearLogLines();
  const auto config = uwf::app::readUserConfig(directory.path());
  QVERIFY(!config.theme && !config.language);
  QVERIFY(config.hubVisible);
  QVERIFY(QDir(directory.path()).isEmpty());
  const QString missingDirectory = directory.filePath(QStringLiteral("absent/application"));
  const auto missing = uwf::app::readUserConfig(missingDirectory);
  QVERIFY(!missing.theme && !missing.language);
  QVERIFY(missing.hubVisible);
  QVERIFY(!QFileInfo::exists(missingDirectory));
  QVERIFY(uwf::recentLogLines().empty());

  // 不扫描其它文件名，也不能把 Windows 隐藏扩展名造成的 UWF.ini.txt 当作配置。
  for (const QString& name : {QStringLiteral("UWF.ini.txt"), QStringLiteral("UWF.1.0.18.ini")}) {
    QFile decoy(directory.filePath(name));
    QVERIFY(decoy.open(QIODevice::WriteOnly));
    const QByteArray contents("[UI]\nTheme=dark\nLanguage=zh_CN\n");
    QCOMPARE(decoy.write(contents), contents.size());
  }
  const auto withoutConfig = uwf::app::readUserConfig(directory.path());
  QVERIFY(!withoutConfig.theme && !withoutConfig.language);
  QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("UWF.ini"))));
  QCOMPARE(QDir(directory.path()).entryList(QDir::Files | QDir::Hidden | QDir::System).size(), 2);
}

void StartupConfigurationTests::userConfigIgnoresWorkingDirectoryAndRejectsRelativeLocations() {
  QTemporaryDir applicationDirectory;
  QTemporaryDir workingDirectory;
  QVERIFY(applicationDirectory.isValid() && workingDirectory.isValid());
  QFile decoy(workingDirectory.filePath(QStringLiteral("UWF.ini")));
  QVERIFY(decoy.open(QIODevice::WriteOnly));
  const QByteArray contents("[UI]\nTheme=dark\nLanguage=zh_CN\n");
  QCOMPARE(decoy.write(contents), contents.size());
  decoy.close();

  const QString originalDirectory = QDir::currentPath();
  const auto restoreDirectory = qScopeGuard([&] { QDir::setCurrent(originalDirectory); });
  QVERIFY(QDir::setCurrent(workingDirectory.path()));
  const auto config = uwf::app::readUserConfig(applicationDirectory.path());
  QVERIFY(!config.theme && !config.language);
  QVERIFY(QDir(applicationDirectory.path()).isEmpty());
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::readUserConfig({})));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::readUserConfig(QStringLiteral("."))));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::readUserConfig(QStringLiteral("relative/path"))));
  for (const auto& relative : {QString{}, QStringLiteral("."), QStringLiteral("relative/path"), QStringLiteral("C:relative/path")})
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::userConfigPath(relative)));
  QVERIFY(decoy.open(QIODevice::ReadOnly));
  QCOMPARE(decoy.readAll(), contents);

  // 应用路径含空格和非 ASCII 字符时仍按绝对目录读取，不能采用工作目录的同名文件。
  const QString nestedDirectory = applicationDirectory.filePath(QStringLiteral("配置 目录"));
  QVERIFY(QDir().mkpath(nestedDirectory));
  QCOMPARE(uwf::app::userConfigPath(nestedDirectory), QDir(nestedDirectory).filePath(QStringLiteral("UWF.ini")));
  QFile actual(QDir(nestedDirectory).filePath(QStringLiteral("UWF.ini")));
  QVERIFY(actual.open(QIODevice::WriteOnly));
  const QByteArray actualContents("[UI]\nTheme=light\nLanguage=en\n");
  QCOMPARE(actual.write(actualContents), actualContents.size());
  actual.close();
  const auto actualConfig = uwf::app::readUserConfig(nestedDirectory);
  QCOMPARE(actualConfig.theme, std::optional{uwf::app::Theme::Light});
  QCOMPARE(actualConfig.language, std::optional{uwf::app::LanguageId::En});
}

void StartupConfigurationTests::userConfigMalformedFileFallsBackWithoutDiscardingTheUserFile() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(directory.filePath(QStringLiteral("UWF.ini")));
  QVERIFY(file.open(QIODevice::WriteOnly));
  // 即使有效选项位于前面，后续节语法损坏也不能把部分解析结果当成有效配置。
  const QByteArray contents("[UI]\nTheme=dark\nLanguage=zh_CN\nHubVisible=false\n[unterminated\n");
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  uwf::clearLogLines();
  const auto config = uwf::app::readUserConfig(directory.path());
  QVERIFY(!config.theme && !config.language);
  QVERIFY(config.hubVisible);
  const auto lines = uwf::recentLogLines();
  QVERIFY(std::ranges::any_of(lines, [](const auto& line) { return line.find("invalid-ini") != std::string::npos; }));
  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
}

void StartupConfigurationTests::userConfigInvalidOptionsAreLoggedIndependently() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(directory.filePath(QStringLiteral("UWF.ini")));
  QVERIFY(file.open(QIODevice::WriteOnly));
  const QByteArray contents("[UI]\nTheme=night\nLanguage=zh_CN\nHubVisible=system\nOther=ignored\n");
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  uwf::clearLogLines();
  const auto config = uwf::app::readUserConfig(directory.path());
  QVERIFY(!config.theme);
  QCOMPARE(config.language, std::optional{uwf::app::LanguageId::Zh_CN});
  QVERIFY(config.hubVisible);
  const auto lines = uwf::recentLogLines();
  QVERIFY(std::ranges::any_of(lines, [](const auto& line) { return line.find("key=UI/Theme") != std::string::npos; }));
  QVERIFY(std::ranges::any_of(lines, [](const auto& line) { return line.find("key=UI/HubVisible") != std::string::npos; }));
  QVERIFY(std::ranges::any_of(lines, [](const auto& line) { return line.find("key=UI/Other") != std::string::npos; }));
}

void StartupConfigurationTests::userConfigReadFailuresFallBackWithoutChangingPermissions() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString path = directory.filePath(QStringLiteral("UWF.ini"));
  QFile file(path);
  QVERIFY(file.open(QIODevice::WriteOnly));
  const QByteArray contents("[UI]\nTheme=dark\nLanguage=en\nHubVisible=false\n");
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  const auto nativePath = path.toStdWString();

  // 无共享读取的句柄模拟不可读文件；不修改真实用户配置或系统 ACL。
  {
    const HANDLE lock = CreateFileW(nativePath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    QVERIFY(lock != INVALID_HANDLE_VALUE);
    const auto closeLock = qScopeGuard([&] { CloseHandle(lock); });
    uwf::clearLogLines();
    const auto config = uwf::app::readUserConfig(directory.path());
    QVERIFY(!config.theme && !config.language);
    QVERIFY(config.hubVisible);
    const auto lines = uwf::recentLogLines();
    QVERIFY(std::ranges::any_of(lines, [](const auto& line) { return line.find("read-failed") != std::string::npos; }));
  }

  const DWORD attributes = GetFileAttributesW(nativePath.c_str());
  QVERIFY(attributes != INVALID_FILE_ATTRIBUTES);
  QVERIFY(SetFileAttributesW(nativePath.c_str(), attributes | FILE_ATTRIBUTE_READONLY));
  const auto restoreAttributes = qScopeGuard([&] { SetFileAttributesW(nativePath.c_str(), attributes); });
  const auto config = uwf::app::readUserConfig(directory.path());
  QCOMPARE(config.theme, std::optional{uwf::app::Theme::Dark});
  QCOMPARE(config.language, std::optional{uwf::app::LanguageId::En});
  QVERIFY(!config.hubVisible);
  QVERIFY(GetFileAttributesW(nativePath.c_str()) & FILE_ATTRIBUTE_READONLY);
  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
}

void StartupConfigurationTests::userConfigSerializationRoundTripsSupportedPreferences() {
  using namespace uwf::app;
  QCOMPARE(serializeUserConfig({}), QStringLiteral("[UI]\nTheme=system\nLanguage=system\nHubVisible=true\n"));
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const auto path = userConfigPath(directory.path());
  QVERIFY(QDir(directory.path()).isEmpty());
  const std::array themes{std::optional<Theme>{}, std::optional{Theme::Dark}, std::optional{Theme::Light}};
  QList<std::optional<LanguageId>> supportedLanguages{std::nullopt};
  for (const auto& language : languages()) supportedLanguages.append(language.id);
  for (const auto theme : themes)
    for (const auto language : supportedLanguages)
      for (const bool visible : {false, true}) {
        const UserConfig expected{theme, language, visible};
        const auto text = serializeUserConfig(expected).toUtf8();
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(text), text.size());
        file.close();
        const auto actual = readUserConfig(directory.path());
        QCOMPARE(actual.theme, expected.theme);
        QCOMPARE(actual.language, expected.language);
        QCOMPARE(actual.hubVisible, expected.hubVisible);
      }
  QCOMPARE(QDir(directory.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System), QStringList{QStringLiteral("UWF.ini")});
}

void StartupConfigurationTests::userConfigSerializationRejectsInvalidPreferenceValues() {
  using namespace uwf::app;
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(serializeUserConfig({static_cast<Theme>(-1), std::nullopt, true})));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(serializeUserConfig({std::nullopt, static_cast<LanguageId>(-1), true})));
}

}  // namespace

QTEST_APPLESS_MAIN(StartupConfigurationTests)

#include "StartupConfigurationTests.moc"
