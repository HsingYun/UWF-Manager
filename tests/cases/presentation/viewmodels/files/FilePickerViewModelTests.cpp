// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QThreadPool>
#include <QTranslator>
#include <algorithm>
#include <array>
#include <atomic>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "application/configuration/editing/ConfigurationEditSession.h"
#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/volumes/VolumeConfigurationEditor.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/editing/FileStagingEditor.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/filesystem/browsing/LocalFileBrowser.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/configuration/ExclusionListViewModel.h"
#include "presentation/viewmodels/files/FilePickerViewModel.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/WorkerGate.h"
#include "support/uwf/FakeUwfBackend.h"
#include "support/windows/LiteralFileBrowserFixture.h"

using namespace uwf;
using Model = presentation::FilePickerViewModel;
using State = presentation::ContentLoadState;
namespace {
void writeFile(const QString& path, const QByteArray& content = "original content") {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size()) qFatal("Unable to create test file");
}
int findRow(const Model& model, const QString& name) {
  for (int row = 0; row < model.rowCount(); ++row)
    if (model.data(model.index(row, 0)).toString() == name) return row;
  return -1;
}
Model::Acceptance completedAcceptance(Model& model) {
  if (!QTest::qWaitFor([&] { return !model.busy(); })) {
    QTest::qFail("File query did not finish", __FILE__, __LINE__);
    return Model::Acceptance::Invalid;
  }
  QSignalSpy completed(&model, &Model::acceptanceFinished);
  model.requestAcceptance();
  if (!QTest::qWaitFor([&] { return !completed.isEmpty(); }, 5000)) {
    QTest::qFail("File selection did not finish", __FILE__, __LINE__);
    return Model::Acceptance::Invalid;
  }
  return completed.constFirst().constFirst().value<Model::Acceptance>();
}
struct GuidAddress {
  QString root;
  QString path;
};
std::optional<GuidAddress> guidAddressOf(const QString& drivePath) {
  // An independent Windows oracle: production path helpers must preserve the
  // address that the OS gives this fixture, including literal leaf characters.
  const auto native = QDir::toNativeSeparators(drivePath).toStdWString();
  std::array<wchar_t, 32768> mount{};
  std::array<wchar_t, MAX_PATH + 1> volume{};
  if (!GetVolumePathNameW(native.c_str(), mount.data(), static_cast<DWORD>(mount.size())) ||
      !GetVolumeNameForVolumeMountPointW(mount.data(), volume.data(), static_cast<DWORD>(volume.size())))
    return std::nullopt;
  const auto prefix = std::char_traits<wchar_t>::length(mount.data());
  if (native.size() < prefix) return std::nullopt;
  const auto root = QString::fromWCharArray(volume.data());
  return GuidAddress{root, root + QString::fromStdWString(native.substr(prefix))};
}
class ControlledBrowser final : public app::FileBrowser {
 public:
  enum class Io { Places, InitialInspection, Listing, Acceptance, Create, Rename, None };
  std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
  std::atomic<Io> blocked{Io::None};
  std::atomic_bool reject{false};
  mutable std::atomic_bool wrongThread{false};
  mutable std::atomic_int placeReads{0}, listReads{0}, inspections{0}, writes{0};
  std::thread::id uiThread = std::this_thread::get_id();
  void visit(const Io operation) const {
    if (std::this_thread::get_id() == uiThread) wrongThread = true;
    if (blocked == operation) {
      gate->wait({}, false);
      if (reject) throw std::runtime_error("controlled I/O failure");
    }
  }
  QVector<app::FileBrowserEntry> list(const QString& path, std::stop_token) const override {
    visit(Io::Listing);
    ++listReads;
    return {{path + QStringLiteral("/file.txt"), QStringLiteral("file.txt")}};
  }
  app::FileBrowserEntry inspect(const QString& path) const override {
    visit(blocked == Io::Acceptance ? Io::Acceptance : Io::InitialInspection);
    ++inspections;
    return {path, QFileInfo(path).fileName(), !path.endsWith(QLatin1String(".txt"))};
  }
  QVector<app::FileBrowserPlace> places() const override {
    visit(Io::Places);
    ++placeReads;
    return {{QStringLiteral("Test"), QStringLiteral("C:/test")}};
  }
  void createDirectory(const QString&) const override {
    visit(Io::Create);
    ++writes;
  }
  void renameEntry(const QString&, const QString&) const override {
    visit(Io::Rename);
    ++writes;
  }
};
class QueryBrowser final : public app::FileBrowser {
 public:
  std::shared_ptr<test::WorkerGate> listingGate;
  QVector<app::FileBrowserEntry> list(const QString& path, const std::stop_token stop) const override {
    if (listingGate) listingGate->wait(stop);
    return {{path + QStringLiteral("/Alpha.txt"), QStringLiteral("Alpha.txt")},
            {path + QStringLiteral("/Beta.cmd"), QStringLiteral("Beta.cmd")},
            {path + QStringLiteral("/Gamma.txt"), QStringLiteral("Gamma.txt")},
            {path + QStringLiteral("/delta.txt"), QStringLiteral("delta.txt")}};
  }
  app::FileBrowserEntry inspect(const QString& path) const override { return {path, QFileInfo(path).fileName(), true}; }
  QVector<app::FileBrowserPlace> places() const override { return {}; }
  void createDirectory(const QString&) const override {}
};
class DelayedBrowser final : public app::FileBrowser {
 public:
  std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
  QVector<app::FileBrowserEntry> list(const QString& path, const std::stop_token stop) const override {
    if (path.endsWith(QLatin1String("/slow"))) {
      gate->wait(stop);
      return {{path + QStringLiteral("/stale.txt"), QStringLiteral("stale.txt")}};
    }
    if (path.endsWith(QLatin1String("/denied"))) throw std::runtime_error("Access denied: complete diagnostic");
    return {{path + QStringLiteral("/current.txt"), QStringLiteral("current.txt")}};
  }
  app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, !path.endsWith(QLatin1String(".txt"))}; }
  QVector<app::FileBrowserPlace> places() const override { return {}; }
  void createDirectory(const QString&) const override {}
};
}  // namespace
class FilePickerViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void guidDirectoriesKeepExactBrowserAndAcceptedFileAddresses() {
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    const QStringList names{QStringLiteral("Entry"), QStringLiteral("Entry "), QStringLiteral("Entry.")};
    for (const auto& name : names) QVERIFY2(fixture.seed(name, name.toUtf8()), qPrintable(fixture.error));
    const auto address = guidAddressOf(fixture.root.path());
    QVERIFY(address);
    QVERIFY(address->root.startsWith(QStringLiteral("\\\\?\\Volume{")));
    Model model(Model::Mode::OpenFile, fixture.browser);
    model.setAllowedRoots({address->path});
    model.start(address->path);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.path(), address->path);
    QCOMPARE(model.rowCount(), names.size());
    for (const auto& name : names) {
      const auto target = address->path + u'\\' + name;
      const auto observed = fixture.browser->inspect(target);
      QVERIFY(observed.exists);
      QVERIFY(!observed.directory);
      QCOMPARE(observed.path, target);
      QCOMPARE(observed.name, name);
      QCOMPARE(observed.size, name.toUtf8().size());
      const auto row = findRow(model, name);
      QVERIFY(row >= 0);
      QCOMPARE(model.entryPath(row), target);
      model.selectRows({row});
      QCOMPARE(model.selectedEntries(), QStringList{target});
      QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
      QCOMPARE(model.selectedPaths(), QStringList{target});
      QCOMPARE(model.selectedItems().size(), 1);
      QCOMPARE(model.selectedItems().constFirst().path, target);
      QCOMPARE(model.selectedItems().constFirst().size, name.toUtf8().size());
      QCOMPARE(fixture.read(name), std::optional<QByteArray>{name.toUtf8()});
    }
    Model directory(Model::Mode::Directory, fixture.browser);
    directory.start(address->path);
    QTRY_VERIFY(!directory.busy());
    QCOMPARE(completedAcceptance(directory), Model::Acceptance::Ready);
    QCOMPARE(directory.selectedPaths(), QStringList{address->path});
    QVERIFY(directory.selectedItems().constFirst().directory);
    QCOMPARE(fixture.browser->creates.load(), 0);
    QCOMPARE(fixture.browser->renames.load(), 0);
  }
  void guidRelativeNavigationAndAllowedRootsKeepTheVolumeBoundary() {
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QVERIFY(QDir(temporary.path()).mkpath(QStringLiteral("Child/Grand")));
    const auto address = guidAddressOf(temporary.path());
    QVERIFY(address);
    Model model(Model::Mode::Directory, windows::systemFileBrowser());
    model.setAllowedRoots({address->path});
    model.start(address->path);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.path(), address->path);
    QVERIFY(!model.canGoUp());
    QVERIFY(!model.isPathAllowed(address->path + QStringLiteral("-sibling\\outside.txt")));
    model.navigate(QStringLiteral("Child"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.path(), address->path + QStringLiteral("\\Child"));
    QVERIFY(model.canGoUp());
    model.navigate(QStringLiteral("Grand"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.path(), address->path + QStringLiteral("\\Child\\Grand"));
    model.navigate(QStringLiteral(".."));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.path(), address->path + QStringLiteral("\\Child"));
    model.goUp();
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.path(), address->path);
    QSignalSpy errors(&model, &Model::validationFailed);
    model.navigate(QStringLiteral(".."));
    QCOMPARE(errors.size(), 1);
    QCOMPARE(model.path(), address->path);
    QVERIFY(!model.canGoUp());

    // The actual volume root has no parent in this namespace. Up/relative '..'
    // must not turn it into a filesystem path under the current working folder.
    Model root(Model::Mode::Directory, windows::systemFileBrowser());
    root.start(address->root);
    QTRY_VERIFY(!root.busy());
    QCOMPARE(root.path(), address->root);
    QVERIFY(!root.canGoUp());
    root.goUp();
    QCOMPARE(root.path(), address->root);
    root.navigate(QStringLiteral("..\\.."));
    QTRY_VERIFY(!root.busy());
    QCOMPARE(root.path(), address->root);
    QVERIFY(!root.canGoUp());
  }
  void guidCreateAndRenameAffectOnlyTheSelectedTemporaryObjects() {
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    QVERIFY2(fixture.seed(QStringLiteral("original.txt"), "selected contents"), qPrintable(fixture.error));
    QVERIFY2(fixture.seed(QStringLiteral("existing.txt"), "keep sibling"), qPrintable(fixture.error));
    const auto address = guidAddressOf(fixture.root.path());
    QVERIFY(address);
    Model model(Model::Mode::Mixed, fixture.browser);
    model.setAllowedRoots({address->path});
    model.start(address->path);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.loadState(), State::Ready);
    QSignalSpy errors(&model, &Model::validationFailed);
    model.createDirectory(QStringLiteral("Created"));
    QTRY_VERIFY(!model.busy());
    const auto createdRow = findRow(model, QStringLiteral("Created"));
    QVERIFY(createdRow >= 0);
    QCOMPARE(model.entryPath(createdRow), address->path + QStringLiteral("\\Created"));
    const auto attributes = GetFileAttributesW(QDir::toNativeSeparators(fixture.path(QStringLiteral("Created"))).toStdWString().c_str());
    QVERIFY(attributes != INVALID_FILE_ATTRIBUTES);
    QVERIFY((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    model.renameEntry(findRow(model, QStringLiteral("original.txt")), QStringLiteral("renamed.txt"));
    QTRY_VERIFY(!model.busy());
    const auto renamedRow = findRow(model, QStringLiteral("renamed.txt"));
    QVERIFY(renamedRow >= 0);
    QCOMPARE(model.entryPath(renamedRow), address->path + QStringLiteral("\\renamed.txt"));
    QVERIFY(!fixture.exists(QStringLiteral("original.txt")));
    QCOMPARE(fixture.read(QStringLiteral("renamed.txt")), std::optional<QByteArray>{"selected contents"});
    QCOMPARE(fixture.read(QStringLiteral("existing.txt")), std::optional<QByteArray>{"keep sibling"});
    model.renameEntry(findRow(model, QStringLiteral("Created")), QStringLiteral("RenamedFolder"));
    QTRY_VERIFY(!model.busy());
    const auto folderRow = findRow(model, QStringLiteral("RenamedFolder"));
    QVERIFY(folderRow >= 0);
    QVERIFY(model.isDirectory(folderRow));
    QCOMPARE(model.entryPath(folderRow), address->path + QStringLiteral("\\RenamedFolder"));
    QVERIFY(!fixture.exists(QStringLiteral("Created")));
    QVERIFY(fixture.exists(QStringLiteral("RenamedFolder")));
    QCOMPARE(errors.size(), 0);
    QCOMPARE(fixture.browser->creates.load(), 1);
    QCOMPARE(fixture.browser->renames.load(), 2);
  }
  void guidSaveFileAcceptsANewTargetWithoutCreatingIt() {
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    QVERIFY2(fixture.seed(QStringLiteral("existing.txt"), "keep existing"), qPrintable(fixture.error));
    const auto address = guidAddressOf(fixture.root.path());
    QVERIFY(address);
    const auto target = address->path + QStringLiteral("\\new.txt");
    Model model(Model::Mode::SaveFile, fixture.browser);
    model.setAllowedRoots({address->path});
    model.setFilters(QStringLiteral("Text (*.txt)"));
    model.start(target);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.path(), address->path);
    QCOMPARE(model.filename(), QStringLiteral("new.txt"));
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
    QCOMPARE(model.selectedPaths(), QStringList{target});
    QCOMPARE(model.selectedItems().size(), 1);
    QVERIFY(!model.selectedItems().constFirst().exists);
    QVERIFY(!fixture.exists(QStringLiteral("new.txt")));

    model.setFilename(QStringLiteral("typed-new"));
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
    QCOMPARE(model.selectedPaths(), QStringList{address->path + QStringLiteral("\\typed-new.txt")});
    QVERIFY(!fixture.exists(QStringLiteral("typed-new.txt")));

    const auto existing = address->path + QStringLiteral("\\existing.txt");
    model.start(existing);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.path(), address->path);
    QCOMPARE(model.filename(), QStringLiteral("existing.txt"));
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Replace);
    QCOMPARE(model.selectedPaths(), QStringList{existing});
    QCOMPARE(fixture.read(QStringLiteral("existing.txt")), std::optional<QByteArray>{"keep existing"});
    QCOMPARE(fixture.browser->creates.load(), 0);
    QCOMPARE(fixture.browser->renames.load(), 0);
  }
  void extendedDrivePickerSelectionCanBeStagedAndRestoredThroughItsOrdinaryAlias() {
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const auto driveFile = QDir::toNativeSeparators(temporary.filePath(QStringLiteral("selected.txt")));
    writeFile(driveFile, "selected staging contents");
    const auto address = guidAddressOf(driveFile);
    QVERIFY(address);
    std::array<wchar_t, 32768> mounts{};
    DWORD required = 0;
    const auto volumeRoot = address->root.toStdWString();
    QVERIFY(GetVolumePathNamesForVolumeNameW(volumeRoot.c_str(), mounts.data(), static_cast<DWORD>(mounts.size()), &required));
    QString actualDrive;
    for (const auto* mount = mounts.data(); *mount; mount += std::char_traits<wchar_t>::length(mount) + 1) {
      const auto size = std::char_traits<wchar_t>::length(mount);
      if (size == 3 && mount[1] == L':' && mount[2] == L'\\') {
        actualDrive = QString::fromWCharArray(mount, 2).toUpper();
        break;
      }
    }
    QVERIFY(!actualDrive.isEmpty());
    const auto extendedDirectory = QStringLiteral("\\\\?\\") + QDir::toNativeSeparators(temporary.path());
    const auto extendedFile = QStringLiteral("\\\\?\\") + driveFile;
    Model picker(Model::Mode::OpenFile, windows::systemFileBrowser());
    picker.start(extendedDirectory);
    QTRY_VERIFY(!picker.busy());
    QCOMPARE(picker.loadState(), State::Ready);
    QCOMPARE(picker.path(), extendedDirectory);
    const auto row = findRow(picker, QStringLiteral("selected.txt"));
    QVERIFY(row >= 0);
    QCOMPARE(picker.entryPath(row), extendedFile);
    picker.selectRows({row});
    QCOMPARE(completedAcceptance(picker), Model::Acceptance::Ready);
    QCOMPARE(picker.selectedPaths(), QStringList{extendedFile});
    QCOMPARE(picker.selectedItems().size(), 1);
    QList<app::FileStagingEntry> selections;
    for (const auto& item : picker.selectedItems())
      selections.append({item.directory ? app::FileStagingKind::Directory : app::FileStagingKind::File, item.path});

    // Only the filesystem traversal source is real. Persistence stays in this
    // memory store; no UWF maintenance, service or power operation is involved.
    const auto source = windows::systemCommitTraversalSource();
    const auto ordinaryTarget = source->resolveFileTarget(driveFile);
    QCOMPARE(source->resolveFileTarget(extendedFile), ordinaryTarget);
    const auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor editor(actualDrive, draft, source);
    editor.load();
    QSignalSpy rejected(&editor, &app::FileStagingEditing::pathRejected);
    QSignalSpy updateFailed(&editor, &app::FileStagingEditing::updateFailed);
    editor.addEntries(selections);
    QCOMPARE(rejected.size(), 0);
    QCOMPARE(updateFailed.size(), 0);
    QVERIFY(editor.storageAvailable());
    QCOMPARE(editor.entries().size(), 1);
    QCOMPARE(editor.entries().constFirst().path, extendedFile);
    QVERIFY(editor.entries().constFirst().fileTarget);
    QCOMPARE(*editor.entries().constFirst().fileTarget, ordinaryTarget);
    const auto added = draft.changes();
    QVERIFY(added);
    draft.apply(*added);
    QCOMPARE(store->writes, 1);
    QCOMPARE(store->entries.size(), 1);
    QCOMPARE(store->entries.constFirst().path, extendedFile);
    draft.reload();
    editor.load();
    QVERIFY(!draft.changes());
    QCOMPARE(editor.entries().size(), 1);
    QCOMPARE(editor.entries().constFirst().path, extendedFile);

    editor.addEntries({{app::FileStagingKind::File, driveFile}});
    QCOMPARE(rejected.size(), 0);
    QCOMPARE(editor.entries().size(), 1);
    QVERIFY(editor.entries().constFirst().fileTarget);
    QCOMPARE(*editor.entries().constFirst().fileTarget, ordinaryTarget);
    QCOMPARE(editor.entries().constFirst().path, driveFile);
    // Choosing another address is an explicit edit, even though read-only
    // projection still deduplicates aliases of this same object.
    const auto replacement = draft.changes();
    QVERIFY(replacement);
    QCOMPARE(replacement->count(), std::size_t{2});
    QCOMPARE(replacement->added().constFirst().path, driveFile);
    QCOMPARE(replacement->removed().constFirst().path, extendedFile);
    editor.removePaths({driveFile});
    QVERIFY(editor.entries().isEmpty());
    QVERIFY(draft.changes());
    editor.restorePaths({extendedFile});
    QCOMPARE(rejected.size(), 0);
    QCOMPARE(updateFailed.size(), 0);
    QCOMPARE(editor.entries().size(), 1);
    QCOMPARE(editor.entries().constFirst().path, extendedFile);
    QVERIFY(editor.entries().constFirst().fileTarget);
    QCOMPARE(*editor.entries().constFirst().fileTarget, ordinaryTarget);
    QVERIFY(!draft.changes());
    QCOMPARE(store->writes, 1);
  }

  void placesPublicationKeepsParentAndChildTransactionsSafe_data() {
    QTest::addColumn<bool>("after");
    QTest::addColumn<bool>("destroyOwner");
    for (const bool after : {false, true})
      for (const bool destroyOwner : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2").arg(after ? "published" : "about-to-publish", destroyOwner ? "destroy" : "reenter")))
            << after << destroyOwner;
  }
  void placesPublicationKeepsParentAndChildTransactionsSafe() {
    QFETCH(bool, after);
    QFETCH(bool, destroyOwner);
    auto owner = std::make_unique<Model>(Model::Mode::OpenFile, std::make_shared<ControlledBrowser>());
    owner->start(QStringLiteral("C:/test"));
    QTRY_VERIFY(!owner->busy());
    auto* places = owner->placesModel();
    const QPointer alive(owner.get());
    int depth = 0, maximumDepth = 0;
    bool observed = false;
    QObject subscriptions;
    connect(places, &QAbstractItemModel::modelAboutToBeReset, &subscriptions, [&] { maximumDepth = std::max(maximumDepth, ++depth); });
    connect(places, &QAbstractItemModel::modelReset, &subscriptions, [&] { --depth; });
    const auto observer = [&] {
      if (std::exchange(observed, true)) return;
      if (destroyOwner)
        owner.reset();
      else
        owner->setAllowedRoots({QStringLiteral("C:/elsewhere")});
    };
    if (after)
      connect(places, &QAbstractItemModel::modelReset, &subscriptions, observer);
    else
      connect(places, &QAbstractItemModel::modelAboutToBeReset, &subscriptions, observer);
    owner->setAllowedRoots({QStringLiteral("C:/test")});
    QVERIFY(observed);
    QCOMPARE(maximumDepth, 1);
    if (destroyOwner)
      QVERIFY(!alive);
    else {
      QTRY_VERIFY(!owner->busy());
      QCOMPARE(places->rowCount(), 0);
      QCOMPARE(depth, 0);
    }
  }
  void standardPlacesTranslateWithoutChangingPathsOrReadingTheBrowserAgain() {
    using Kind = app::FileBrowserPlaceKind;
    class Browser final : public app::FileBrowser {
     public:
      mutable int placeReads = 0;
      mutable std::atomic_int listReads = 0;
      QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override {
        ++listReads;
        return {};
      }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override {
        ++placeReads;
        return {{QStringLiteral("provider home"), QStringLiteral("C:/Users/Actual"), false, false, Kind::Home},
                {QStringLiteral("provider desktop"), QStringLiteral("D:/Redirected/Work"), false, false, Kind::Desktop},
                {QStringLiteral("provider downloads"), QStringLiteral("D:/Inbox"), false, false, Kind::Downloads},
                {QStringLiteral("provider documents"), QStringLiteral("D:/Projects"), false, false, Kind::Documents},
                {QStringLiteral("Desktop"), QStringLiteral("C:/Custom")},
                {QStringLiteral("Downloads (E:)"), QStringLiteral("E:/"), true, false}};
      }
      void createDirectory(const QString&) const override {}
    };
    class Translator final : public QTranslator {
     public:
      QString language;
      explicit Translator(QString value) : language(std::move(value)) {}
      bool isEmpty() const override { return false; }
      QString translate(const char* context, const char* source, const char*, int) const override {
        return QByteArray(context) == "uwf" ? language + QString::fromUtf8(source) : QString{};
      }
    } first(QStringLiteral("first:")), second(QStringLiteral("second:"));
    auto browser = std::make_shared<Browser>();
    Model model(Model::Mode::Directory, browser);
    auto* places = model.placesModel();
    QAbstractItemModelTester contract(places, QAbstractItemModelTester::FailureReportingMode::QtTest);
    model.start(QStringLiteral("D:/Redirected/Work"));
    QTRY_COMPARE(model.loadState(), State::Empty);
    QCOMPARE(browser->placeReads, 1);
    QCOMPARE(browser->listReads.load(), 1);
    QPersistentModelIndex selected(places->index(2, 0));
    QCOMPARE(selected.data(presentation::PathRole).toString(), QStringLiteral("D:/Redirected/Work"));
    QCOMPARE(selected.data(Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Checked));
    QSignalSpy resets(places, &QAbstractItemModel::modelReset);
    QSignalSpy changed(places, &QAbstractItemModel::dataChanged);
    for (auto* translator : {&first, &second}) {
      QVERIFY(QCoreApplication::installTranslator(translator));
      model.retranslate();
      QCOMPARE(places->index(0, 0).data().toString(), translator->language + QStringLiteral("Quick access"));
      const QStringList names{QStringLiteral("Home"), QStringLiteral("Desktop"), QStringLiteral("Downloads"), QStringLiteral("Documents")};
      for (int row = 0; row < names.size(); ++row) {
        QCOMPARE(places->index(row + 1, 0).data().toString(), translator->language + names[row]);
        QCOMPARE(places->index(row + 1, 0).data(Qt::AccessibleTextRole), places->index(row + 1, 0).data());
      }
      QCOMPARE(places->index(5, 0).data().toString(), QStringLiteral("Desktop"));
      QCOMPARE(places->index(6, 0).data().toString(), translator->language + QStringLiteral("This PC"));
      QCOMPARE(places->index(7, 0).data().toString(), QStringLiteral("Downloads (E:)"));
      QVERIFY(selected.isValid());
      QCOMPARE(selected.data(presentation::PathRole).toString(), QStringLiteral("D:/Redirected/Work"));
      QCOMPARE(selected.data(Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Checked));
      QCOMPARE(browser->placeReads, 1);
      QCOMPARE(browser->listReads.load(), 1);
      QVERIFY(QCoreApplication::removeTranslator(translator));
    }
    QCOMPARE(resets.size(), 0);
    QCOMPARE(changed.size(), 2);
    QCOMPARE(model.places()[1].name, QStringLiteral("provider desktop"));
    QCOMPARE(selected.data().toString(), QStringLiteral("Desktop"));
  }
  void windowsPlacesIdentifyStandardLocationsWithoutChangingTheirPaths() {
    using Kind = app::FileBrowserPlaceKind;
    const auto places = windows::systemFileBrowser()->places();
    const std::array standard{std::pair{QStandardPaths::HomeLocation, Kind::Home}, std::pair{QStandardPaths::DesktopLocation, Kind::Desktop},
                              std::pair{QStandardPaths::DownloadLocation, Kind::Downloads}, std::pair{QStandardPaths::DocumentsLocation, Kind::Documents}};
    for (const auto& [location, kind] : standard) {
      const auto expected = QStandardPaths::writableLocation(location);
      const auto found = std::find_if(places.begin(), places.end(), [kind](const auto& place) { return place.kind == kind; });
      if (expected.isEmpty()) {
        QVERIFY(found == places.end());
      } else {
        QVERIFY(found != places.end());
        QCOMPARE(QDir::cleanPath(found->path), QDir::cleanPath(expected));
        QVERIFY(!found->volume);
      }
    }
    for (const auto& place : places)
      if (place.volume) QCOMPARE(place.kind, Kind::Custom);
  }
  void fileBrowserFailuresUsePresentationTranslations_data() {
    using Error = app::FileBrowserErrorCode;
    QTest::addColumn<Error>("code");
    QTest::addColumn<QString>("message");
    const std::array messages{
        std::pair{Error::MissingDirectory, "The folder does not exist."},
        std::pair{Error::AccessDenied, "Access denied."},
        std::pair{Error::AlreadyExists, "An item with this name already exists."},
        std::pair{Error::CreateDirectoryFailed, "The folder could not be created. Check the name and permissions."},
        std::pair{Error::RenameFailed, "The item could not be renamed. Check the name and permissions."},
        std::pair{Error::RenameUnsupported, "Renaming is not supported by this browser."},
    };
    for (const auto& [code, text] : messages) QTest::newRow(text) << code << QString::fromUtf8(text);
  }
  void fileBrowserFailuresUsePresentationTranslations() {
    QFETCH(app::FileBrowserErrorCode, code);
    QFETCH(QString, message);
    class Translator final : public QTranslator {
     public:
      bool isEmpty() const override { return false; }
      QString translate(const char* context, const char* source, const char*, int) const override {
        return QByteArray(context) == "uwf" ? QStringLiteral("localized: ") + QString::fromUtf8(source) : QString();
      }
    } translator;
    class Browser final : public app::FileBrowser {
     public:
      app::FileBrowserErrorCode failure;
      explicit Browser(app::FileBrowserErrorCode code) : failure(code) {}
      QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override { throw app::FileBrowserError(failure); }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString&) const override {}
    };
    QVERIFY(QCoreApplication::installTranslator(&translator));
    Model model(Model::Mode::OpenFile, std::make_shared<Browser>(code));
    model.start(QStringLiteral("C:/test"));
    QTRY_COMPARE(model.loadState(), State::Failed);
    QCOMPARE(model.error(), QStringLiteral("localized: ") + message);
  }
  void slowReadsRevealProgressAndKeepItsMinimumVisibleDuration() {
    auto browser = std::make_shared<DelayedBrowser>();
    const auto unblock = qScopeGuard([&] { browser->gate->release(); });
    Model model(Model::Mode::OpenFile, browser);
    QElapsedTimer visible;
    connect(&model, &Model::stateChanged, this, [&] {
      if (model.progressVisible() && !visible.isValid()) visible.start();
    });
    model.start(QStringLiteral("C:/slow"));
    QVERIFY(model.busy());
    QVERIFY(!model.progressVisible());
    QTRY_VERIFY(model.progressVisible());
    QVERIFY(visible.isValid());
    browser->gate->release();
    QVERIFY(model.busy());
    QTRY_VERIFY(!model.busy());
    QVERIFY(visible.elapsed() >= 950);
    QVERIFY(!model.progressVisible());
    QCOMPARE(model.loadState(), State::Ready);
  }
  void driveScopeFiltersPlacesNavigationAndTypedAcceptance() {
    class Browser final : public app::FileBrowser {
     public:
      mutable int outsideInspections = 0;
      QVector<app::FileBrowserEntry> list(const QString& path, std::stop_token) const override {
        return {{QDir(path).filePath(QStringLiteral("local.txt")), QStringLiteral("local.txt")},
                {QStringLiteral("D:/outside.txt"), QStringLiteral("outside.txt")}};
      }
      app::FileBrowserEntry inspect(const QString& path) const override {
        if (path.startsWith(QLatin1String("D:"), Qt::CaseInsensitive)) ++outsideInspections;
        return {path, path, !path.endsWith(QLatin1String(".txt"))};
      }
      QVector<app::FileBrowserPlace> places() const override {
        return {{QStringLiteral("Home"), QStringLiteral("C:/Users/User")},
                {QStringLiteral("AppData"), QStringLiteral("C:/Users/User/AppData")},
                {QStringLiteral("C"), QStringLiteral("C:/"), true, true},
                {QStringLiteral("D"), QStringLiteral("D:/"), true, false}};
      }
      void createDirectory(const QString&) const override {}
    };
    auto browser = std::make_shared<Browser>();
    Model model(Model::Mode::OpenFile, browser);
    QAbstractItemModelTester placesContract(model.placesModel(), QAbstractItemModelTester::FailureReportingMode::QtTest);
    model.setAllowedRoots({QStringLiteral("C:/")});
    model.start(QStringLiteral("D:/"));
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.path(), QStringLiteral("C:/"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.places().size(), 3);
    QVERIFY(!model.canGoUp());
    model.navigate(QStringLiteral("D:/"));
    QCOMPARE(model.path(), QStringLiteral("C:/"));
    model.setFilename(QStringLiteral("D:/outside.txt"));
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Invalid);
    QCOMPARE(browser->outsideInspections, 0);
    model.setAllowedRoots({QStringLiteral("D:/")});
    QCOMPARE(model.places().size(), 1);
    QVERIFY(model.places().constFirst().volume);
    QTRY_VERIFY(!model.busy());
    model.setAllowedRoots({QStringLiteral("C:/allowed")});
    QVERIFY(!model.isPathAllowed(QStringLiteral("C:/allowed-other/file.txt")));
    QVERIFY(!model.isPathAllowed(QStringLiteral("C:/allowed/../other/file.txt")));
    QVERIFY(model.isPathAllowed(QStringLiteral("c:/ALLOWED/sub/file.txt")));
  }
  void initialResolutionKeepsAllOperationsWithinTheRequestedScope_data() {
    QTest::addColumn<int>("mode");
    QTest::addColumn<bool>("missingRoot");
    for (const auto mode : {Model::Mode::OpenFile, Model::Mode::OpenFiles, Model::Mode::Directory, Model::Mode::Mixed, Model::Mode::SaveFile})
      for (const bool missingRoot : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("mode-%1-%2").arg(static_cast<int>(mode)).arg(missingRoot ? "missing-root" : "new-file-in-root")))
            << static_cast<int>(mode) << missingRoot;
  }
  void initialResolutionKeepsAllOperationsWithinTheRequestedScope() {
    QFETCH(int, mode);
    QFETCH(bool, missingRoot);
    class Browser final : public app::FileBrowser {
     public:
      const QString root = QStringLiteral("C:/Fixture/Allowed");
      bool missingRoot = false;
      mutable std::atomic_int outsideLists{0}, writes{0};
      QVector<app::FileBrowserEntry> list(const QString& path, std::stop_token) const override {
        if (path != root) ++outsideLists;
        return {};
      }
      app::FileBrowserEntry inspect(const QString& path) const override {
        const bool exists = path == QStringLiteral("C:/Fixture") || (path == root && !missingRoot);
        return {path, QFileInfo(path).fileName(), exists, 0, {}, exists};
      }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString& path) const override {
        if (path != root + QStringLiteral("/Created")) throw std::logic_error("unexpected mutation target");
        ++writes;
      }
    };
    const auto browser = std::make_shared<Browser>();
    browser->missingRoot = missingRoot;
    Model model(static_cast<Model::Mode>(mode), browser);
    model.setAllowedRoots({browser->root});
    model.start(missingRoot ? browser->root : browser->root + QStringLiteral("/new.txt"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.path(), browser->root);
    QCOMPARE(browser->outsideLists.load(), 0);
    QCOMPARE(model.loadState(), missingRoot ? State::Failed : State::Empty);
    model.createDirectory(QStringLiteral("Created"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(browser->writes.load(), missingRoot ? 0 : 1);
    QCOMPARE(browser->outsideLists.load(), 0);
  }
  void literalBrowserObjectsSurviveNavigationScopeAndAcceptance_data() {
    QTest::addColumn<int>("directoryIndex");
    QTest::addColumn<int>("fileIndex");
    QTest::addColumn<bool>("initial");
    for (int directory = 0; directory < 3; ++directory)
      for (int file = 0; file < 3; ++file)
        for (const bool initial : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("directory-%1-file-%2-initial-%3").arg(directory).arg(file).arg(initial))) << directory << file << initial;
  }
  void literalBrowserObjectsSurviveNavigationScopeAndAcceptance() {
    QFETCH(int, directoryIndex);
    QFETCH(int, fileIndex);
    QFETCH(bool, initial);
    class Browser final : public app::FileBrowser {
     public:
      // Literal browser entries keep expected object identities independent of path normalization.
      std::map<QString, app::FileBrowserEntry> objects;
      std::map<QString, QVector<app::FileBrowserEntry>> listings;
      QVector<app::FileBrowserPlace> locations;
      QVector<app::FileBrowserEntry> list(const QString& path, std::stop_token) const override {
        const auto found = listings.find(path);
        if (found == listings.end()) throw app::FileBrowserError(app::FileBrowserErrorCode::MissingDirectory);
        return found->second;
      }
      app::FileBrowserEntry inspect(const QString& path) const override {
        const auto found = objects.find(path);
        return found == objects.end() ? app::FileBrowserEntry{path, {}, false, 0, {}, false} : found->second;
      }
      QVector<app::FileBrowserPlace> places() const override { return locations; }
      void createDirectory(const QString&) const override { throw std::logic_error("selection must not create an object"); }
    };
    const QString root = QStringLiteral("C:/Fixture");
    const QStringList directoryNames{QStringLiteral("Entry"), QStringLiteral("Entry "), QStringLiteral("Entry.")};
    const QStringList fileNames{QStringLiteral("Item"), QStringLiteral("Item "), QStringLiteral("Item.")};
    const auto browser = std::make_shared<Browser>();
    browser->objects.emplace(root, app::FileBrowserEntry{root, QStringLiteral("Fixture"), true});
    for (qsizetype directory = 0; directory < directoryNames.size(); ++directory) {
      const auto path = root + u'/' + directoryNames.at(directory);
      const app::FileBrowserEntry folder{path, directoryNames.at(directory), true};
      browser->objects.emplace(path, folder);
      browser->listings[root].append(folder);
      browser->locations.append({directoryNames.at(directory), path});
      for (qsizetype file = 0; file < fileNames.size(); ++file) {
        const auto target = path + u'/' + fileNames.at(file);
        const app::FileBrowserEntry entry{target, fileNames.at(file), false, static_cast<qint64>((directory + 1) * 100 + file + 1)};
        browser->objects.emplace(target, entry);
        browser->listings[path].append(entry);
      }
    }
    const auto directoryPath = root + u'/' + directoryNames.at(directoryIndex);
    const auto selectedPath = directoryPath + u'/' + fileNames.at(fileIndex);
    const auto selectedSize = static_cast<qint64>((directoryIndex + 1) * 100 + fileIndex + 1);
    Model model(Model::Mode::OpenFile, browser);
    if (initial) model.setAllowedRoots({directoryPath});
    model.start(initial ? directoryPath : root);
    QTRY_COMPARE(model.loadState(), State::Ready);
    if (!initial) {
      const auto row = findRow(model, directoryNames.at(directoryIndex));
      QVERIFY(row >= 0);
      QCOMPARE(model.entryPath(row), directoryPath);
      model.selectRows({row});
      QCOMPARE(completedAcceptance(model), Model::Acceptance::Navigate);
      QTRY_COMPARE(model.loadState(), State::Ready);
      model.setAllowedRoots({directoryPath});
    }
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.path(), directoryPath);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.places().size(), 1);
    QCOMPARE(model.places().constFirst().path, directoryPath);
    QVERIFY(!model.canGoUp());
    for (qsizetype index = 0; index < directoryNames.size(); ++index)
      QCOMPARE(model.isPathAllowed(root + u'/' + directoryNames.at(index)), index == directoryIndex);
    model.setFilename(fileNames.at(fileIndex));
    QVERIFY(model.canAccept());
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
    QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(selectedPath)});
    QCOMPARE(model.selectedItems().constFirst().size, selectedSize);
    const auto row = findRow(model, fileNames.at(fileIndex));
    QVERIFY(row >= 0);
    model.selectRows({row});
    QCOMPARE(model.filename(), fileNames.at(fileIndex));
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
    QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(selectedPath)});
    QCOMPARE(model.selectedItems().constFirst().size, selectedSize);
    // Pass the picker result through exclusion editing and verify distinct siblings remain unchanged.
    constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
    test::FakeUwfBackend::State initialState;
    initialState.current.volumes.emplace(
        volumeId, test::FakeUwfBackend::VolumeConfiguration{.volume = {volumeId, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initialState.next = initialState.current;
    initialState.next.filterEnabled = true;
    initialState.runtime.fileMaintenancePrefixes[volumeId] = "C:";
    const auto system = std::make_shared<test::FakeUwfBackend>(initialState);
    const auto relative = [](const QString& path) { return app::toUwfFilePath(QDir::toNativeSeparators(path).toStdString(), "C:"); };
    for (const auto& [path, object] : browser->objects)
      if (!object.directory) system->seedFile(volumeId, relative(path), "base");
    const auto baseline = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession session;
    session.globalEditor()->reset(baseline.next);
    const auto volume = session.editVolume({.driveLetter = "C:",
                                            .volumeName = "Volume{11111111-2222-3333-4444-555555555555}",
                                            .fileSystem = "NTFS",
                                            .label = {},
                                            .totalBytes = 0,
                                            .freeBytes = 0,
                                            .support = core::VolumeSupport::Supported},
                                           false);
    volume->applySnapshot(baseline);
    const auto selected = model.selectedPaths().constFirst();
    presentation::ExclusionListViewModel exclusions(volume->files());
    const auto addition = exclusions.addSelectedPath(selected);
    QCOMPARE(addition.outcome, app::ExclusionImportOutcome::Applied);
    QCOMPARE(addition.path, selected);
    const auto plan = app::planConfiguration(session.pendingChanges(), baseline);
    QCOMPARE(plan.changes.addFileExclusions.at(volumeId), std::vector<std::string>{selected.toStdString()});
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(system)).apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(system->snapshot().next.volumes.at(volumeId).fileExclusions, std::vector<std::string>{relative(selected)});
    system->simulateRestart();
    for (const auto& [path, object] : browser->objects)
      if (!object.directory) {
        system->writeFile(volumeId, relative(path), "changed");
        QCOMPARE(system->readFile(volumeId, relative(path)), std::optional<std::string>{relative(path) == relative(selected) ? "changed" : "base"});
      }
    system->simulateRestart();
    for (const auto& [path, object] : browser->objects)
      if (!object.directory)
        QCOMPARE(system->visibleFile(volumeId, relative(path)), std::optional<std::string>{relative(path) == relative(selected) ? "changed" : "base"});
    model.refresh();
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.path(), directoryPath);
    model.navigate(root + u'/' + directoryNames.at((directoryIndex + 1) % directoryNames.size()));
    QCOMPARE(model.path(), directoryPath);
  }

  void fileFolderAndMixedSelectionPreserveTargets() {
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QDir directory(temporary.path());
    QVERIFY(directory.mkdir(QStringLiteral("folder")));
    writeFile(directory.filePath(QStringLiteral("one.txt")));
    writeFile(directory.filePath(QStringLiteral("two.cmd")));
    for (const auto mode : {Model::Mode::OpenFile, Model::Mode::OpenFiles, Model::Mode::Directory, Model::Mode::Mixed}) {
      Model model(mode, windows::systemFileBrowser());
      model.start(directory.path());
      QVERIFY(!model.canAccept());
      QTRY_COMPARE_WITH_TIMEOUT(model.loadState(), State::Ready, 3000);
      QCOMPARE(model.rowCount(), mode == Model::Mode::Directory ? 1 : 3);
      if (mode == Model::Mode::Directory) {
        QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
        QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(directory.path())});
        model.selectRows({0});
        QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
        QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(directory.filePath(QStringLiteral("folder")))});
      } else {
        const int file = findRow(model, QStringLiteral("one.txt"));
        QVERIFY(file >= 0);
        model.selectRows(mode == Model::Mode::OpenFile
                             ? QList<int>{file}
                             : QList<int>{file, findRow(model, mode == Model::Mode::Mixed ? QStringLiteral("folder") : QStringLiteral("two.cmd"))});
        QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
        QCOMPARE(model.selectedPaths().size(), mode == Model::Mode::OpenFile ? 1 : 2);
        QVERIFY(model.selectedPaths().contains(QDir::toNativeSeparators(directory.filePath(QStringLiteral("one.txt")))));
      }
    }
  }
  void filtersSearchSortAndNavigationRetainIdentity() {
    QTemporaryDir temporary;
    QDir directory(temporary.path());
    QVERIFY(directory.mkdir(QStringLiteral("folder")));
    writeFile(directory.filePath(QStringLiteral("one.txt")));
    writeFile(directory.filePath(QStringLiteral("two.cmd")));
    Model model(Model::Mode::OpenFiles, windows::systemFileBrowser());
    model.setFilters(QStringLiteral("Text (*.txt);;Scripts (*.cmd);;All files (*)"));
    model.start(directory.path());
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.rowCount(), 2);
    model.selectRows({findRow(model, QStringLiteral("one.txt"))});
    model.sort(0, Qt::DescendingOrder);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
    QVERIFY(model.selectedPaths().first().endsWith(QLatin1String("one.txt")));
    model.setQuery(QStringLiteral("ONe"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.rowCount(), 1);
    model.setQuery({});
    QTRY_VERIFY(!model.busy());
    model.setFilterIndex(1);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.rowCount(), 2);
    QVERIFY(findRow(model, QStringLiteral("one.txt")) < 0);
    QVERIFY(findRow(model, QStringLiteral("two.cmd")) >= 0);
    model.navigate(directory.filePath(QStringLiteral("folder")));
    QTRY_COMPARE(model.loadState(), State::Empty);
    QVERIFY(model.canGoBack());
    model.goBack();
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.path(), directory.path());
    QVERIFY(model.canGoForward());
    model.goForward();
    QTRY_COMPARE(model.loadState(), State::Empty);
    model.goUp();
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.path(), directory.path());
  }
  void selectionTextTracksFiltering_data() {
    QTest::addColumn<bool>("search");
    QTest::addColumn<int>("remaining");
    for (const bool search : {false, true})
      for (const int remaining : {0, 1, 2})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-selected").arg(search ? QStringLiteral("search") : QStringLiteral("filter")).arg(remaining)))
            << search << remaining;
  }
  void selectionTextTracksFiltering() {
    QFETCH(bool, search);
    QFETCH(int, remaining);
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QDir directory(temporary.path());
    writeFile(directory.filePath(QStringLiteral("one.txt")));
    writeFile(directory.filePath(QStringLiteral("two.cmd")));
    Model model(Model::Mode::OpenFiles, windows::systemFileBrowser());
    model.setFilters(QStringLiteral("All files (*);;Text (*.txt);;No matching files (*.bad)"));
    model.start(directory.path());
    QTRY_COMPARE(model.loadState(), State::Ready);
    model.selectRows({findRow(model, QStringLiteral("one.txt")), findRow(model, QStringLiteral("two.cmd"))});
    QCOMPARE(model.filename(), QStringLiteral("one.txt; two.cmd"));
    QSignalSpy changes(&model, &Model::selectionChanged);
    if (search)
      model.setQuery(remaining == 0 ? QStringLiteral("missing") : remaining == 1 ? QStringLiteral("one") : QString{});
    else
      model.setFilterIndex(remaining == 0 ? 2 : remaining == 1 ? 1 : 0);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.selectedEntries().size(), remaining);
    QCOMPARE(model.filename(), remaining == 0 ? QString{} : remaining == 1 ? QStringLiteral("one.txt") : QStringLiteral("one.txt; two.cmd"));
    QCOMPARE(model.canAccept(), remaining != 0);
    QCOMPARE(changes.count(), remaining == 2 ? 0 : 1);
    if (remaining != 0) {
      QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
      QCOMPARE(model.selectedPaths().size(), remaining);
    }
    model.setFilename(QStringLiteral("one.txt"));
    QVERIFY(model.selectedEntries().isEmpty());
    model.setQuery(QStringLiteral("not-visible"));
    QTRY_VERIFY(!model.busy());
    model.setFilterIndex(2);
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.filename(), QStringLiteral("one.txt"));
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
    QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(directory.filePath(QStringLiteral("one.txt")))});
  }
  void saveValidationNeverTouchesExistingContents() {
    QTemporaryDir temporary;
    QDir directory(temporary.path());
    const auto existing = directory.filePath(QStringLiteral("existing.txt"));
    writeFile(existing);
    Model model(Model::Mode::SaveFile, windows::systemFileBrowser());
    model.setFilters(QStringLiteral("Text (*.txt);;All files (*)"));
    model.start(existing);
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.filename(), QStringLiteral("existing.txt"));
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Replace);
    QFile original(existing);
    QVERIFY(original.open(QIODevice::ReadOnly));
    QCOMPARE(original.readAll(), QByteArray("original content"));
    model.setFilename(QStringLiteral("fresh"));
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Ready);
    QVERIFY(model.selectedPaths().first().endsWith(QLatin1String("fresh.txt")));
    QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("fresh.txt"))));
    QSignalSpy errors(&model, &Model::validationFailed);
    for (const auto& name : {QStringLiteral("bad?.txt"), QStringLiteral("CON.txt"), QStringLiteral("../missing/name.txt")}) {
      model.setFilename(name);
      QCOMPARE(completedAcceptance(model), Model::Acceptance::Invalid);
      QVERIFY(model.selectedPaths().isEmpty());
    }
    QCOMPARE(errors.count(), 3);
    model.cancel();
    QVERIFY(model.selectedPaths().isEmpty());
    QVERIFY(!model.canAccept());
  }
  void selectedExistingSaveTargetsKeepTheirLiteralIdentity_data() {
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("initialFile");
    QTest::addColumn<bool>("defaultSuffix");
    const std::array names{std::pair{"no-extension", QStringLiteral("Entry")}, std::pair{"trailing-space", QStringLiteral("Entry ")},
                           std::pair{"trailing-dot", QStringLiteral("Entry.")}};
    for (const auto& [label, name] : names)
      for (const bool initialFile : {false, true})
        for (const bool defaultSuffix : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("%1/%2/%3")
                                       .arg(QString::fromLatin1(label), initialFile ? QStringLiteral("prefill") : QStringLiteral("selection"),
                                            defaultSuffix ? QStringLiteral("text-filter") : QStringLiteral("all-files"))))
              << name << initialFile << defaultSuffix;
  }
  void selectedExistingSaveTargetsKeepTheirLiteralIdentity() {
    QFETCH(QString, name);
    QFETCH(bool, initialFile);
    QFETCH(bool, defaultSuffix);
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    for (const auto& sibling : {QStringLiteral("Entry"), QStringLiteral("Entry "), QStringLiteral("Entry."), QStringLiteral("Entry.txt")})
      QVERIFY2(fixture.seed(sibling, sibling.toUtf8()), qPrintable(fixture.error));
    Model model(Model::Mode::SaveFile, fixture.browser);
    model.setFilters(QStringLiteral("All files (*);;Text (*.txt)"));
    model.setFilterIndex(initialFile && defaultSuffix ? 1 : 0);
    QTRY_VERIFY(!model.busy());
    model.start(initialFile ? fixture.path(name) : fixture.root.path());
    QTRY_COMPARE(model.loadState(), State::Ready);
    if (!initialFile) {
      const auto row = findRow(model, name);
      QVERIFY(row >= 0);
      model.selectRows({row});
      model.setFilterIndex(defaultSuffix ? 1 : 0);
      QTRY_VERIFY(!model.busy());
    }
    QCOMPARE(model.filename(), name);
    QCOMPARE(model.selectedEntries(), QStringList{fixture.path(name)});
    // Search and type filters change the listing, not the unedited save target.
    model.setQuery(QStringLiteral("not-visible"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Replace);
    QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(fixture.path(name))});
    QCOMPARE(model.selectedItems().size(), 1);
    QCOMPARE(model.selectedItems().constFirst().path, QDir::toNativeSeparators(fixture.path(name)));
    QSignalSpy errors(&model, &Model::validationFailed);
    // A text edit explicitly changes origin, even when the text is identical.
    model.setFilename(name);
    QVERIFY(model.selectedEntries().isEmpty());
    if (name == QLatin1String("Entry")) {
      QCOMPARE(completedAcceptance(model), Model::Acceptance::Replace);
      const auto manualName = defaultSuffix ? QStringLiteral("Entry.txt") : name;
      QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(fixture.path(manualName))});
      QCOMPARE(errors.count(), 0);
    } else {
      QCOMPARE(completedAcceptance(model), Model::Acceptance::Invalid);
      QVERIFY(model.selectedPaths().isEmpty());
      QCOMPARE(errors.count(), 1);
    }
    for (const auto& sibling : {QStringLiteral("Entry"), QStringLiteral("Entry "), QStringLiteral("Entry."), QStringLiteral("Entry.txt")})
      QCOMPARE(fixture.read(sibling), std::optional<QByteArray>{sibling.toUtf8()});
    QVERIFY(!fixture.exists(QStringLiteral("Entry..txt")));
    QVERIFY(!fixture.exists(QStringLiteral("Entry .txt")));
    QCOMPARE(fixture.browser->creates.load(), 0);
    QCOMPARE(fixture.browser->renames.load(), 0);
  }
  void selectingADirectoryForSaveStillNavigates() {
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    const auto folder = QStringLiteral("Folder");
    QVERIFY(QDir(fixture.root.path()).mkdir(folder));
    Model model(Model::Mode::SaveFile, fixture.browser);
    model.setFilters(QStringLiteral("Text (*.txt)"));
    model.start(fixture.root.path());
    QTRY_COMPARE(model.loadState(), State::Ready);
    const auto row = findRow(model, folder);
    QVERIFY(row >= 0);
    model.selectRows({row});
    QCOMPARE(completedAcceptance(model), Model::Acceptance::Navigate);
    QTRY_COMPARE(model.loadState(), State::Empty);
    QCOMPARE(model.path(), fixture.path(folder));
    QVERIFY(model.selectedPaths().isEmpty());
    QVERIFY(!fixture.exists(QStringLiteral("Folder.txt")));
    QCOMPARE(fixture.browser->creates.load(), 0);
    QCOMPARE(fixture.browser->renames.load(), 0);
  }
  void newNamesAreValidatedWithoutRewriting_data() {
    QTest::addColumn<bool>("rename");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("accepted");
    struct Name {
      const char* label;
      const char16_t* value;
      bool accepted;
    };
    constexpr std::array names{Name{"ordinary", u"NewEntry", true}, Name{"leading-space", u" NewEntry", true}, Name{"trailing-space", u"NewEntry ", false},
                               Name{"trailing-dot", u"NewEntry.", false}, Name{"only-spaces", u"  ", false}};
    for (const bool rename : {false, true})
      for (const auto& name : names)
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(rename ? QStringLiteral("rename") : QStringLiteral("create"), QString::fromLatin1(name.label))))
            << rename << QString::fromUtf16(name.value) << name.accepted;
  }
  void newNamesAreValidatedWithoutRewriting() {
    QFETCH(bool, rename);
    QFETCH(QString, name);
    QFETCH(bool, accepted);
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    QVERIFY2(fixture.seed(QStringLiteral("Original"), "selected contents"), qPrintable(fixture.error));
    Model model(Model::Mode::Mixed, fixture.browser);
    model.start(fixture.root.path());
    QTRY_COMPARE(model.loadState(), State::Ready);
    QSignalSpy errors(&model, &Model::validationFailed);
    if (rename)
      model.renameEntry(findRow(model, QStringLiteral("Original")), name);
    else
      model.createDirectory(name);
    if (accepted) {
      QTRY_VERIFY(!model.busy());
      QTRY_VERIFY(findRow(model, name) >= 0);
      QVERIFY(fixture.exists(name));
      QCOMPARE(fixture.browser->renames.load(), rename ? 1 : 0);
      QCOMPARE(fixture.browser->creates.load(), rename ? 0 : 1);
      QCOMPARE(errors.count(), 0);
      if (rename) QCOMPARE(fixture.read(name), std::optional<QByteArray>{"selected contents"});
    } else {
      QCOMPARE(errors.count(), 1);
      QVERIFY(!model.busy());
      QCOMPARE(fixture.browser->creates.load(), 0);
      QCOMPARE(fixture.browser->renames.load(), 0);
      QVERIFY(!fixture.exists(name));
    }
    if (name != name.trimmed() && !name.trimmed().isEmpty()) QVERIFY(!fixture.exists(name.trimmed()));
    QCOMPARE(fixture.read(QStringLiteral("Original")), rename && accepted ? std::nullopt : std::optional<QByteArray>{"selected contents"});
  }
  void anUnchangedLiteralPrefillNeverRenamesAnotherObject_data() {
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("sibling");
    const std::array names{std::pair{"trailing-space", QStringLiteral("Entry ")}, std::pair{"leading-space", QStringLiteral(" Entry")},
                           std::pair{"trailing-dot", QStringLiteral("Entry.")}};
    for (const auto& [label, name] : names)
      for (const bool sibling : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1/sibling-%2").arg(QString::fromLatin1(label)).arg(sibling))) << name << sibling;
  }
  void anUnchangedLiteralPrefillNeverRenamesAnotherObject() {
    QFETCH(QString, name);
    QFETCH(bool, sibling);
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    QVERIFY2(fixture.seed(name, "selected contents"), qPrintable(fixture.error));
    if (sibling) QVERIFY2(fixture.seed(QStringLiteral("Entry"), "keep sibling"), qPrintable(fixture.error));
    Model model(Model::Mode::OpenFiles, fixture.browser);
    model.start(fixture.root.path());
    QTRY_COMPARE(model.loadState(), State::Ready);
    const auto row = findRow(model, name);
    QVERIFY(row >= 0);
    model.selectRows({row});
    QSignalSpy errors(&model, &Model::validationFailed);
    const auto prefill = QFileInfo(model.entryPath(row)).fileName();
    QCOMPARE(prefill, name);
    model.renameEntry(row, prefill);
    QVERIFY(!model.busy());
    QCOMPARE(errors.count(), 0);
    QCOMPARE(model.error(), QString{});
    QCOMPARE(model.filename(), name);
    QCOMPARE(fixture.browser->renames.load(), 0);
    QCOMPARE(fixture.browser->creates.load(), 0);
    QCOMPARE(fixture.read(name), std::optional<QByteArray>{"selected contents"});
    QCOMPARE(fixture.read(QStringLiteral("Entry")), sibling ? std::optional<QByteArray>{"keep sibling"} : std::nullopt);
    // An explicit name edit may rename the selected object but must preserve its distinct sibling.
    model.renameEntry(row, QStringLiteral("Entry"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(fixture.browser->renames.load(), 1);
    QCOMPARE(errors.count(), sibling ? 1 : 0);
    QCOMPARE(fixture.read(name), sibling ? std::optional<QByteArray>{"selected contents"} : std::nullopt);
    QCOMPARE(fixture.read(QStringLiteral("Entry")), std::optional<QByteArray>{sibling ? QByteArray("keep sibling") : QByteArray("selected contents")});
    if (!sibling) QTRY_VERIFY(findRow(model, QStringLiteral("Entry")) >= 0);
  }
  void creatingFolderIsExplicitAndContained() {
    QTemporaryDir temporary;
    Model model(Model::Mode::Directory, windows::systemFileBrowser());
    model.start(temporary.path());
    QTRY_COMPARE(model.loadState(), State::Empty);
    QSignalSpy errors(&model, &Model::validationFailed);
    for (const auto& name : {QStringLiteral("../escape"), QStringLiteral("."), QStringLiteral("COM1"), QStringLiteral("name/")}) model.createDirectory(name);
    QCOMPARE(errors.count(), 4);
    model.createDirectory(QStringLiteral("新建文件夹"));
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.rowCount(), 1);
    QVERIFY(QFileInfo(QDir(temporary.path()).filePath(QStringLiteral("新建文件夹"))).isDir());
  }
  void renamingEntriesPreservesContentsAndCannotEscapeOrOverwrite() {
    QTemporaryDir temporary;
    QDir directory(temporary.path());
    writeFile(directory.filePath(QStringLiteral("original.txt")));
    writeFile(directory.filePath(QStringLiteral("existing.txt")), QByteArray("keep existing"));
    Model model(Model::Mode::OpenFiles, windows::systemFileBrowser());
    model.setAllowedRoots({temporary.path()});
    model.start(temporary.path());
    QTRY_COMPARE(model.loadState(), State::Ready);
    QSignalSpy errors(&model, &Model::validationFailed);
    for (const auto& name : {QStringLiteral("../escape.txt"), QStringLiteral("C:/outside.txt"), QStringLiteral("CON.txt"), QStringLiteral("existing.txt")})
      model.renameEntry(findRow(model, QStringLiteral("original.txt")), name);
    QTRY_COMPARE(errors.count(), 4);
    QTRY_VERIFY(!model.busy());
    model.renameEntry(findRow(model, QStringLiteral("original.txt")), QStringLiteral("renamed.txt"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.loadState(), State::Ready);
    QVERIFY(findRow(model, QStringLiteral("renamed.txt")) >= 0);
    QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("original.txt"))));
    QFile renamed(directory.filePath(QStringLiteral("renamed.txt")));
    QVERIFY(renamed.open(QIODevice::ReadOnly));
    QCOMPARE(renamed.readAll(), QByteArray("original content"));
    QFile existing(directory.filePath(QStringLiteral("existing.txt")));
    QVERIFY(existing.open(QIODevice::ReadOnly));
    QCOMPARE(existing.readAll(), QByteArray("keep existing"));
  }
  void slowIoKeepsTheEventLoopResponsive_data() {
    QTest::addColumn<int>("operation");
    QTest::addColumn<int>("outcome");
    for (int operation = 0; operation < 6; ++operation)
      for (int outcome = 0; outcome < 4; ++outcome)
        QTest::newRow(qUtf8Printable(QStringLiteral("io-%1-outcome-%2").arg(operation).arg(outcome))) << operation << outcome;
  }
  void slowIoKeepsTheEventLoopResponsive() {
    QFETCH(int, operation);
    QFETCH(int, outcome);
    using Io = ControlledBrowser::Io;
    const std::array operations{Io::Places, Io::InitialInspection, Io::Listing, Io::Acceptance, Io::Create, Io::Rename};
    auto browser = std::make_shared<ControlledBrowser>();
    const auto unblock = qScopeGuard([&] {
      browser->gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    auto model = std::make_unique<Model>(Model::Mode::OpenFile, browser);
    QCOMPARE(browser->placeReads.load(), 0);
    QCOMPARE(browser->inspections.load(), 0);
    if (operation >= 3) {
      model->start(QStringLiteral("C:/test"));
      QTRY_COMPARE(model->loadState(), State::Ready);
      model->setFilename(QStringLiteral("file.txt"));
    }
    browser->blocked = operations[static_cast<std::size_t>(operation)];
    browser->reject = outcome == 1;
    int accepted = 0, errors = 0;
    connect(model.get(), &Model::acceptanceFinished, this, [&](auto) { ++accepted; });
    connect(model.get(), &Model::validationFailed, this, [&](const auto&) { ++errors; });
    if (operation < 3) model->start(QStringLiteral("C:/test"));
    if (operation == 3) model->requestAcceptance();
    if (operation == 4) model->createDirectory(QStringLiteral("created"));
    if (operation == 5) model->renameEntry(0, QStringLiteral("renamed.txt"));
    QTRY_COMPARE(browser->gate->started.load(), 1);
    bool heartbeat = false;
    QTimer::singleShot(0, this, [&] { heartbeat = true; });
    QTRY_VERIFY(heartbeat);
    QCOMPARE(browser->gate->finished.load(), 0);
    QVERIFY(model->busy());
    QVERIFY(!model->canAccept());
    QVERIFY(!model->canGoBack());
    QVERIFY(!model->canGoForward());
    QVERIFY(!model->canGoUp());
    const auto previousPath = model->path();
    model->navigate(QStringLiteral("C:/other"));
    model->refresh();
    model->requestAcceptance();
    model->createDirectory(QStringLiteral("ignored"));
    model->renameEntry(0, QStringLiteral("ignored.txt"));
    QCOMPARE(model->path(), previousPath);
    QCOMPARE(browser->writes.load(), 0);
    QCOMPARE(browser->gate->started.load(), 1);
    if (outcome == 2) model->cancel();
    if (outcome == 3) model.reset();
    browser->gate->release();
    QVERIFY(QThreadPool::globalInstance()->waitForDone(2000));
    if (outcome >= 2) {
      QCoreApplication::sendPostedEvents();
      QCOMPARE(accepted, 0);
      QCOMPARE(errors, 0);
      if (model) {
        QCOMPARE(model->loadState(), State::Canceled);
        QVERIFY(model->selectedPaths().isEmpty());
      }
    } else {
      QTRY_VERIFY(!model->busy());
      if (operation < 3) {
        QCOMPARE(model->loadState(), outcome == 1 ? State::Failed : State::Ready);
        if (outcome == 1) QCOMPARE(model->error(), QStringLiteral("controlled I/O failure"));
      } else {
        QCOMPARE(errors, outcome == 1 ? 1 : 0);
        QCOMPARE(accepted, operation == 3 ? 1 : 0);
        QCOMPARE(browser->writes.load(), operation >= 4 && outcome == 0 ? 1 : 0);
      }
    }
    QVERIFY(!browser->wrongThread.load());
  }
  void pendingListingUsesTheLatestQueryFilterAndSort() {
    const auto browser = std::make_shared<QueryBrowser>();
    browser->listingGate = std::make_shared<test::WorkerGate>();
    const auto release = qScopeGuard([gate = browser->listingGate] { gate->release(); });
    Model model(Model::Mode::OpenFiles, browser);
    model.setFilters(QStringLiteral("All (*);;Text (*.txt);;Scripts (*.cmd)"));
    model.start(QStringLiteral("C:/query"));
    QTRY_COMPARE(browser->listingGate->started.load(), 1);
    model.setQuery(QStringLiteral("AL"));
    model.setQuery(QStringLiteral("a"));
    model.setFilterIndex(2);
    model.setFilterIndex(1);
    model.sort(0, Qt::AscendingOrder);
    model.sort(0, Qt::DescendingOrder);
    browser->listingGate->release();
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.query(), QStringLiteral("a"));
    QCOMPARE(model.filterIndex(), 1);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("Gamma.txt"));
    QCOMPARE(model.index(1, 0).data().toString(), QStringLiteral("delta.txt"));
    QCOMPARE(model.index(2, 0).data().toString(), QStringLiteral("Alpha.txt"));
  }
  void pendingQueriesPreserveNewerSelectionIntent_data() {
    QTest::addColumn<bool>("save");
    QTest::addColumn<bool>("typed");
    QTest::addColumn<bool>("duringReset");
    for (const bool save : {false, true})
      for (const bool typed : {false, true})
        for (const bool duringReset : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("mode-%1-intent-%2-reset-%3").arg(save).arg(typed).arg(duringReset))) << save << typed << duringReset;
  }
  void pendingQueriesPreserveNewerSelectionIntent() {
    QFETCH(bool, save);
    QFETCH(bool, typed);
    QFETCH(bool, duringReset);
    Model model(save ? Model::Mode::SaveFile : Model::Mode::OpenFiles, std::make_shared<QueryBrowser>());
    QAbstractItemModelTester contract(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    model.start(QStringLiteral("C:/query"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.loadState(), State::Ready);
    model.selectRows({findRow(model, QStringLiteral("Alpha.txt"))});
    const auto updateSelection = [&] {
      if (typed)
        model.setFilename(QStringLiteral("Manual.txt"));
      else
        model.selectRows({findRow(model, QStringLiteral("Gamma.txt"))});
    };
    bool changed = false;
    if (duringReset)
      connect(&model, &QAbstractItemModel::modelAboutToBeReset, &model, [&] {
        if (!std::exchange(changed, true)) updateSelection();
      });
    model.sort(0, Qt::DescendingOrder);
    if (!duringReset) updateSelection();
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.filename(), typed ? QStringLiteral("Manual.txt") : QStringLiteral("Gamma.txt"));
    QCOMPARE(model.selectedEntries(), typed ? QStringList{} : QStringList{QStringLiteral("C:/query/Gamma.txt")});
    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("Gamma.txt"));
    QCOMPARE(model.loadState(), State::Ready);
  }
  void reentrantQueryPublicationKeepsTheLatestGeneration_data() {
    QTest::addColumn<bool>("beforeReset");
    QTest::newRow("about-to-reset") << true;
    QTest::newRow("reset") << false;
  }
  void reentrantQueryPublicationKeepsTheLatestGeneration() {
    QFETCH(bool, beforeReset);
    Model model(Model::Mode::OpenFiles, std::make_shared<QueryBrowser>());
    QAbstractItemModelTester contract(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    model.start(QStringLiteral("C:/query"));
    QTRY_VERIFY(!model.busy());
    bool changed = false;
    const auto supersede = [&] {
      if (!std::exchange(changed, true)) model.setQuery(QStringLiteral("Beta"));
    };
    if (beforeReset)
      connect(&model, &QAbstractItemModel::modelAboutToBeReset, &model, supersede);
    else
      connect(&model, &QAbstractItemModel::modelReset, &model, supersede);
    model.setQuery(QStringLiteral("a"));
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.query(), QStringLiteral("Beta"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("Beta.cmd"));
    QCOMPARE(model.loadState(), State::Ready);
  }
  void canceledLoadsCannotPublishStaleRowsAndFailuresCanRecover() {
    auto browser = std::make_shared<DelayedBrowser>();
    Model model(Model::Mode::OpenFile, browser);
    model.start(QStringLiteral("C:/slow"));
    QTRY_COMPARE(browser->gate->started.load(), 1);
    model.cancel();
    QTRY_COMPARE(browser->gate->canceled.load(), 1);
    model.navigate(QStringLiteral("C:/denied"));
    QTRY_COMPARE(model.loadState(), State::Failed);
    QCOMPARE(model.error(), QStringLiteral("Access denied: complete diagnostic"));
    QVERIFY(!model.canAccept());
    model.navigate(QStringLiteral("C:/current"));
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.data(model.index(0, 0)).toString(), QStringLiteral("current.txt"));
  }
};
QTEST_GUILESS_MAIN(FilePickerViewModelTests)
#include "FilePickerViewModelTests.moc"
