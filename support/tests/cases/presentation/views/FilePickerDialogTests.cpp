// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDialogButtonBox>
#include <QFile>
#include <QFocusFrame>
#include <QHeaderView>
#include <QImage>
#include <QInputMethodEvent>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QRegion>
#include <QScopeGuard>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTableView>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QThreadPool>
#include <QTimer>
#include <array>
#include <atomic>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "infrastructure/windows/filesystem/browsing/LocalFileBrowser.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/files/FilePickerViewModel.h"
#include "presentation/views/files/FilePickerDialog.h"
#include "support/app/WorkerGate.h"
#include "support/ui/DialogControls.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/WidgetCapture.h"
#include "support/windows/LiteralFileBrowserFixture.h"

using namespace uwf;
using Model = presentation::FilePickerViewModel;
enum class RenameResult { Renamed, Unchanged, Collision };
Q_DECLARE_METATYPE(RenameResult)

namespace {
void createFile(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) qFatal("Unable to create test file");
}
int rowNamed(QTableView* table, const QString& name) {
  for (int row = 0; row < table->model()->rowCount(); ++row)
    if (table->model()->index(row, 0).data().toString() == name) return row;
  return -1;
}
void clickRow(QTableView* table, const int row, const Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
  QTest::mouseClick(table->viewport(), Qt::LeftButton, modifiers, table->visualRect(table->model()->index(row, 0)).center());
}
}  // namespace
class FilePickerDialogTests final : public QObject {
  Q_OBJECT
 private slots:
  void pastingIntoPickerFieldsDoesNotEnableTheNavigationFocusRing_data() {
    QTest::addColumn<bool>("dark");
    QTest::addColumn<QString>("fieldName");
    for (const bool dark : {false, true})
      for (const auto& field : {QStringLiteral("pickerAddress"), QStringLiteral("pickerSearch"), QStringLiteral("pickerFilename")})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(dark ? "dark" : "light", field))) << dark << field;
  }
  void pastingIntoPickerFieldsDoesNotEnableTheNavigationFocusRing() {
    QFETCH(bool, dark);
    QFETCH(QString, fieldName);
    const auto previousTheme = ui::ThemeManager::instance().current();
    const auto previousClipboard = QApplication::clipboard()->text();
    const auto restore = qScopeGuard([&] {
      ui::ThemeManager::instance().apply(previousTheme);
      QApplication::clipboard()->setText(previousClipboard);
    });
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    class Browser final : public app::FileBrowser {
     public:
      QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override { return {}; }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString&) const override {}
    };
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, std::make_shared<Browser>());
    ui::FilePickerDialog dialog(model, QStringLiteral("Open file"));
    model->start(QStringLiteral("C:/Example"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    dialog.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&dialog));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Empty);
    auto* field = dialog.findChild<QLineEdit*>(fieldName);
    QVERIFY(field && !field->isReadOnly());
    QTest::mouseClick(field, Qt::LeftButton);
    QTRY_VERIFY(field->hasFocus());
    field->selectAll();
    const QString pasted = QStringLiteral("pasted-name.txt");
    QVERIFY(test::prepareClipboardText(pasted));
    QTest::keyClick(dialog.windowHandle(), Qt::Key_V, Qt::ControlModifier);
    QCOMPARE(field->text(), pasted);
    for (auto frames = dialog.findChildren<QFocusFrame*>(); auto* frame : std::as_const(frames)) QVERIFY(!frame->isVisible() || frame->widget() != field);
    QVERIFY(test::saveScreenshot(dialog, QStringLiteral("picker-pasted-field")));
  }
  void fileTypePopupContainsOnlyRealSelectableFilters_data() {
    QTest::addColumn<QString>("filters");
    QTest::addColumn<int>("expectedCount");
    QTest::addColumn<bool>("dark");
    for (const bool dark : {false, true}) {
      const auto theme = dark ? QStringLiteral("dark") : QStringLiteral("light");
      QTest::newRow(qUtf8Printable(theme + QStringLiteral("-default"))) << QString{} << 1 << dark;
      QTest::newRow(qUtf8Printable(theme + QStringLiteral("-single"))) << QStringLiteral("Text files (*.txt)") << 1 << dark;
      QTest::newRow(qUtf8Printable(theme + QStringLiteral("-multiple"))) << QStringLiteral("Text files (*.txt);;All files (*)") << 2 << dark;
    }
  }
  void fileTypePopupContainsOnlyRealSelectableFilters() {
    QFETCH(QString, filters);
    QFETCH(int, expectedCount);
    QFETCH(bool, dark);
    auto& theme = ui::ThemeManager::instance();
    const auto previousTheme = theme.current();
    const auto restore = qScopeGuard([&] { theme.apply(previousTheme); });
    theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
    class Browser final : public app::FileBrowser {
     public:
      QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override { return {}; }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString&) const override {}
    };
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, std::make_shared<Browser>());
    model->setFilters(filters);
    ui::FilePickerDialog dialog(model, QStringLiteral("Open file"));
    model->start(QStringLiteral("C:/Example"));
    dialog.resize(720, 550);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_VERIFY(!model->busy());
    auto* combo = dialog.findChild<QComboBox*>(QStringLiteral("pickerFileType"));
    QVERIFY(combo);
    QCOMPARE(combo->count(), expectedCount);
    const auto closePopup = qScopeGuard([combo] { combo->hidePopup(); });
    QSignalSpy activated(combo, &QComboBox::activated);
    QTest::mouseClick(combo, Qt::LeftButton);
    auto* popup = QApplication::activePopupWidget();
    QVERIFY(popup);
    auto* options = popup->findChild<QListView*>(QStringLiteral("selectOptions"));
    QVERIFY(options);
    QTRY_COMPARE(options->model()->rowCount(), expectedCount);
    for (int row = 0; row < expectedCount; ++row) {
      const auto index = options->model()->index(row, 0);
      QVERIFY(!index.data().toString().isEmpty());
      QCOMPARE(index.data().toString(), combo->itemText(row));
      QVERIFY(index.flags().testFlag(Qt::ItemIsEnabled) && index.flags().testFlag(Qt::ItemIsSelectable));
      QTRY_VERIFY(options->viewport()->rect().contains(options->visualRect(index)));
      QCOMPARE(options->indexAt(options->visualRect(index).center()), index);
    }
    const auto last = options->model()->index(expectedCount - 1, 0);
    QTRY_COMPARE(options->viewport()->height(), options->visualRect(last).bottom() + 1);
    QCOMPARE(options->indexAt(QPoint(options->viewport()->width() / 2, options->viewport()->height() - 1)), last);
    QTest::mouseClick(options->viewport(), Qt::LeftButton, Qt::NoModifier, options->visualRect(last).center());
    QTRY_VERIFY(!popup->isVisible());
    QCOMPARE(combo->currentIndex(), expectedCount - 1);
    QCOMPARE(model->filterIndex(), expectedCount - 1);
    QCOMPARE(activated.count(), 1);
  }

  void failureDetailsWrapAndCopyTheirCompleteText_data() {
    QTest::addColumn<bool>("dark");
    QTest::addColumn<int>("language");
    QTest::addColumn<bool>("longError");
    for (const bool dark : {false, true})
      for (const auto language : {ui::I18n::Lang::En, ui::I18n::Lang::Zh_CN, ui::I18n::Lang::Ar, ui::I18n::Lang::De})
        for (const bool longError : {false, true})
          QTest::newRow(
              qUtf8Printable(QStringLiteral("%1-%2-%3").arg(dark ? "dark" : "light").arg(static_cast<int>(language)).arg(longError ? "long" : "short")))
              << dark << static_cast<int>(language) << longError;
  }
  void failureDetailsWrapAndCopyTheirCompleteText() {
    QFETCH(bool, dark);
    QFETCH(int, language);
    QFETCH(bool, longError);
    auto& theme = ui::ThemeManager::instance();
    const auto originalTheme = theme.current();
    const auto originalLanguage = ui::I18n::instance().lang();
    const auto originalClipboard = QApplication::clipboard()->text();
    const auto restore = qScopeGuard([&] {
      theme.apply(originalTheme);
      ui::I18n::instance().setLang(originalLanguage);
      QApplication::clipboard()->setText(originalClipboard);
    });
    theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    class Browser final : public app::FileBrowser {
     public:
      explicit Browser(QString error) : m_error(error.toStdString()) {}
      QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override { throw std::runtime_error(m_error); }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString&) const override {}

     private:
      std::string m_error;
    };
    const auto original =
        longError ? QStringLiteral("Access denied: C:/Enterprise deployment/Managed application data/<literal & complete>/Protected directory\n") +
                        QStringLiteral("原始错误详情必须完整换行。 The full diagnostic remains readable and selectable after resizing.\n").repeated(6)
                  : QStringLiteral("Access denied · 0x80070005");
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, std::make_shared<Browser>(original));
    model->setFilters(QStringLiteral("Text files (*.txt);;All files (*)"));
    ui::FilePickerDialog dialog(model, QStringLiteral("Select file"));
    model->start(QStringLiteral("C:/Enterprise deployment"));
    dialog.resize(720, 550);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Failed);
    QCOMPARE(model->error(), original);
    auto* placeholder = dialog.findChild<QWidget*>(QStringLiteral("filePickerPlaceholder"));
    auto* title = dialog.findChild<ui::TextLabel*>(QStringLiteral("filePickerStateTitle"));
    auto* description = dialog.findChild<ui::TextLabel*>(QStringLiteral("filePickerStateDescription"));
    auto* retry = dialog.findChild<QPushButton*>(QStringLiteral("filePickerRetry"));
    auto* detailsButton = dialog.findChild<QPushButton*>(QStringLiteral("filePickerErrorDetails"));
    QVERIFY(placeholder && title && description && retry && detailsButton);
    QVERIFY(placeholder->isVisible() && retry->isVisible() && detailsButton->isVisible());
    QVERIFY(dialog.findChildren<QScrollArea*>().isEmpty());
    QVERIFY(placeholder->findChildren<QAbstractScrollArea*>().isEmpty());
    const QStringList fixedControls{QStringLiteral("pickerAddress"),  QStringLiteral("pickerSearch"), QStringLiteral("pickerFilename"),
                                    QStringLiteral("pickerFileType"), QStringLiteral("pickerAccept"), QStringLiteral("dialogFooter")};
    for (const int width : {720, 1200, 720}) {
      dialog.resize(width, 550);
      QTRY_COMPARE(dialog.size(), QSize(width, 550));
      QVector<QRect> positions;
      for (const auto& name : fixedControls) {
        auto* field = dialog.findChild<QWidget*>(name);
        QVERIFY(field && field->isVisible());
        const QRect bounds(field->mapTo(&dialog, QPoint()), field->size());
        QVERIFY(dialog.rect().contains(bounds));
        positions.append(bounds);
      }
      for (auto* text : {title, description}) {
        QTRY_VERIFY(text->height() >= text->heightForWidth(text->width()));
        const QRect bounds(text->mapTo(placeholder, QPoint()), text->size());
        QVERIFY(placeholder->rect().contains(bounds));
        QCOMPARE(placeholder->visibleRegion().intersected(QRegion(bounds)), QRegion(bounds));
      }
      for (auto* button : {retry, detailsButton}) {
        const QRect bounds(button->mapTo(placeholder, QPoint()), button->size());
        QCOMPARE(placeholder->visibleRegion().intersected(QRegion(bounds)), QRegion(bounds));
      }
      bool inspected = false;
      QVERIFY(test::driveModalDialog(
          dialog, [&] { detailsButton->click(); },
          [&](QDialog& active) {
            auto* popup = &active;
            QVERIFY(popup != &dialog);
            QVERIFY(QTest::qWaitForWindowExposed(popup));
            QCOMPARE(popup->windowTitle(), ui::I18n::tr("Error details"));
            auto* text = popup->findChild<ui::ContentHintBox*>();
            QVERIFY(text);
            QCOMPARE(text->toPlainText(), original);
            QTRY_VERIFY(text->height() >= text->heightForWidth(text->width()));
            QCOMPARE(text->verticalScrollBar()->maximum(), 0);
            QCOMPARE(text->horizontalScrollBar()->maximum(), 0);
            text->setFocus();
            text->selectAll();
            QCOMPARE(QString(text->textCursor().selectedText()).replace(QChar::ParagraphSeparator, u'\n'), original);
            QVERIFY(test::copySelectionToClipboard(*text, original));
            QCOMPARE(QApplication::clipboard()->text(), original);
            inspected = true;
          }));
      QVERIFY(inspected);
      for (qsizetype index = 0; index < fixedControls.size(); ++index) {
        auto* field = dialog.findChild<QWidget*>(fixedControls[index]);
        QCOMPARE(QRect(field->mapTo(&dialog, QPoint()), field->size()), positions[index]);
      }
      QVERIFY(dialog.findChildren<QScrollArea*>().isEmpty());
    }
  }

  void manyPlacesUseTheViewModelAndNavigateWithoutRowWidgets_data() {
    QTest::addColumn<bool>("dark");
    QTest::addColumn<int>("language");
    for (const bool dark : {false, true})
      for (const auto language : {ui::I18n::Lang::En, ui::I18n::Lang::Zh_CN, ui::I18n::Lang::Ar, ui::I18n::Lang::De})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(dark ? "dark" : "light").arg(static_cast<int>(language))))
            << dark << static_cast<int>(language);
  }
  void manyPlacesUseTheViewModelAndNavigateWithoutRowWidgets() {
    QFETCH(bool, dark);
    QFETCH(int, language);
    const auto previousTheme = ui::ThemeManager::instance().current();
    const auto previousLanguage = ui::I18n::instance().lang();
    const auto restore = qScopeGuard([&] {
      ui::ThemeManager::instance().apply(previousTheme);
      ui::I18n::instance().setLang(previousLanguage);
    });
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    class Browser final : public app::FileBrowser {
     public:
      QVector<app::FileBrowserEntry> list(const QString& path, std::stop_token) const override {
        QVector<app::FileBrowserEntry> result;
        for (int row = 0; row < 1000; ++row) {
          const auto name = QStringLiteral("file-%1.txt").arg(row);
          result.append({QDir(path).filePath(name), name, false, 1024});
        }
        return result;
      }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override {
        QVector<app::FileBrowserPlace> result;
        for (int row = 0; row < 1000; ++row) result.append({QStringLiteral("Place %1").arg(row), QStringLiteral("C:/Example/place-%1").arg(row), false, false});
        return result;
      }
      void createDirectory(const QString&) const override {}
    };
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, std::make_shared<Browser>());
    model->setFilters(QStringLiteral("Text files (*.txt);;All files (*)"));
    ui::FilePickerDialog dialog(model, QStringLiteral("Places"));
    model->start(QStringLiteral("C:/Example"));
    dialog.resize(720, 550);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Ready);
    auto* view = dialog.findChild<QListView*>(QStringLiteral("filePickerPlaces"));
    QVERIFY(view);
    QCOMPARE(view->model(), model->placesModel());
    QCOMPARE(view->model()->rowCount(), 1001);
    QVERIFY(view->findChildren<QAbstractButton*>().isEmpty());
    auto* entries = dialog.findChild<QTableView*>(QStringLiteral("pickerEntries"));
    QVERIFY(entries);
    QVERIFY(dialog.findChildren<QScrollArea*>().isEmpty());
    for (const int width : {720, 1200, 720}) {
      dialog.resize(width, 550);
      QTRY_COMPARE(dialog.size(), QSize(width, 550));
      QTRY_VERIFY(view->verticalScrollBar()->maximum() > 0);
      QTRY_VERIFY(entries->verticalScrollBar()->maximum() > 0);
      const QStringList names{QStringLiteral("pickerAddress"),  QStringLiteral("pickerSearch"), QStringLiteral("pickerFilename"),
                              QStringLiteral("pickerFileType"), QStringLiteral("pickerAccept"), QStringLiteral("dialogFooter")};
      QVector<QRect> positions;
      for (const auto& name : names) {
        auto* control = dialog.findChild<QWidget*>(name);
        QVERIFY(control && control->isVisible());
        const QRect bounds(control->mapTo(&dialog, QPoint()), control->size());
        QVERIFY(dialog.rect().contains(bounds));
        positions.append(bounds);
      }
      const int entriesBefore = entries->verticalScrollBar()->value();
      view->verticalScrollBar()->setValue(view->verticalScrollBar()->maximum());
      QCOMPARE(entries->verticalScrollBar()->value(), entriesBefore);
      const int placesBefore = view->verticalScrollBar()->value();
      entries->verticalScrollBar()->setValue(entries->verticalScrollBar()->maximum());
      QCOMPARE(view->verticalScrollBar()->value(), placesBefore);
      for (qsizetype index = 0; index < names.size(); ++index) {
        auto* control = dialog.findChild<QWidget*>(names[index]);
        QCOMPARE(QRect(control->mapTo(&dialog, QPoint()), control->size()), positions[index]);
      }
    }
    const auto target = view->model()->index(501, 0);
    view->scrollTo(target, QAbstractItemView::PositionAtCenter);
    QTRY_VERIFY(view->visualRect(target).intersects(view->viewport()->rect()));
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, view->visualRect(target).center());
    QTRY_COMPARE(model->path(), QStringLiteral("C:/Example/place-500"));
    QCOMPARE(target.data(Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Checked));
  }

  void multiSelectionTogglesAndSurvivesSorting() {
    ui::ThemeManager::instance().apply(app::Theme::Light);
    QTemporaryDir temporary;
    QDir directory(temporary.path());
    QVERIFY(directory.mkdir(QStringLiteral("folder")));
    createFile(directory.filePath(QStringLiteral("first.txt")));
    createFile(directory.filePath(QStringLiteral("second.txt")));
    auto model = std::make_shared<Model>(Model::Mode::Mixed, windows::systemFileBrowser());
    ui::FilePickerDialog dialog(model, QStringLiteral("Add files or folders"));
    model->start(directory.path());
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Ready);
    auto* table = dialog.findChild<QTableView*>(QStringLiteral("pickerEntries"));
    QVERIFY(table);
    clickRow(table, rowNamed(table, QStringLiteral("first.txt")));
    QCOMPARE(table->selectionModel()->selectedRows().size(), 1);
    clickRow(table, rowNamed(table, QStringLiteral("folder")));
    QCOMPARE(table->selectionModel()->selectedRows().size(), 2);
    const auto selectedPaths = model->selectedEntries();
    table->sortByColumn(0, Qt::DescendingOrder);
    QTRY_VERIFY(!model->busy());
    QVERIFY(table->isEnabled());
    QCOMPARE(table->model()->index(0, 0).data().toString(), QStringLiteral("folder"));
    QCOMPARE(table->model()->index(1, 0).data().toString(), QStringLiteral("second.txt"));
    QCOMPARE(table->model()->index(2, 0).data().toString(), QStringLiteral("first.txt"));
    QCOMPARE(model->selectedEntries(), selectedPaths);
    QCOMPARE(table->selectionModel()->selectedRows().size(), 2);
    for (auto selectedRows = table->selectionModel()->selectedRows(); const auto& index : std::as_const(selectedRows))
      QVERIFY(selectedPaths.contains(index.data(presentation::PathRole).toString()));
    clickRow(table, rowNamed(table, QStringLiteral("first.txt")));
    QCOMPARE(table->selectionModel()->selectedRows().size(), 1);
    auto* open = dialog.findChild<QPushButton*>(QStringLiteral("pickerAccept"));
    QVERIFY(open && open->isEnabled());
    QTest::mouseClick(open, Qt::LeftButton);
    QTRY_COMPARE(dialog.result(), QDialog::Accepted);
    QCOMPARE(dialog.selectedItems().size(), 1);
    QVERIFY(dialog.selectedItems().constFirst().directory);
    QCOMPARE(dialog.selectedPaths(), QStringList{QDir::toNativeSeparators(directory.filePath(QStringLiteral("folder")))});
  }
  void keyboardNavigationAndTypedNamesReachTheSameResult() {
    QTemporaryDir temporary;
    QDir directory(temporary.path());
    QVERIFY(directory.mkdir(QStringLiteral("folder")));
    createFile(directory.filePath(QStringLiteral("folder/complete.txt")));
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, windows::systemFileBrowser());
    ui::FilePickerDialog dialog(model, QStringLiteral("Open file"));
    model->start(directory.path());
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    dialog.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&dialog));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Ready);
    auto* table = dialog.findChild<QTableView*>(QStringLiteral("pickerEntries"));
    QVERIFY(table);
    clickRow(table, 0);
    table->setFocus();
    QTRY_VERIFY(table->hasFocus());
    QTest::keyClick(table, Qt::Key_Return);
    QTRY_COMPARE(model->path(), directory.filePath(QStringLiteral("folder")));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Ready);
    auto* name = dialog.findChild<QLineEdit*>(QStringLiteral("pickerFilename"));
    QVERIFY(name);
    name->setFocus();
    QTRY_VERIFY(name->hasFocus());
    QTest::keyClicks(name, QStringLiteral("complete.txt"));
    QTest::keyClick(name, Qt::Key_Return);
    QTRY_COMPARE(dialog.result(), QDialog::Accepted);
    QCOMPARE(dialog.selectedPaths(), QStringList{QDir::toNativeSeparators(directory.filePath(QStringLiteral("folder/complete.txt")))});
  }
  void busyOperationsDisableEditingAndKeepCancelAvailable_data() {
    QTest::addColumn<int>("operation");
    QTest::newRow("selection-inspection") << 1;
    QTest::newRow("create-folder") << 2;
    QTest::newRow("rename-entry") << 3;
  }
  void busyOperationsDisableEditingAndKeepCancelAvailable() {
    QFETCH(int, operation);
    class Browser final : public app::FileBrowser {
     public:
      std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
      std::atomic_int blocked{0};
      QVector<app::FileBrowserEntry> list(const QString& path, std::stop_token) const override {
        return {{path + QStringLiteral("/file.txt"), QStringLiteral("file.txt")}};
      }
      app::FileBrowserEntry inspect(const QString& path) const override {
        if (blocked == 1) gate->wait({}, false);
        return {path, path, !path.endsWith(QLatin1String(".txt"))};
      }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString&) const override { gate->wait({}, false); }
      void renameEntry(const QString&, const QString&) const override { gate->wait({}, false); }
    };
    auto browser = std::make_shared<Browser>();
    const auto unblock = qScopeGuard([&] {
      browser->gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, browser);
    ui::FilePickerDialog dialog(model, QStringLiteral("Open file"));
    model->start(QStringLiteral("C:/test"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_VERIFY(!model->busy());
    model->setFilename(QStringLiteral("file.txt"));
    browser->blocked = operation;
    auto* accept = dialog.findChild<QPushButton*>(QStringLiteral("pickerAccept"));
    QVERIFY(accept && accept->isEnabled());
    if (operation == 1) QTest::mouseClick(accept, Qt::LeftButton);
    if (operation == 2) model->createDirectory(QStringLiteral("created"));
    if (operation == 3) model->renameEntry(0, QStringLiteral("renamed.txt"));
    QTRY_COMPARE(browser->gate->started.load(), 1);
    for (const auto& name :
         {QStringLiteral("pickerAddress"), QStringLiteral("pickerSearch"), QStringLiteral("pickerFilename"), QStringLiteral("pickerFileType"),
          QStringLiteral("pickerEntries"), QStringLiteral("filePickerPlaces"), QStringLiteral("pickerAccept")}) {
      auto* control = dialog.findChild<QWidget*>(name);
      QVERIFY(control && !control->isEnabled());
    }
    auto* buttons = dialog.findChild<QDialogButtonBox*>();
    QVERIFY(buttons);
    QAbstractButton* cancel = nullptr;
    for (auto actions = buttons->buttons(); auto* button : std::as_const(actions))
      if (buttons->buttonRole(button) == QDialogButtonBox::RejectRole) cancel = button;
    QVERIFY(cancel && cancel->isEnabled());
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    QTest::keyClick(&dialog, Qt::Key_Escape);
    QVERIFY(!dialog.isVisible());
    browser->gate->release();
    QVERIFY(QThreadPool::globalInstance()->waitForDone(2000));
    QCoreApplication::sendPostedEvents();
    QCOMPARE(accepted.count(), 0);
    QVERIFY(dialog.selectedPaths().isEmpty());
    QCOMPARE(model->loadState(), presentation::ContentLoadState::Canceled);
  }
  void cancelDuringLoadingCannotAcceptOrReopen() {
    class Browser final : public app::FileBrowser {
     public:
      QVector<app::FileBrowserEntry> list(const QString&, std::stop_token stop) const override {
        while (!stop.stop_requested()) std::this_thread::sleep_for(std::chrono::milliseconds{1});
        return {};
      }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString&) const override {}
    };
    QTemporaryDir temporary;
    auto model = std::make_shared<Model>(Model::Mode::Directory, std::make_shared<Browser>());
    ui::FilePickerDialog dialog(model, QStringLiteral("Select folder"));
    model->start(temporary.path());
    dialog.resize(720, 550);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_VERIFY(model->progressVisible());
    auto* placeholder = dialog.findChild<QWidget*>(QStringLiteral("filePickerPlaceholder"));
    QVERIFY(placeholder && placeholder->isVisible());
    QVERIFY(dialog.findChildren<QScrollArea*>().isEmpty());
    QVERIFY(placeholder->findChildren<QAbstractScrollArea*>().isEmpty());
    for (const auto& name : {QStringLiteral("filePickerStateTitle"), QStringLiteral("filePickerStateDescription")}) {
      auto* text = dialog.findChild<ui::TextLabel*>(name);
      QVERIFY(text);
      QTRY_VERIFY(text->height() >= text->heightForWidth(text->width()));
      const QRegion bounds(QRect(text->mapTo(placeholder, QPoint()), text->size()));
      QCOMPARE(placeholder->visibleRegion().intersected(bounds), bounds);
    }
    auto* open = dialog.findChild<QPushButton*>(QStringLiteral("pickerAccept"));
    QVERIFY(open && !open->isEnabled());
    QTest::keyClick(&dialog, Qt::Key_Escape);
    QCOMPARE(dialog.result(), QDialog::Rejected);
    QTest::qWait(1200);
    QVERIFY(!dialog.isVisible());
    QVERIFY(dialog.selectedPaths().isEmpty());
    QCOMPARE(model->loadState(), presentation::ContentLoadState::Canceled);
  }
  void emptyPlaceholderDoesNotScrollOrMoveThePickerFields() {
    class Browser final : public app::FileBrowser {
     public:
      QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override { return {}; }
      app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
      QVector<app::FileBrowserPlace> places() const override { return {}; }
      void createDirectory(const QString&) const override {}
    };
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, std::make_shared<Browser>());
    ui::FilePickerDialog dialog(model, QStringLiteral("Open file"));
    dialog.resize(720, 550);
    model->start(QStringLiteral("C:/Empty"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Empty);
    auto* placeholder = dialog.findChild<QWidget*>(QStringLiteral("filePickerPlaceholder"));
    QVERIFY(placeholder && placeholder->isVisible());
    QVERIFY(dialog.findChildren<QScrollArea*>().isEmpty());
    QVERIFY(placeholder->findChildren<QAbstractScrollArea*>().isEmpty());
    for (const auto& name : {QStringLiteral("filePickerStateTitle"), QStringLiteral("filePickerStateDescription")}) {
      auto* text = dialog.findChild<ui::TextLabel*>(name);
      QVERIFY(text);
      QTRY_VERIFY(text->height() >= text->heightForWidth(text->width()));
      const QRegion bounds(QRect(text->mapTo(placeholder, QPoint()), text->size()));
      QCOMPARE(placeholder->visibleRegion().intersected(bounds), bounds);
    }
    auto* details = dialog.findChild<QPushButton*>(QStringLiteral("filePickerErrorDetails"));
    QVERIFY(details && !details->isVisible());
    auto* retry = dialog.findChild<QPushButton*>(QStringLiteral("filePickerRetry"));
    QVERIFY(retry && !retry->isVisible());
  }
  void savingAnUneditedExistingFileConfirmsTheLiteralTarget_data() {
    QTest::addColumn<bool>("dark");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("initialFile");
    const std::array names{std::pair{"no-extension", QStringLiteral("Entry")}, std::pair{"trailing-space", QStringLiteral("Entry ")},
                           std::pair{"trailing-dot", QStringLiteral("Entry.")}};
    for (const bool dark : {false, true})
      for (const auto& [label, name] : names)
        for (const bool initialFile : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("%1/%2/%3")
                                       .arg(dark ? QStringLiteral("dark") : QStringLiteral("light"), QString::fromLatin1(label),
                                            initialFile ? QStringLiteral("prefill") : QStringLiteral("selection"))))
              << dark << name << initialFile;
  }
  void savingAnUneditedExistingFileConfirmsTheLiteralTarget() {
    QFETCH(bool, dark);
    QFETCH(QString, name);
    QFETCH(bool, initialFile);
    if (QGuiApplication::platformName() == QLatin1String("offscreen")) QSKIP("Native save and replace dialog focus requires Windows.");
    const auto previousTheme = ui::ThemeManager::instance().current();
    const auto restoreTheme = qScopeGuard([&] { ui::ThemeManager::instance().apply(previousTheme); });
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    for (const auto& sibling : {QStringLiteral("Entry"), QStringLiteral("Entry "), QStringLiteral("Entry."), QStringLiteral("Entry.txt")})
      QVERIFY2(fixture.seed(sibling, sibling.toUtf8()), qPrintable(fixture.error));
    auto model = std::make_shared<Model>(Model::Mode::SaveFile, fixture.browser);
    model->setFilters(QStringLiteral("All files (*);;Text (*.txt)"));
    if (initialFile) model->setFilterIndex(1);
    ui::FilePickerDialog dialog(model, QStringLiteral("Save file"));
    model->start(initialFile ? fixture.path(name) : fixture.root.path());
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    dialog.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&dialog, 1000));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Ready);
    auto* table = dialog.findChild<QTableView*>(QStringLiteral("pickerEntries"));
    auto* filename = dialog.findChild<QLineEdit*>(QStringLiteral("pickerFilename"));
    auto* type = dialog.findChild<QComboBox*>(QStringLiteral("pickerFileType"));
    auto* save = dialog.findChild<QPushButton*>(QStringLiteral("pickerAccept"));
    QVERIFY(table && filename && type && save);
    if (!initialFile) {
      const auto row = rowNamed(table, name);
      QVERIFY(row >= 0);
      clickRow(table, row);
      type->setCurrentIndex(1);
    }
    QTRY_VERIFY(!model->busy());
    QVERIFY(table->isEnabled());
    QCOMPARE(model->filterIndex(), 1);
    QCOMPARE(table->model()->rowCount(), 1);
    QCOMPARE(table->model()->index(0, 0).data().toString(), QStringLiteral("Entry.txt"));
    QCOMPARE(filename->text(), name);
    QCOMPARE(model->selectedEntries(), QStringList{fixture.path(name)});
    QVERIFY(save->isEnabled());
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    QSignalSpy errors(model.get(), &Model::validationFailed);
    int confirmations = 0;
    bool confirmReplacement = false;
    bool unexpectedDialog = false;
    QTimer driver;
    driver.setInterval(20);
    connect(&driver, &QTimer::timeout, this, [&] {
      auto* popup = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!popup || popup == &dialog) return;
      driver.stop();
      const auto close = qScopeGuard([popup] {
        if (popup->isVisible()) popup->reject();
      });
      if (popup->windowTitle() != ui::I18n::tr("Replace the existing file?")) {
        unexpectedDialog = true;
        popup->reject();
        return;
      }
      QVERIFY(QTest::qWaitForWindowExposed(popup));
      auto* body = popup->findChild<ui::ContentHintBox*>();
      auto* action = test::buttonByRole(popup, confirmReplacement ? QDialogButtonBox::AcceptRole : QDialogButtonBox::RejectRole);
      QVERIFY(body && action && action->isVisible() && action->isEnabled());
      QVERIFY(body->toPlainText().contains(QDir::toNativeSeparators(fixture.path(name))));
      QCOMPARE(model->selectedPaths(), QStringList{QDir::toNativeSeparators(fixture.path(name))});
      ++confirmations;
      QTest::mouseClick(action, Qt::LeftButton);
      driver.start();
    });
    driver.start();
    QTimer::singleShot(6000, &dialog, [&] {
      if (auto* popup = qobject_cast<QDialog*>(QApplication::activeModalWidget()); popup && popup != &dialog) popup->reject();
    });
    QTest::mouseClick(save, Qt::LeftButton);
    QTRY_COMPARE(confirmations, 1);
    QTRY_VERIFY(!model->busy());
    QVERIFY(!unexpectedDialog);
    QVERIFY(dialog.isVisible());
    QCOMPARE(accepted.count(), 0);
    QCOMPARE(filename->text(), name);
    for (const auto& sibling : {QStringLiteral("Entry"), QStringLiteral("Entry "), QStringLiteral("Entry."), QStringLiteral("Entry.txt")})
      QCOMPARE(fixture.read(sibling), std::optional<QByteArray>{sibling.toUtf8()});
    confirmReplacement = true;
    QTest::mouseClick(save, Qt::LeftButton);
    QTRY_COMPARE(accepted.count(), 1);
    QCOMPARE(confirmations, 2);
    driver.stop();
    QVERIFY(!unexpectedDialog);
    QVERIFY(!dialog.isVisible());
    QCOMPARE(dialog.result(), QDialog::Accepted);
    QCOMPARE(dialog.selectedPaths(), QStringList{QDir::toNativeSeparators(fixture.path(name))});
    const auto selectedItems = dialog.selectedItems();
    QCOMPARE(selectedItems.size(), 1);
    QCOMPARE(selectedItems.constFirst().path, QDir::toNativeSeparators(fixture.path(name)));
    QCOMPARE(errors.count(), 0);
    for (const auto& sibling : {QStringLiteral("Entry"), QStringLiteral("Entry "), QStringLiteral("Entry."), QStringLiteral("Entry.txt")})
      QCOMPARE(fixture.read(sibling), std::optional<QByteArray>{sibling.toUtf8()});
    QVERIFY(!fixture.exists(QStringLiteral("Entry..txt")));
    QVERIFY(!fixture.exists(QStringLiteral("Entry .txt")));
    QCOMPARE(fixture.browser->creates.load(), 0);
    QCOMPARE(fixture.browser->renames.load(), 0);
  }
  void editedFilenameReplacesOneSelectionIntent_data() {
    QTest::addColumn<bool>("saving");
    QTest::addColumn<bool>("selected");
    QTest::addColumn<bool>("filterPending");
    for (const bool saving : {false, true})
      for (const bool selected : {false, true})
        for (const bool filterPending : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("save-%1-selected-%2-filter-pending-%3").arg(saving).arg(selected).arg(filterPending)))
              << saving << selected << filterPending;
  }
  void editedFilenameReplacesOneSelectionIntent() {
    QFETCH(bool, saving);
    QFETCH(bool, selected);
    QFETCH(bool, filterPending);
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    const auto originalName = QStringLiteral("Entry ");
    const auto editedName = QStringLiteral("Fresh.txt");
    QVERIFY2(fixture.seed(originalName, "original literal target"), qPrintable(fixture.error));
    QVERIFY2(fixture.seed(QStringLiteral("Entry.txt"), "separate sibling"), qPrintable(fixture.error));
    if (!saving) QVERIFY2(fixture.seed(editedName, "typed target"), qPrintable(fixture.error));
    auto model = std::make_shared<Model>(saving ? Model::Mode::SaveFile : Model::Mode::OpenFile, fixture.browser);
    model->setFilters(QStringLiteral("All files (*);;Text (*.txt)"));
    ui::FilePickerDialog dialog(model, QStringLiteral("Edit selection"));
    model->start(fixture.root.path());
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_VERIFY(!model->busy());
    auto* table = dialog.findChild<QTableView*>(QStringLiteral("pickerEntries"));
    auto* filename = dialog.findChild<QLineEdit*>(QStringLiteral("pickerFilename"));
    auto* type = dialog.findChild<QComboBox*>(QStringLiteral("pickerFileType"));
    auto* accept = dialog.findChild<QPushButton*>(QStringLiteral("pickerAccept"));
    QVERIFY(table && filename && type && accept);
    QVERIFY(table->isEnabled());
    if (selected) {
      const auto row = rowNamed(table, originalName);
      QVERIFY(row >= 0);
      clickRow(table, row);
      QCOMPARE(model->selectedEntries(), QStringList{fixture.path(originalName)});
      QCOMPARE(filename->text(), originalName);
      QCOMPARE(table->selectionModel()->selectedRows().size(), 1);
    } else
      QVERIFY(model->selectedEntries().isEmpty());
    QSignalSpy selectionChanges(model.get(), &Model::selectionChanged);
    if (filterPending) {
      type->setCurrentIndex(1);
      QVERIFY(model->busy());
      QVERIFY(!table->isEnabled());
      QVERIFY(!accept->isEnabled());
    }
    QVERIFY(filename->isEnabled());
    filename->selectAll();
    // An IME commit is a real widget edit delivered synchronously, so the
    // pending rows cannot publish before the complete new text is submitted.
    QInputMethodEvent input;
    input.setCommitString(editedName);
    QVERIFY(QApplication::sendEvent(filename, &input));
    QCOMPARE(selectionChanges.count(), 1);
    QCOMPARE(filename->text(), editedName);
    QCOMPARE(model->filename(), editedName);
    QVERIFY(model->selectedEntries().isEmpty());
    QCOMPARE(table->selectionModel()->selectedRows().size(), 0);
    if (filterPending) QVERIFY(model->busy());
    QTRY_VERIFY(!model->busy());
    QVERIFY(table->isEnabled());
    QVERIFY(filename->isEnabled());
    QVERIFY(type->isEnabled());
    QVERIFY(accept->isEnabled());
    QCOMPARE(selectionChanges.count(), 1);
    QCOMPARE(filename->text(), editedName);
    QCOMPARE(model->filename(), editedName);
    QVERIFY(model->selectedEntries().isEmpty());
    QCOMPARE(table->selectionModel()->selectedRows().size(), 0);
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    QTest::mouseClick(accept, Qt::LeftButton);
    QTRY_COMPARE(accepted.count(), 1);
    QCOMPARE(dialog.selectedPaths(), QStringList{QDir::toNativeSeparators(fixture.path(editedName))});
    QCOMPARE(dialog.selectedItems().size(), 1);
    QCOMPARE(dialog.selectedItems().constFirst().path, QDir::toNativeSeparators(fixture.path(editedName)));
    QCOMPARE(fixture.read(originalName), std::optional<QByteArray>{"original literal target"});
    QCOMPARE(fixture.read(QStringLiteral("Entry.txt")), std::optional<QByteArray>{"separate sibling"});
    QCOMPARE(fixture.browser->creates.load(), 0);
    QCOMPARE(fixture.browser->renames.load(), 0);
    if (saving)
      QVERIFY(!fixture.exists(editedName));
    else
      QCOMPARE(fixture.read(editedName), std::optional<QByteArray>{"typed target"});
  }
  void renameKeepsTheFilenameAndActionsVisibleWithoutScrollbars_data() {
    QTest::addColumn<bool>("dark");
    QTest::addColumn<QString>("originalName");
    QTest::addColumn<QString>("requestedName");
    QTest::addColumn<bool>("sibling");
    QTest::addColumn<RenameResult>("result");
    const std::array names{std::pair{"trailing-space", QStringLiteral("Entry ")}, std::pair{"leading-space", QStringLiteral(" Entry")},
                           std::pair{"trailing-dot", QStringLiteral("Entry.")}};
    for (const bool dark : {false, true}) {
      const auto add = [dark](const QString& label, const QString& original, const QString& requested, const bool sibling, const RenameResult result) {
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(dark ? QStringLiteral("dark") : QStringLiteral("light"), label)))
            << dark << original << requested << sibling << result;
      };
      add(QStringLiteral("ordinary-rename"), QStringLiteral("before.txt"), QStringLiteral("after.txt"), false, RenameResult::Renamed);
      add(QStringLiteral("raw-leading-new-name"), QStringLiteral("before.txt"), QStringLiteral(" after.txt"), false, RenameResult::Renamed);
      for (const auto& [label, name] : names) {
        for (const bool sibling : {false, true})
          add(QStringLiteral("%1/unchanged-sibling-%2").arg(QString::fromLatin1(label)).arg(sibling), name, name, sibling, RenameResult::Unchanged);
        add(QStringLiteral("%1/explicit-collision").arg(QString::fromLatin1(label)), name, QStringLiteral("Entry"), true, RenameResult::Collision);
      }
    }
  }
  void renameKeepsTheFilenameAndActionsVisibleWithoutScrollbars() {
    QFETCH(bool, dark);
    QFETCH(QString, originalName);
    QFETCH(QString, requestedName);
    QFETCH(bool, sibling);
    QFETCH(RenameResult, result);
    if (QGuiApplication::platformName() == QLatin1String("offscreen"))
      QSKIP("The offscreen platform activates the non-focusable shadow window and clears QLineEdit selection; initial native focus requires Windows.");
    const auto previousTheme = ui::ThemeManager::instance().current();
    const auto restoreTheme = qScopeGuard([&] { ui::ThemeManager::instance().apply(previousTheme); });
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    test::LiteralFileBrowserFixture fixture;
    QVERIFY(fixture.root.isValid());
    QVERIFY2(fixture.seed(originalName, "selected contents"), qPrintable(fixture.error));
    if (sibling) QVERIFY2(fixture.seed(QStringLiteral("Entry"), "keep sibling"), qPrintable(fixture.error));
    auto model = std::make_shared<Model>(Model::Mode::OpenFile, fixture.browser);
    ui::FilePickerDialog dialog(model, QStringLiteral("Open file"));
    model->start(fixture.root.path());
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    dialog.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&dialog, 1000));
    QTRY_COMPARE(model->loadState(), presentation::ContentLoadState::Ready);
    auto* table = dialog.findChild<QTableView*>(QStringLiteral("pickerEntries"));
    QVERIFY(table);
    const auto row = rowNamed(table, originalName);
    QVERIFY(row >= 0);
    QSignalSpy errors(model.get(), &Model::validationFailed);
    bool inspected = false, sawFailure = false;
    QTimer driver;
    driver.setInterval(20);
    connect(&driver, &QTimer::timeout, this, [&] {
      auto* popup = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!popup || popup == &dialog) return;
      if (popup->windowTitle() == ui::I18n::tr("Unable to select this item")) {
        sawFailure = true;
        popup->reject();
        return;
      }
      if (popup->windowTitle() != ui::I18n::tr("Rename") || inspected) return;
      driver.stop();
      const auto close = qScopeGuard([popup] {
        if (popup->isVisible()) popup->reject();
      });
      QVERIFY(QTest::qWaitForWindowExposed(popup));
      popup->activateWindow();
      QVERIFY(QTest::qWaitForWindowActive(popup, 1000));
      auto* name = popup->findChild<QLineEdit*>();
      auto* accept = test::buttonByRole(popup, QDialogButtonBox::AcceptRole);
      QVERIFY(name && name->isVisible() && accept && accept->isVisible() && accept->isEnabled());
      name->setFocus(Qt::OtherFocusReason);
      QTRY_VERIFY_WITH_TIMEOUT(name->hasFocus(), 1000);
      QTRY_COMPARE_WITH_TIMEOUT(name->selectedText(), originalName, 1000);
      QCOMPARE(name->text(), originalName);
      for (auto bars = popup->findChildren<QScrollBar*>(); auto* bar : std::as_const(bars)) QVERIFY(!bar->isVisibleTo(popup));
      QVERIFY(test::saveScreenshot(*popup, dark ? QStringLiteral("file-picker-rename-dark") : QStringLiteral("file-picker-rename-light")));
      inspected = true;
      // An unchanged confirmation must exercise the actual prefill without
      // replacing it through a second test-side normalizer or input operation.
      if (requestedName != originalName) QTest::keyClicks(name, requestedName);
      QTest::mouseClick(accept, Qt::LeftButton);
      driver.start();
    });
    driver.start();
    QTimer::singleShot(6000, &dialog, [&] {
      if (auto* modal = qobject_cast<QDialog*>(QApplication::activeModalWidget()); modal && modal != &dialog) modal->reject();
      if (auto* popup = QApplication::activePopupWidget()) popup->close();
    });
    bool opened = false;
    QTimer::singleShot(0, &dialog, [&] {
      auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      QVERIFY(menu);
      const auto close = qScopeGuard([menu] { menu->close(); });
      for (auto actions = menu->actions(); auto* action : std::as_const(actions)) {
        if (action->isSeparator()) continue;
        QVERIFY(!action->icon().isNull());
      }
      for (auto actions = menu->actions(); auto* action : std::as_const(actions))
        if (action->text() == ui::I18n::tr("Rename…")) {
          opened = true;
          menu->close();
          action->trigger();
          break;
        }
    });
    const auto point = table->visualRect(table->model()->index(row, 0)).center();
    QContextMenuEvent context(QContextMenuEvent::Mouse, point, table->viewport()->mapToGlobal(point));
    QApplication::sendEvent(table->viewport(), &context);
    QVERIFY(opened && inspected);
    QTRY_VERIFY(!model->busy());
    if (result == RenameResult::Collision) {
      QTRY_VERIFY(sawFailure);
      QCOMPARE(errors.count(), 1);
    } else {
      QCOMPARE(errors.count(), 0);
      QVERIFY(!sawFailure);
    }
    driver.stop();
    QCOMPARE(fixture.browser->creates.load(), 0);
    QCOMPARE(fixture.browser->renames.load(), result == RenameResult::Unchanged ? 0 : 1);
    if (result == RenameResult::Renamed) {
      QTRY_VERIFY(rowNamed(table, requestedName) >= 0);
      QVERIFY(!fixture.exists(originalName));
      QCOMPARE(fixture.read(requestedName), std::optional<QByteArray>{"selected contents"});
      if (requestedName != requestedName.trimmed()) QVERIFY(!fixture.exists(requestedName.trimmed()));
    } else {
      QCOMPARE(fixture.read(originalName), std::optional<QByteArray>{"selected contents"});
      QCOMPARE(fixture.read(QStringLiteral("Entry")), sibling ? std::optional<QByteArray>{"keep sibling"} : std::nullopt);
      QVERIFY(rowNamed(table, originalName) >= 0);
    }
  }
  void productionProviderUsesTheCustomDialog() {
    QTemporaryDir temporary;
    const auto path = QDir(temporary.path()).filePath(QStringLiteral("chosen.txt"));
    createFile(path);
    bool observed = false;
    QTimer driver;
    driver.setInterval(20);
    connect(&driver, &QTimer::timeout, this, [&] {
      auto* dialog = qobject_cast<ui::FilePickerDialog*>(QApplication::activeModalWidget());
      if (!dialog) return;
      auto* table = dialog->findChild<QTableView*>(QStringLiteral("pickerEntries"));
      if (!table || !table->isVisible() || table->model()->rowCount() != 1) return;
      observed = true;
      clickRow(table, 0);
      QTest::mouseClick(dialog->findChild<QPushButton*>(QStringLiteral("pickerAccept")), Qt::LeftButton);
    });
    driver.start();
    QTimer::singleShot(6000, &driver, [] {
      if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) dialog->reject();
    });
    const auto selected = uwf::test::localFileDialogs().openFile(nullptr, {QStringLiteral("Open file"), temporary.path(), QStringLiteral("Text (*.txt)")});
    driver.stop();
    QVERIFY(observed);
    QCOMPARE(selected, QDir::toNativeSeparators(path));
  }
};
QTEST_MAIN(FilePickerDialogTests)
#include "FilePickerDialogTests.moc"
