/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QTreeWidget>
#include <QtTest>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <limits>
#include <map>
#include <mutex>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ui/CommitReportDialog.h"
#include "ui/Dialogs.h"
#include "ui/I18n.h"
#include "ui/OverlayFilesDialog.h"
#include "ui/RegistryPickerDialog.h"

namespace {

using uwf::ui::RegistryPickerDialog;

class MemoryFileDialogs final : public uwf::ui::dialogs::FileDialogProvider {
 public:
  QString savePath;
  QList<uwf::ui::dialogs::FileDialogRequest> requests;

  QStringList openFiles(QWidget*, const uwf::ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return {};
  }
  QString openFile(QWidget*, const uwf::ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return {};
  }
  QString selectDirectory(QWidget*, const uwf::ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return {};
  }
  QString saveFile(QWidget*, const uwf::ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return savePath;
  }
};

class MemoryRegistryBrowser final : public uwf::ui::RegistryBrowser {
 public:
  std::vector<std::string> roots{"HKEY_LOCAL_MACHINE", "HKEY_CURRENT_USER", "HKEY_CLASSES_ROOT", "HKEY_USERS", "HKEY_CURRENT_CONFIG"};
  std::map<std::string, std::vector<std::string>, std::less<>> children;
  std::map<std::string, std::vector<uwf::regkey::RegValueInfo>, std::less<>> keyValues;
  bool failSubkeyEnumeration = false;

  [[nodiscard]] std::vector<std::string> rootHiveLongNames() const override { return roots; }

  [[nodiscard]] std::vector<std::string> subkeyNames(const std::string_view key) const override {
    if (failSubkeyEnumeration) throw std::runtime_error("mock registry enumeration failure");
    if (const auto it = children.find(key); it != children.end()) return it->second;
    return {};
  }

  [[nodiscard]] bool hasSubkeys(const std::string_view key) const override {
    if (const auto it = children.find(key); it != children.end()) return !it->second.empty();
    return false;
  }

  [[nodiscard]] std::vector<uwf::regkey::RegValueInfo> values(const std::string_view key) const override {
    if (const auto it = keyValues.find(key); it != keyValues.end()) return it->second;
    return {};
  }
};

std::vector<uint8_t> utf16Le(const std::u16string_view text) {
  std::vector<uint8_t> bytes;
  bytes.reserve((text.size() + 1) * 2);
  for (const char16_t unit : text) {
    bytes.push_back(static_cast<uint8_t>(unit & 0xffu));
    bytes.push_back(static_cast<uint8_t>((unit >> 8u) & 0xffu));
  }
  bytes.push_back(0);
  bytes.push_back(0);
  return bytes;
}

MemoryRegistryBrowser registrySnapshot() {
  MemoryRegistryBrowser browser;
  browser.children["HKEY_LOCAL_MACHINE"] = {"SOFTWARE", "SYSTEM", "PERSISTENT_SOFTWARE"};
  browser.children["HKEY_LOCAL_MACHINE\\SOFTWARE"] = {"Vendor"};
  browser.children["HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor"] = {"Product"};
  browser.keyValues["HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product"] = {
      {"", 1u, utf16Le(u"Default text")},
      {"Name", 1u, utf16Le(u"UWF Manager")},
      {"Enabled", 4u, std::vector<uint8_t>{1, 0, 0, 0}},
      {"Large", 3u, std::nullopt},
  };
  return browser;
}

QPushButton* buttonByText(QWidget* parent, const QString& text) {
  const auto buttons = parent->findChildren<QPushButton*>();
  const auto it = std::ranges::find_if(buttons, [&text](const QPushButton* button) { return button->text() == text; });
  return it == buttons.end() ? nullptr : *it;
}

QPushButton* buttonByRole(QWidget* parent, const QDialogButtonBox::ButtonRole role) {
  const auto boxes = parent->findChildren<QDialogButtonBox*>();
  for (auto* box : boxes) {
    for (auto* button : box->buttons()) {
      if (box->buttonRole(button) == role) return qobject_cast<QPushButton*>(button);
    }
  }
  return nullptr;
}

QString labelContaining(const QWidget* parent, const QString& needle) {
  for (const auto* label : parent->findChildren<QLabel*>()) {
    if (label->text().contains(needle, Qt::CaseInsensitive)) return label->text();
  }
  return {};
}

uwf::ui::OverlayFilesServices overlayServices(uwf::ui::OverlayFileLoader loader,
                                              uwf::ui::dialogs::FileDialogProvider& fileDialogs = uwf::ui::dialogs::systemFileDialogs()) {
  return {std::move(loader), fileDialogs};
}

QTreeWidgetItem* findTreeItem(QTreeWidgetItem* parent, const QString& text) {
  if (!parent) return nullptr;
  if (parent->text(0).compare(text, Qt::CaseInsensitive) == 0) return parent;
  for (int index = 0; index < parent->childCount(); ++index) {
    if (auto* match = findTreeItem(parent->child(index), text)) return match;
  }
  return nullptr;
}

QTreeWidgetItem* findTreeItem(QTreeWidget* tree, const QString& text) {
  for (int index = 0; index < tree->topLevelItemCount(); ++index) {
    if (auto* match = findTreeItem(tree->topLevelItem(index), text)) return match;
  }
  return nullptr;
}

class DialogBehaviorTests final : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase() { uwf::ui::I18n::instance().setLang(uwf::ui::I18n::Lang::En); }
  void overlayFilesNormalizesMergesAndClassifiesMockRows();
  void overlayFilesSizeAggregationSaturatesWithoutWrapping();
  void overlayFilesPaginatesWithoutReloadingTheSource();
  void overlayFilesExportWritesTheCompleteSnapshotAndHonorsCancellation();
  void overlayFilesFailureKeepsActionsUnavailable();
  void overlayFilesDestructionCancelsAndJoinsTheActiveLoader();
  void overlayFilesRejectsMissingLoader();
  void overlayFilesContextMenuCommitsOnlyOrdinaryEntries();
  void registryPickerBrowsesSnapshotAndFiltersPersistentHives();
  void registryPickerFailedEnumerationCanBeRetriedWithoutPartialChildren();
  void registryValuePreviewCoversNumericStringMultiStringAndBinaryBoundaries();
  void registryPickerReturnsNamedValueAndWholeKeyIntents();
  void registryAddressBarNavigatesShorthandAndClearResetsSelection();
  void registryExclusionDisablesValueSelection();
  void commitReportPaginatesAndCopiesTheCompleteDeleteReport();
  void commitReportKeepsPersistColumnsAndEmptyStateConsistent();
};

void DialogBehaviorTests::overlayFilesNormalizesMergesAndClassifiesMockRows() {
  std::atomic<int> loads{0};
  QString observedDriveLetter;
  uwf::ui::OverlayFilesDialog dialog(QStringLiteral("c"), overlayServices([&loads, &observedDriveLetter](const QString& driveLetter, std::stop_token) {
                                       observedDriveLetter = driveLetter;
                                       ++loads;
                                       return QVector<uwf::ui::OverlayFileEntry>{
                                           {QStringLiteral("\\Users\\Alice\\note.txt:$DATA"), {}, false, false, 100},
                                           {QStringLiteral("\\Users\\Alice\\note.txt:history:$DATA"), {}, false, false, 50},
                                           {QStringLiteral("\\Folder:$INDEX_ALLOCATION"), {}, false, false, 4096},
                                           {QStringLiteral("\\$Extend\\$Deleted\\42:$DATA"), {}, false, false, 32},
                                       };
                                     }));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* list = dialog.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  QVERIFY(list);
  QTRY_COMPARE_WITH_TIMEOUT(loads.load(), 1, 1000);
  QCOMPARE(observedDriveLetter, QStringLiteral("C:"));
  QTRY_COMPARE_WITH_TIMEOUT(list->count(), 3, 1000);
  QTRY_VERIFY_WITH_TIMEOUT(labelContaining(&dialog, QStringLiteral("3 file")).size() > 0, 1000);

  QMap<QString, QListWidgetItem*> byPath;
  for (int row = 0; row < list->count(); ++row) byPath.insert(list->item(row)->data(Qt::UserRole).toString(), list->item(row));
  QVERIFY(byPath.contains(QStringLiteral("C:\\Users\\Alice\\note.txt")));
  QVERIFY(byPath.value(QStringLiteral("C:\\Users\\Alice\\note.txt"))->text().contains(QStringLiteral("150 B")));
  QVERIFY(byPath.value(QStringLiteral("C:\\Folder"))->data(Qt::UserRole + 1).toBool());
  QVERIFY(byPath.value(QStringLiteral("C:\\$Extend\\$Deleted\\42"))->data(Qt::UserRole + 2).toBool());
  auto* exportButton = buttonByText(&dialog, QStringLiteral("Export to file…"));
  QVERIFY(exportButton);
  QVERIFY(exportButton->isEnabled());
}

void DialogBehaviorTests::overlayFilesSizeAggregationSaturatesWithoutWrapping() {
  MemoryFileDialogs files;
  uwf::ui::OverlayFilesDialog dialog(QStringLiteral("C:"),
                                     overlayServices(
                                         [](const QString&, std::stop_token) {
                                           return QVector<uwf::ui::OverlayFileEntry>{
                                               {QStringLiteral("\\Data\\huge.bin:$DATA"), {}, false, false, std::numeric_limits<qulonglong>::max()},
                                               {QStringLiteral("\\Data\\huge.bin:extra:$DATA"), {}, false, false, 1},
                                               {QStringLiteral("\\Data\\other.bin:$DATA"), {}, false, false, std::numeric_limits<qulonglong>::max()},
                                           };
                                         },
                                         files));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* list = dialog.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  auto* exportButton = buttonByText(&dialog, QStringLiteral("Export to file…"));
  QVERIFY(list);
  QVERIFY(exportButton);
  QTRY_COMPARE_WITH_TIMEOUT(list->count(), 2, 1000);
  for (int row = 0; row < list->count(); ++row) QVERIFY(!list->item(row)->text().contains(QStringLiteral("0 B")));

  QTemporaryDir outputDirectory;
  QVERIFY(outputDirectory.isValid());
  files.savePath = outputDirectory.filePath(QStringLiteral("saturated.txt"));
  QTimer::singleShot(0, this, [] {
    if (auto* information = qobject_cast<QDialog*>(QApplication::activeModalWidget())) information->accept();
  });
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QFile written(files.savePath);
  QVERIFY(written.open(QIODevice::ReadOnly | QIODevice::Text));
  const QString exported = QString::fromUtf8(written.readAll());
  QCOMPARE(exported.count(QStringLiteral("\t18446744073709551615\t")), 2);
  QVERIFY(!exported.contains(QStringLiteral("\t0\t0 B\t")));
}

void DialogBehaviorTests::overlayFilesPaginatesWithoutReloadingTheSource() {
  std::atomic<int> loads{0};
  uwf::ui::OverlayFilesDialog dialog(
      QStringLiteral("C:"), overlayServices([&loads](const QString&, std::stop_token) {
        ++loads;
        QVector<uwf::ui::OverlayFileEntry> rows;
        for (int index = 0; index < 80; ++index) {
          rows.append({QStringLiteral("\\Files\\item-%1.txt:$DATA").arg(index, 3, 10, QChar('0')), {}, false, false, static_cast<qulonglong>(index + 1)});
        }
        return rows;
      }));
  dialog.resize(760, 420);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* list = dialog.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  auto* last = buttonByText(&dialog, QStringLiteral("»"));
  QVERIFY(list);
  QVERIFY(last);
  QTRY_VERIFY_WITH_TIMEOUT(last->isEnabled(), 1000);
  const QString firstPagePath = list->item(0)->data(Qt::UserRole).toString();
  QTest::mouseClick(last, Qt::LeftButton);
  QVERIFY(list->count() > 0);
  QVERIFY(list->item(0)->data(Qt::UserRole).toString() != firstPagePath);
  QCOMPARE(loads.load(), 1);
}

void DialogBehaviorTests::overlayFilesExportWritesTheCompleteSnapshotAndHonorsCancellation() {
  MemoryFileDialogs files;
  uwf::ui::OverlayFilesDialog dialog(QStringLiteral("C:"), overlayServices(
                                                               [](const QString&, std::stop_token) {
                                                                 return QVector<uwf::ui::OverlayFileEntry>{
                                                                     {QStringLiteral("\\Data\\note.txt:$DATA"), {}, false, false, 12},
                                                                     {QStringLiteral("\\Cache:$INDEX_ALLOCATION"), {}, false, false, 4096}};
                                                               },
                                                               files));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* exportButton = buttonByText(&dialog, QStringLiteral("Export to file…"));
  QVERIFY(exportButton);
  QTRY_VERIFY_WITH_TIMEOUT(exportButton->isEnabled(), 1000);

  QTest::mouseClick(exportButton, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 1);
  QVERIFY(files.requests.front().initialPath.contains(QStringLiteral("overlay-files-C-")));

  QTemporaryDir outputDirectory;
  QVERIFY(outputDirectory.isValid());
  files.savePath = outputDirectory.filePath(QStringLiteral("overlay-files.txt"));
  {
    QFile previous(files.savePath);
    QVERIFY(previous.open(QIODevice::WriteOnly));
    QCOMPARE(previous.write("stale"), qint64{5});
  }
  bool exportSucceeded = false;
  QTimer::singleShot(0, this, [&] {
    if (auto* information = qobject_cast<QDialog*>(QApplication::activeModalWidget())) {
      exportSucceeded = information->windowTitle() == QStringLiteral("Export finished");
      information->accept();
    }
  });
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QVERIFY(exportSucceeded);

  QFile written(files.savePath);
  QVERIFY(written.open(QIODevice::ReadOnly | QIODevice::Text));
  const QString text = QString::fromUtf8(written.readAll());
  QVERIFY(text.contains(QStringLiteral("# UWF overlay files for C:")));
  QVERIFY(text.contains(QStringLiteral("file\t12\t")));
  QVERIFY(text.contains(QStringLiteral("\tC:\\Data\\note.txt")));
  QVERIFY(text.contains(QStringLiteral("dir\t4096\t")));
  QVERIFY(text.contains(QStringLiteral("\tC:\\Cache")));
  QCOMPARE(files.requests.size(), 2);

  files.savePath = QDir::tempPath();
  bool failureReported = false;
  QTimer::singleShot(0, this, [&] {
    auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    failureReported = warning && labelContaining(warning, QStringLiteral("Could not open file for writing")).size() > 0;
    if (warning) warning->accept();
  });
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QVERIFY(failureReported);
  QCOMPARE(files.requests.size(), 3);
}

void DialogBehaviorTests::overlayFilesFailureKeepsActionsUnavailable() {
  uwf::ui::OverlayFilesDialog dialog(QStringLiteral("D:"), overlayServices([](const QString&, std::stop_token) -> QVector<uwf::ui::OverlayFileEntry> {
                                       throw std::runtime_error("mock provider failure");
                                     }));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* progress = dialog.findChild<QProgressBar*>();
  auto* exportButton = buttonByText(&dialog, QStringLiteral("Export to file…"));
  QVERIFY(progress);
  QVERIFY(exportButton);
  QTRY_VERIFY_WITH_TIMEOUT(!progress->isVisible(), 1000);
  QTRY_VERIFY_WITH_TIMEOUT(labelContaining(&dialog, QStringLiteral("mock provider failure")).size() > 0, 1000);
  QVERIFY(!exportButton->isEnabled());
  auto* list = dialog.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  QVERIFY(list);
  QCOMPARE(list->count(), 0);
}

void DialogBehaviorTests::overlayFilesDestructionCancelsAndJoinsTheActiveLoader() {
  std::mutex mutex;
  std::condition_variable_any changed;
  std::atomic<bool> started{false};
  std::atomic<bool> stopped{false};
  {
    uwf::ui::OverlayFilesDialog dialog(QStringLiteral("C:"), overlayServices([&](const QString&, const std::stop_token stopToken) {
                                         started = true;
                                         changed.notify_all();
                                         std::unique_lock lock(mutex);
                                         changed.wait(lock, stopToken, [] { return false; });
                                         stopped = stopToken.stop_requested();
                                         return QVector<uwf::ui::OverlayFileEntry>{};
                                       }));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_VERIFY_WITH_TIMEOUT(started.load(), 1000);
  }
  QVERIFY(stopped.load());
}

void DialogBehaviorTests::overlayFilesRejectsMissingLoader() {
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, uwf::ui::OverlayFilesDialog(QStringLiteral("C:"), overlayServices(uwf::ui::OverlayFileLoader{})));
}

void DialogBehaviorTests::overlayFilesContextMenuCommitsOnlyOrdinaryEntries() {
  uwf::ui::OverlayFilesDialog dialog(QStringLiteral("C:"), overlayServices([](const QString&, std::stop_token) {
                                       return QVector<uwf::ui::OverlayFileEntry>{
                                           {QStringLiteral("\\Users\\Alice\\note.txt:$DATA"), {}, false, false, 100},
                                           {QStringLiteral("\\$MFT:$DATA"), {}, false, false, 50},
                                       };
                                     }));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* list = dialog.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  QVERIFY(list);
  QTRY_COMPARE_WITH_TIMEOUT(list->count(), 2, 1000);
  QSignalSpy commitSpy(&dialog, &uwf::ui::OverlayFilesDialog::commitFileRequested);

  auto openContextMenu = [list](const int row, const std::function<void(QMenu*)>& inspect) {
    list->scrollToItem(list->item(row));
    const QPoint local = list->visualItemRect(list->item(row)).center();
    QTimer::singleShot(0, qApp, [inspect] {
      auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      if (menu) inspect(menu);
    });
    QTimer::singleShot(250, qApp, [] {
      if (auto* popup = QApplication::activePopupWidget()) popup->close();
    });
    QContextMenuEvent event(QContextMenuEvent::Mouse, local, list->viewport()->mapToGlobal(local));
    QApplication::sendEvent(list->viewport(), &event);
  };

  int ordinaryRow = -1;
  int metadataRow = -1;
  for (int row = 0; row < list->count(); ++row) {
    if (list->item(row)->data(Qt::UserRole + 2).toBool())
      metadataRow = row;
    else
      ordinaryRow = row;
  }
  QVERIFY(ordinaryRow >= 0);
  QVERIFY(metadataRow >= 0);
  const QString ordinaryPath = list->item(ordinaryRow)->data(Qt::UserRole).toString();
  openContextMenu(ordinaryRow, [](QMenu* menu) {
    QCOMPARE(menu->actions().size(), 2);
    menu->actions().at(1)->trigger();
  });
  QCOMPARE(commitSpy.count(), 1);
  QCOMPARE(commitSpy.at(0).at(0).toString(), ordinaryPath);

  openContextMenu(metadataRow, [](QMenu* menu) {
    QCOMPARE(menu->actions().size(), 1);
    menu->close();
  });
  QCOMPARE(commitSpy.count(), 1);
}

void DialogBehaviorTests::registryPickerBrowsesSnapshotAndFiltersPersistentHives() {
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(RegistryPickerDialog::Mode::CommitValue, QStringLiteral("Registry snapshot"), browser);
  dialog.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* tree = dialog.findChild<QTreeWidget*>();
  QVERIFY(tree);
  auto* hklm = findTreeItem(tree, QStringLiteral("HKEY_LOCAL_MACHINE"));
  QVERIFY(hklm);
  hklm->setExpanded(true);
  QCoreApplication::processEvents();
  QVERIFY(findTreeItem(hklm, QStringLiteral("SOFTWARE")));
  QVERIFY(findTreeItem(hklm, QStringLiteral("SYSTEM")));
  QVERIFY(!findTreeItem(hklm, QStringLiteral("PERSISTENT_SOFTWARE")));

  dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  QCOMPARE(tree->currentItem()->text(0), QStringLiteral("Product"));
  auto* values = dialog.findChild<QTableWidget*>();
  QVERIFY(values);
  QCOMPARE(values->rowCount(), 4);
  QCOMPARE(values->item(0, 0)->text(), QStringLiteral("(Default)"));
  QVERIFY(!(values->item(0, 0)->flags() & Qt::ItemIsSelectable));
  QCOMPARE(values->item(1, 0)->text(), QStringLiteral("Name"));
  QCOMPARE(values->item(1, 1)->text(), QStringLiteral("REG_SZ"));
  QCOMPARE(values->item(1, 2)->text(), QStringLiteral("UWF Manager"));
  QCOMPARE(values->item(2, 2)->text(), QStringLiteral("0x00000001 (1)"));
  QVERIFY(values->item(3, 2)->text().contains(QStringLiteral("unavailable"), Qt::CaseInsensitive));
}

void DialogBehaviorTests::registryPickerFailedEnumerationCanBeRetriedWithoutPartialChildren() {
  auto browser = registrySnapshot();
  browser.failSubkeyEnumeration = true;
  bool warningObserved = false;
  QTimer warningCloser;
  warningCloser.setInterval(1);
  connect(&warningCloser, &QTimer::timeout, this, [&warningObserved] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) {
      warningObserved = labelContaining(warning, QStringLiteral("mock registry enumeration failure")).size() > 0;
      warning->accept();
    }
  });
  warningCloser.start();
  RegistryPickerDialog dialog(RegistryPickerDialog::Mode::CommitValue, QStringLiteral("Registry retry"), browser);
  warningCloser.stop();
  QVERIFY(warningObserved);
  auto* tree = dialog.findChild<QTreeWidget*>();
  QVERIFY(tree);
  auto* hklm = findTreeItem(tree, QStringLiteral("HKEY_LOCAL_MACHINE"));
  QVERIFY(hklm);
  QVERIFY(!findTreeItem(hklm, QStringLiteral("SOFTWARE")));

  browser.failSubkeyEnumeration = false;
  hklm->setExpanded(true);
  QCoreApplication::processEvents();
  QVERIFY(findTreeItem(hklm, QStringLiteral("SOFTWARE")));
  QVERIFY(findTreeItem(hklm, QStringLiteral("SYSTEM")));
}

void DialogBehaviorTests::registryValuePreviewCoversNumericStringMultiStringAndBinaryBoundaries() {
  auto browser = registrySnapshot();
  std::vector<uint8_t> longBinary(65);
  std::iota(longBinary.begin(), longBinary.end(), uint8_t{0});
  browser.keyValues["HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product"] = {
      {"EmptyString", 1u, std::vector<uint8_t>{}},
      {"Expand", 2u, utf16Le(u"%SystemRoot%")},
      {"BigEndian", 5u, std::vector<uint8_t>{0, 0, 0, 2}},
      {"Multi", 7u, utf16Le(std::u16string_view(u"one\0two\0", 8))},
      {"Qword", 11u, std::vector<uint8_t>{1, 0, 0, 0, 0, 0, 0, 0}},
      {"ShortDword", 4u, std::vector<uint8_t>{1, 2, 3}},
      {"OddUtf16", 1u, std::vector<uint8_t>{'A', 0, 'B'}},
      {"LoneHighSurrogate", 1u, std::vector<uint8_t>{0, 0xD8}},
      {"LoneLowSurrogate", 1u, std::vector<uint8_t>{0, 0xDC}},
      {"SurrogatePair", 1u, std::vector<uint8_t>{0x3D, 0xD8, 0x00, 0xDE, 0, 0}},
      {"Binary", 3u, std::move(longBinary)},
  };
  RegistryPickerDialog dialog(RegistryPickerDialog::Mode::CommitValue, QStringLiteral("Registry values"), browser);
  dialog.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* values = dialog.findChild<QTableWidget*>();
  QVERIFY(values);

  auto preview = [values](const QString& name) {
    for (int row = 0; row < values->rowCount(); ++row) {
      if (values->item(row, 0)->text() == name) return values->item(row, 2)->text();
    }
    return QString{};
  };
  QCOMPARE(preview(QStringLiteral("EmptyString")), QStringLiteral("(value not set)"));
  QCOMPARE(preview(QStringLiteral("Expand")), QStringLiteral("%SystemRoot%"));
  QCOMPARE(preview(QStringLiteral("BigEndian")), QStringLiteral("0x00000002 (2)"));
  QCOMPARE(preview(QStringLiteral("Multi")), QStringLiteral("one · two"));
  QCOMPARE(preview(QStringLiteral("Qword")), QStringLiteral("0x0000000000000001 (1)"));
  QCOMPARE(preview(QStringLiteral("ShortDword")), QStringLiteral("(empty)"));
  QCOMPARE(preview(QStringLiteral("OddUtf16")), QString::fromUtf8("A\xEF\xBF\xBD"));
  QCOMPARE(preview(QStringLiteral("LoneHighSurrogate")), QString(QChar(QChar::ReplacementCharacter)));
  QCOMPARE(preview(QStringLiteral("LoneLowSurrogate")), QString(QChar(QChar::ReplacementCharacter)));
  QCOMPARE(preview(QStringLiteral("SurrogatePair")), QString::fromUtf8("\xF0\x9F\x98\x80"));
  QVERIFY(preview(QStringLiteral("Binary")).endsWith(QStringLiteral(" …")));
}

void DialogBehaviorTests::registryPickerReturnsNamedValueAndWholeKeyIntents() {
  auto browser = registrySnapshot();
  RegistryPickerDialog named(RegistryPickerDialog::Mode::CommitValue, QStringLiteral("Commit registry"), browser);
  named.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  named.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  named.show();
  QVERIFY(QTest::qWaitForWindowExposed(&named));
  auto* values = named.findChild<QTableWidget*>();
  auto* ok = buttonByRole(&named, QDialogButtonBox::AcceptRole);
  QVERIFY(values);
  QVERIFY(ok);
  values->selectRow(1);
  QTest::mouseClick(ok, Qt::LeftButton);
  const auto namedResult = named.selection();
  QVERIFY(namedResult.has_value());
  const auto namedSelection = namedResult.value_or(RegistryPickerDialog::Result{});
  QCOMPARE(namedSelection.key, QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product"));
  QCOMPARE(namedSelection.valueName, QStringLiteral("Name"));

  RegistryPickerDialog wholeKey(RegistryPickerDialog::Mode::DeleteValue, QStringLiteral("Delete registry"), browser);
  wholeKey.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  wholeKey.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  wholeKey.show();
  QVERIFY(QTest::qWaitForWindowExposed(&wholeKey));
  auto* wholeKeyOk = buttonByRole(&wholeKey, QDialogButtonBox::AcceptRole);
  QVERIFY(wholeKeyOk);
  QVERIFY(wholeKeyOk->isEnabled());
  QTest::mouseClick(wholeKeyOk, Qt::LeftButton);
  const auto wholeKeyResult = wholeKey.selection();
  QVERIFY(wholeKeyResult.has_value());
  QVERIFY(wholeKeyResult.value_or(RegistryPickerDialog::Result{}).valueName.isEmpty());
}

void DialogBehaviorTests::registryAddressBarNavigatesShorthandAndClearResetsSelection() {
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(RegistryPickerDialog::Mode::CommitValue, QStringLiteral("Registry address"), browser);
  dialog.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* address = dialog.findChild<QLineEdit*>();
  auto* tree = dialog.findChild<QTreeWidget*>();
  auto* values = dialog.findChild<QTableWidget*>();
  auto* ok = buttonByRole(&dialog, QDialogButtonBox::AcceptRole);
  QVERIFY(address && tree && values && ok);

  address->setText(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  QTest::keyClick(address, Qt::Key_Return);
  QVERIFY(tree->currentItem());
  QCOMPARE(tree->currentItem()->text(0), QStringLiteral("Product"));
  QCOMPARE(address->text(), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product"));
  QCOMPARE(values->rowCount(), 4);

  address->clear();
  QCOMPARE(tree->selectedItems().size(), 0);
  QCOMPARE(values->selectedItems().size(), 0);
  QVERIFY(!ok->isEnabled());
  QCOMPARE(tree->topLevelItemCount(), 5);
}

void DialogBehaviorTests::registryExclusionDisablesValueSelection() {
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(RegistryPickerDialog::Mode::Exclusion, QStringLiteral("Registry exclusion"), browser);
  dialog.setAvailabilityChecker([](const QString& key) {
    return key.endsWith(QStringLiteral("Product")) ? RegistryPickerDialog::KeyAvailability::Selectable : RegistryPickerDialog::KeyAvailability::ContainerOnly;
  });
  dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* values = dialog.findChild<QTableWidget*>();
  auto* ok = buttonByRole(&dialog, QDialogButtonBox::AcceptRole);
  QVERIFY(values);
  QVERIFY(ok);
  QCOMPARE(values->selectionMode(), QAbstractItemView::NoSelection);
  QVERIFY(ok->isEnabled());
  QTest::mouseClick(ok, Qt::LeftButton);
  const auto result = dialog.selection();
  QVERIFY(result.has_value());
  QVERIFY(result.value_or(RegistryPickerDialog::Result{}).valueName.isEmpty());
}

void DialogBehaviorTests::commitReportPaginatesAndCopiesTheCompleteDeleteReport() {
  QList<uwf::ui::CommitReportRow> rows;
  for (int index = 0; index < 205; ++index) {
    rows.append({index % 2 == 0 ? QStringLiteral("Succeeded") : QStringLiteral("Failed"), QStringLiteral("C:\\Files\\item-%1.txt").arg(index),
                 index % 2 == 0 ? QStringLiteral("-") : QStringLiteral("0x80041001"), index % 2 == 0 ? QStringLiteral("-") : QStringLiteral("Mock failure"),
                 true, index % 2 == 0 ? std::optional<bool>(false) : std::nullopt});
  }

  bool inspected = false;
  bool structureValid = false;
  bool paginationValid = false;
  bool copyValid = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* table = dialog->findChild<QTableWidget*>();
    if (!table) {
      dialog->reject();
      return;
    }
    structureValid = table->columnCount() == 6 && table->rowCount() == 200;
    auto* next = buttonByText(dialog, QStringLiteral("Next page"));
    if (next && next->isEnabled()) QTest::mouseClick(next, Qt::LeftButton);
    paginationValid = table->rowCount() == 5 && table->item(0, 1) && table->item(0, 1)->text() == QStringLiteral("C:\\Files\\item-200.txt");
    auto* copyAll = buttonByRole(dialog, QDialogButtonBox::ActionRole);
    if (copyAll) QTest::mouseClick(copyAll, Qt::LeftButton);
    copyValid = QApplication::clipboard()->text().contains(QStringLiteral("item-204.txt"));
    inspected = true;
    if (auto* accept = buttonByRole(dialog, QDialogButtonBox::AcceptRole))
      accept->click();
    else
      dialog->accept();
  });

  uwf::ui::showCommitReport(nullptr, rows, uwf::ui::CommitOperation::DeleteAndPersist, 3);
  QVERIFY(inspected);
  QVERIFY(structureValid);
  QVERIFY(paginationValid);
  QVERIFY(copyValid);
}

void DialogBehaviorTests::commitReportKeepsPersistColumnsAndEmptyStateConsistent() {
  bool inspected = false;
  bool structureValid = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* table = dialog->findChild<QTableWidget*>();
    structureValid = table && table->columnCount() == 4 && table->rowCount() == 0 && !labelContaining(dialog, QStringLiteral("0 succeeded")).isEmpty() &&
                     !labelContaining(dialog, QStringLiteral("0 entries total")).isEmpty();
    inspected = true;
    if (auto* accept = buttonByRole(dialog, QDialogButtonBox::AcceptRole))
      accept->click();
    else
      dialog->accept();
  });

  uwf::ui::showCommitReport(nullptr, {}, uwf::ui::CommitOperation::Persist);
  QVERIFY(inspected);
  QVERIFY(structureValid);
}

}  // namespace

QTEST_MAIN(DialogBehaviorTests)

#include "DialogBehaviorTests.moc"
