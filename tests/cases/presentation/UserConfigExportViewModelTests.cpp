// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QTest>
#include <stdexcept>

#include "presentation/settings/UserConfigExportViewModel.h"

using namespace uwf;
using Outcome = app::TextFileCreationResult::Outcome;

class UserConfigExportViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void exportsEachCurrentSnapshotThroughTheCreateOnlyPort() {
    QString observedPath;
    QString observedText;
    int calls = 0;
    presentation::UserConfigExportViewModel model(QStringLiteral("C:/Applications/UWF"), [&](const QString& path, const QString& text) {
      ++calls;
      observedPath = path;
      observedText = text;
      return app::TextFileCreationResult{};
    });
    app::UserConfig config;
    config.theme = app::Theme::Dark;
    config.language = app::LanguageId::Zh_CN;
    config.hubVisible = false;
    auto result = model.exportConfig(config);
    QCOMPARE(result.outcome, Outcome::Created);
    QCOMPARE(QDir::fromNativeSeparators(observedPath), QStringLiteral("C:/Applications/UWF/UWF.ini"));
    QCOMPARE(result.path, observedPath);
    QVERIFY(result.error.isEmpty());
    QVERIFY(observedText.contains(QStringLiteral("Theme=dark")));
    QVERIFY(observedText.contains(QStringLiteral("Language=zh_CN")));
    QVERIFY(observedText.contains(QStringLiteral("HubVisible=false")));

    config.theme = app::Theme::Light;
    config.language = app::LanguageId::En;
    config.hubVisible = true;
    result = model.exportConfig(config);
    QCOMPARE(calls, 2);
    QCOMPARE(result.outcome, Outcome::Created);
    QVERIFY(observedText.contains(QStringLiteral("Theme=light")));
    QVERIFY(observedText.contains(QStringLiteral("Language=en")));
    QVERIFY(observedText.contains(QStringLiteral("HubVisible=true")));
  }

  void preservesPortOutcomesWithoutRetryingOrReplacing() {
    for (const auto outcome : {Outcome::AlreadyExists, Outcome::Failed}) {
      int calls = 0;
      presentation::UserConfigExportViewModel model(QStringLiteral("C:/Applications/UWF"), [&](const QString&, const QString&) {
        ++calls;
        return app::TextFileCreationResult{outcome, QStringLiteral("access denied")};
      });
      const auto result = model.exportConfig({});
      QCOMPARE(calls, 1);
      QCOMPARE(result.outcome, outcome);
      QCOMPARE(result.error, QStringLiteral("access denied"));
    }
  }

  void translatesPortExceptionsIntoFailureResults() {
    presentation::UserConfigExportViewModel model(
        QStringLiteral("C:/Applications/UWF"), [](const QString&, const QString&) -> app::TextFileCreationResult { throw std::runtime_error("disk error"); });
    const auto result = model.exportConfig({});
    QCOMPARE(result.outcome, Outcome::Failed);
    QCOMPARE(result.error, QStringLiteral("disk error"));
    QVERIFY(result.path.endsWith(QStringLiteral("/UWF.ini")));
  }

  void rejectsInvalidConstruction() {
    const app::TextFileCreator creator = [](const QString&, const QString&) { return app::TextFileCreationResult{}; };
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, presentation::UserConfigExportViewModel(QStringLiteral("relative"), creator));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, presentation::UserConfigExportViewModel(QStringLiteral("C:/Applications/UWF"), {}));
  }
};

QTEST_APPLESS_MAIN(UserConfigExportViewModelTests)
#include "UserConfigExportViewModelTests.moc"
