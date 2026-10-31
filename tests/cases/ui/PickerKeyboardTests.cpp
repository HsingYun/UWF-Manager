// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QIntValidator>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableView>
#include <QTest>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>

#include "presentation/common/ItemRoles.h"
#include "presentation/files/FilePickerViewModel.h"
#include "presentation/registry/RegistryPickerViewModel.h"
#include "design/DesignTokens.h"
#include "ui/dialogs/PanelDialog.h"
#include "ui/files/FilePickerDialog.h"
#include "ui/registry/RegistryPickerDialog.h"

namespace {
using namespace uwf;

class MemoryFileBrowser final : public app::FileBrowser {
 public:
  QVector<app::FileBrowserEntry> list(const QString& path, std::stop_token) const override {
    return {{path + QStringLiteral("/note.txt"), QStringLiteral("note.txt"), false}};
  }
  app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, !path.endsWith(QStringLiteral(".txt"))}; }
  QVector<app::FileBrowserPlace> places() const override { return {}; }
  void createDirectory(const QString&) const override {}
};

class MemoryRegistryBrowser final : public app::RegistryBrowser {
 public:
  std::vector<std::string> rootHiveLongNames() const override { return {"HKEY_LOCAL_MACHINE"}; }
  std::vector<std::string> subkeyNames(const std::string_view key) const override {
    if (key == "HKEY_LOCAL_MACHINE") return {"SOFTWARE"};
    if (key == "HKEY_LOCAL_MACHINE\\SOFTWARE") return {"Vendor"};
    return {};
  }
  bool hasSubkeys(const std::string_view key) const override { return !subkeyNames(key).empty(); }
  std::vector<app::RegistryValueInfo> values(std::string_view) const override { return {}; }
};

void addEnterKeys() {
  QTest::addColumn<int>("key");
  QTest::newRow("return") << static_cast<int>(Qt::Key_Return);
  QTest::newRow("keypad-enter") << static_cast<int>(Qt::Key_Enter);
}

void pressEnter(QWidget* target, const int key) {
  target->setFocus();
  QTest::keyClick(target, static_cast<Qt::Key>(key), key == Qt::Key_Enter ? Qt::KeypadModifier : Qt::NoModifier);
}

class PickerKeyboardTests final : public QObject {
  Q_OBJECT

 private slots:
  void filePickerFieldsKeepEnterLocal_data() {
    QTest::addColumn<int>("key");
    QTest::addColumn<QString>("fieldName");
    for (const auto key : {Qt::Key_Return, Qt::Key_Enter})
      for (const auto* field : {"pickerAddress", "pickerSearch", "pickerFilename"})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2").arg(static_cast<int>(key)).arg(QString::fromLatin1(field))))
            << static_cast<int>(key) << QString::fromLatin1(field);
  }

  void filePickerFieldsKeepEnterLocal() {
    QFETCH(int, key);
    QFETCH(QString, fieldName);
    using Model = presentation::FilePickerViewModel;
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, std::make_shared<MemoryFileBrowser>());
    ui::FilePickerDialog dialog(model, QStringLiteral("Picker keyboard"));
    model->start(QStringLiteral("C:/Root"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Ready);
    auto* field = dialog.findChild<QLineEdit*>(fieldName);
    auto* table = dialog.findChild<QTableView*>(QStringLiteral("pickerEntries"));
    QVERIFY(field && table);
    QCOMPARE(table->verticalHeader()->defaultSectionSize(), ui::design::tableRowHeight);
    QCOMPARE(table->horizontalHeader()->height(), ui::design::tableHeaderHeight);
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    QSignalSpy rejected(&dialog, &QDialog::rejected);
    if (fieldName == QStringLiteral("pickerAddress")) field->setText(QStringLiteral("C:/Next"));
    if (fieldName == QStringLiteral("pickerFilename")) {
      field->clear();
      QTest::keyClicks(field, QStringLiteral("note.txt"));
    }
    pressEnter(field, key);
    if (fieldName == QStringLiteral("pickerFilename")) {
      QCOMPARE(accepted.count(), 1);
      QCOMPARE(dialog.selectedPaths(), QStringList{QStringLiteral("C:\\Root\\note.txt")});
    } else {
      QTRY_COMPARE(model->path(), fieldName == QStringLiteral("pickerAddress") ? QStringLiteral("C:/Next") : QStringLiteral("C:/Root"));
      QVERIFY(dialog.isVisible());
      QCOMPARE(accepted.count(), 0);
    }
    QCOMPARE(rejected.count(), 0);
  }

  void registryAddressEnterNavigatesWithoutAccepting_data() { addEnterKeys(); }
  void registryAddressEnterNavigatesWithoutAccepting() {
    QFETCH(int, key);
    MemoryRegistryBrowser browser;
    auto model = std::make_shared<presentation::RegistryPickerViewModel>(presentation::RegistryPickerViewModel::Mode::CommitValue, browser);
    ui::RegistryPickerDialog dialog(model, QStringLiteral("Registry keyboard"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* address = dialog.findChild<QLineEdit*>(QStringLiteral("registryAddress"));
    auto* tree = dialog.findChild<QTreeView*>(QStringLiteral("registryTree"));
    QVERIFY(address && tree);
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    QSignalSpy rejected(&dialog, &QDialog::rejected);
    address->setText(QStringLiteral("HKLM\\SOFTWARE\\Vendor"));
    pressEnter(address, key);
    QCOMPARE(tree->currentIndex().data(presentation::PathRole).toString(), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor"));
    QVERIFY(dialog.isVisible());
    QVERIFY(!dialog.selection());
    QCOMPARE(accepted.count(), 0);
    QCOMPARE(rejected.count(), 0);
  }

  void editorEnterDoesNotFallThroughToTheDialog_data() {
    QTest::addColumn<int>("key");
    QTest::addColumn<QString>("kind");
    for (const auto key : {Qt::Key_Return, Qt::Key_Enter})
      for (const auto* kind : {"action", "no-action", "readonly", "invalid", "editable-combo"})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2").arg(static_cast<int>(key)).arg(QString::fromLatin1(kind))))
            << static_cast<int>(key) << QString::fromLatin1(kind);
  }

  void editorEnterDoesNotFallThroughToTheDialog() {
    QFETCH(int, key);
    QFETCH(QString, kind);
    ui::PanelDialog dialog;
    auto* layout = new QVBoxLayout(&dialog);
    auto* input = new QLineEdit(&dialog);
    QComboBox* combo = nullptr;
    if (kind == QStringLiteral("editable-combo")) {
      delete input;
      combo = new QComboBox(&dialog);
      combo->setEditable(true);
      combo->addItem(QStringLiteral("Existing"));
      input = combo->lineEdit();
      layout->addWidget(combo);
    } else {
      input->setText(QStringLiteral("text"));
      input->setReadOnly(kind == QStringLiteral("readonly"));
      if (kind == QStringLiteral("invalid")) {
        input->setValidator(new QIntValidator(10, 20, input));
        input->setText(QStringLiteral("1"));
      }
      layout->addWidget(input);
    }
    auto* cancel = new QPushButton(QStringLiteral("Cancel"), &dialog);
    cancel->setDefault(true);
    layout->addWidget(cancel);
    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    int actions = 0;
    if (kind == QStringLiteral("action")) connect(input, &QLineEdit::returnPressed, &dialog, [&] { ++actions; });
    QSignalSpy returnPressed(input, &QLineEdit::returnPressed);
    QSignalSpy canceled(&dialog, &QDialog::rejected);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    if (combo) {
      input->setFocus();
      input->selectAll();
      QTest::keyClicks(input, QStringLiteral("Added"));
    }
    pressEnter(input, key);
    QCOMPARE(actions, kind == QStringLiteral("action") ? 1 : 0);
    if (kind == QStringLiteral("invalid")) QCOMPARE(returnPressed.count(), 0);
    if (combo) QVERIFY(combo->findText(QStringLiteral("Added")) >= 0);
    QVERIFY(dialog.isVisible());
    QCOMPARE(canceled.count(), 0);
    // A focused button retains its normal Enter action.
    pressEnter(cancel, key);
    QCOMPARE(canceled.count(), 1);
  }
};
}  // namespace

QTEST_MAIN(PickerKeyboardTests)
#include "PickerKeyboardTests.moc"
