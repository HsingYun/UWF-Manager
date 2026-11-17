// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QStringList>
#include <QTest>
#include <optional>
#include <stdexcept>
#include <vector>

#include "application/settings/UserConfig.h"
#include "application/startup/StartupDispatch.h"
#include "application/startup/StartupOptions.h"

Q_DECLARE_METATYPE(uwf::app::StartupMode)

namespace {
class StartupConfigurationTests final : public QObject {
  Q_OBJECT
 private slots:
  void configurationInterpretationUsesIndependentFallbacks() {
    using namespace uwf::app;
    const auto normalized = interpretUserConfig({QStringLiteral(" LIGHT "), QStringLiteral(" ZH-cn "), QStringLiteral(" FaLsE ")});
    QCOMPARE(normalized.theme, std::optional{Theme::Light});
    QCOMPARE(normalized.language, std::optional{LanguageId::Zh_CN});
    QVERIFY(!normalized.hubVisible);
    const auto partial = interpretUserConfig({QStringLiteral("invalid"), QStringLiteral("en"), QStringLiteral("false")});
    QVERIFY(!partial.theme);
    QCOMPARE(partial.language, std::optional{LanguageId::En});
    QVERIFY(!partial.hubVisible);
    const auto defaults = interpretUserConfig({});
    QVERIFY(!defaults.theme);
    QVERIFY(!defaults.language);
    QVERIFY(defaults.hubVisible);
  }
  void startupRoutingKeepsUiServiceAndConsoleBoundaries_data();
  void startupRoutingKeepsUiServiceAndConsoleBoundaries();
  void malformedUnicodeNeverProbesOrReclassifiesControlEntries_data();
  void malformedUnicodeNeverProbesOrReclassifiesControlEntries();
  void startupActionsExecuteOnlyTheSelectedBoundary_data();
  void startupActionsExecuteOnlyTheSelectedBoundary();
  void selectedStartupFailuresNeverFallBackToAnotherMode();
  void languageMatchingRespectsPreferenceOrderScriptAndRegion_data();
  void languageMatchingRespectsPreferenceOrderScriptAndRegion();
  void userConfigSerializationRejectsInvalidPreferenceValues();
};

void StartupConfigurationTests::startupRoutingKeepsUiServiceAndConsoleBoundaries_data() {
  using uwf::app::StartupMode;
  QTest::addColumn<QStringList>("arguments");
  QTest::addColumn<bool>("consoleContext");
  QTest::addColumn<StartupMode>("expected");
  QTest::addColumn<bool>("rejected");
  struct Scenario {
    const char* name;
    QStringList arguments;
    StartupMode withoutContext;
    StartupMode withContext;
    bool rejected = false;
  };
  const auto cases = std::vector<Scenario>{
      {"empty-list", {}, StartupMode::Interactive, StartupMode::Console},
      {"executable-only", {"UWF.exe"}, StartupMode::Interactive, StartupMode::Console},
      {"argv-zero-is-not-an-option", {"--quiet"}, StartupMode::Interactive, StartupMode::Console},
      {"quiet", {"UWF.exe", "--quiet"}, StartupMode::Quiet, StartupMode::Quiet},
      {"service", {"UWF.exe", "--service"}, StartupMode::Service, StartupMode::Service},
      {"help", {"UWF.exe", "--help"}, StartupMode::Console, StartupMode::Console},
      {"command", {"UWF.exe", "filter", "get"}, StartupMode::Console, StartupMode::Console},
      {"non-ascii-target", {"UWF.exe", "file", "commit", QStringLiteral("C:\\资料\\应用.ini")}, StartupMode::Console, StartupMode::Console},
      {"unicode-pair-target",
       {"UWF.exe", "file", "commit", QStringLiteral("C:\\") + QString::fromUcs4(U"\U0001f642")},
       StartupMode::Console,
       StartupMode::Console},
      {"legacy-install", {"UWF.exe", "--install"}, StartupMode::Console, StartupMode::Console},
      {"legacy-uninstall", {"UWF.exe", "--uninstall"}, StartupMode::Console, StartupMode::Console},
      {"legacy-commit-stage", {"UWF.exe", "--commit-stage"}, StartupMode::Console, StartupMode::Console},
      {"unknown-option", {"UWF.exe", "--other"}, StartupMode::Console, StartupMode::Console},
      {"empty-argument", {"UWF.exe", ""}, StartupMode::Console, StartupMode::Console},
      {"quiet-wrong-case", {"UWF.exe", "--Quiet"}, StartupMode::Console, StartupMode::Console},
      {"quiet-option-data", {"UWF.exe", "registry", "commit", "HKLM\\Software", "--quiet"}, StartupMode::Console, StartupMode::Console},
      {"service-option-data", {"UWF.exe", "registry", "commit", "HKLM\\Software", "--service"}, StartupMode::Console, StartupMode::Console},
      {"quiet-extra", {"UWF.exe", "--quiet", "extra"}, StartupMode::Quiet, StartupMode::Quiet, true},
      {"service-extra", {"UWF.exe", "--service", "extra"}, StartupMode::Service, StartupMode::Service, true},
      {"quiet-duplicate", {"UWF.exe", "--quiet", "--quiet"}, StartupMode::Quiet, StartupMode::Quiet, true},
      {"service-duplicate", {"UWF.exe", "--service", "--service"}, StartupMode::Service, StartupMode::Service, true},
      {"mixed-quiet-first", {"UWF.exe", "--quiet", "--service"}, StartupMode::Quiet, StartupMode::Quiet, true},
      {"mixed-service-first", {"UWF.exe", "--service", "--quiet"}, StartupMode::Service, StartupMode::Service, true},
  };
  for (const auto& scene : cases)
    for (const bool context : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1/context-%2").arg(scene.name).arg(context)))
          << scene.arguments << context << (context ? scene.withContext : scene.withoutContext) << scene.rejected;
}

void StartupConfigurationTests::startupRoutingKeepsUiServiceAndConsoleBoundaries() {
  using namespace uwf::app;
  QFETCH(QStringList, arguments);
  QFETCH(bool, consoleContext);
  QFETCH(StartupMode, expected);
  QFETCH(bool, rejected);
  int probes = 0;
  const auto context = [&] {
    ++probes;
    return consoleContext;
  };
  if (rejected) {
    QVERIFY_THROWS_EXCEPTION(StartupOptionsError, static_cast<void>(resolveStartupOptions(arguments, context)));
    QVERIFY(startupControlMode(arguments));
  } else {
    const auto selected = resolveStartupOptions(arguments, context);
    QCOMPARE(selected.mode, expected);
    QCOMPARE(parseStartupOptions(arguments, consoleContext).mode, expected);
    switch (expected) {
      case StartupMode::Interactive:
        QCOMPARE(selected.initialCommand(), ApplicationCommandKind::Activate);
        break;
      case StartupMode::Quiet:
        QCOMPARE(selected.initialCommand(), ApplicationCommandKind::EnsureRunning);
        break;
      case StartupMode::Service:
      case StartupMode::Console:
        QVERIFY_THROWS_EXCEPTION(StartupOptionsError, static_cast<void>(selected.initialCommand()));
        break;
    }
  }
  QCOMPARE(probes, arguments.size() <= 1 ? 1 : 0);
}

void StartupConfigurationTests::malformedUnicodeNeverProbesOrReclassifiesControlEntries_data() {
  QTest::addColumn<QStringList>("arguments");
  QTest::addColumn<bool>("controlEntry");
  const QString high(QChar(char16_t{0xd800}));
  const QString low(QChar(char16_t{0xdc00}));
  const std::vector<QString> malformed{high, low, low + high, high + QStringLiteral("X"), QStringLiteral("X") + low};
  for (std::size_t index = 0; index < malformed.size(); ++index) {
    const auto& bad = malformed[index];
    for (const auto& token : {QStringLiteral("--quiet"), QStringLiteral("--service")}) {
      QTest::newRow(qUtf8Printable(QStringLiteral("%1/bad-extra-%2").arg(token).arg(index))) << QStringList{"UWF.exe", token, bad} << true;
      QTest::newRow(qUtf8Printable(QStringLiteral("%1/bad-executable-%2").arg(token).arg(index))) << QStringList{bad, token} << true;
    }
    QTest::newRow(qUtf8Printable(QStringLiteral("invalid-control-front-%1").arg(index)))
        << QStringList{"UWF.exe", QStringLiteral("--qui") + bad + QStringLiteral("et")} << false;
    QTest::newRow(qUtf8Printable(QStringLiteral("invalid-cli-data-%1").arg(index))) << QStringList{"UWF.exe", "file", "commit", bad} << false;
    QTest::newRow(qUtf8Printable(QStringLiteral("invalid-executable-only-%1").arg(index))) << QStringList{bad} << false;
  }
}

void StartupConfigurationTests::malformedUnicodeNeverProbesOrReclassifiesControlEntries() {
  using namespace uwf::app;
  QFETCH(QStringList, arguments);
  QFETCH(bool, controlEntry);
  const bool reportConsoleError = !startupControlMode(arguments).has_value();
  QCOMPARE(reportConsoleError, !controlEntry);
  int probes = 0;
  QVERIFY_THROWS_EXCEPTION(StartupOptionsError, static_cast<void>(resolveStartupOptions(arguments, [&] {
                             ++probes;
                             return true;
                           })));
  QCOMPARE(probes, 0);
  QCOMPARE(startupControlMode(arguments).has_value(), controlEntry);
}

void StartupConfigurationTests::startupActionsExecuteOnlyTheSelectedBoundary_data() {
  using uwf::app::StartupMode;
  QTest::addColumn<StartupMode>("mode");
  QTest::addColumn<bool>("elevated");
  QTest::addColumn<int>("exitCode");
  for (const auto mode : {StartupMode::Interactive, StartupMode::Quiet, StartupMode::Service, StartupMode::Console})
    for (const bool elevated : {false, true})
      for (const int code : {0, 1, 17})
        QTest::newRow(qUtf8Printable(QStringLiteral("mode-%1/elevated-%2/exit-%3").arg(static_cast<int>(mode)).arg(elevated).arg(code)))
            << mode << elevated << code;
}

void StartupConfigurationTests::startupActionsExecuteOnlyTheSelectedBoundary() {
  using namespace uwf::app;
  QFETCH(StartupMode, mode);
  QFETCH(bool, elevated);
  QFETCH(int, exitCode);
  QStringList calls;
  const auto execute = [&](const QString& name) {
    calls.append(name);
    return exitCode;
  };
  const auto result = dispatchStartup({mode}, {.isElevated =
                                                   [&] {
                                                     calls.append("token");
                                                     return elevated;
                                                   },
                                               .elevateUi = [&] { return execute("elevate"); },
                                               .desktop = [&] { return execute("desktop"); },
                                               .service = [&] { return execute("service"); },
                                               .console = [&] { return execute("console"); }});
  QCOMPARE(result, exitCode);
  const QStringList expected = mode == StartupMode::Console   ? QStringList{"console"}
                               : mode == StartupMode::Service ? QStringList{"service"}
                                                              : QStringList{"token", elevated ? "desktop" : "elevate"};
  QCOMPARE(calls, expected);
}

void StartupConfigurationTests::selectedStartupFailuresNeverFallBackToAnotherMode() {
  using namespace uwf::app;
  QVERIFY_THROWS_EXCEPTION(std::runtime_error,
                           static_cast<void>(resolveStartupOptions({"UWF.exe"}, []() -> bool { throw std::runtime_error("console context query failure"); })));
  QVERIFY_THROWS_EXCEPTION(std::logic_error, static_cast<void>(dispatchStartup({static_cast<StartupMode>(99)}, {})));
  QVERIFY_THROWS_EXCEPTION(StartupOptionsError, static_cast<void>(StartupOptions{static_cast<StartupMode>(99)}.initialCommand()));
  for (const auto mode : {StartupMode::Interactive, StartupMode::Quiet, StartupMode::Service, StartupMode::Console})
    for (const bool elevated : {false, true}) {
      int executions = 0;
      const auto failure = [&]() -> int {
        ++executions;
        throw std::runtime_error("selected startup failure");
      };
      QVERIFY_THROWS_EXCEPTION(
          std::runtime_error,
          static_cast<void>(dispatchStartup(
              {mode}, {.isElevated = [=] { return elevated; }, .elevateUi = failure, .desktop = failure, .service = failure, .console = failure})));
      QCOMPARE(executions, 1);
    }
  for (const auto mode : {StartupMode::Interactive, StartupMode::Quiet}) {
    int uiExecutions = 0;
    const auto ui = [&] {
      ++uiExecutions;
      return 0;
    };
    QVERIFY_THROWS_EXCEPTION(
        std::runtime_error,
        static_cast<void>(dispatchStartup(
            {mode},
            {.isElevated = []() -> bool { throw std::runtime_error("token query failure"); }, .elevateUi = ui, .desktop = ui, .service = ui, .console = ui})));
    QCOMPARE(uiExecutions, 0);
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

void StartupConfigurationTests::userConfigSerializationRejectsInvalidPreferenceValues() {
  using namespace uwf::app;
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(serializeUserConfig({static_cast<Theme>(-1), std::nullopt, true})));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(serializeUserConfig({std::nullopt, static_cast<LanguageId>(-1), true})));
}
}  // namespace
QTEST_APPLESS_MAIN(StartupConfigurationTests)
#include "StartupConfigurationTests.moc"
