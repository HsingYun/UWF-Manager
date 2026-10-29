// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <dwmapi.h>
#include <qt_windows.h>

#include <QAbstractItemModelTester>
#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QFontMetrics>
#include <QHeaderView>
#include <QInputMethodEvent>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QStyle>
#include <QTableView>
#include <QTableWidget>
#include <QTemporaryFile>
#include <QTest>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextFragment>
#include <QTextLayout>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWindow>
#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "WidgetUiFixture.h"
#include "app/ports/FileStagingStore.h"
#include "core/UwfModel.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PageProxyModel.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/InkBounds.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/AppearanceTransition.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/appearance/WindowAppearance.h"
#include "ui/configuration/ApplyPlanDialog.h"
#include "ui/configuration/ExclusionListWidget.h"
#include "ui/configuration/GlobalStatusPanel.h"
#include "ui/configuration/ImportDialog.h"
#include "ui/configuration/VolumeStatusPanel.h"
#include "ui/design/DesignSystem.h"
#include "ui/design/TextPainting.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/dialogs/PanelDialog.h"
#include "ui/power/PowerConfirmDialog.h"
#include "ui/shell/HoverHintController.h"
#include "ui/shell/StatusSurface.h"
#include "ui/shell/WindowChromeController.h"
#include "ui/shell/WindowTitleBar.h"
#include "ui/widgets/CodeEditor.h"
#include "ui/widgets/CommandBar.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/EntryListView.h"
#include "ui/widgets/FlexibleColumnLayout.h"
#include "ui/widgets/FlowLayout.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/NumberBox.h"
#include "ui/widgets/PagedTableView.h"
#include "ui/widgets/PagedViewController.h"
#include "ui/widgets/PagedWindowLayout.h"
#include "ui/widgets/RecyclingListView.h"
#include "ui/widgets/SummaryLabel.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/StatusBadge.h"
#include "ui/widgets/SwitchButton.h"
#include "ui/widgets/TabView.h"
#include "ui/widgets/TableText.h"
#include "ui/widgets/TextLabel.h"
#include "ui/widgets/TransientLabel.h"
#include "ui/widgets/VerticalScrollArea.h"
#include "windows/filesystem/BoundedTextFileReader.h"
#include "windows/system/SystemHardwareInfo.h"

using namespace uwf::test::widgets;
using uwf::test::FileDialogStub;
namespace {
class WidgetConfigurationTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { initializeWidgets(); }
  void currentSessionShowsAllThreeStateIcons();
  void globalStatusEditsPublishOnlyConstrainedValues();
  void globalStatusTypeChangesAndImportsPreserveValidTargets();
  void globalStatusThresholdEventsCanBeDisabledIndependently();
  void globalStatusKeepsLargeSystemValuesInTheModel();
  void exclusionListDoubleClickCopiesTheDisplayedFullPath();
  void exclusionListButtonsFilterAndPersistenceRowsUseDisplayedSelection();
  void exclusionFileActionsUseThePlatformBoundaryAndPreserveValidation();
  void importDialogFiltersCommentsAndAppendsBatchReports();
  void importFileActionLoadsOnlyCommandsFromEverySelectedFile_data();
  void importFileActionLoadsOnlyCommandsFromEverySelectedFile();
  void recommendedConfigurationSelectionAppendsOnlyChosenGroups();
  void stagingDependencyReviewKeepsTheBuiltinBadgeAndCompleteDescription_data();
  void stagingDependencyReviewKeepsTheBuiltinBadgeAndCompleteDescription();
};

void WidgetConfigurationTests::stagingDependencyReviewKeepsTheBuiltinBadgeAndCompleteDescription_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto& locale : uwf::ui::I18n::languages())
    for (const bool dark : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString(locale.code), dark ? "dark" : "light"))) << locale.id << dark;
}

void WidgetConfigurationTests::stagingDependencyReviewKeepsTheBuiltinBadgeAndCompleteDescription() {
  using namespace uwf;
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  const auto language = ui::I18n::instance().lang();
  const auto theme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(language);
    ui::ThemeManager::instance().apply(theme);
  });
  ui::I18n::instance().setLang(locale);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  class Actions final : public app::ConfigurationActions {
   public:
    int writes = 0;
    ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
      ++writes;
      return [] { return app::ConfigurationApplyResult{}; };
    }
  };
  auto actions = std::make_shared<Actions>();
  app::FileStagingChanges staging;
  staging.after = {{app::FileStagingKind::File, QStringLiteral("C:\\Users\\User\\Documents\\settings.json")},
                   {app::FileStagingKind::Directory, QStringLiteral("D:\\Data\\Project")}};
  auto model = std::make_shared<presentation::ApplyPlanViewModel>(core::PendingChanges{}, core::UwfSnapshot{}, actions, staging);
  FileDialogStub files;
  ui::ApplyPlanDialog dialog(model, files);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* list = dialog.findChild<QListView*>(QStringLiteral("applyChangesList"));
  QVERIFY(list);
  const QString root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
  const QString dependency = ui::I18n::tr("File staging relies on this registry exclusion to preserve its list across restarts.");
  const auto rootText = [&]() -> ui::ContentHintBox* {
    const auto hints = list->findChildren<ui::ContentHintBox*>();
    for (auto* text : hints)
      if (text->isVisibleTo(list) && text->toPlainText() == root) return text;
    return nullptr;
  };
  const auto rowsAreContiguous = [&] {
    for (int row = 1; row < model->rowCount(); ++row) {
      const auto previous = list->visualRect(model->index(row - 1));
      const auto current = list->visualRect(model->index(row));
      if (!previous.isValid() || !current.isValid() || current.top() != previous.bottom() + 1) return false;
    }
    return true;
  };
  // Match the real show-then-resize flow and cross the scrollbar threshold in
  // both directions. A taller row must move every following row with it.
  for (const QSize size : {QSize(720, 600), QSize(980, 760), QSize(720, 760), QSize(720, 600)}) {
    dialog.resize(size);
    QTRY_VERIFY(rootText());
    QTRY_COMPARE(list->verticalScrollBar()->isVisible(), size.height() == 600);
    QTRY_VERIFY_WITH_TIMEOUT(rowsAreContiguous(), 1500);
    auto* row = rootText()->parentWidget();
    ui::StatusBadge* badge = nullptr;
    for (auto* child : row->children())
      if (auto* candidate = dynamic_cast<ui::StatusBadge*>(child)) badge = candidate;
    QVERIFY(badge && badge->isVisibleTo(list));
    QCOMPARE(badge->text(), ui::I18n::tr("Built-in"));
    QTRY_VERIFY(badge->width() >= badge->sizeHint().width());
    ui::TextLabel* description = nullptr;
    const auto labels = row->findChildren<ui::TextLabel*>();
    for (auto* label : labels)
      if (label->text() == dependency) description = label;
    QVERIFY(description && description->isVisibleTo(list));
    QVERIFY(description->wordWrap());
    QTRY_VERIFY(description->height() >= description->heightForWidth(description->width()));
    QTRY_VERIFY(row->rect().contains(QRect(description->mapTo(row, QPoint()), description->size())));
    QTRY_VERIFY(row->rect().contains(QRect(badge->mapTo(row, QPoint()), badge->size())));
    QVERIFY(description->geometry().right() < badge->geometry().left() || badge->geometry().right() < description->geometry().left());
    QVERIFY(rootText()->textInteractionFlags().testFlag(Qt::TextSelectableByMouse));
    QTRY_COMPARE(rootText()->verticalScrollBar()->maximum(), 0);
  }
  QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("staging-dependency-review")));
  QCOMPARE(actions->writes, 0);
}

void WidgetConfigurationTests::currentSessionShowsAllThreeStateIcons() {
  using namespace uwf::ui;
  GlobalStatusPanel panel(std::make_shared<uwf::presentation::GlobalConfigurationViewModel>(8192));
  panel.show();
  QVERIFY(QTest::qWaitForWindowExposed(&panel));
  uwf::core::SessionSnapshot current;
  current.overlay.maximumSizeMb = 4096;
  current.filter.enabled = true;
  panel.viewModel().setData(current, current, {3072, 1024});
  auto* icon = panel.findChild<QWidget*>(QStringLiteral("currentSessionStateIcon"));
  auto* label = panel.findChild<QLabel*>(QStringLiteral("statusCurLabel"));
  QVERIFY(icon && label);
  const auto enabled = icon->grab().toImage();
  QCOMPARE(label->text(), I18n::tr("Enabled"));
  current.filter.enabled = false;
  panel.viewModel().setData(current, current, {3072, 1024});
  const auto disabled = icon->grab().toImage();
  QVERIFY(enabled != disabled);
  QCOMPARE(label->text(), I18n::tr("Disabled"));
  auto* usage = panel.findChild<QLabel*>(QStringLiteral("statusCurrent"));
  QVERIFY(usage);
  QCOMPARE(usage->text(), I18n::tr("Filter is disabled"));
  QVERIFY(panel.usageWarningText().isEmpty());
  panel.viewModel().setAvailable(false);
  QVERIFY(disabled != icon->grab().toImage());
  QCOMPARE(label->text(), I18n::tr("Unavailable"));
  QCOMPARE(usage->text(), I18n::tr("Usage unavailable"));
}


void WidgetConfigurationTests::globalStatusEditsPublishOnlyConstrainedValues() {
  uwf::ui::GlobalStatusPanel panel(std::make_shared<uwf::presentation::GlobalConfigurationViewModel>(uwf::windows::systemTotalRamMb()));
  uwf::core::SessionSnapshot session;
  session.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  panel.viewModel().setData(session, session, {});
  auto* maximum = panel.findChild<QSpinBox*>(QStringLiteral("overlayMaximumNext"));
  auto* warning = panel.findChild<QSpinBox*>(QStringLiteral("overlayWarningNext"));
  auto* critical = panel.findChild<QSpinBox*>(QStringLiteral("overlayCriticalNext"));
  QVERIFY(maximum && warning && critical);
  bool validAtEveryNotification = true;
  connect(&panel.viewModel(), &uwf::presentation::GlobalConfigurationViewModel::pendingChanged, this,
          [&] { validAtEveryNotification = validAtEveryNotification && warning->value() < critical->value(); });
  QSignalSpy changed(&panel.viewModel(), &uwf::presentation::GlobalConfigurationViewModel::pendingChanged);

  maximum->setValue(3072);
  QCOMPARE(critical->value(), 3072);
  QCOMPARE(warning->value(), 2048);
  QCOMPARE(changed.count(), 1);
  maximum->stepDown();
  QCOMPARE(critical->value(), 3072);
  QCOMPARE(changed.count(), 2);

  panel.show();
  QVERIFY(QTest::qWaitForWindowExposed(&panel));
  maximum->setFocus();
  QTest::keyClick(maximum, Qt::Key_A, Qt::ControlModifier);
  QTest::keyClicks(maximum, QStringLiteral("2000"));
  QCOMPARE(maximum->value(), 3071);
  QTest::keyClick(maximum, Qt::Key_Return);
  QCOMPARE(maximum->value(), 2000);
  QCOMPARE(critical->value(), 3072);
  QCOMPARE(warning->value(), 2048);
  QVERIFY(validAtEveryNotification);
}

void WidgetConfigurationTests::globalStatusTypeChangesAndImportsPreserveValidTargets() {
  using uwf::core::OverlayType;
  uwf::ui::GlobalStatusPanel panel(std::make_shared<uwf::presentation::GlobalConfigurationViewModel>(uwf::windows::systemTotalRamMb()));
  uwf::core::SessionSnapshot session;
  session.overlay = {OverlayType::RAM, 512, 256, 384};
  panel.viewModel().setData(session, session, {});
  auto* type = panel.findChild<QComboBox*>(QStringLiteral("overlayTypeNext"));
  auto* maximum = panel.findChild<QSpinBox*>(QStringLiteral("overlayMaximumNext"));
  auto* warning = panel.findChild<QSpinBox*>(QStringLiteral("overlayWarningNext"));
  auto* critical = panel.findChild<QSpinBox*>(QStringLiteral("overlayCriticalNext"));
  QVERIFY(type && maximum && warning && critical);
  type->setCurrentIndex(type->findData(static_cast<int>(OverlayType::Disk)));
  QCOMPARE(maximum->minimum(), 1024);
  QCOMPARE(maximum->value(), 1024);
  QCOMPARE(critical->value(), 384);
  QCOMPARE(warning->value(), 256);
  type->setCurrentIndex(type->findData(static_cast<int>(OverlayType::RAM)));
  QCOMPARE(maximum->minimum(), 0);
  QCOMPARE(maximum->value(), 1024);

  // 导入中的临时类型不能截断最终磁盘容量。
  session.overlay = {OverlayType::Disk, 2048, 256, 384};
  panel.viewModel().setData(session, session, {});
  QVERIFY(panel.viewModel().importOverlayMaxMb(1000000));
  QVERIFY(panel.viewModel().importOverlayType(OverlayType::RAM));
  QVERIFY(panel.viewModel().importOverlayType(OverlayType::Disk));
  panel.viewModel().finishImport();
  QCOMPARE(maximum->value(), 1000000);
  QCOMPARE(warning->value(), 256);
  QCOMPARE(critical->value(), 384);
  QCOMPARE(panel.viewModel().pendingOverlay().maximumSizeMb, std::optional<uint32_t>(1000000));
  QVERIFY(!panel.viewModel().pendingOverlay().type.has_value());

  QVERIFY(panel.viewModel().importOverlayMaxMb(512));
  QVERIFY(panel.viewModel().importOverlayType(OverlayType::RAM));
  panel.viewModel().finishImport();
  QCOMPARE(maximum->value(), 512);
  QCOMPARE(warning->value(), 256);
  QCOMPARE(critical->value(), 384);
}

void WidgetConfigurationTests::globalStatusThresholdEventsCanBeDisabledIndependently() {
  uwf::ui::GlobalStatusPanel panel(std::make_shared<uwf::presentation::GlobalConfigurationViewModel>(uwf::windows::systemTotalRamMb()));
  uwf::core::SessionSnapshot session;
  auto* maximum = panel.findChild<QSpinBox*>(QStringLiteral("overlayMaximumNext"));
  auto* warning = panel.findChild<QSpinBox*>(QStringLiteral("overlayWarningNext"));
  auto* critical = panel.findChild<QSpinBox*>(QStringLiteral("overlayCriticalNext"));
  QVERIFY(maximum && warning && critical);
  for (const auto [warningValue, criticalValue] :
       {std::pair{512u, 1024u}, std::pair{1536u, 2048u}, std::pair{512u, 0u}, std::pair{0u, 768u}, std::pair{0u, 0u}}) {
    session.overlay = {uwf::core::OverlayType::RAM, 1024, warningValue, criticalValue};
    panel.viewModel().setData(session, session, {});
    QCOMPARE(maximum->value(), 1024);
    QCOMPARE(warning->value(), static_cast<int>(warningValue));
    QCOMPARE(critical->value(), static_cast<int>(criticalValue));
    QVERIFY(panel.viewModel().pendingOverlay().empty());
    panel.viewModel().finishImport();
    QVERIFY(panel.viewModel().pendingOverlay().empty());
  }
  session.overlay = {uwf::core::OverlayType::RAM, 4096, 512, 768};
  panel.viewModel().setData(session, session, {});
  critical->setValue(0);
  QCOMPARE(warning->value(), 512);
  QCOMPARE(warning->maximum(), std::numeric_limits<int>::max());
  const auto disabled = panel.viewModel().pendingOverlay();
  QCOMPARE(disabled.criticalThresholdMb, std::optional<uint32_t>{0});
  QVERIFY(!disabled.warningThresholdMb);
  warning->setValue(1024);
  QCOMPARE(warning->value(), 1024);
  QCOMPARE(critical->value(), 0);
  critical->setValue(700);
  QCOMPARE(warning->value(), 699);
  warning->setValue(0);
  QCOMPARE(critical->value(), 700);
  maximum->setValue(512);
  QCOMPARE(critical->value(), 700);
  QCOMPARE(warning->value(), 0);
}

void WidgetConfigurationTests::globalStatusKeepsLargeSystemValuesInTheModel() {
  uwf::ui::GlobalStatusPanel panel(std::make_shared<uwf::presentation::GlobalConfigurationViewModel>(8192));
  uwf::core::SessionSnapshot session;
  constexpr auto providerMaximum = std::numeric_limits<uint32_t>::max();
  constexpr int editorMaximum = std::numeric_limits<int>::max();
  session.overlay = {uwf::core::OverlayType::Disk, providerMaximum, providerMaximum - 1, providerMaximum};
  panel.viewModel().setData(session, session, {});
  const auto* maximum = panel.findChild<QSpinBox*>(QStringLiteral("overlayMaximumNext"));
  const auto* warning = panel.findChild<QSpinBox*>(QStringLiteral("overlayWarningNext"));
  const auto* critical = panel.findChild<QSpinBox*>(QStringLiteral("overlayCriticalNext"));
  QVERIFY(maximum && warning && critical);
  QCOMPARE(maximum->value(), editorMaximum);
  QCOMPARE(critical->value(), editorMaximum);
  QCOMPARE(warning->value(), editorMaximum);
  QCOMPARE(panel.viewModel().draft().overlay.maximumSizeMb, providerMaximum);
  QCOMPARE(panel.viewModel().draft().overlay.warningThresholdMb, providerMaximum - 1);
  QCOMPARE(panel.viewModel().draft().overlay.criticalThresholdMb, providerMaximum);
  QVERIFY(panel.viewModel().pendingOverlay().empty());
}

void WidgetConfigurationTests::exclusionListDoubleClickCopiesTheDisplayedFullPath() {
  uwf::ui::ExclusionListWidget list(std::make_shared<uwf::presentation::ExclusionListViewModel>(uwf::ui::ExclusionListWidget::Kind::File),
                                    uwf::test::localFileDialogs());
  list.setDriveLetter(QStringLiteral("Z:"));
  list.setBaseline({QStringLiteral("\\Data\\State.db")}, {QStringLiteral("\\Data\\State.db")});
  list.resize(500, 260);
  list.show();
  QVERIFY(QTest::qWaitForWindowExposed(&list));

  auto* items = list.findChild<QListView*>();
  QVERIFY(items);
  QCOMPARE(items->model()->rowCount(), 1);
  QSignalSpy copied(&list, &uwf::ui::ExclusionListWidget::copiedToClipboard);
  const QRect rect = items->visualRect(items->model()->index(0, 0));
  QVERIFY(rect.isValid());
  QTest::mouseClick(items->viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
  QCOMPARE(items->currentIndex(), items->model()->index(0, 0));
  QTest::mouseDClick(items->viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
  QTest::mouseRelease(items->viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
  QCOMPARE(copied.count(), 1);
  QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("Z:\\Data\\State.db"));
}

void WidgetConfigurationTests::exclusionListButtonsFilterAndPersistenceRowsUseDisplayedSelection() {
  uwf::ui::ExclusionListWidget files(std::make_shared<uwf::presentation::ExclusionListViewModel>(uwf::ui::ExclusionListWidget::Kind::File),
                                     uwf::test::localFileDialogs());
  files.setDriveLetter(QStringLiteral("Z:"));
  files.setBaseline({QStringLiteral("Z:\\Alpha"), QStringLiteral("Z:\\Beta"), QStringLiteral("Z:\\Gamma")},
                    {QStringLiteral("Z:\\Alpha"), QStringLiteral("Z:\\Beta"), QStringLiteral("Z:\\Gamma")});
  files.resize(560, 320);
  files.show();
  QVERIFY(QTest::qWaitForWindowExposed(&files));
  auto* list = files.findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* filter = files.findChild<QLineEdit*>(QStringLiteral("filterInput"));
  auto* remove = buttonWithText(&files, QStringLiteral("Remove selected"));
  QVERIFY(list);
  QVERIFY(filter);
  QVERIFY(remove);
  QCOMPARE(list->model()->rowCount(), 3);

  filter->setText(QStringLiteral("beta"));
  int visible = 0;
  for (int row = 0; row < list->model()->rowCount(); ++row) visible += list->isRowHidden(row) ? 0 : 1;
  QCOMPARE(visible, 1);
  filter->clear();
  list->selectionModel()->select(list->model()->index(0, 0), QItemSelectionModel::Select);
  list->selectionModel()->select(list->model()->index(2, 0), QItemSelectionModel::Select);
  filter->setText(QStringLiteral("beta"));
  QCOMPARE(list->model()->rowCount(), 1);
  QCOMPARE(list->selectionModel()->selectedRows().size(), 0);
  QVERIFY(!remove->isEnabled());
  filter->clear();
  QCOMPARE(list->selectionModel()->selectedRows().size(), 0);
  list->selectionModel()->select(list->model()->index(0, 0), QItemSelectionModel::Select);
  list->selectionModel()->select(list->model()->index(2, 0), QItemSelectionModel::Select);
  QCOMPARE(list->selectionModel()->selectedRows().size(), 2);
  QTest::mouseClick(remove, Qt::LeftButton);
  QCOMPARE(files.pendingRemoved().size(), 2);

  auto registryEditor = std::make_shared<uwf::app::ExclusionEditor>(uwf::app::ExclusionEditor::Kind::Registry);
  uwf::ui::ExclusionListWidget registry(std::make_shared<uwf::presentation::ExclusionListViewModel>(registryEditor), uwf::test::localFileDialogs());
  const QString stagingRegistryRoot = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager");
  bool fileStagingHasData = true;
  registryEditor->setFileStagingDataProbe([&fileStagingHasData] { return fileStagingHasData; });
  registry.setBaseline({stagingRegistryRoot}, {stagingRegistryRoot});
  registry.setPersistBaseline(true, true, false, false);
  registry.resize(620, 320);
  registry.show();
  QVERIFY(QTest::qWaitForWindowExposed(&registry));
  auto* enableTscal = actionWithText(&registry, QStringLiteral("Terminal Services license persistence"));
  QVERIFY(enableTscal);
  enableTscal->trigger();
  QCOMPARE(registry.pendingPersistTSCAL(), std::optional<bool>(true));

  auto* registryList = registry.findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* registryRemove = buttonWithText(&registry, QStringLiteral("Remove selected"));
  QVERIFY(registryList);
  QVERIFY(registryRemove);
  bool stagingRegistryObserved = false;
  QModelIndex stagingRegistryItem;
  for (int row = 0; row < registryList->model()->rowCount(); ++row) {
    const auto item = registryList->model()->index(row, 0);
    if (item.data(Qt::UserRole).toString().compare(stagingRegistryRoot, Qt::CaseInsensitive) != 0) continue;
    stagingRegistryObserved = true;
    stagingRegistryItem = item;
    QVERIFY(item.data(Qt::ToolTipRole).toString().startsWith(QStringLiteral("Required by UWF for File staging.")));
    QVERIFY(!item.data(Qt::ToolTipRole).toString().contains(QStringLiteral("Registry:")));
  }
  QVERIFY(stagingRegistryObserved);
  QVERIFY(stagingRegistryItem.isValid());
  registryList->selectionModel()->select(stagingRegistryItem, QItemSelectionModel::Select);
  QVERIFY(!registryRemove->isEnabled());
  QTest::mouseClick(registryRemove, Qt::LeftButton);
  QCOMPARE(registry.pendingRemoved().size(), 0);

  fileStagingHasData = false;
  registryEditor->refreshFileStagingConstraints();
  QVERIFY(registryRemove->isEnabled());
  fileStagingHasData = true;
  registryEditor->refreshFileStagingConstraints();
  QVERIFY(!registryRemove->isEnabled());
  fileStagingHasData = false;
  registryEditor->refreshFileStagingConstraints();
  QVERIFY(registryRemove->isEnabled());
  registryList->clearSelection();
  bool selectedTscal = false;
  for (int row = 0; row < registryList->model()->rowCount(); ++row) {
    const auto persistence = registryList->model()->index(row, 0).data(uwf::presentation::PersistenceRole);
    if (persistence.isValid() && persistence.toInt() == static_cast<int>(uwf::presentation::ExclusionListViewModel::PersistenceItem::Tscal)) {
      registryList->selectionModel()->select(registryList->model()->index(row, 0), QItemSelectionModel::Select);
      selectedTscal = true;
    }
  }
  QVERIFY(selectedTscal);
  QTest::mouseClick(registryRemove, Qt::LeftButton);
  QVERIFY(!registry.pendingPersistTSCAL().has_value());

  registryList->clearSelection();
  for (int row = 0; row < registryList->model()->rowCount(); ++row)
    registryList->selectionModel()->select(registryList->model()->index(row, 0), QItemSelectionModel::Select);
  QTest::mouseClick(registryRemove, Qt::LeftButton);
  QCOMPARE(registry.pendingPersistDomainSecretKey(), std::optional<bool>(false));
}

void WidgetConfigurationTests::exclusionFileActionsUseThePlatformBoundaryAndPreserveValidation() {
  FileDialogStub files;
  files.openedFiles = {QStringLiteral("Z:/Data/one.db"), QStringLiteral("Y:/wrong.db"), QString()};
  files.selectedDirectory = QStringLiteral("Z:/Cache");
  uwf::ui::ExclusionListWidget list(std::make_shared<uwf::presentation::ExclusionListViewModel>(uwf::ui::ExclusionListWidget::Kind::File), files);
  list.setDriveLetter(QStringLiteral("z:"));
  list.setBaseline({}, {});

  auto* addFile = actionWithText(&list, QStringLiteral("File…"));
  auto* addFolder = actionWithText(&list, QStringLiteral("Folder…"));
  QVERIFY(addFile);
  QVERIFY(addFolder);

  bool warningSeen = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    warningSeen = dialog != nullptr;
    if (dialog) dialog->accept();
  });
  addFile->trigger();
  QVERIFY(warningSeen);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("Z:\\Data\\one.db")}));
  QCOMPARE(files.requests.size(), 1);
  QCOMPARE(files.requests.front().initialPath, QStringLiteral("Z:\\"));

  addFolder->trigger();
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("Z:\\Cache"), QStringLiteral("Z:\\Data\\one.db")}));
  QCOMPARE(files.requests.size(), 2);

  list.setReadOnly(true);
  files.openedFiles = {QStringLiteral("Z:/Blocked")};
  addFile->trigger();
  QCOMPARE(files.requests.size(), 2);
  QCOMPARE(list.pendingAdded().size(), 2);
}

void WidgetConfigurationTests::importDialogFiltersCommentsAndAppendsBatchReports() {
  QVERIFY(uwf::app::parseErrorMessage(uwf::app::ParseError::MalformedQuoting, {}).contains(QStringLiteral("quote"), Qt::CaseInsensitive));
  QVERIFY(uwf::app::parseErrorMessage(uwf::app::ParseError::UnexpectedArgument, QStringLiteral("extra")).contains(QStringLiteral("extra")));

  uwf::ui::ImportDialog dialog(std::make_shared<uwf::presentation::ImportViewModel>(uwf::windows::readBoundedTextFile), uwf::test::localFileDialogs());
  auto* input = dialog.findChild<uwf::ui::CodeEditor*>(QStringLiteral("importTextEdit"));
  auto* importButton = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* report = dialog.findChild<QTableView*>();
  QVERIFY(input);
  QVERIFY(importButton);
  QVERIFY(report);
  QVERIFY(!importButton->isEnabled());

  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  QList<QList<uwf::app::UwfmgrCommand>> batches;
  dialog.setApplier([&](const QList<uwf::app::UwfmgrCommand>& commands) {
    batches.append(commands);
    QList<uwf::app::ImportResult> rows;
    for (const auto& command : commands) {
      uwf::app::ImportResult row;
      row.lineNo = command.sourceLineNo;
      row.lineText = QString::fromStdString(command.rawLine);
      row.status = command.parseError == uwf::app::ParseError::None ? uwf::app::ImportResult::Status::Success : uwf::app::ImportResult::Status::Failed;
      row.detail = command.parseError == uwf::app::ParseError::None ? QStringLiteral("queued") : QStringLiteral("invalid");
      rows.append(row);
    }
    return rows;
  });

  input->setPlainText(QStringLiteral("# note\nuwfmgr filter enable\noverlay set-size bad\n"));
  QVERIFY(importButton->isEnabled());
  QTest::mouseClick(importButton, Qt::LeftButton);
  QCOMPARE(batches.size(), 1);
  QCOMPARE(batches[0].size(), 2);
  QCOMPARE(batches[0][0].sourceLineNo, 2);
  QCOMPARE(batches[0][1].parseError, uwf::app::ParseError::InvalidSize);
  QCOMPARE(input->toPlainText(), QStringLiteral("# note\nuwfmgr filter enable\noverlay set-size bad\n"));
  QCOMPARE(report->model()->rowCount(), 2);
  QVERIFY(report->verticalHeader()->isHidden());
  for (const QSize size : {QSize(760, 500), QSize(1096, 754), QSize(1320, 820)}) {
    dialog.resize(size);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QTRY_COMPARE(report->horizontalHeader()->length(), report->viewport()->width());
    for (int column = 0; column < report->model()->columnCount(); ++column) {
      const QRect cell = report->visualRect(report->model()->index(0, column));
      QCOMPARE(cell.left(), report->horizontalHeader()->sectionViewportPosition(column));
      QCOMPARE(cell.width(), report->columnWidth(column));
    }
  }

  auto* continueEditing = buttonWithText(&dialog, QStringLiteral("Continue editing"));
  bool resultInspected = false;
  QTimer::singleShot(0, &dialog, [&] {
    auto* result = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(result && result != &dialog);
    const auto close = qScopeGuard([&] { result->reject(); });
    QCOMPARE(result->objectName(), QStringLiteral("importCommandResultDialog"));
    QCOMPARE(result->windowTitle(), QStringLiteral("Command processing result"));
    QCOMPARE(result->width(), 520);
    auto* command = result->findChild<uwf::ui::ContentHintBox*>(QStringLiteral("importResultCommand"));
    QVERIFY(command);
    QCOMPARE(command->toPlainText(), QStringLiteral("uwfmgr filter enable"));
    auto* copy = buttonWithText(result, QStringLiteral("Copy command"));
    QVERIFY(copy);
    QTest::mouseClick(copy, Qt::LeftButton);
    QCOMPARE(QApplication::clipboard()->text(), command->toPlainText());
    resultInspected = true;
  });
  QTest::mouseClick(report->viewport(), Qt::LeftButton, Qt::NoModifier, report->visualRect(report->model()->index(0, 2)).center());
  QVERIFY(resultInspected);
  QVERIFY(continueEditing);
  QTest::mouseClick(continueEditing, Qt::LeftButton);
  QVERIFY(input->isVisibleTo(&dialog));
  QCOMPARE(input->toPlainText(), QStringLiteral("# note\nuwfmgr filter enable\noverlay set-size bad\n"));
  auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("importTabs"));
  QVERIFY(tabs);
  tabs->setCurrentIndex(1);
  tabs->setCurrentIndex(0);
  QCOMPARE(input->toPlainText(), QStringLiteral("# note\nuwfmgr filter enable\noverlay set-size bad\n"));
  input->setPlainText(QStringLiteral("volume protect d"));
  QTest::mouseClick(importButton, Qt::LeftButton);
  QCOMPARE(batches.size(), 2);
  QCOMPARE(batches[1].size(), 1);
  QCOMPARE(batches[1][0].args.at(0), std::string("D:"));
  QCOMPARE(report->model()->rowCount(), 4);  // 两条首批结果 + 第二批分隔行 + 一条结果
  QVERIFY(report->isVisibleTo(&dialog));
}

void WidgetConfigurationTests::importFileActionLoadsOnlyCommandsFromEverySelectedFile_data() {
  QTest::addColumn<QString>("initialText");
  QTest::addColumn<QString>("expectedPrefix");
  QTest::newRow("empty") << QString{} << QString{};
  QTest::newRow("without-trailing-newline") << QStringLiteral("uwfmgr filter disable") << QStringLiteral("uwfmgr filter disable\n");
  QTest::newRow("with-trailing-newline") << QStringLiteral("uwfmgr filter disable\n") << QStringLiteral("uwfmgr filter disable\n");
}

void WidgetConfigurationTests::importFileActionLoadsOnlyCommandsFromEverySelectedFile() {
  QFETCH(QString, initialText);
  QFETCH(QString, expectedPrefix);
  QTemporaryFile first;
  QTemporaryFile second;
  QVERIFY(first.open());
  QVERIFY(second.open());
  QVERIFY(first.write("noise\nuwfmgr filter enable\nUWFMgr volume protect C:\n") > 0);
  QVERIFY(second.write("nothing useful\n") > 0);
  first.flush();
  second.flush();
  first.close();
  second.close();

  FileDialogStub files;
  files.openedFiles = {first.fileName(), second.fileName()};
  uwf::ui::ImportDialog dialog(std::make_shared<uwf::presentation::ImportViewModel>(uwf::windows::readBoundedTextFile), files);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* load = buttonWithText(&dialog, QStringLiteral("Load from file…"));
  auto* input = dialog.findChild<uwf::ui::CodeEditor*>(QStringLiteral("importTextEdit"));
  QVERIFY(load);
  QVERIFY(input);
  input->setPlainText(initialText);

  QTest::mouseClick(load, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 1);
  QVERIFY(files.requests.front().filter.contains(QStringLiteral("*.cmd")));
  const QString text = input->toPlainText();
  QCOMPARE(text, expectedPrefix + QStringLiteral("# File: %1\nuwfmgr filter enable\nUWFMgr volume protect C:\n# File: %2\n"
                                                 "# No uwfmgr commands found in the input.\n")
                                      .arg(QFileInfo(first.fileName()).fileName(), QFileInfo(second.fileName()).fileName()));
  std::vector<uwf::app::UwfmgrKind> kinds;
  const auto commands = uwf::app::parseUwfmgrText(text.toStdString());
  for (const auto& command : commands) {
    if (command.parseError == uwf::app::ParseError::Comment) continue;
    QCOMPARE(command.parseError, uwf::app::ParseError::None);
    kinds.push_back(command.kind);
  }
  std::vector expectedKinds{uwf::app::UwfmgrKind::FilterEnable, uwf::app::UwfmgrKind::VolumeProtect};
  if (!initialText.isEmpty()) expectedKinds.insert(expectedKinds.begin(), uwf::app::UwfmgrKind::FilterDisable);
  QCOMPARE(kinds, expectedKinds);

  files.openedFiles.clear();
  QTest::mouseClick(load, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 2);
  QCOMPARE(input->toPlainText(), text);

  QTemporaryFile missing;
  QVERIFY(missing.open());
  const QString missingPath = missing.fileName();
  QVERIFY(missing.remove());
  files.openedFiles = {missingPath};
  QTimer::singleShot(0, this, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->accept();
  });
  QTest::mouseClick(load, Qt::LeftButton);
  QCOMPARE(input->toPlainText(), text);

  QTemporaryFile oversized;
  QVERIFY(oversized.open());
  const QByteArray payload(5 * 1024 * 1024 + 1, 'x');
  QCOMPARE(oversized.write(payload), static_cast<qint64>(payload.size()));
  oversized.close();
  files.openedFiles = {oversized.fileName()};
  QTimer::singleShot(0, this, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->accept();
  });
  QTest::mouseClick(load, Qt::LeftButton);
  QCOMPARE(input->toPlainText(), text);
}

void WidgetConfigurationTests::recommendedConfigurationSelectionAppendsOnlyChosenGroups() {
  uwf::ui::ImportDialog dialog(std::make_shared<uwf::presentation::ImportViewModel>(uwf::windows::readBoundedTextFile), uwf::test::localFileDialogs());
  auto* input = dialog.findChild<uwf::ui::CodeEditor*>(QStringLiteral("importTextEdit"));
  QPushButton* recommended = nullptr;
  for (const auto buttons = dialog.findChildren<QPushButton*>(); auto* button : buttons) {
    if (button->text() == QStringLiteral("Load recommended configuration")) recommended = button;
  }
  QVERIFY(input);
  QVERIFY(recommended);

  bool chooserStructureValid = false;
  QTimer::singleShot(0, this, [&chooserStructureValid] {
    auto* chooser = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!chooser) return;
    const auto checks = chooser->findChildren<QCheckBox*>();
    if (checks.size() != 6 || std::ranges::any_of(checks, [](const QCheckBox* check) { return !check->isChecked(); })) {
      chooser->reject();
      return;
    }
    for (int index = 1; index < checks.size(); ++index) checks[index]->setChecked(false);
    auto* box = chooser->findChild<QDialogButtonBox*>();
    if (!box) {
      chooser->reject();
      return;
    }
    QPushButton* accept = nullptr;
    for (const auto buttons = box->buttons(); auto* button : buttons) {
      if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) accept = qobject_cast<QPushButton*>(button);
    }
    if (!accept || !accept->isEnabled()) {
      chooser->reject();
      return;
    }
    chooserStructureValid = true;
    QTest::mouseClick(accept, Qt::LeftButton);
  });
  QTest::mouseClick(recommended, Qt::LeftButton);

  QVERIFY(chooserStructureValid);
  const QString text = input->toPlainText();
  QVERIFY(text.contains(QStringLiteral("Customer Experience Improvement Program")));
  QVERIFY(text.contains(QStringLiteral("uwfmgr registry add-exclusion")));
  QVERIFY(!text.contains(QStringLiteral("Background Intelligent Transfer Service")));
  auto* import = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(import);
  QVERIFY(import->isEnabled());
}

}  // namespace
QTEST_MAIN(WidgetConfigurationTests)
#include "ConfigurationCases.moc"
