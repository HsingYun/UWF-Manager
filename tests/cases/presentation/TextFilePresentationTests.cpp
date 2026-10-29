// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include "presentation/common/TextFileExport.h"
#include "presentation/configuration/ApplyPlanViewModel.h"
#include "presentation/configuration/ImportViewModel.h"
using namespace uwf;

class TextFilePresentationTests final : public QObject {
  Q_OBJECT
 private slots:
  void textExportsUseTheInjectedWriter_data();
  void textExportsUseTheInjectedWriter();
  void importsRespectReadOutcomesAndTheSizeLimit_data();
  void importsRespectReadOutcomesAndTheSizeLimit();
  void commandExportsRoundTripThroughFileImports();
  void recommendedRulesUseTheSuppliedSystemVolume() {
    const presentation::ImportViewModel model({}, QStringLiteral("Z:"));
    const auto rules = model.defaultRulesText(presentation::ImportViewModel::AllRecommendedSections);
    QVERIFY(rules.contains(QStringLiteral("Z:\\Windows")));
    QVERIFY(!rules.contains(QStringLiteral("C:\\Windows")));
  }
};

void TextFilePresentationTests::commandExportsRoundTripThroughFileImports() {
  core::UwfSnapshot snapshot;
  snapshot.next.overlay = {core::OverlayType::RAM, 1024, 512, 768};
  core::PendingChanges changes;
  const std::string key = R"(HKEY_LOCAL_MACHINE\SOFTWARE\%USERNAME% !name! 资料)";
  changes.addRegistryExclusions = {key};
  QString exported;
  presentation::ApplyPlanViewModel review(changes, snapshot, {}, {}, [&](const QString&, const QString& payload) {
    exported = payload;
    return app::TextFileWriteResult{};
  });
  QVERIFY(review.exportFile(QStringLiteral("C:/Export/configuration.cmd")).isEmpty());
  QCOMPARE(exported, review.targetCommandsText());
  QVERIFY(exported.contains(QStringLiteral("%%USERNAME%%")));

  presentation::ImportViewModel importer([&](const QString&, qint64) {
    return app::TextFileReadResult{app::TextFileReadResult::Failure::None, {}, exported.toUtf8()};
  });
  QSignalSpy blocks(&importer, &presentation::ImportViewModel::textBlockReady);
  importer.loadFile(QStringLiteral("C:/Export/configuration.cmd"));
  QCOMPARE(blocks.size(), 1);
  const auto loaded = blocks.front().front().toString();
  QVERIFY(loaded.contains(exported));
  QList<app::UwfmgrCommand> observed;
  importer.setApplier([&](const auto& commands) {
    observed = commands;
    return QList<app::ImportResult>{};
  });
  importer.setText(loaded);
  importer.importText();
  QCOMPARE(observed.size(), static_cast<qsizetype>(review.exportCommands().size()));
  QCOMPARE(observed.back().parseError, app::ParseError::None);
  QCOMPARE(observed.back().args.front(), key);
}

void TextFilePresentationTests::textExportsUseTheInjectedWriter_data() {
  QTest::addColumn<int>("failure");
  QTest::addColumn<QString>("expected");
  using Failure = app::TextFileWriteResult::Failure;
  QTest::newRow("success") << static_cast<int>(Failure::None) << QString{};
  QTest::newRow("open-failure") << static_cast<int>(Failure::Open) << QStringLiteral("Could not open file for writing: denied");
  QTest::newRow("write-failure") << static_cast<int>(Failure::Write) << QStringLiteral("Could not write file: denied");
}
void TextFilePresentationTests::textExportsUseTheInjectedWriter() {
  QFETCH(int, failure);
  QFETCH(QString, expected);
  int calls = 0;
  const auto path = QStringLiteral("C:/Export/report.txt");
  const auto text = QStringLiteral("中文\nplain\n");
  QString observedPath, observedText;
  const app::TextFileWriter writer = [&](const QString& output, const QString& payload) {
    ++calls;
    observedPath = output;
    observedText = payload;
    return app::TextFileWriteResult{static_cast<app::TextFileWriteResult::Failure>(failure), QStringLiteral("denied")};
  };
  QCOMPARE(presentation::exportTextFile(path, text, writer), expected);
  QCOMPARE(calls, 1);
  QCOMPARE(observedPath, path);
  QCOMPARE(observedText, text);
  QVERIFY(!presentation::exportTextFile(path, text, {}).isEmpty());
}

void TextFilePresentationTests::importsRespectReadOutcomesAndTheSizeLimit_data() {
  QTest::addColumn<int>("failure");
  QTest::addColumn<QString>("warning");
  using Failure = app::TextFileReadResult::Failure;
  QTest::newRow("success") << static_cast<int>(Failure::None) << QString{};
  QTest::newRow("open-failure") << static_cast<int>(Failure::Open) << QStringLiteral("Cannot read file");
  QTest::newRow("read-failure") << static_cast<int>(Failure::Read) << QStringLiteral("Cannot read file");
  QTest::newRow("oversize") << static_cast<int>(Failure::TooLarge) << QStringLiteral("File too large");
}
void TextFilePresentationTests::importsRespectReadOutcomesAndTheSizeLimit() {
  QFETCH(int, failure);
  QFETCH(QString, warning);
  int calls = 0;
  qint64 limit = 0;
  QString observedPath;
  presentation::ImportViewModel model([&](const QString& path, qint64 maximumBytes) {
    ++calls;
    limit = maximumBytes;
    observedPath = path;
    return app::TextFileReadResult{static_cast<app::TextFileReadResult::Failure>(failure), QStringLiteral("denied"),
                                   QByteArray("unrelated text\nuwfmgr filter enable\n")};
  });
  QSignalSpy blocks(&model, &presentation::ImportViewModel::textBlockReady);
  QSignalSpy warnings(&model, &presentation::ImportViewModel::warningRaised);
  model.loadFile(QStringLiteral("C:/Import/rules.txt"));
  QCOMPARE(calls, 1);
  QCOMPARE(observedPath, QStringLiteral("C:/Import/rules.txt"));
  QCOMPARE(limit, qint64{5 * 1024 * 1024});
  if (warning.isEmpty()) {
    QCOMPARE(warnings.count(), 0);
    QCOMPARE(blocks.count(), 1);
    const auto block = blocks.constFirst().constFirst().toString();
    QVERIFY(block.contains(QStringLiteral("uwfmgr filter enable")));
    QVERIFY(!block.contains(QStringLiteral("unrelated text")));
  } else {
    QCOMPARE(blocks.count(), 0);
    QCOMPARE(warnings.count(), 1);
    QCOMPARE(warnings.constFirst().constFirst().toString(), warning);
  }
}
QTEST_APPLESS_MAIN(TextFilePresentationTests)
#include "TextFilePresentationTests.moc"
