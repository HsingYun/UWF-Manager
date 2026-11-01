// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractItemModelTester>
#include <QPointer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QTranslator>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "cases/presentation/viewmodels/registry/ControlledRegistryBrowser.h"
#include "cases/presentation/viewmodels/registry/RegistryPickerTestActions.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/registry/RegistryPickerViewModel.h"
#include "support/app/WorkerGate.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

class GatedRegistryBrowser final : public app::RegistryBrowser {
 public:
  enum class Stage { Roots, Children, Values, None };
  Stage blocked = Stage::None;
  std::string blockedKey;
  bool failBlocked = false;
  bool unexpectedFailure = false;
  bool honorCancellation = false;
  std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
  const std::thread::id uiThread = std::this_thread::get_id();
  mutable std::atomic_bool wrongThread{false};
  void observe(const Stage stage, const std::string_view key = {}, const std::stop_token stop = {}) const {
    if (std::this_thread::get_id() == uiThread) wrongThread = true;
    if (blocked != Stage::None && blocked == stage && (blockedKey.empty() || key == blockedKey)) {
      gate->wait(stop, honorCancellation);
      if (failBlocked) {
        if (unexpectedFailure) throw 17;
        throw std::runtime_error("blocked branch failed");
      }
    }
  }
  std::vector<std::string> rootHiveLongNames(const std::stop_token stop = {}) const override {
    observe(Stage::Roots, {}, stop);
    return {"HKEY_LOCAL_MACHINE"};
  }
  std::vector<std::string> subkeyNames(const std::string_view key, const std::stop_token stop = {}) const override {
    observe(Stage::Children, key, stop);
    return key == "HKEY_LOCAL_MACHINE" ? std::vector<std::string>{"Slow", "Fast"} : std::vector<std::string>{};
  }
  bool hasSubkeys(std::string_view, const std::stop_token stop = {}) const override {
    observe(Stage::None, {}, stop);
    return true;
  }
  std::vector<app::RegistryValueInfo> values(const std::string_view key, const std::stop_token stop = {}) const override {
    observe(Stage::Values, key, stop);
    return {{std::string(key), 1u, std::vector<uint8_t>{}}};
  }
};

class RegistryPickerViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void registryTextBoundariesPreserveOrRejectTheEntireIdentity_data();
  void registryTextBoundariesPreserveOrRejectTheEntireIdentity();
  void registryNavigationResolvesAccessibleAncestors_data();
  void registryNavigationResolvesAccessibleAncestors();
  void registrySelectionPreservesTheEnumeratedObject_data();
  void registrySelectionPreservesTheEnumeratedObject();
  void registryModelRetainsDefaultValueAndRestrictedChildSemantics();
  void valueSelectionUsesObservedExistenceAndOperation_data();
  void valueSelectionUsesObservedExistenceAndOperation();
  void valueSelectionRequiresTheCurrentSuccessfulObservation();
  void registryPickerAdmissionMatchesTheOperation_data();
  void registryPickerAdmissionMatchesTheOperation();
  void selectedRegistryNamesSurviveTheCompleteConfigurationBoundary_data();
  void selectedRegistryNamesSurviveTheCompleteConfigurationBoundary();
  void registryAccessErrorsTrackTheRequestedKeyAndRecover();
  void observationsOwnInputsAndCancelDelivery_data();
  void observationsOwnInputsAndCancelDelivery();
  void latestNavigationWinsWhileAParentIsLoading();
  void reentrantValueRequestsKeepTheNewestKey();
  void unselectedBranchFailureDoesNotReplaceCurrentDetails();
  void reentrantResetCoordinatesAllPublications_data();
  void reentrantResetCoordinatesAllPublications();
  void accessFailuresTranslateFallbacksAndPreserveProviderDetails_data();
  void accessFailuresTranslateFallbacksAndPreserveProviderDetails();
};

void RegistryPickerViewModelTests::registryTextBoundariesPreserveOrRejectTheEntireIdentity_data() {
  QTest::addColumn<int>("mode");
  QTest::addColumn<QString>("name");
  QTest::addColumn<bool>("valid");
  const std::tuple<const char*, QString, bool> names[]{{"ascii", QStringLiteral("Literal"), true},
                                                       {"scalar-pair", QStringLiteral("Literal") + QString::fromUcs4(U"\U0001F600"), true},
                                                       {"replacement-scalar", QStringLiteral("Literal") + QChar::ReplacementCharacter, true},
                                                       {"lone-high", QStringLiteral("Literal") + QChar(0xD800), false},
                                                       {"lone-low", QStringLiteral("Literal") + QChar(0xDC00), false},
                                                       {"embedded-nul", QStringLiteral("Literal") + QChar::Null + QStringLiteral("suffix"), false}};
  using Mode = presentation::RegistryPickerViewModel::Mode;
  for (const auto mode : {Mode::Exclusion, Mode::CommitValue, Mode::DeleteValue})
    for (const auto& [label, name, valid] : names)
      QTest::newRow(qPrintable(QStringLiteral("%1-%2").arg(static_cast<int>(mode)).arg(QString::fromLatin1(label)))) << static_cast<int>(mode) << name << valid;
}

void RegistryPickerViewModelTests::registryTextBoundariesPreserveOrRejectTheEntireIdentity() {
  QFETCH(int, mode);
  QFETCH(QString, name);
  QFETCH(bool, valid);
  class LiteralBrowser final : public app::RegistryBrowser {
   public:
    QString leaf;
    QString valueName = QStringLiteral("Named");
    mutable std::atomic_int reads{0};
    std::vector<std::string> rootHiveLongNames(std::stop_token) const override {
      ++reads;
      return {"HKEY_LOCAL_MACHINE"};
    }
    std::vector<std::string> subkeyNames(const std::string_view key, std::stop_token) const override {
      ++reads;
      return key == "HKEY_LOCAL_MACHINE" ? std::vector<std::string>{"SOFTWARE"} : std::vector<std::string>{"Initial", leaf.toStdString()};
    }
    bool hasSubkeys(const std::string_view key, std::stop_token) const override {
      ++reads;
      return key == "HKEY_LOCAL_MACHINE\\SOFTWARE";
    }
    std::vector<app::RegistryValueInfo> values(std::string_view, std::stop_token) const override {
      ++reads;
      return {{valueName.toStdString(), 1u, std::vector<std::uint8_t>{}}};
    }
  };
  const auto browser = std::make_shared<LiteralBrowser>();
  // The provider publishes valid names; malformed input belongs to the
  // navigation request, and must be rejected before reaching that provider.
  browser->leaf = valid ? name : QStringLiteral("Unrelated");
  presentation::RegistryPickerViewModel model(static_cast<presentation::RegistryPickerViewModel::Mode>(mode), browser);
  const auto previous = test::navigateRegistry(model, QStringLiteral("HKLM\\SOFTWARE\\Initial"));
  QVERIFY(previous.isValid());
  test::loadRegistryValues(model, previous.data(presentation::PathRole).toString());
  model.accept(previous.data(presentation::PathRole).toString(), QStringLiteral("Named"));
  QVERIFY(model.selection());
  const int reads = browser->reads;
  const auto input = QStringLiteral("hKlM\\SOFTWARE\\") + name;
  const auto expected = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\") + name;
  const auto navigated = test::navigateRegistry(model, input);
  if (valid) {
    QCOMPARE(navigated.data(presentation::PathRole).toString(), expected);
    browser->valueName = name;
    test::loadRegistryValues(model, expected);
    QVERIFY(model.canAccept(input, name));
    model.accept(input, name);
    QVERIFY(model.selection());
    QCOMPARE(model.selection()->key, expected);
    if (model.supportsValueSelection()) QCOMPARE(model.selection()->valueName, std::optional{name});
    QCOMPARE(model.valueModel().rowCount(), 2);
  } else {
    QVERIFY(!navigated.isValid());
    QVERIFY(!model.selection());
    QCOMPARE(browser->reads.load(), reads);
    QCOMPARE(model.availabilityOf(input), presentation::RegistryPickerViewModel::KeyAvailability::Pruned);
    QVERIFY(!model.canAccept(input, QStringLiteral("Named")));
    model.accept(QStringLiteral("HKLM\\SOFTWARE"), name);
    QVERIFY(!model.selection());
    model.loadValues(input);
    QVERIFY(!model.valuesLoading());
    QVERIFY(model.accessError());
    QCOMPARE(model.accessError()->key, input);
    QCOMPARE(model.valueModel().rowCount(), 0);
    QCOMPARE(browser->reads.load(), reads);
  }
}

void RegistryPickerViewModelTests::accessFailuresTranslateFallbacksAndPreserveProviderDetails_data() {
  QTest::addColumn<int>("stage");
  QTest::addColumn<bool>("unexpected");
  for (const auto stage : {GatedRegistryBrowser::Stage::Roots, GatedRegistryBrowser::Stage::Children, GatedRegistryBrowser::Stage::Values})
    for (const bool unexpected : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("stage-%1-unexpected-%2").arg(int(stage)).arg(unexpected))) << int(stage) << unexpected;
}

void RegistryPickerViewModelTests::accessFailuresTranslateFallbacksAndPreserveProviderDetails() {
  QFETCH(int, stage);
  QFETCH(bool, unexpected);
  class FailureTranslator final : public QTranslator {
   public:
    bool isEmpty() const override { return false; }
    QString translate(const char* context, const char* source, const char*, int) const override {
      return context && source && qstrcmp(context, "uwf") == 0 && qstrcmp(source, "The operation failed with an unknown error.") == 0
                 ? QStringLiteral("Localized unknown registry failure")
                 : QString{};
    }
  } translator;
  QVERIFY(QCoreApplication::installTranslator(&translator));
  const auto restore = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
  const auto browser = std::make_shared<GatedRegistryBrowser>();
  browser->blocked = static_cast<GatedRegistryBrowser::Stage>(stage);
  browser->failBlocked = true;
  browser->unexpectedFailure = unexpected;
  browser->gate->release();
  using Model = presentation::RegistryPickerViewModel;
  Model model(Model::Mode::CommitValue, browser);
  const auto rootPath = QStringLiteral("HKEY_LOCAL_MACHINE");
  if (browser->blocked != GatedRegistryBrowser::Stage::Roots) {
    QVERIFY(test::awaitRegistryRoots(model));
    if (browser->blocked == GatedRegistryBrowser::Stage::Children)
      model.loadChildren(model.index(0, 0));
    else
      model.loadValues(rootPath);
  }
  QTRY_VERIFY(model.accessError());
  QCOMPARE(model.accessError()->message, unexpected ? QStringLiteral("Localized unknown registry failure") : QStringLiteral("blocked branch failed"));
  QCOMPARE(model.accessError()->operation,
           browser->blocked == GatedRegistryBrowser::Stage::Values ? Model::AccessError::Operation::Values : Model::AccessError::Operation::Keys);
  QCOMPARE(model.accessError()->key, browser->blocked == GatedRegistryBrowser::Stage::Roots ? QString{} : rootPath);
}

void RegistryPickerViewModelTests::registryNavigationResolvesAccessibleAncestors_data() {
  QTest::addColumn<QString>("path");
  QTest::addColumn<QString>("expected");
  QTest::addColumn<bool>("denied");
  const QString root = QStringLiteral("HKEY_LOCAL_MACHINE");
  QTest::newRow("alias") << QStringLiteral("HKLM\\SOFTWARE") << root + QStringLiteral("\\SOFTWARE") << false;
  QTest::newRow("case") << QStringLiteral("hkey_local_machine\\software") << root + QStringLiteral("\\SOFTWARE") << false;
  QTest::newRow("missing-descendant") << QStringLiteral("HKLM\\SOFTWARE\\missing") << root + QStringLiteral("\\SOFTWARE") << false;
  QTest::newRow("access-denied") << QStringLiteral("HKLM\\SOFTWARE") << root << true;
  QTest::newRow("unavailable-hive") << QStringLiteral("HKCU\\SOFTWARE") << QString{} << false;
  QTest::newRow("invalid-hive") << QStringLiteral("invalid\\SOFTWARE") << QString{} << false;
  QTest::newRow("empty") << QString{} << QString{} << false;
}

void RegistryPickerViewModelTests::registryNavigationResolvesAccessibleAncestors() {
  QFETCH(QString, path);
  QFETCH(QString, expected);
  QFETCH(bool, denied);
  const auto browserOwner = std::make_shared<ControlledRegistryBrowser>();
  auto& browser = *browserOwner;
  if (denied) browser.deniedChildren = "HKEY_LOCAL_MACHINE";
  presentation::RegistryPickerViewModel model(presentation::RegistryPickerViewModel::Mode::CommitValue, browserOwner);
  const auto selected = test::navigateRegistry(model, path);
  QCOMPARE(selected.data(presentation::PathRole).toString(), expected);
}

void RegistryPickerViewModelTests::registrySelectionPreservesTheEnumeratedObject_data() {
  QTest::addColumn<int>("mode");
  QTest::addColumn<QString>("name");
  QTest::addColumn<bool>("alias");
  using Mode = presentation::RegistryPickerViewModel::Mode;
  for (const auto mode : {Mode::Exclusion, Mode::CommitValue, Mode::DeleteValue})
    for (const auto& name : {QStringLiteral("Vendor "), QStringLiteral(".."), QStringLiteral("Vendor")})
      for (const bool alias : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("mode-%1-name-%2-alias-%3").arg(int(mode)).arg(name).arg(alias))) << int(mode) << name << alias;
}

void RegistryPickerViewModelTests::registrySelectionPreservesTheEnumeratedObject() {
  QFETCH(int, mode);
  QFETCH(QString, name);
  QFETCH(bool, alias);
  using Model = presentation::RegistryPickerViewModel;
  const auto browserOwner = std::make_shared<ControlledRegistryBrowser>();
  auto& browser = *browserOwner;
  browser.childNames = {"SOFTWARE", "Vendor", "Vendor ", ".."};
  browser.valueEntries = {{"Named ", 1u, std::vector<uint8_t>{}}};
  Model model(static_cast<Model::Mode>(mode), browserOwner);
  const auto suffix = QStringLiteral("\\SOFTWARE\\") + name;
  const auto expected = QStringLiteral("HKEY_LOCAL_MACHINE") + suffix;
  const auto input = (alias ? QStringLiteral("hKlM") : QStringLiteral("HKEY_LOCAL_MACHINE")) + suffix;
  const auto index = test::navigateRegistry(model, input);
  QCOMPARE(index.data(presentation::PathRole).toString(), expected);
  test::loadRegistryValues(model, expected);
  QVERIFY(model.canAccept(expected, QStringLiteral("Named ")));
  QCOMPARE(browser.lastValueKey, expected.toStdString());
  model.accept(index.data(presentation::PathRole).toString(), QStringLiteral("Named "));
  QVERIFY(model.selection());
  QCOMPARE(model.selection()->key, expected);
  QCOMPARE(model.selection()->valueName, model.supportsValueSelection() ? std::optional{QStringLiteral("Named ")} : std::nullopt);
  model.resetTree();
  const auto restored = test::navigateRegistry(model, input);
  QCOMPARE(restored.data(presentation::PathRole).toString(), expected);
}

void RegistryPickerViewModelTests::registryModelRetainsDefaultValueAndRestrictedChildSemantics() {
  const auto browserOwner = std::make_shared<ControlledRegistryBrowser>();
  using Model = presentation::RegistryPickerViewModel;
  Model model(Model::Mode::Exclusion, browserOwner);
  QVERIFY(!model.canAccept(QStringLiteral("HKEY_LOCAL_MACHINE")));
  QVERIFY(test::awaitRegistryRoots(model));
  const auto root = model.index(0, 0);
  QVERIFY(test::loadRegistryChildren(model, root));
  const auto software = model.index(0, 0, root);
  QCOMPARE(model.rowCount(root), 2);
  QVERIFY(model.hasChildren(model.index(1, 0, root)));
  Model valuesModel(Model::Mode::CommitValue, browserOwner);
  test::loadRegistryValues(valuesModel, QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE"));
  QCOMPARE(valuesModel.valueModel().rowCount(), 2);
  QCOMPARE(valuesModel.valueModel().index(0, 0).data(presentation::RegistryValueModel::NameRole).toString(), QString{});

  QVERIFY(model.flags(software).testFlag(Qt::ItemIsEnabled));
  QVERIFY(!model.flags(software).testFlag(Qt::ItemIsSelectable));
  QVERIFY(model.canFetchMore(software));
  QVERIFY(model.canAccept(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor")));
  model.accept(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor"), QStringLiteral("Named"));
  QVERIFY(!model.selection()->valueName);
}

void RegistryPickerViewModelTests::valueSelectionUsesObservedExistenceAndOperation_data() {
  QTest::addColumn<int>("mode");
  QTest::addColumn<int>("observation");
  using Mode = presentation::RegistryPickerViewModel::Mode;
  const std::array observations{"missing-default", "content", "empty-data", "preview-unavailable"};
  for (const auto mode : {Mode::CommitValue, Mode::DeleteValue, Mode::Exclusion})
    for (std::size_t observation = 0; observation < observations.size(); ++observation)
      QTest::newRow(qUtf8Printable(QStringLiteral("mode-%1/%2").arg(int(mode)).arg(QString::fromLatin1(observations[observation]))))
          << int(mode) << static_cast<int>(observation);
}

void RegistryPickerViewModelTests::valueSelectionUsesObservedExistenceAndOperation() {
  QFETCH(int, mode);
  QFETCH(int, observation);
  using Model = presentation::RegistryPickerViewModel;
  const auto browser = std::make_shared<ControlledRegistryBrowser>();
  const auto data = observation == 3 ? std::nullopt : std::optional{observation == 2 ? std::vector<uint8_t>{} : std::vector<uint8_t>{'A', 0, 0, 0}};
  browser->valueEntries = {{"Named", 1u, data}};
  if (observation != 0) browser->valueEntries.push_back({{}, 1u, data});
  const auto observedCount = browser->valueEntries.size();
  Model model(static_cast<Model::Mode>(mode), browser);
  const auto key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor");
  test::loadRegistryValues(model, key);
  auto& values = model.valueModel();
  QCOMPARE(browser->valueEntries.size(), observedCount);
  QCOMPARE(values.rowCount(), 2);
  QCOMPARE(values.index(0, 0).data(presentation::RegistryValueModel::NameRole).toString(), QString{});
  QCOMPARE(values.index(0, 0).data().toString(), QStringLiteral("(Default)"));
  QCOMPARE(values.index(1, 0).data(presentation::RegistryValueModel::NameRole).toString(), QStringLiteral("Named"));
  const auto preview = observation == 3   ? QStringLiteral("Preview unavailable (value is too large)")
                       : observation == 2 ? QStringLiteral("(value not set)")
                                          : QStringLiteral("A");
  QCOMPARE(values.index(1, 2).data().toString(), preview);
  QCOMPARE(values.index(0, 2).data().toString(), observation == 0 ? QStringLiteral("(value not set)") : preview);
  for (int row = 0; row < values.rowCount(); ++row) {
    const auto name = values.index(row, 0).data(presentation::RegistryValueModel::NameRole).toString();
    const bool selectable = model.mode() == Model::Mode::CommitValue   ? row != 0 || observation != 0
                            : model.mode() == Model::Mode::DeleteValue ? row != 0
                                                                       : false;
    for (int column = 0; column < values.columnCount(); ++column) QCOMPARE(values.index(row, column).flags().testFlag(Qt::ItemIsSelectable), selectable);
    QCOMPARE(values.canSelectValue(name), selectable);
    // Exclusion mode accepts a key and deliberately ignores value selection.
    const bool accepted = model.mode() == Model::Mode::Exclusion || selectable;
    QCOMPARE(model.canAccept(key, std::optional{name}), accepted);
    model.accept(key, std::optional{name});
    QCOMPARE(model.selection().has_value(), accepted);
    if (accepted) QCOMPARE(model.selection()->valueName, model.supportsValueSelection() ? std::optional{name} : std::nullopt);
  }
  if (model.supportsValueSelection()) {
    QVERIFY(!values.canSelectValue(QStringLiteral("Not observed")));
    QVERIFY(!model.canAccept(key, QStringLiteral("Not observed")));
    model.accept(key, QStringLiteral("Not observed"));
    QVERIFY(!model.selection());
  }
  // The display-only default row never enters provider data or whole-key traversal.
  QVERIFY(model.canAccept(key));
  model.accept(key, {});
  QVERIFY(model.selection());
  QVERIFY(!model.selection()->valueName);
}

void RegistryPickerViewModelTests::valueSelectionRequiresTheCurrentSuccessfulObservation() {
  using Model = presentation::RegistryPickerViewModel;
  const auto browser = std::make_shared<ControlledRegistryBrowser>();
  browser->valueEntries = {{{}, 1u, std::nullopt}, {"Named", 1u, std::vector<uint8_t>{}}};
  Model model(Model::Mode::CommitValue, browser);
  const auto first = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\First");
  const auto second = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Second");
  const auto defaultName = std::optional{QString{}};
  QVERIFY(!model.canAccept(first, defaultName));
  test::loadRegistryValues(model, first);
  QVERIFY(model.canAccept(first, defaultName));
  model.loadValues(second);
  QVERIFY(!model.canAccept(first, defaultName));
  QVERIFY(!model.canAccept(second, defaultName));
  QTRY_VERIFY(!model.valuesLoading());
  QVERIFY(model.canAccept(QStringLiteral("hklm\\software\\second"), defaultName));
  QVERIFY(model.canAccept(second, QStringLiteral("nAmEd")));
  QVERIFY(!model.canAccept(first, defaultName));
  browser->denyValues = true;
  test::loadRegistryValues(model, second);
  QVERIFY(model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 0);
  QVERIFY(!model.canAccept(second, defaultName));
  QVERIFY(!model.canAccept(second, QStringLiteral("Named")));
  QVERIFY(model.canAccept(second));
}

void RegistryPickerViewModelTests::registryPickerAdmissionMatchesTheOperation_data() {
  using Model = presentation::RegistryPickerViewModel;
  using Availability = Model::KeyAvailability;
  struct Target {
    const char* name;
    const char* key;
    Availability exclusion;
    bool maintenance;
    bool keyDeletion;
  };
  const std::array targets{Target{"hive-root", "HKLM", Availability::ContainerOnly, true, false},
                           Target{"system", "HKLM\\SYSTEM", Availability::ContainerOnly, true, false},
                           Target{"current-control-set", "HKLM\\SYSTEM\\CurrentControlSet", Availability::ContainerOnly, true, true},
                           Target{"control-set-001", "HKLM\\SYSTEM\\ControlSet001\\Services", Availability::Selectable, true, true},
                           Target{"control-set-002", "HKLM\\SYSTEM\\ControlSet002\\Services", Availability::Selectable, true, true},
                           Target{"retargeted-extra", "HKLM\\SYSTEM\\CurrentControlSetExtra", Availability::Selectable, false, false},
                           Target{"retargeted-child", "HKLM\\SYSTEM\\CurrentControlSetExtra\\Child", Availability::Selectable, false, false},
                           Target{"retargeted-backup", "HKLM\\SYSTEM\\CurrentControlSetBackup", Availability::Selectable, false, false},
                           Target{"software", "HKLM\\SOFTWARE", Availability::ContainerOnly, true, false},
                           Target{"sam", "HKLM\\SAM", Availability::ContainerOnly, true, false},
                           Target{"security", "HKLM\\SECURITY", Availability::ContainerOnly, true, false},
                           Target{"components", "HKLM\\COMPONENTS", Availability::ContainerOnly, true, false},
                           Target{"hardware", "HKLM\\HARDWARE\\Vendor", Availability::Selectable, true, true},
                           Target{"bcd-root", "HKLM\\BCD00000000", Availability::Selectable, true, true},
                           Target{"custom", "HKLM\\Custom\\Vendor", Availability::Selectable, true, true},
                           Target{"reserved-deletion-prefix", "HKLM\\S", Availability::Selectable, true, false},
                           Target{"machine-account", "HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC", Availability::Pruned, true, true},
                           Target{"other-hive", "HKCU\\Software\\Vendor", Availability::Pruned, false, false},
                           Target{"unknown-hive", "HKLMOther\\Vendor", Availability::Pruned, false, false}};
  QTest::addColumn<int>("mode");
  QTest::addColumn<QString>("key");
  QTest::addColumn<int>("availability");
  QTest::addColumn<bool>("wholeKey");
  QTest::addColumn<bool>("namedValue");
  for (const auto mode : {Model::Mode::Exclusion, Model::Mode::CommitValue, Model::Mode::DeleteValue})
    for (const auto& target : targets) {
      const bool exclusion = mode == Model::Mode::Exclusion;
      const auto availability = exclusion ? target.exclusion : target.maintenance ? Availability::Selectable : Availability::Pruned;
      const bool selectable = availability == Availability::Selectable;
      QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(static_cast<int>(mode)).arg(QString::fromLatin1(target.name))))
          << int(mode) << QString::fromLatin1(target.key) << int(availability) << (selectable && (mode != Model::Mode::DeleteValue || target.keyDeletion))
          << selectable;
    }
}

void RegistryPickerViewModelTests::registryPickerAdmissionMatchesTheOperation() {
  QFETCH(int, mode);
  QFETCH(QString, key);
  QFETCH(int, availability);
  QFETCH(bool, wholeKey);
  QFETCH(bool, namedValue);
  const auto browserOwner = std::make_shared<ControlledRegistryBrowser>();
  using Model = presentation::RegistryPickerViewModel;
  Model model(static_cast<Model::Mode>(mode), browserOwner);
  test::loadRegistryValues(model, key);
  QCOMPARE(model.availabilityOf(key), static_cast<Model::KeyAvailability>(availability));
  QCOMPARE(model.canAccept(key), wholeKey);
  model.accept(key, {});
  QCOMPARE(model.selection().has_value(), wholeKey);
  QCOMPARE(model.canAccept(key, QStringLiteral("Named")), namedValue);
  model.accept(key, QStringLiteral("Named"));
  QCOMPARE(model.selection().has_value(), namedValue);
  if (model.selection()) QCOMPARE(model.selection()->valueName, model.mode() == Model::Mode::Exclusion ? std::nullopt : std::optional{QStringLiteral("Named")});
  const auto defaultName = std::optional{QString{}};
  const bool defaultAccepted = model.mode() == Model::Mode::Exclusion && namedValue;
  QCOMPARE(model.canAccept(key, defaultName), defaultAccepted);
  model.accept(key, defaultName);
  QCOMPARE(model.selection().has_value(), defaultAccepted);
  if (model.selection()) QCOMPARE(model.selection()->valueName, model.mode() == Model::Mode::Exclusion ? std::nullopt : defaultName);
  if (mode != int(Model::Mode::Exclusion) && key.contains(QStringLiteral("CurrentControlSetExtra"))) {
    model.setAvailabilityChecker([](const QString&) { return Model::KeyAvailability::Selectable; });
    QCOMPARE(model.availabilityOf(key), Model::KeyAvailability::Pruned);
    QVERIFY(!model.canAccept(key, QStringLiteral("Named")));
  }
  model.accept({}, {});
  QVERIFY(!model.selection());
}

void RegistryPickerViewModelTests::selectedRegistryNamesSurviveTheCompleteConfigurationBoundary_data() {
  QTest::addColumn<QString>("name");
  QTest::newRow("ordinary") << QStringLiteral("Vendor");
  QTest::newRow("literal-space") << QStringLiteral("Vendor ");
  QTest::newRow("literal-dot") << QStringLiteral("Vendor.");
}

void RegistryPickerViewModelTests::selectedRegistryNamesSurviveTheCompleteConfigurationBoundary() {
  QFETCH(QString, name);
  // These immutable browser entries describe the same objects seeded in the
  // shared UWF Fake below. The browser implements no UWF or persistence logic.
  class Browser final : public app::RegistryBrowser {
   public:
    std::vector<std::string> rootHiveLongNames(std::stop_token = {}) const override { return {"HKEY_LOCAL_MACHINE"}; }
    std::vector<std::string> subkeyNames(const std::string_view key, std::stop_token = {}) const override {
      if (key == "HKEY_LOCAL_MACHINE") return {"SOFTWARE"};
      if (key == "HKEY_LOCAL_MACHINE\\SOFTWARE") return {"Vendor", "Vendor ", "Vendor."};
      return {};
    }
    bool hasSubkeys(const std::string_view key, std::stop_token = {}) const override { return !subkeyNames(key).empty(); }
    std::vector<app::RegistryValueInfo> values(std::string_view, std::stop_token = {}) const override {
      throw std::logic_error("Exclusion selection must not request values");
    }
  };
  const auto browserOwner = std::make_shared<Browser>();
  const QString root = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\");
  constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
  test::FakeUwfBackend::State initial;
  initial.runtime.systemVolumeId = volumeId;
  initial.current.volumes.emplace(
      volumeId, test::FakeUwfBackend::VolumeConfiguration{.volume = {volumeId, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
  initial.next = initial.current;
  initial.next.filterEnabled = true;
  const auto system = std::make_shared<test::FakeUwfBackend>(initial);
  const QStringList names{QStringLiteral("Vendor"), QStringLiteral("Vendor "), QStringLiteral("Vendor.")};
  for (const auto& entry : names) system->seedRegistryValue((root + entry).toStdString(), "Setting", "base");
  const auto baseline = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {});
  presentation::ConfigurationViewModel configuration(0);
  const auto volume = configuration.addVolume({.driveLetter = "C:",
                                               .volumeName = "Volume{11111111-2222-3333-4444-555555555555}",
                                               .fileSystem = "NTFS",
                                               .label = {},
                                               .totalBytes = 0,
                                               .freeBytes = 0,
                                               .support = core::VolumeSupport::Supported},
                                              true);
  configuration.resetSnapshot(baseline);
  presentation::RegistryPickerViewModel picker(presentation::RegistryPickerViewModel::Mode::Exclusion, browserOwner);
  const auto index = test::navigateRegistry(picker, root + name);
  QVERIFY(index.isValid());
  QCOMPARE(index.data(presentation::PathRole).toString(), root + name);
  picker.accept(index.data(presentation::PathRole).toString(), {});
  QVERIFY(picker.selection());
  const auto selected = picker.selection()->key;
  QCOMPARE(selected, root + name);
  const auto addition = volume->registry()->addSelectedPath(selected);
  QCOMPARE(addition.outcome, app::ExclusionImportOutcome::Applied);
  QCOMPARE(addition.path, selected);
  const auto plan = app::planConfiguration(configuration.pendingChanges(), baseline);
  QCOMPARE(plan.changes.addRegistryExclusions, std::vector<std::string>{selected.toStdString()});
  const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(system)).apply(plan);
  QCOMPARE(result.failedOperations(), 0);
  QCOMPARE(system->snapshot().next.registryExclusions, std::vector<std::string>{selected.toStdString()});
  system->simulateRestart();
  for (const auto& entry : names) {
    system->writeRegistryValue((root + entry).toStdString(), "Setting", "changed");
    QCOMPARE(system->readRegistryValue((root + entry).toStdString(), "Setting"), std::optional<std::string>{entry == name ? "changed" : "base"});
  }
  system->simulateRestart();
  for (const auto& entry : names)
    QCOMPARE(system->visibleRegistryValue((root + entry).toStdString(), "Setting"), std::optional<std::string>{entry == name ? "changed" : "base"});
}

void RegistryPickerViewModelTests::registryAccessErrorsTrackTheRequestedKeyAndRecover() {
  const auto browserOwner = std::make_shared<ControlledRegistryBrowser>();
  auto& browser = *browserOwner;
  using Model = presentation::RegistryPickerViewModel;
  Model model(Model::Mode::CommitValue, browserOwner);
  QSignalSpy changed(&model, &Model::accessStateChanged);
  QVERIFY(test::awaitRegistryRoots(model));
  const auto root = model.index(0, 0);
  const auto rootPath = QStringLiteral("HKEY_LOCAL_MACHINE");
  test::loadRegistryValues(model, rootPath);
  QCOMPARE(model.valueModel().rowCount(), 2);
  browser.deniedChildren = rootPath.toStdString();
  QVERIFY(!test::loadRegistryChildren(model, root));
  QVERIFY(!changed.isEmpty());
  QVERIFY(model.accessError().has_value());
  QCOMPARE(model.accessError()->operation, Model::AccessError::Operation::Keys);
  QCOMPARE(model.accessError()->key, rootPath);
  QCOMPARE(model.accessError()->message, QStringLiteral("拒绝访问"));
  QCOMPARE(model.valueModel().rowCount(), 0);
  QVERIFY(!model.canFetchMore(root));

  test::loadRegistryValues(model, QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE"));
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);
  test::loadRegistryValues(model, rootPath);
  QVERIFY(model.accessError());
  QCOMPARE(model.accessError()->operation, Model::AccessError::Operation::Keys);
  QCOMPARE(model.valueModel().rowCount(), 0);
  browser.deniedChildren.clear();
  QVERIFY(test::loadRegistryChildren(model, root));
  QTRY_VERIFY(!model.valuesLoading());
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);

  const auto unselectedChild = model.index(0, 0, root);
  browser.deniedChildren = "HKEY_LOCAL_MACHINE\\SOFTWARE";
  QVERIFY(!test::loadRegistryChildren(model, unselectedChild));
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);
  browser.deniedChildren.clear();
  QVERIFY(test::loadRegistryChildren(model, unselectedChild));
  QVERIFY(!model.accessError());
  QCOMPARE(browser.lastValueKey, rootPath.toStdString());
  QCOMPARE(model.valueModel().rowCount(), 2);

  browser.denyValues = true;
  test::loadRegistryValues(model, QStringLiteral("HKEY_LOCAL_MACHINE\\Restricted"));
  QVERIFY(model.accessError());
  QCOMPARE(model.accessError()->operation, Model::AccessError::Operation::Values);
  QCOMPARE(model.accessError()->key, QStringLiteral("HKEY_LOCAL_MACHINE\\Restricted"));
  QCOMPARE(model.valueModel().rowCount(), 0);
  test::loadRegistryValues(model, {});
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 0);
  test::loadRegistryValues(model, rootPath);
  QVERIFY(model.accessError());
  model.resetTree();
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 0);
  browser.denyValues = false;
  test::loadRegistryValues(model, rootPath);
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);
}

void RegistryPickerViewModelTests::observationsOwnInputsAndCancelDelivery_data() {
  QTest::addColumn<int>("stage");
  QTest::addColumn<int>("action");
  QTest::addColumn<bool>("cooperative");
  for (int stage = 0; stage < 3; ++stage)
    for (int action = 0; action < 3; ++action)
      for (const bool cooperative : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("stage-%1-action-%2-cooperative-%3").arg(stage).arg(action).arg(cooperative)))
            << stage << action << cooperative;
}
void RegistryPickerViewModelTests::observationsOwnInputsAndCancelDelivery() {
  QFETCH(int, stage);
  QFETCH(int, action);
  QFETCH(bool, cooperative);
  using Model = presentation::RegistryPickerViewModel;
  auto browser = std::make_shared<GatedRegistryBrowser>();
  browser->blocked = static_cast<GatedRegistryBrowser::Stage>(stage);
  browser->honorCancellation = cooperative;
  const auto gate = browser->gate;
  const std::weak_ptr<GatedRegistryBrowser> lifetime = browser;
  auto model = std::make_unique<Model>(Model::Mode::CommitValue, browser);
  QSignalSpy roots(model.get(), &Model::rootsLoaded);
  QSignalSpy children(model.get(), &Model::childrenLoaded);
  if (stage != 0) QVERIFY(test::awaitRegistryRoots(*model));
  const auto root = model->index(0, 0);
  if (stage == 1) model->loadChildren(root);
  if (stage == 2) model->loadValues(QStringLiteral("HKEY_LOCAL_MACHINE\\Fast"));
  QTRY_VERIFY(gate->started.load() > 0);
  bool eventDelivered = false;
  QTimer::singleShot(0, this, [&] { eventDelivered = true; });
  QTRY_VERIFY(eventDelivered);
  QVERIFY(!browser->wrongThread);
  roots.clear();
  browser.reset();
  if (action == 0)
    model->cancel();
  else if (action == 1)
    model->resetTree();
  else
    model.reset();
  if (cooperative)
    QTRY_VERIFY(gate->canceled.load() > 0);
  else
    QVERIFY(!lifetime.expired());
  gate->release();
  QTRY_VERIFY(gate->finished.load() > 0);
  if (action == 1) {
    QTRY_VERIFY(model->rootsReady());
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->rowCount(model->index(0, 0)), 0);
    QCOMPARE(model->valueModel().rowCount(), 0);
  } else if (model) {
    QCoreApplication::sendPostedEvents();
    QCOMPARE(roots.count(), 0);
    QCOMPARE(children.count(), 0);
    QCOMPARE(model->valueModel().rowCount(), 0);
  } else
    QTRY_VERIFY(lifetime.expired());
}
void RegistryPickerViewModelTests::latestNavigationWinsWhileAParentIsLoading() {
  using Model = presentation::RegistryPickerViewModel;
  const auto browser = std::make_shared<GatedRegistryBrowser>();
  browser->blocked = GatedRegistryBrowser::Stage::Children;
  Model model(Model::Mode::CommitValue, browser);
  QVERIFY(test::awaitRegistryRoots(model));
  QSignalSpy finished(&model, &Model::navigationFinished);
  model.navigateTo(QStringLiteral("HKLM\\Slow"));
  QTRY_VERIFY(browser->gate->started.load() > 0);
  model.navigateTo(QStringLiteral("HKLM\\Fast"));
  browser->gate->release();
  QTRY_COMPARE(finished.count(), 1);
  QCOMPARE(finished.constFirst().constFirst().value<QModelIndex>().data(presentation::PathRole).toString(), QStringLiteral("HKEY_LOCAL_MACHINE\\Fast"));
  QVERIFY(!browser->wrongThread);
}
void RegistryPickerViewModelTests::reentrantValueRequestsKeepTheNewestKey() {
  using Model = presentation::RegistryPickerViewModel;
  const auto browser = std::make_shared<ControlledRegistryBrowser>();
  Model model(Model::Mode::CommitValue, browser);
  QVERIFY(test::awaitRegistryRoots(model));
  bool switched = false;
  connect(&model.valueModel(), &QAbstractItemModel::modelReset, &model, [&] {
    if (std::exchange(switched, true)) return;
    model.loadValues(QStringLiteral("HKEY_LOCAL_MACHINE\\Newest"));
  });
  model.loadValues(QStringLiteral("HKEY_LOCAL_MACHINE\\Old"));
  QTRY_VERIFY(!model.valuesLoading());
  QCOMPARE(browser->lastValueKey, std::string("HKEY_LOCAL_MACHINE\\Newest"));
  QCOMPARE(model.valueModel().rowCount(), 2);
}
void RegistryPickerViewModelTests::unselectedBranchFailureDoesNotReplaceCurrentDetails() {
  using Model = presentation::RegistryPickerViewModel;
  const auto browser = std::make_shared<GatedRegistryBrowser>();
  browser->blocked = GatedRegistryBrowser::Stage::Children;
  browser->blockedKey = "HKEY_LOCAL_MACHINE\\Slow";
  browser->failBlocked = true;
  Model model(Model::Mode::CommitValue, browser);
  QVERIFY(test::awaitRegistryRoots(model));
  const auto root = model.index(0, 0);
  QVERIFY(test::loadRegistryChildren(model, root));
  const auto slow = model.index(0, 0, root);
  model.loadChildren(slow);
  QTRY_VERIFY(browser->gate->started.load() > 0);
  test::loadRegistryValues(model, QStringLiteral("HKEY_LOCAL_MACHINE\\Fast"));
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);
  browser->gate->release();
  QTRY_VERIFY(!model.childrenLoading(slow));
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);
  QCOMPARE(model.valueModel().index(1, 0).data(presentation::RegistryValueModel::NameRole).toString(), QStringLiteral("HKEY_LOCAL_MACHINE\\Fast"));
}
void RegistryPickerViewModelTests::reentrantResetCoordinatesAllPublications_data() {
  QTest::addColumn<int>("publication");
  QTest::addColumn<bool>("before");
  const std::array names{"tree-clear", "roots-result", "children-result", "values-clear", "values-result"};
  for (std::size_t publication = 0; publication < names.size(); ++publication)
    for (const bool before : {false, true})
      QTest::newRow(
          qPrintable(QStringLiteral("%1/%2").arg(QString::fromLatin1(names[publication]), before ? QStringLiteral("about") : QStringLiteral("complete"))))
          << static_cast<int>(publication) << before;
}
void RegistryPickerViewModelTests::reentrantResetCoordinatesAllPublications() {
  QFETCH(int, publication);
  QFETCH(bool, before);
  using Model = presentation::RegistryPickerViewModel;
  const auto browser = std::make_shared<ControlledRegistryBrowser>();
  Model model(Model::Mode::CommitValue, browser);
  QAbstractItemModelTester treeContract(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
  treeContract.setUseFetchMore(false);
  QAbstractItemModelTester valuesContract(&model.valueModel(), QAbstractItemModelTester::FailureReportingMode::QtTest);
  QVERIFY(test::awaitRegistryRoots(model));
  if (publication >= 3) test::loadRegistryValues(model, QStringLiteral("HKEY_LOCAL_MACHINE\\Existing"));
  int treeDepth = 0, valueDepth = 0, maxTreeDepth = 0, maxValueDepth = 0, valueTransactions = 0;
  connect(&model, &QAbstractItemModel::modelAboutToBeReset, &model, [&] { maxTreeDepth = std::max(maxTreeDepth, ++treeDepth); });
  connect(&model, &QAbstractItemModel::modelReset, &model, [&] { --treeDepth; });
  connect(&model, &QAbstractItemModel::rowsAboutToBeInserted, &model, [&] { maxTreeDepth = std::max(maxTreeDepth, ++treeDepth); });
  connect(&model, &QAbstractItemModel::rowsInserted, &model, [&] { --treeDepth; });
  connect(&model.valueModel(), &QAbstractItemModel::modelAboutToBeReset, &model, [&] {
    maxValueDepth = std::max(maxValueDepth, ++valueDepth);
    ++valueTransactions;
  });
  connect(&model.valueModel(), &QAbstractItemModel::modelReset, &model, [&] { --valueDepth; });
  bool resetRequested = false;
  const auto requestReset = [&] {
    if (!std::exchange(resetRequested, true)) model.resetTree();
  };
  if (publication <= 1) {
    const auto treePublished = [&] {
      const bool rootResult = before ? model.rowCount() == 0 : model.rootsReady();
      if (publication == 0 || rootResult) requestReset();
    };
    if (before)
      connect(&model, &QAbstractItemModel::modelAboutToBeReset, &model, treePublished);
    else
      connect(&model, &QAbstractItemModel::modelReset, &model, treePublished);
  } else if (publication == 2) {
    if (before)
      connect(&model, &QAbstractItemModel::rowsAboutToBeInserted, &model, requestReset);
    else
      connect(&model, &QAbstractItemModel::rowsInserted, &model, requestReset);
  } else {
    const auto valuesPublished = [&] {
      if (valueTransactions == (publication == 3 ? 1 : 2)) requestReset();
    };
    if (before)
      connect(&model.valueModel(), &QAbstractItemModel::modelAboutToBeReset, &model, valuesPublished);
    else
      connect(&model.valueModel(), &QAbstractItemModel::modelReset, &model, valuesPublished);
  }
  QSignalSpy completed(&model, &Model::rootsLoaded);
  QSignalSpy children(&model, &Model::childrenLoaded);
  if (publication <= 1)
    model.resetTree();
  else if (publication == 2)
    model.loadChildren(model.index(0, 0));
  else
    model.loadValues(QStringLiteral("HKEY_LOCAL_MACHINE\\Old"));
  QTRY_COMPARE(completed.count(), 1);
  QVERIFY(resetRequested);
  QVERIFY(model.rootsReady());
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(model.rowCount(model.index(0, 0)), 0);
  QCOMPARE(model.valueModel().rowCount(), 0);
  QVERIFY(!model.valuesLoading());
  QVERIFY(!model.accessError());
  QCOMPARE(children.count(), 0);
  QCOMPARE(treeDepth, 0);
  QCOMPARE(valueDepth, 0);
  QCOMPARE(maxTreeDepth, 1);
  QCOMPARE(maxValueDepth, 1);
  if (publication == 3) QCOMPARE(browser->lastValueKey, std::string("HKEY_LOCAL_MACHINE\\Existing"));
  if (publication == 4) QCOMPARE(browser->lastValueKey, std::string("HKEY_LOCAL_MACHINE\\Old"));
}

}  // namespace

QTEST_GUILESS_MAIN(RegistryPickerViewModelTests)
#include "RegistryPickerViewModelTests.moc"
