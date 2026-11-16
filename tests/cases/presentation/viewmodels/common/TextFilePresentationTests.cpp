// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include "application/staging/contracts/FileStagingStore.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "domain/uwf/policies/ExclusionCoverage.h"
#include "presentation/viewmodels/common/TextFileExport.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/configuration/ImportViewModel.h"
using namespace uwf;

class TextFilePresentationTests final : public QObject {
  Q_OBJECT
  enum class ExportSetting {
    VolumeBinding,
    DomainPersistence,
    LicensePersistence,
    StagingAdd,
    StagingRemove,
    StagingRepair,
    StagingCleanup,
    EmptyStagingDraft,
    UnchangedStagingList
  };
  Q_ENUM(ExportSetting)
 private slots:
  void textExportsUseTheInjectedWriter_data();
  void textExportsUseTheInjectedWriter();
  void exportsOwnTheRequestAcrossHostCallbacks_data();
  void exportsOwnTheRequestAcrossHostCallbacks();
  void importsRespectReadOutcomesAndTheSizeLimit_data();
  void importsRespectReadOutcomesAndTheSizeLimit();
  void commandExportsRoundTripThroughFileImports();
  void commandPreviewsExcludeScriptFraming_data();
  void commandPreviewsExcludeScriptFraming();
  void importsRejectLossyEncodingBeforeAnyApplicationEffect_data();
  void importsRejectLossyEncodingBeforeAnyApplicationEffect();
  void recommendedRulesUseTheSuppliedSystemVolume() {
    const presentation::ImportViewModel model({}, QStringLiteral("Z:"));
    const auto rules = model.defaultRulesText(presentation::ImportViewModel::AllRecommendedSections);
    QVERIFY(rules.contains(QStringLiteral("Z:\\Windows")));
    QVERIFY(!rules.contains(QStringLiteral("C:\\Windows")));
  }
  void recommendedRuleSelectionsProduceImportableExclusions_data();
  void recommendedRuleSelectionsProduceImportableExclusions();
  void recommendedCeipRulesCoverOptInValueKeys_data();
  void recommendedCeipRulesCoverOptInValueKeys();
};

void TextFilePresentationTests::recommendedCeipRulesCoverOptInValueKeys_data() {
  QTest::addColumn<int>("sections");
  QTest::newRow("ceip") << int(presentation::ImportViewModel::Ceip);
  QTest::newRow("all") << int(presentation::ImportViewModel::AllRecommendedSections);
}

void TextFilePresentationTests::recommendedCeipRulesCoverOptInValueKeys() {
  QFETCH(int, sections);
  presentation::ImportViewModel model;
  model.setText(model.defaultRulesText(sections));
  QList<app::UwfmgrCommand> imported;
  model.setApplier([&](const auto& commands) {
    imported = commands;
    return QList<app::ImportResult>{};
  });
  model.importText();

  const auto covered = [&](const QString& key) {
    return std::ranges::any_of(imported, [&](const auto& command) {
      return command.kind == app::UwfmgrKind::RegistryAddExclusion && command.args.size() == 1 &&
             core::registryExclusionEffects(QString::fromStdString(command.args.front()).toStdU16String(), key.toStdU16String()).driverBypassesWrites;
    });
  };
  // CEIPEnable is a value in each of these keys, not a child key.
  for (const auto& key : {QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Policies\\Microsoft\\SQMClient\\Windows"),
                          QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\SQMClient\\Windows")}) {
    QVERIFY2(covered(key), qPrintable(key));
    QVERIFY(!covered(key + QStringLiteral("Other")));
  }
}

void TextFilePresentationTests::recommendedRuleSelectionsProduceImportableExclusions_data() {
  using Model = presentation::ImportViewModel;
  QTest::addColumn<int>("sections");
  QTest::addColumn<int>("fileCount");
  QTest::addColumn<int>("registryCount");
  QTest::newRow("none") << 0 << 0 << 0;
  QTest::newRow("ceip") << int(Model::Ceip) << 0 << 3;
  QTest::newRow("bits") << int(Model::Bits) << 1 << 1;
  QTest::newRow("network") << int(Model::Network) << 2 << 7;
  QTest::newRow("dst") << int(Model::Dst) << 0 << 2;
  QTest::newRow("defender") << int(Model::Defender) << 4 << 6;
  QTest::newRow("scep") << int(Model::Scep) << 4 << 1;
  QTest::newRow("bits-and-defender") << (Model::Bits | Model::Defender) << 5 << 7;
  QTest::newRow("all") << int(Model::AllRecommendedSections) << 11 << 20;
}

void TextFilePresentationTests::recommendedRuleSelectionsProduceImportableExclusions() {
  QFETCH(int, sections);
  QFETCH(int, fileCount);
  QFETCH(int, registryCount);
  presentation::ImportViewModel model({}, QStringLiteral("Z:"));
  model.setText(model.defaultRulesText(sections));
  QList<app::UwfmgrCommand> imported;
  model.setApplier([&](const auto& commands) {
    imported = commands;
    return QList<app::ImportResult>{};
  });
  model.importText();
  QCOMPARE(imported.size(), fileCount + registryCount);
  int files = 0, registry = 0;
  for (const auto& command : std::as_const(imported)) {
    QCOMPARE(command.parseError, app::ParseError::None);
    QCOMPARE(command.args.size(), std::size_t{1});
    if (command.kind == app::UwfmgrKind::FileAddExclusion) {
      ++files;
      QVERIFY(command.args.front().starts_with("Z:\\"));
    } else {
      QCOMPARE(command.kind, app::UwfmgrKind::RegistryAddExclusion);
      ++registry;
      QVERIFY(command.args.front().starts_with("HKEY_LOCAL_MACHINE\\"));
    }
  }
  QCOMPARE(files, fileCount);
  QCOMPARE(registry, registryCount);
  if (sections & presentation::ImportViewModel::Defender)
    QVERIFY(std::ranges::any_of(imported, [](const auto& command) { return command.args.front() == "Z:\\Program Files\\Windows Defender"; }));
}

void TextFilePresentationTests::exportsOwnTheRequestAcrossHostCallbacks_data() {
  QTest::addColumn<int>("outcome");
  QTest::addColumn<bool>("releaseOwner");
  for (const int outcome : {0, 1, 2, 3})
    for (const bool releaseOwner : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("outcome-%1/release-owner-%2").arg(outcome).arg(releaseOwner))) << outcome << releaseOwner;
}

void TextFilePresentationTests::exportsOwnTheRequestAcrossHostCallbacks() {
  QFETCH(int, outcome);
  QFETCH(bool, releaseOwner);
  struct Owner {
    QString path = QStringLiteral("C:/Export/资料.txt");
    QString text = QStringLiteral("exact contents");
    app::TextFileWriter writer;
  };
  auto owner = std::make_unique<Owner>();
  auto marker = std::make_shared<int>(42);
  const std::weak_ptr<int> lifetime = marker;
  QString observedPath, observedText;
  bool callbackAlive = false;
  owner->writer = [&, retained = marker](const QString& path, const QString& text) {
    if (releaseOwner) owner.reset();
    callbackAlive = !lifetime.expired() && *retained == 42;
    observedPath = path;
    observedText = text;
    if (outcome == 3) throw std::runtime_error("host failure");
    const auto failure = outcome == 1   ? app::TextFileWriteResult::Failure::Open
                         : outcome == 2 ? app::TextFileWriteResult::Failure::Write
                                        : app::TextFileWriteResult::Failure::None;
    return app::TextFileWriteResult{failure, QStringLiteral("host diagnostic")};
  };
  marker.reset();
  const auto error = presentation::exportTextFile(owner->path, owner->text, owner->writer);
  QVERIFY(callbackAlive);
  QCOMPARE(observedPath, QStringLiteral("C:/Export/资料.txt"));
  QCOMPARE(observedText, QStringLiteral("exact contents"));
  QCOMPARE(error.isEmpty(), outcome == 0);
  QCOMPARE(static_cast<bool>(owner), !releaseOwner);
}

void TextFilePresentationTests::importsRejectLossyEncodingBeforeAnyApplicationEffect_data() {
  QTest::addColumn<bool>("fromFile");
  QTest::addColumn<QString>("text");
  QTest::addColumn<QByteArray>("bytes");
  QTest::addColumn<bool>("valid");
  QTest::addColumn<QString>("target");
  const auto command = [](const QString& leaf) { return QStringLiteral("uwfmgr file add-exclusion \"C:\\Data\\%1\"").arg(leaf); };
  const std::tuple<const char*, QString, bool> leaves[]{{"ascii", QStringLiteral("item"), true},
                                                        {"scalar-pair", QString::fromUcs4(U"\U0001F600"), true},
                                                        {"replacement-scalar", QString(QChar::ReplacementCharacter), true},
                                                        {"lone-high", QString(QChar(0xD800)), false},
                                                        {"lone-low", QString(QChar(0xDC00)), false},
                                                        {"high-before-ascii", QString(QChar(0xD800)) + u'x', false},
                                                        {"embedded-nul", QStringLiteral("name") + QChar::Null + QStringLiteral("suffix"), false}};
  for (const auto& [name, leaf, valid] : leaves) {
    QTest::newRow(qPrintable(QStringLiteral("text-%1").arg(QString::fromLatin1(name))))
        << false << command(leaf) << QByteArray{} << valid << (QStringLiteral("C:\\Data\\") + leaf);
    if (valid)
      QTest::newRow(qPrintable(QStringLiteral("file-%1").arg(QString::fromLatin1(name))))
          << true << QString{} << command(leaf).toUtf8() << true << (QStringLiteral("C:\\Data\\") + leaf);
  }
  const std::pair<const char*, QByteArray> invalidBytes[]{
      {"continuation", QByteArray::fromHex("80")},  {"truncated", QByteArray::fromHex("f09f")},           {"overlong", QByteArray::fromHex("c0af")},
      {"surrogate", QByteArray::fromHex("eda080")}, {"outside-unicode", QByteArray::fromHex("f4908080")}, {"nul", QByteArray(1, '\0')}};
  for (const auto& [name, leaf] : invalidBytes)
    QTest::newRow(qPrintable(QStringLiteral("file-%1").arg(QString::fromLatin1(name))))
        << true << QString{} << (QByteArray("uwfmgr file add-exclusion \"C:\\Data\\") + leaf + '"') << false << QString{};
}

void TextFilePresentationTests::importsRejectLossyEncodingBeforeAnyApplicationEffect() {
  QFETCH(bool, fromFile);
  QFETCH(QString, text);
  QFETCH(QByteArray, bytes);
  QFETCH(bool, valid);
  QFETCH(QString, target);
  presentation::ImportViewModel model([&](const QString&, qint64) { return app::TextFileReadResult{app::TextFileReadResult::Failure::None, {}, bytes}; });
  model.setText(text);
  int applications = 0;
  QList<app::UwfmgrCommand> observed;
  model.setApplier([&](const auto& commands) {
    ++applications;
    observed = commands;
    return QList<app::ImportResult>{};
  });
  QSignalSpy blocks(&model, &presentation::ImportViewModel::textBlockReady);
  QSignalSpy warnings(&model, &presentation::ImportViewModel::warningRaised);
  if (fromFile) {
    model.loadFile(QStringLiteral("C:/input.cmd"));
    QCOMPARE(blocks.size(), valid ? 1 : 0);
    if (valid) model.setText(blocks.constFirst().constFirst().toString());
  }
  if (!fromFile || valid) model.importText();
  QCOMPARE(applications, valid ? 1 : 0);
  QCOMPARE(model.totals().batches, valid ? 1 : 0);
  QCOMPARE(warnings.size(), valid ? 0 : 1);
  if (valid) {
    QCOMPARE(observed.size(), 1);
    QCOMPARE(observed.constFirst().parseError, app::ParseError::None);
    QCOMPARE(QString::fromStdString(observed.constFirst().args.front()), target);
  } else {
    QCOMPARE(model.text(), text);
    QVERIFY(observed.isEmpty());
    QVERIFY(model.batches().isEmpty());
  }
}

void TextFilePresentationTests::commandPreviewsExcludeScriptFraming_data() {
  QTest::addColumn<ExportSetting>("setting");
  QTest::addColumn<QString>("description");
  QTest::addColumn<bool>("omissionExpected");
  QTest::addColumn<bool>("stagingExpected");
  QTest::newRow("volume-binding") << ExportSetting::VolumeBinding << QStringLiteral("Volume binding") << true << false;
  QTest::newRow("domain-persistence") << ExportSetting::DomainPersistence << QStringLiteral("Domain Secret Key persistence") << true << false;
  QTest::newRow("license-persistence") << ExportSetting::LicensePersistence << QStringLiteral("Terminal Services license persistence") << true << false;
  QTest::newRow("staging-add") << ExportSetting::StagingAdd << QStringLiteral("File staging") << true << true;
  QTest::newRow("staging-remove") << ExportSetting::StagingRemove << QStringLiteral("File staging") << true << true;
  QTest::newRow("staging-repair") << ExportSetting::StagingRepair << QStringLiteral("File staging") << true << true;
  QTest::newRow("staging-cleanup-confirmation") << ExportSetting::StagingCleanup << QStringLiteral("File staging") << true << true;
  QTest::newRow("empty-staging-draft") << ExportSetting::EmptyStagingDraft << QStringLiteral("File staging") << false << false;
  QTest::newRow("unchanged-staging-list") << ExportSetting::UnchangedStagingList << QStringLiteral("File staging") << false << false;
}

void TextFilePresentationTests::commandPreviewsExcludeScriptFraming() {
  QFETCH(ExportSetting, setting);
  QFETCH(QString, description);
  QFETCH(bool, omissionExpected);
  QFETCH(bool, stagingExpected);
  core::UwfSnapshot snapshot;
  snapshot.next.overlay = {core::OverlayType::RAM, 1024, 512, 768};
  snapshot.next.volumes = {{"Volume{A}", "C:", true, uwf::core::VolumeBinding::DriveLetter}};
  core::PendingChanges changes;
  changes.setFilterEnabled = true;
  std::optional<app::FileStagingChanges> staging;
  const QList<app::FileStagingEntry> savedEntries{{app::FileStagingKind::File, QStringLiteral("C:\\Data\\settings.json")}};
  const auto stagingRoot = std::string(app::kFileStagingRegistryRoot);
  switch (setting) {
    case ExportSetting::VolumeBinding:
      changes.volumeBindings["C:"] = uwf::core::VolumeBinding::VolumeId;
      break;
    case ExportSetting::DomainPersistence:
      changes.setPersistDomainSecretKey = true;
      break;
    case ExportSetting::LicensePersistence:
      changes.setPersistTSCAL = true;
      break;
    case ExportSetting::StagingAdd:
      staging = app::FileStagingChanges{{}, savedEntries};
      break;
    case ExportSetting::StagingRemove:
      staging = app::FileStagingChanges{savedEntries, {}};
      snapshot.current.registryExclusions = snapshot.next.registryExclusions = {stagingRoot};
      break;
    case ExportSetting::StagingRepair:
      staging = app::FileStagingChanges{savedEntries, savedEntries, true};
      snapshot.current.registryExclusions = snapshot.next.registryExclusions = {stagingRoot};
      break;
    case ExportSetting::StagingCleanup:
      staging = app::FileStagingChanges{{}, {}, true};
      snapshot.current.registryExclusions = snapshot.next.registryExclusions = {stagingRoot};
      changes.removeRegistryExclusions = {stagingRoot};
      break;
    case ExportSetting::EmptyStagingDraft:
      staging = app::FileStagingChanges{};
      break;
    case ExportSetting::UnchangedStagingList:
      staging = app::FileStagingChanges{savedEntries, savedEntries};
      snapshot.current.registryExclusions = snapshot.next.registryExclusions = {stagingRoot};
      break;
  }
  QString written;
  presentation::ApplyPlanViewModel model(changes, snapshot, {}, staging, [&](const QString&, const QString& value) {
    written = value;
    return app::TextFileWriteResult{};
  });
  QCOMPARE(model.hasStagingChanges(), stagingExpected);
  for (const auto& preview : {model.changeCommandsText(), model.targetCommandsText()}) {
    QVERIFY(!preview.contains(QStringLiteral(":: - ")));
    QVERIFY(!preview.contains(QStringLiteral("setlocal"), Qt::CaseInsensitive));
    QVERIFY(!preview.contains(QStringLiteral("chcp"), Qt::CaseInsensitive));
    QVERIFY(!preview.contains(QStringLiteral(" && ")));
  }
  QCOMPARE(model.canExport(), !stagingExpected);
  if (model.canExport()) {
    QVERIFY(model.exportRestrictionText().isEmpty());
    QVERIFY(model.exportFile(QStringLiteral("C:/Export/configuration.cmd")).isEmpty());
    QVERIFY(written != model.targetCommandsText());
    QVERIFY(written.contains(QStringLiteral("setlocal"), Qt::CaseInsensitive));
    QVERIFY(!written.contains(QStringLiteral(" && ")));
    QCOMPARE(written.contains(QStringLiteral(":: - ") + description), omissionExpected);
  } else {
    QVERIFY(!model.exportRestrictionText().isEmpty());
    QVERIFY(!model.exportFile(QStringLiteral("C:/Export/configuration.cmd")).isEmpty());
    QVERIFY(written.isEmpty());
  }
}

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
  QVERIFY(exported != review.targetCommandsText());
  QVERIFY(!review.targetCommandsText().contains(QStringLiteral("setlocal"), Qt::CaseInsensitive));
  QVERIFY(exported.contains(QStringLiteral("%%USERNAME%%")));

  presentation::ImportViewModel importer(
      [&](const QString&, qint64) { return app::TextFileReadResult{app::TextFileReadResult::Failure::None, {}, exported.toUtf8()}; });
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
