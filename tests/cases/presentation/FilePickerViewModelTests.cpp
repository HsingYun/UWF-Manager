// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTranslator>
#include <algorithm>
#include <array>
#include <atomic>
#include <utility>

#include "presentation/common/ItemRoles.h"
#include "presentation/files/FilePickerViewModel.h"
#include "support/app/WorkerGate.h"
#include "windows/filesystem/LocalFileBrowser.h"

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
  void fastLoadsSkipProgressAndSlowLoadsKeepItVisible() {
    class Browser final : public app::FileBrowser {
     public:
      std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
      QVector<app::FileBrowserEntry> list(const QString& path, std::stop_token stop) const override {
        if (path.endsWith(QLatin1String("/slow"))) gate->wait(stop);
        return {{path + QStringLiteral("/file.txt"), QStringLiteral("file.txt")}};
      }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString&) const override {}
    };
    auto browser = std::make_shared<Browser>();
    Model model(Model::Mode::Directory, browser);
    QElapsedTimer elapsed;
    elapsed.start();
    model.start(QStringLiteral("C:/fast"));
    QTRY_VERIFY_WITH_TIMEOUT(model.loadState() != State::Loading, 180);
    QVERIFY(elapsed.elapsed() < 200);
    QVERIFY(!model.progressVisible());
    elapsed.restart();
    model.navigate(QStringLiteral("C:/slow"));
    QVERIFY(!model.progressVisible());
    QTest::qWait(100);
    QVERIFY(!model.progressVisible());
    QTRY_VERIFY_WITH_TIMEOUT(model.progressVisible(), 300);
    QVERIFY(elapsed.elapsed() >= 190);
    elapsed.restart();
    browser->gate->release();
    QTest::qWait(800);
    QCOMPARE(model.loadState(), State::Loading);
    QTRY_VERIFY_WITH_TIMEOUT(model.loadState() != State::Loading, 500);
    QVERIFY(elapsed.elapsed() >= 950);
    QVERIFY(!model.progressVisible());
    browser->gate->reset();
    model.navigate(QStringLiteral("C:/slow"));
    QTRY_VERIFY_WITH_TIMEOUT(model.progressVisible(), 350);
    model.navigate(QStringLiteral("C:/fast-again"));
    QTRY_VERIFY_WITH_TIMEOUT(model.loadState() != State::Loading, 180);
    QVERIFY(!model.progressVisible());
    QCOMPARE(model.path(), QStringLiteral("C:/fast-again"));
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
    QCOMPARE(model.prepareAcceptance(), Model::Acceptance::Invalid);
    QCOMPARE(browser->outsideInspections, 0);
    model.setAllowedRoots({QStringLiteral("D:/")});
    QCOMPARE(model.places().size(), 1);
    QVERIFY(model.places().constFirst().volume);
    model.setAllowedRoots({QStringLiteral("C:/allowed")});
    QVERIFY(!model.isPathAllowed(QStringLiteral("C:/allowed-other/file.txt")));
    QVERIFY(!model.isPathAllowed(QStringLiteral("C:/allowed/../other/file.txt")));
    QVERIFY(model.isPathAllowed(QStringLiteral("c:/ALLOWED/sub/file.txt")));
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
        QCOMPARE(model.prepareAcceptance(), Model::Acceptance::Ready);
        QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(directory.path())});
        model.selectRows({0});
        QCOMPARE(model.prepareAcceptance(), Model::Acceptance::Ready);
        QCOMPARE(model.selectedPaths(), QStringList{QDir::toNativeSeparators(directory.filePath(QStringLiteral("folder")))});
      } else {
        const int file = findRow(model, QStringLiteral("one.txt"));
        QVERIFY(file >= 0);
        model.selectRows(mode == Model::Mode::OpenFile
                             ? QList<int>{file}
                             : QList<int>{file, findRow(model, mode == Model::Mode::Mixed ? QStringLiteral("folder") : QStringLiteral("two.cmd"))});
        QCOMPARE(model.prepareAcceptance(), Model::Acceptance::Ready);
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
    QCOMPARE(model.prepareAcceptance(), Model::Acceptance::Ready);
    QVERIFY(model.selectedPaths().first().endsWith(QLatin1String("one.txt")));
    model.setQuery(QStringLiteral("ONe"));
    QCOMPARE(model.rowCount(), 1);
    model.setQuery({});
    model.setFilterIndex(1);
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
    QCOMPARE(model.prepareAcceptance(), Model::Acceptance::Replace);
    QFile original(existing);
    QVERIFY(original.open(QIODevice::ReadOnly));
    QCOMPARE(original.readAll(), QByteArray("original content"));
    model.setFilename(QStringLiteral("fresh"));
    QCOMPARE(model.prepareAcceptance(), Model::Acceptance::Ready);
    QVERIFY(model.selectedPaths().first().endsWith(QLatin1String("fresh.txt")));
    QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("fresh.txt"))));
    QSignalSpy errors(&model, &Model::validationFailed);
    for (const auto& name : {QStringLiteral("bad?.txt"), QStringLiteral("CON.txt"), QStringLiteral("../missing/name.txt")}) {
      model.setFilename(name);
      QCOMPARE(model.prepareAcceptance(), Model::Acceptance::Invalid);
      QVERIFY(model.selectedPaths().isEmpty());
    }
    QCOMPARE(errors.count(), 3);
    model.cancel();
    QVERIFY(model.selectedPaths().isEmpty());
    QVERIFY(!model.canAccept());
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
    QCOMPARE(errors.count(), 4);
    model.renameEntry(findRow(model, QStringLiteral("original.txt")), QStringLiteral("renamed.txt"));
    QTRY_COMPARE(model.loadState(), State::Ready);
    QTRY_VERIFY(findRow(model, QStringLiteral("renamed.txt")) >= 0);
    QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("original.txt"))));
    QFile renamed(directory.filePath(QStringLiteral("renamed.txt")));
    QVERIFY(renamed.open(QIODevice::ReadOnly));
    QCOMPARE(renamed.readAll(), QByteArray("original content"));
    QFile existing(directory.filePath(QStringLiteral("existing.txt")));
    QVERIFY(existing.open(QIODevice::ReadOnly));
    QCOMPARE(existing.readAll(), QByteArray("keep existing"));
  }
  void supersededAndCanceledLoadsCannotPublishStaleRows() {
    auto browser = std::make_shared<DelayedBrowser>();
    Model model(Model::Mode::OpenFile, browser);
    model.start(QStringLiteral("C:/slow"));
    QTRY_COMPARE(browser->gate->started.load(), 1);
    model.navigate(QStringLiteral("C:/current"));
    QTRY_COMPARE(model.loadState(), State::Ready);
    QCOMPARE(model.data(model.index(0, 0)).toString(), QStringLiteral("current.txt"));
    QTRY_COMPARE(browser->gate->canceled.load(), 1);
    model.navigate(QStringLiteral("C:/slow"));
    QTRY_COMPARE(browser->gate->started.load(), 2);
    model.cancel();
    QTRY_COMPARE(browser->gate->canceled.load(), 2);
    QTest::qWait(1100);
    QVERIFY(model.selectedPaths().isEmpty());
    QCOMPARE(model.loadState(), State::Canceled);
    model.navigate(QStringLiteral("C:/denied"));
    QTRY_COMPARE(model.loadState(), State::Failed);
    QCOMPARE(model.error(), QStringLiteral("Access denied: complete diagnostic"));
    QVERIFY(!model.canAccept());
    model.goBack();
    model.navigate(QStringLiteral("C:/current"));
    QTRY_COMPARE(model.loadState(), State::Ready);
  }
};
QTEST_GUILESS_MAIN(FilePickerViewModelTests)
#include "FilePickerViewModelTests.moc"
