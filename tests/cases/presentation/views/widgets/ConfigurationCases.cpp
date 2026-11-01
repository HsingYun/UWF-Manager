// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QImage>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPoint>
#include <QPushButton>
#include <QRect>
#include <QScopeGuard>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTabBar>
#include <QTabWidget>
#include <QTableView>
#include <QTemporaryFile>
#include <QTest>
#include <QWidget>
#include <QtMath>
#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "application/staging/contracts/FileStagingStore.h"
#include "application/staging/editing/FileStagingEditor.h"
#include "cases/presentation/views/widgets/WidgetUiFixture.h"
#include "domain/uwf/model/UwfModel.h"
#include "domain/uwf/policies/IndividualCommitPolicy.h"
#include "infrastructure/windows/diagnostics/SystemHardwareInfo.h"
#include "infrastructure/windows/filesystem/text/BoundedTextFileReader.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/feedback/ContentState.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/feedback/StatusBadge.h"
#include "presentation/controls/inputs/CodeEditor.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/views/configuration/exclusions/ExclusionListWidget.h"
#include "presentation/views/configuration/global/GlobalStatusPanel.h"
#include "presentation/views/configuration/importing/ImportDialog.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "presentation/views/configuration/volumes/VolumeStatusPanel.h"
#include "presentation/views/configuration/volumes/VolumeTab.h"
#include "presentation/views/overlay/usage/OverlayUsageBar.h"
#include "presentation/views/staging/FileStagingWidget.h"
#include "support/app/CommitScenario.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/OverlayPolicyFixture.h"
#include "support/app/ReadOnlyFileBrowserFixture.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LayoutSettling.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/StagingControls.h"
#include "support/visual/InlineIconTextProbe.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

using namespace uwf::test::widgets;
using uwf::test::FileDialogStub;

namespace {
struct ConfigurationIconSamples {
  QJsonArray measurements;
  int captureIndex = 0;

  bool sample(QWidget& window, const QImage& image, QWidget* graphic, uwf::ui::TextLabel* text, const QString& resource,
              const uwf::ui::design::Color graphicColor, const uwf::ui::design::Color textColor, const bool firstLine) {
    using namespace uwf;
    if (!graphic || !text || !graphic->isVisibleTo(&window) || !text->isVisibleTo(&window)) return false;
    const bool contained = graphic->parentWidget()->rect().contains(graphic->geometry());
    const auto dpr = image.devicePixelRatio();
    const QRectF graphicRegion(graphic->mapTo(&window, QPoint()), graphic->size());
    const auto textRegion = (firstLine ? text->firstLineBoxBounds() : QRectF(text->contentsRect())).translated(text->mapTo(&window, QPoint()));
    const auto background = image.pixelColor(qFloor(graphicRegion.left() * dpr), qFloor(graphicRegion.top() * dpr));
    const auto& colors = ui::DesignSystem::instance();
    const auto graphicInk = test::visibleInkBounds(image, colors.color(graphicColor), background, graphicRegion);
    const auto textInk = test::visibleInkBounds(image, colors.color(textColor), background, textRegion);
    const auto graphicCenter = static_cast<qreal>(qRound64(graphicInk.center().y() * dpr * 2)) / 2;
    const auto textCenter = static_cast<qreal>(qRound64(textInk.center().y() * dpr * 2)) / 2;
    const auto difference = qAbs(graphicCenter - textCenter);
    const auto capture = QStringLiteral("configuration-%1-%2").arg(QString::fromLatin1(QTest::currentDataTag())).arg(++captureIndex);
    measurements.append(QJsonObject{{"resource", resource},
                                    {"widget", graphic->objectName()},
                                    {"reference", text->objectName()},
                                    {"text", text->text()},
                                    {"rule", firstLine ? "first-visible-line" : "visible-center"},
                                    {"dpr", dpr},
                                    {"graphicCenterPhysical", graphicCenter},
                                    {"textCenterPhysical", textCenter},
                                    {"centerDifferencePhysical", difference},
                                    {"graphicFitsHost", contained},
                                    {"nonempty", !graphicInk.isEmpty() && !textInk.isEmpty()},
                                    {"capture", capture}});
    const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR");
    if (!directory.isEmpty()) {
      if (!QDir().mkpath(directory) || !image.save(QDir(directory).filePath(capture + QStringLiteral(".png")))) return false;
      QFile output(QDir(directory).filePath(QStringLiteral("configuration-%1.json").arg(QString::fromLatin1(QTest::currentDataTag()))));
      if (!output.open(QIODevice::WriteOnly) || output.write(QJsonDocument(measurements).toJson()) <= 0) return false;
    }
    if (!contained || graphicInk.isEmpty() || textInk.isEmpty() || difference > 1)
      qWarning().noquote() << QStringLiteral("%1 %2 / %3: graphic=%4 text=%5 delta=%6 physical pixels fits=%7")
                                  .arg(resource, graphic->objectName(), text->text())
                                  .arg(graphicCenter)
                                  .arg(textCenter)
                                  .arg(difference)
                                  .arg(contained);
    return contained && !graphicInk.isEmpty() && !textInk.isEmpty() && difference <= 1;
  }
};

class WidgetConfigurationTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { initializeWidgets(); }
  void currentSessionShowsAllThreeStateIcons();
  void configurationGlobalInlineIconsUseTheirRenderedTextCenter_data();
  void configurationGlobalInlineIconsUseTheirRenderedTextCenter();
  void configurationInlineIconsUseTheirRenderedTextCenter_data();
  void configurationInlineIconsUseTheirRenderedTextCenter();
  void globalStatusEditsPublishOnlyConstrainedValues();
  void globalStatusTypeChangesAndImportsPreserveValidTargets();
  void globalStatusThresholdEventsCanBeDisabledIndependently();
  void globalStatusShowsObservedValuesWithoutEditing_data();
  void globalStatusShowsObservedValuesWithoutEditing();
  void globalUsageChartUsesObservedCapacity_data();
  void globalUsageChartUsesObservedCapacity();
  void globalStatusEditsCompleteUint32Thresholds();
  void exclusionListDoubleClickCopiesTheDisplayedFullPath();
  void exclusionListButtonsFilterAndPersistenceRowsUseDisplayedSelection();
  void exclusionFileActionsUseThePlatformBoundaryAndPreserveValidation();
  void importDialogFiltersCommentsAndAppendsBatchReports();
  void importFileActionLoadsOnlyCommandsFromEverySelectedFile_data();
  void importFileActionLoadsOnlyCommandsFromEverySelectedFile();
  void recommendedConfigurationSelectionAppendsOnlyChosenGroups();
  void stagingDependencyReviewKeepsTheBuiltinBadgeAndCompleteDescription_data();
  void stagingDependencyReviewKeepsTheBuiltinBadgeAndCompleteDescription();
  void configurationAvailabilitySurvivesLanguageTransitions_data();
  void configurationAvailabilitySurvivesLanguageTransitions();
  void exclusionActionsFollowSelectionAndLanguage_data();
  void exclusionActionsFollowSelectionAndLanguage();
  void stagingActionsFollowStorageSelectionAndLanguage_data();
  void stagingActionsFollowStorageSelectionAndLanguage();
};

void WidgetConfigurationTests::globalUsageChartUsesObservedCapacity_data() {
  QTest::addColumn<uint32_t>("usedMb");
  QTest::addColumn<uint32_t>("availableMb");
  QTest::addColumn<uint32_t>("warningMb");
  QTest::addColumn<uint32_t>("criticalMb");
  QTest::newRow("runtime-below-configured-capacity") << uint32_t{2048} << uint32_t{2048} << uint32_t{3072} << uint32_t{3584};
  const auto maximum = std::numeric_limits<uint32_t>::max();
  QTest::newRow("runtime-total-exceeds-uint32") << maximum << maximum << uint32_t{0} << uint32_t{0};
}

void WidgetConfigurationTests::globalUsageChartUsesObservedCapacity() {
  using namespace uwf;
  QFETCH(uint32_t, usedMb);
  QFETCH(uint32_t, availableMb);
  QFETCH(uint32_t, warningMb);
  QFETCH(uint32_t, criticalMb);
  auto model = std::make_shared<presentation::GlobalConfigurationViewModel>(4096);
  core::SessionSnapshot current;
  current.filter.enabled = true;
  current.overlay = {core::OverlayType::RAM, 8192, warningMb, criticalMb};
  model->reset(current);
  model->setCurrentSession(current, {availableMb, usedMb});
  ui::GlobalStatusPanel panel(model);
  panel.resize(720, 600);
  panel.show();
  QVERIFY(QTest::qWaitForWindowExposed(&panel));
  QVERIFY(test::waitForLayoutSettled(panel));
  auto* bar = panel.findChild<ui::OverlayUsageBar*>();
  QVERIFY(bar);
  QCOMPARE(model->usage().percentageText, QStringLiteral("50%"));
  const auto& theme = ui::ThemeManager::instance();
  for (const int width : {160, 420}) {
    bar->resize(width, bar->sizeHint().height());
    QImage image(bar->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    bar->render(&image);
    // Both observations are half full; their configured upper limit is unrelated.
    QVERIFY(test::colorCoverage(image, theme.color(ui::Sem::Accent), QRectF(width * .3, 3, width * .1, 3)) >= test::kVisualMinimumCoverage);
    QVERIFY(test::colorCoverage(image, theme.color(ui::Sem::BarBg), QRectF(width * .6, 3, width * .1, 3)) >= test::kVisualMinimumCoverage);
    if (warningMb != 0) {
      QVERIFY(test::colorCoverage(image, theme.color(ui::Sem::Warn), QRectF(width * .75 + .5, 3, 1, 3)) >= test::kVisualMinimumCoverage);
      QVERIFY(test::colorCoverage(image, theme.color(ui::Sem::Danger), QRectF(width * .875 + .5, 3, 1, 3)) >= test::kVisualMinimumCoverage);
    }
  }
}

void WidgetConfigurationTests::configurationGlobalInlineIconsUseTheirRenderedTextCenter_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("language");
  QTest::addColumn<bool>("dark");
  for (const auto& language : uwf::app::languages())
    for (const bool dark : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString(language.code), dark ? "dark" : "light"))) << language.id << dark;
}

void WidgetConfigurationTests::configurationGlobalInlineIconsUseTheirRenderedTextCenter() {
  using namespace uwf;
  QFETCH(ui::I18n::Lang, language);
  QFETCH(bool, dark);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto previousTheme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(previousLanguage);
    ui::ThemeManager::instance().apply(previousTheme);
  });
  ui::I18n::instance().setLang(language);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  ConfigurationIconSamples samples;
  auto& measurements = samples.measurements;
  auto& captureIndex = samples.captureIndex;
  using Color = ui::design::Color;
  presentation::ConfigurationViewModel configuration(8192);
  ui::GlobalStatusPanel global(configuration.global(), configuration.overlayPolicy());
  global.resize(380, 700);
  global.show();
  QVERIFY(QTest::qWaitForWindowExposed(&global));
  auto current = test::application::editableSnapshot().current;
  for (int state = 0; state < 3; ++state) {
    current.filter.enabled = state == 0;
    auto snapshot = test::application::configurationSnapshot(current, current, {3072, 1024});
    snapshot.uwfAvailable = state != 2;
    configuration.resetSnapshot(snapshot, test::application::diskPersistentPolicy());
    QVERIFY(test::waitForLayoutSettled(global));
    const auto frame = test::captureInlineIconText(global);
    QVERIFY(samples.sample(global, frame, global.findChild<QWidget*>(QStringLiteral("currentSessionStateIcon")),
                           global.findChild<ui::TextLabel*>(QStringLiteral("statusCurLabel")),
                           state == 0   ? QStringLiteral(":/icons/check.svg")
                           : state == 1 ? QStringLiteral(":/icons/remove.svg")
                                        : QStringLiteral(":/icons/status_warning.svg"),
                           state == 0 ? Color::Success : Color::Muted, state == 0 ? Color::Success : Color::Muted, false));
    const auto alignmentError =
        test::probeInlineIconText(global, frame, QStringLiteral("configuration-global-%1").arg(state), QString::fromLatin1(QTest::currentDataTag()));
    QVERIFY2(alignmentError.isEmpty(), qUtf8Printable(alignmentError));
    if (state == 0) {
      int iconOnlyMeasured = 0;
      for (auto buttons = global.findChildren<QPushButton*>(); auto* button : std::as_const(buttons)) {
        if (!button->text().isEmpty() || button->icon().isNull() || !button->isVisibleTo(&global)) continue;
        const qreal dpr = frame.devicePixelRatio();
        const auto origin = button->mapTo(&global, QPoint());
        const QRect interior = button->rect().adjusted(4, 4, -4, -4);
        QVERIFY(button->visibleRegion().contains(button->rect()));
        const auto ink = test::renderedInlineInk(frame, interior.translated(origin));
        const auto center = QRectF(button->rect()).center() + origin;
        const qreal horizontal = qAbs(qRound((ink.center().x() - center.x()) * dpr * 2)) / 2.0;
        const qreal vertical = qAbs(qRound((ink.center().y() - center.y()) * dpr * 2)) / 2.0;
        const auto capture = QStringLiteral("configuration-%1-%2").arg(QString::fromLatin1(QTest::currentDataTag())).arg(++captureIndex);
        measurements.append(QJsonObject{{"resource", ":/icons/settings.svg"},
                                        {"widget", button->objectName()},
                                        {"rule", "icon-only-control-center"},
                                        {"dpr", dpr},
                                        {"horizontalDifferencePhysical", horizontal},
                                        {"centerDifferencePhysical", vertical},
                                        {"nonempty", !ink.isEmpty()},
                                        {"capture", capture}});
        if (const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR"); !directory.isEmpty()) {
          QVERIFY(frame.save(QDir(directory).filePath(capture + QStringLiteral(".png"))));
          QFile output(QDir(directory).filePath(QStringLiteral("configuration-%1.json").arg(QString::fromLatin1(QTest::currentDataTag()))));
          QVERIFY(output.open(QIODevice::WriteOnly));
          QVERIFY(output.write(QJsonDocument(measurements).toJson()) > 0);
        }
        QVERIFY(!ink.isEmpty() && horizontal <= 1 && vertical <= 1);
        ++iconOnlyMeasured;
      }
      QCOMPARE(iconOnlyMeasured, 1);
    }
  }
  global.hide();
}

void WidgetConfigurationTests::configurationInlineIconsUseTheirRenderedTextCenter_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("language");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<int>("width");
  for (const auto& language : uwf::app::languages())
    for (const bool dark : {false, true})
      for (const int width : {720, 1096})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(QString(language.code), dark ? "dark" : "light").arg(width)))
            << language.id << dark << width;
}

void WidgetConfigurationTests::configurationInlineIconsUseTheirRenderedTextCenter() {
  using namespace uwf;
  QFETCH(ui::I18n::Lang, language);
  QFETCH(bool, dark);
  QFETCH(int, width);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto previousTheme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(previousLanguage);
    ui::ThemeManager::instance().apply(previousTheme);
  });
  ui::I18n::instance().setLang(language);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  const auto probe = [&](QWidget& window, const QImage& frame, const QString& name, const bool menus = false) {
    const auto tag = QString::fromLatin1(QTest::currentDataTag());
    QString errors = test::probeInlineIconText(window, frame, name, tag);
    if (menus) errors += test::probeInlineMenus(window, name, tag);
    if (!errors.isEmpty()) qWarning().noquote() << errors;
    return errors.isEmpty();
  };
  ConfigurationIconSamples samples;
  auto& measurements = samples.measurements;
  auto& captureIndex = samples.captureIndex;
  using Color = ui::design::Color;
  const auto sampleStacked = [&](QWidget& window, const QImage& frame, ui::ContentState& state, const QString& title, const QString& resource) {
    QWidget* graphic = nullptr;
    for (auto children = state.findChildren<QWidget*>(); auto* child : std::as_const(children))
      if (dynamic_cast<ui::IconView*>(child) && child->isVisibleTo(&window)) graphic = child;
    ui::TextLabel* text = nullptr;
    for (auto labels = state.findChildren<ui::TextLabel*>(); auto* label : std::as_const(labels))
      if (label->isVisibleTo(&window) && label->text() == title) text = label;
    if (!graphic || !text) return false;
    const qreal dpr = frame.devicePixelRatio();
    const QRectF graphicRegion(graphic->mapTo(&window, QPoint()), graphic->size());
    const QRectF textRegion(text->mapTo(&window, QPoint()), text->size());
    const auto graphicInk = test::renderedInlineInk(frame, graphicRegion);
    const auto textInk = test::renderedInlineInk(frame, textRegion);
    const bool contained = graphic->visibleRegion().contains(graphic->rect()) && text->visibleRegion().contains(text->rect()) &&
                           graphic->parentWidget()->rect().contains(graphic->geometry());
    const auto capture = QStringLiteral("configuration-%1-%2").arg(QString::fromLatin1(QTest::currentDataTag())).arg(++captureIndex);
    measurements.append(QJsonObject{{"resource", resource},
                                    {"widget", state.objectName()},
                                    {"reference", title},
                                    {"rule", "stacked-not-inline"},
                                    {"dpr", dpr},
                                    {"graphicFitsHost", contained},
                                    {"stackedOrder", graphicInk.bottom() < textInk.top()},
                                    {"nonempty", !graphicInk.isEmpty() && !textInk.isEmpty()},
                                    {"capture", capture}});
    if (const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR"); !directory.isEmpty()) {
      if (!QDir().mkpath(directory) || !frame.save(QDir(directory).filePath(capture + QStringLiteral(".png")))) return false;
      QFile output(QDir(directory).filePath(QStringLiteral("configuration-%1.json").arg(QString::fromLatin1(QTest::currentDataTag()))));
      if (!output.open(QIODevice::WriteOnly) || output.write(QJsonDocument(measurements).toJson()) <= 0) return false;
    }
    return contained && !graphicInk.isEmpty() && !textInk.isEmpty() && graphicInk.bottom() < textInk.top();
  };
  const auto sampleNotice = [&](QWidget& window, const QImage& frame, ui::Notice& notice, const QString& resource) {
    QWidget* graphic = nullptr;
    for (auto children = notice.findChildren<QWidget*>(); auto* child : std::as_const(children))
      if (dynamic_cast<ui::IconView*>(child) && child->isVisibleTo(&window)) graphic = child;
    ui::TextLabel* text = nullptr;
    for (auto labels = notice.findChildren<ui::TextLabel*>(); auto* label : std::as_const(labels))
      if (label->isVisibleTo(&window) && !label->text().isEmpty() && (!text || label->mapTo(&window, QPoint()).y() < text->mapTo(&window, QPoint()).y()))
        text = label;
    if (!graphic || !text) return false;
    const qreal dpr = frame.devicePixelRatio();
    const auto iconInk = test::renderedInlineInk(frame, QRectF(graphic->mapTo(&window, QPoint()), graphic->size()));
    const auto textInk = test::renderedInlineInk(frame, text->firstLineBoxBounds().translated(text->mapTo(&window, QPoint())));
    const qreal delta = qAbs(qRound((iconInk.center().y() - textInk.center().y()) * dpr * 2)) / 2.0;
    const bool contained = graphic->parentWidget()->rect().contains(graphic->geometry());
    const auto capture = QStringLiteral("configuration-%1-%2").arg(QString::fromLatin1(QTest::currentDataTag())).arg(++captureIndex);
    measurements.append(QJsonObject{{"resource", resource},
                                    {"widget", notice.objectName()},
                                    {"reference", text->text()},
                                    {"rule", "first-visible-line"},
                                    {"dpr", dpr},
                                    {"graphicCenterPhysical", iconInk.center().y() * dpr},
                                    {"textCenterPhysical", textInk.center().y() * dpr},
                                    {"centerDifferencePhysical", delta},
                                    {"graphicFitsHost", contained},
                                    {"nonempty", !iconInk.isEmpty() && !textInk.isEmpty()},
                                    {"capture", capture}});
    if (const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR"); !directory.isEmpty()) {
      if (!frame.save(QDir(directory).filePath(capture + QStringLiteral(".png")))) return false;
      QFile output(QDir(directory).filePath(QStringLiteral("configuration-%1.json").arg(QString::fromLatin1(QTest::currentDataTag()))));
      if (!output.open(QIODevice::WriteOnly) || output.write(QJsonDocument(measurements).toJson()) <= 0) return false;
    }
    return contained && !iconInk.isEmpty() && !textInk.isEmpty() && delta <= 1;
  };
  presentation::ConfigurationViewModel configuration(8192);
  auto volume =
      configuration.addVolume(core::VolumeInfo{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}, true);
  configuration.resetSnapshot(test::application::editableSnapshot());
  auto store = std::make_shared<test::application::MemoryFileStagingStore>();
  store->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\staged.db")}};
  app::FileStagingDraft staging(store);
  auto stagingEditor = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), staging, std::make_shared<test::commit::Tree>(false));
  ui::VolumeTab volumeTab(volume, test::localFileDialogs(), std::make_shared<presentation::FileStagingViewModel>(stagingEditor), {});
  QTRY_VERIFY(!stagingEditor->busy());
  volumeTab.resize(width, 754);
  volumeTab.show();
  QVERIFY(QTest::qWaitForWindowExposed(&volumeTab));
  auto* volumeTabs = volumeTab.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(volumeTabs);
  for (int tab = 0; tab < volumeTabs->count(); ++tab) {
    volumeTabs->setCurrentIndex(tab);
    QVERIFY(test::waitForLayoutSettled(volumeTab));
    const auto name = QStringLiteral("configuration-volume-tab-%1").arg(tab);
    auto* bar = volumeTabs->tabBar();
    const QRect region = bar->tabRect(tab).adjusted(4, 4, -4, -4);
    QVERIFY(bar->visibleRegion().contains(region));
    const auto frame = test::captureInlineIconText(volumeTab);
    const qreal dpr = frame.devicePixelRatio();
    const auto runs = test::inlineInkRuns(frame, region.translated(bar->mapTo(&volumeTab, QPoint())), qCeil(4 * dpr));
    QVERIFY(runs.size() >= 2);
    const bool rtl = bar->layoutDirection() == Qt::RightToLeft;
    const auto iconInk = rtl ? runs.back() : runs.front();
    QRectF textInk;
    for (qsizetype run = rtl ? 0 : 1; run < runs.size() - (rtl ? 1 : 0); ++run) textInk = textInk.united(runs.at(run));
    const qreal delta = qAbs(qRound((iconInk.center().y() - textInk.center().y()) * dpr * 2)) / 2.0;
    const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR");
    if (!directory.isEmpty()) {
      const QString stem = QStringLiteral("configuration-tab-%1-%2").arg(QString::fromLatin1(QTest::currentDataTag())).arg(tab);
      QVERIFY(QDir().mkpath(directory));
      QVERIFY(frame.save(QDir(directory).filePath(stem + QStringLiteral(".png"))));
      QFile output(QDir(directory).filePath(stem + QStringLiteral(".json")));
      QVERIFY(output.open(QIODevice::WriteOnly));
      QVERIFY(output.write(QJsonDocument(QJsonObject{{"kind", "tab"},
                                                     {"text", bar->tabText(tab)},
                                                     {"dpr", dpr},
                                                     {"graphicCenterPhysical", iconInk.center().y() * dpr},
                                                     {"textCenterPhysical", textInk.center().y() * dpr},
                                                     {"centerDifferencePhysical", delta},
                                                     {"nonempty", !iconInk.isEmpty() && !textInk.isEmpty()}})
                               .toJson()) > 0);
    }
    QVERIFY2(delta <= 1, qUtf8Printable(QStringLiteral("%1: actual tab icon/text differ by %2 physical pixels").arg(bar->tabText(tab)).arg(delta)));
    QVERIFY(probe(volumeTab, frame, name, true));
  }
  for (const bool horm : {false, true}) {
    volume->applySnapshot(test::application::editableSnapshot(), horm, horm ? std::optional<std::uint32_t>{} : core::kLoadedReadOnlyMediaFlag);
    QVERIFY(test::waitForLayoutSettled(volumeTab));
    auto* runtimeNotice = volumeTab.findChild<ui::Notice*>(QStringLiteral("volumeRuntimeNotice"));
    QVERIFY(runtimeNotice && runtimeNotice->isVisibleTo(&volumeTab));
    QVERIFY(sampleNotice(volumeTab, test::captureInlineIconText(volumeTab), *runtimeNotice, QStringLiteral(":/icons/lock.svg")));
  }
  auto* volumeStatus = volumeTab.findChild<ui::VolumeStatusPanel*>();
  QVERIFY(volumeStatus);
  volumeStatus->setUnsupported(QStringLiteral("FAT32"));
  QVERIFY(test::waitForLayoutSettled(volumeTab));
  auto* banner = volumeStatus->findChild<ui::Notice*>(QStringLiteral("statusBanner"));
  QVERIFY(banner && banner->isVisibleTo(&volumeTab));
  QVERIFY(sampleNotice(volumeTab, test::captureInlineIconText(volumeTab), *banner, QStringLiteral(":/icons/status_warning.svg")));
  auto unavailable = test::application::editableSnapshot();
  unavailable.uwfAvailable = false;
  configuration.resetSnapshot(unavailable);
  QVERIFY(test::waitForLayoutSettled(volumeTab));
  const auto unavailableFrame = test::captureInlineIconText(volumeTab);
  int unavailableStatesMeasured = 0;
  for (auto states = volumeTab.findChildren<ui::ContentState*>(); auto* state : std::as_const(states))
    if (state->isVisibleTo(&volumeTab)) {
      QVERIFY(sampleStacked(volumeTab, unavailableFrame, *state, ui::I18n::tr("UWF is currently unavailable"), QStringLiteral(":/icons/enhanced.svg")));
      ++unavailableStatesMeasured;
    }
  QCOMPARE(unavailableStatesMeasured, 1);
  QVERIFY(probe(volumeTab, unavailableFrame, QStringLiteral("configuration-volume-unavailable")));
  volumeTab.hide();
  for (int kind = 0; kind < 3; ++kind) {
    const bool registry = kind == 2;
    const bool directory = kind == 1;
    const auto path = registry    ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\AlignmentAudit")
                      : directory ? QStringLiteral("C:\\Data\\AuditFolder")
                                  : QStringLiteral("C:\\Data\\audit.db");
    auto files = std::make_shared<test::application::ReadOnlyFileBrowser>();
    files->entries.emplace(path, app::FileBrowserEntry{path, directory ? QStringLiteral("AuditFolder") : QStringLiteral("audit.db"), directory});
    auto editor = std::make_shared<app::ExclusionEditor>(registry ? app::ExclusionEditor::Kind::Registry : app::ExclusionEditor::Kind::File, nullptr, files);
    auto exclusions = std::make_shared<presentation::ExclusionListViewModel>(editor);
    exclusions->setDriveLetter(QStringLiteral("C:"));
    exclusions->setBaseline(registry ? QStringList{path} : QStringList{}, {path});
    exclusions->setCommitAvailability(true);
    if (registry) exclusions->setPersistBaseline(true, true, true, true);
    ui::ExclusionListWidget widget(exclusions, test::localFileDialogs());
    widget.resize(width, 430);
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));
    auto* entries = widget.findChild<QListView*>(QStringLiteral("exclusionList"));
    QVERIFY(entries);
    const auto selectPath = [&] {
      for (int row = 0; row < entries->model()->rowCount(); ++row) {
        const auto index = entries->model()->index(row, 0);
        if (index.data(presentation::PathRole).toString() != path) continue;
        entries->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        return true;
      }
      return false;
    };
    QVERIFY(selectPath());
    QVERIFY(test::waitForLayoutSettled(widget));
    if (!registry) QTRY_VERIFY(exclusions->fileCommitState(path).enabled);
    const QString name = registry ? QStringLiteral("registry") : directory ? QStringLiteral("folder") : QStringLiteral("file");
    QVERIFY(probe(widget, test::captureInlineIconText(widget), QStringLiteral("configuration-%1-exclusions").arg(name), true));
    QVERIFY(exclusions->removeSelection({{path, std::nullopt}}));
    QVERIFY(selectPath());
    QVERIFY(test::waitForLayoutSettled(widget));
    QVERIFY(probe(widget, test::captureInlineIconText(widget), QStringLiteral("configuration-%1-removed").arg(name), true));
  }
  auto importer = std::make_shared<presentation::ImportViewModel>();
  importer->setApplier([](const QList<app::UwfmgrCommand>& commands) {
    QList<app::ImportResult> result;
    for (const auto& command : commands) {
      app::ImportResult item;
      item.lineNo = command.sourceLineNo;
      item.lineText = QString::fromStdString(command.rawLine);
      item.status = app::ImportResult::Status::Failed;
      result.append(item);
    }
    return result;
  });
  ui::ImportDialog importDialog(importer, test::localFileDialogs());
  importDialog.resize(width, 754);
  importDialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&importDialog));
  QVERIFY(test::waitForLayoutSettled(importDialog));
  const auto importFrame = test::captureInlineIconText(importDialog);
  QVERIFY(probe(importDialog, importFrame, QStringLiteral("configuration-import-before-report")));
  for (const auto& names : {std::pair{"importEditorIcon", "importEditorTitle"}, std::pair{"importDraftHintIcon", "importDraftHint"},
                            std::pair{"importFooterIcon", "importFooterText"}}) {
    const QString resource = QString::fromLatin1(names.first).contains(QStringLiteral("Editor"))   ? QStringLiteral(":/icons/code.svg")
                             : QString::fromLatin1(names.first).contains(QStringLiteral("Footer")) ? QStringLiteral(":/icons/lock.svg")
                                                                                                   : QStringLiteral(":/icons/info.svg");
    QVERIFY(samples.sample(importDialog, importFrame, importDialog.findChild<QWidget*>(QLatin1String(names.first)),
                           importDialog.findChild<ui::TextLabel*>(QLatin1String(names.second)), resource, Color::Muted, Color::Muted, true));
  }
  auto* input = importDialog.findChild<ui::CodeEditor*>(QStringLiteral("importTextEdit"));
  auto* importButton = importDialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(input && importButton);
  input->setPlainText(QStringLiteral("uwfmgr filter enable"));
  importButton->click();
  QVERIFY(test::waitForLayoutSettled(importDialog));
  const auto reportFrame = test::captureInlineIconText(importDialog);
  const std::array resources{":/icons/status_success.svg", ":/icons/copy.svg", ":/icons/info.svg", ":/icons/status_error.svg"};
  const std::array tones{Color::Success, Color::Muted, Color::Warning, Color::Danger};
  for (std::size_t index = 0; index < resources.size(); ++index)
    QVERIFY(samples.sample(importDialog, reportFrame, importDialog.findChild<QWidget*>(QStringLiteral("importSummaryIcon%1").arg(index)),
                           importDialog.findChild<ui::TextLabel*>(QStringLiteral("importSummaryText%1").arg(index)), QString::fromLatin1(resources[index]),
                           tones[index], tones[index], false));
  QVERIFY(probe(importDialog, reportFrame, QStringLiteral("configuration-import")));
  importDialog.hide();
  class Actions final : public app::ConfigurationActions {
   public:
    ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override { throw std::logic_error("Pixel audits must not apply"); }
  };
  app::ConfigurationPlan plan;
  plan.snapshot = test::application::editableSnapshot();
  plan.policySnapshot = test::application::diskPersistentPolicy();
  plan.changes.setFilterEnabled = false;
  plan.changes.setOverlay.maximumSizeMb = 8192;
  plan.changes.addFileExclusions["C:"] = {"\\Data"};
  plan.changes.addRegistryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\AlignmentAudit"};
  plan.changes.setPersistTSCAL = true;
  plan.overlayPolicy.passthrough = false;
  plan.overlayPolicy.reset = app::PersistentReset::Clear;
  plan.staging = app::FileStagingChanges{};
  plan.staging->after = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\audit.txt")}};
  plan.maintenance.hormEnabled = false;
  auto reviewModel = std::make_shared<presentation::ApplyPlanViewModel>(plan, std::make_shared<Actions>());
  ui::ApplyPlanDialog review(reviewModel, test::localFileDialogs());
  review.resize(width, 754);
  review.show();
  QVERIFY(QTest::qWaitForWindowExposed(&review));
  auto* list = review.findChild<QListView*>(QStringLiteral("applyChangesList"));
  QVERIFY(list);
  for (int row = 0; row < reviewModel->rowCount(); ++row) {
    const auto index = reviewModel->index(row);
    list->scrollTo(index, QAbstractItemView::PositionAtCenter);
    QVERIFY(test::waitForLayoutSettled(review));
    if (index.data(presentation::ApplyPlanViewModel::KindRole).toInt() == presentation::ApplyPlanViewModel::Heading) {
      ui::TextLabel* title = nullptr;
      for (auto candidates = review.findChildren<ui::TextLabel*>(QStringLiteral("applyPlanGroupTitle")); auto* candidate : std::as_const(candidates))
        if (candidate->isVisibleTo(&review) && candidate->text() == index.data().toString()) title = candidate;
      QVERIFY(title);
      QVERIFY(samples.sample(review, test::captureInlineIconText(review), title->parentWidget()->findChild<QWidget*>(QStringLiteral("applyPlanGroupIcon")),
                             title, index.data(Qt::DecorationRole).toString(), Color::Muted, Color::Text, false));
    } else if (!index.data(presentation::ApplyPlanViewModel::PathRole).toBool() &&
               index.data(presentation::ApplyPlanViewModel::KindRole).toInt() == presentation::ApplyPlanViewModel::Entry) {
      bool measured = false;
      const auto frame = test::captureInlineIconText(review);
      for (auto arrows = review.findChildren<QWidget*>(QStringLiteral("applyPlanChangeArrow")); auto* arrow : std::as_const(arrows)) {
        if (!arrow->isVisibleTo(&review)) continue;
        auto* owner = arrow->parentWidget();
        while (owner && owner->parentWidget() != list->viewport()) owner = owner->parentWidget();
        if (!owner) continue;
        if (owner->geometry().top() != list->visualRect(index).top()) continue;
        auto* before = owner->findChild<ui::TextLabel*>(QStringLiteral("applyPlanBefore"));
        auto* after = owner->findChild<ui::TextLabel*>(QStringLiteral("applyPlanAfter"));
        if (!before || !after || before->text() != index.data(presentation::ApplyPlanViewModel::BeforeRole).toString() ||
            after->text() != index.data(presentation::ApplyPlanViewModel::AfterRole).toString())
          continue;
        if (!before->text().isEmpty())
          QVERIFY(samples.sample(review, frame, arrow, before, QStringLiteral(":/icons/arrow.svg"), Color::Faint, Color::Faint, false));
        if (!after->text().isEmpty())
          QVERIFY(samples.sample(review, frame, arrow, after, QStringLiteral(":/icons/arrow.svg"), Color::Faint, Color::Text, false));
        measured = true;
      }
      QVERIFY(measured);
    }
  }
  const auto reviewFrame = test::captureInlineIconText(review);
  QVERIFY(probe(review, reviewFrame, QStringLiteral("configuration-apply-review")));
  int badgesMeasured = 0;
  for (auto badges = review.findChildren<ui::TextLabel*>(); auto* candidate : std::as_const(badges)) {
    auto* badge = dynamic_cast<ui::StatusBadge*>(candidate);
    if (!badge || badge->objectName() != QStringLiteral("applyPlanSummaryBadge") || !badge->isVisibleTo(&review)) continue;
    const auto& frame = reviewFrame;
    const qreal dpr = frame.devicePixelRatio();
    const auto runs = test::inlineInkRuns(frame, badge->rect().adjusted(3, 3, -3, -3).translated(badge->mapTo(&review, QPoint())), qCeil(3 * dpr));
    QVERIFY(runs.size() >= 2);
    const bool rtl = badge->layoutDirection() == Qt::RightToLeft;
    const auto icon = rtl ? runs.back() : runs.front();
    QRectF text;
    for (qsizetype run = rtl ? 0 : 1; run < runs.size() - (rtl ? 1 : 0); ++run) text = text.united(runs.at(run));
    const qreal delta = qAbs(qRound((icon.center().y() - text.center().y()) * dpr * 2)) / 2.0;
    const auto capture = QStringLiteral("configuration-%1-%2").arg(QString::fromLatin1(QTest::currentDataTag())).arg(++captureIndex);
    measurements.append(QJsonObject{{"resource", ":/icons/info.svg"},
                                    {"widget", "apply-plan-summary-badge"},
                                    {"reference", badge->text()},
                                    {"rule", "visible-center"},
                                    {"dpr", dpr},
                                    {"graphicCenterPhysical", icon.center().y() * dpr},
                                    {"textCenterPhysical", text.center().y() * dpr},
                                    {"centerDifferencePhysical", delta},
                                    {"nonempty", true},
                                    {"capture", capture}});
    if (const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR"); !directory.isEmpty()) {
      QVERIFY(frame.save(QDir(directory).filePath(capture + QStringLiteral(".png"))));
      QFile output(QDir(directory).filePath(QStringLiteral("configuration-%1.json").arg(QString::fromLatin1(QTest::currentDataTag()))));
      QVERIFY(output.open(QIODevice::WriteOnly));
      QVERIFY(output.write(QJsonDocument(measurements).toJson()) > 0);
    }
    QVERIFY2(delta <= 1, qUtf8Printable(QStringLiteral("Actual review status badge differs by %1 physical pixels").arg(delta)));
    ++badgesMeasured;
  }
  QCOMPARE(badgesMeasured, 1);
  review.hide();
  plan.changes = {};
  plan.overlayPolicy = {};
  plan.staging.reset();
  plan.maintenance = {};
  auto emptyModel = std::make_shared<presentation::ApplyPlanViewModel>(plan, std::make_shared<Actions>());
  ui::ApplyPlanDialog emptyReview(emptyModel, test::localFileDialogs());
  emptyReview.resize(width, 754);
  emptyReview.show();
  QVERIFY(QTest::qWaitForWindowExposed(&emptyReview));
  QVERIFY(test::waitForLayoutSettled(emptyReview));
  auto* empty = emptyReview.findChild<ui::ContentState*>();
  QVERIFY(empty);
  const auto emptyFrame = test::captureInlineIconText(emptyReview);
  QVERIFY(sampleStacked(emptyReview, emptyFrame, *empty, ui::I18n::tr("No pending changes"), QStringLiteral(":/icons/status_success.svg")));
  QVERIFY(probe(emptyReview, emptyFrame, QStringLiteral("configuration-empty-review")));
  emptyReview.hide();
  class RecordedActions final : public app::ConfigurationActions {
   public:
    ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override { throw std::logic_error("Expected the reviewed plan"); }
    ApplyOperation prepareApply(const app::ConfigurationPlan& reviewed) override {
      return [operations = reviewed.operations()] {
        app::ConfigurationApplyResult result;
        for (const auto& operation : operations)
          result.steps.push_back({"Pixel audit fixture", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed, operation});
        return result;
      };
    }
  };
  core::PendingChanges appliedChange;
  appliedChange.setFilterEnabled = false;
  auto appliedModel =
      std::make_shared<presentation::ApplyPlanViewModel>(appliedChange, test::application::editableSnapshot(), std::make_shared<RecordedActions>());
  ui::ApplyPlanDialog appliedReview(appliedModel, test::localFileDialogs());
  appliedReview.resize(width, 754);
  appliedReview.show();
  QVERIFY(QTest::qWaitForWindowExposed(&appliedReview));
  QVERIFY(appliedModel->canApply());
  appliedModel->apply();
  QTRY_VERIFY(!appliedModel->applying() && appliedModel->canRestart());
  QVERIFY(test::waitForLayoutSettled(appliedReview));
  auto* safeRestart = appliedReview.findChild<QPushButton*>(QStringLiteral("restartBtn"));
  QVERIFY(safeRestart && safeRestart->isVisibleTo(&appliedReview));
  const auto appliedFrame = test::captureInlineIconText(appliedReview);
  QVERIFY(probe(appliedReview, appliedFrame, QStringLiteral("configuration-applied-review")));
  auto* resultNotice = appliedReview.findChild<ui::Notice*>(QStringLiteral("applyResultNotice"));
  QVERIFY(resultNotice && resultNotice->isVisibleTo(&appliedReview));
  QVERIFY(sampleNotice(appliedReview, appliedFrame, *resultNotice, QStringLiteral(":/icons/status_success.svg")));
}

void WidgetConfigurationTests::configurationAvailabilitySurvivesLanguageTransitions_data() {
  QTest::addColumn<bool>("available");
  QTest::addColumn<bool>("elevated");
  QTest::addColumn<bool>("filter");
  QTest::addColumn<bool>("protectedVolume");
  QTest::addColumn<bool>("horm");
  for (const bool available : {false, true})
    for (const bool elevated : {false, true})
      for (const bool filter : {false, true})
        for (const bool protectedVolume : {false, true})
          for (const bool horm : {false, true}) {
            const auto name = QStringLiteral("available-%1/elevated-%2/filter-%3/protected-%4/horm-%5")
                                  .arg(available)
                                  .arg(elevated)
                                  .arg(filter)
                                  .arg(protectedVolume)
                                  .arg(horm);
            QTest::newRow(qUtf8Printable(name)) << available << elevated << filter << protectedVolume << horm;
          }
}

void WidgetConfigurationTests::configurationAvailabilitySurvivesLanguageTransitions() {
  using namespace uwf;
  QFETCH(bool, available);
  QFETCH(bool, elevated);
  QFETCH(bool, filter);
  QFETCH(bool, protectedVolume);
  QFETCH(bool, horm);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto restore = qScopeGuard([&] { ui::I18n::instance().setLang(previousLanguage); });
  ui::I18n::instance().setLang(ui::I18n::Lang::En);
  presentation::ConfigurationViewModel configuration(16384);
  auto volume = configuration.addVolume(
      core::VolumeInfo{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}, true);
  ui::VolumeTab tab(volume, test::localFileDialogs(), {}, {});
  auto globalModel = configuration.global();
  ui::GlobalStatusPanel global(globalModel);
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = available;
  snapshot.elevated = elevated;
  snapshot.current.filter.enabled = filter;
  snapshot.current.overlay = {core::OverlayType::RAM, 2048, 1024, 1800};
  snapshot.current.volumes = {{uwf::test::application::fixtureVolumeCName, "C:", protectedVolume, core::VolumeBinding::VolumeId}};
  snapshot.next = snapshot.current;
  snapshot.runtime = {.availableSpaceMb = 1500, .currentConsumptionMb = 548, .romDisposition = 0, .fullCommit = false,
                      .hormEnabled = horm, .currentOverlayFlags = 0, .systemVolumeProtected = protectedVolume};
  configuration.resetSnapshot(snapshot, {}, horm);
  auto* commit = tab.findChild<QPushButton*>(QStringLiteral("volumeCommitButton"));
  auto* overlay = buttonWithText(&tab, ui::I18n::tr("View overlay"));
  auto* protection = tab.findChild<ui::SwitchButton*>(QStringLiteral("volumeProtectionNext"));
  auto* status = global.findChild<QLabel*>(QStringLiteral("statusCurLabel"));
  auto* usage = global.findChild<QLabel*>(QStringLiteral("statusCurrent"));
  auto* type = global.findChild<QComboBox*>(QStringLiteral("overlayTypeNext"));
  auto* maximum = global.findChild<QDoubleSpinBox*>(QStringLiteral("overlayMaximumNext"));
  auto* warning = global.findChild<QDoubleSpinBox*>(QStringLiteral("overlayWarningNext"));
  QVERIFY(commit && overlay && protection && status && usage && type && maximum && warning);
  QSignalSpy globalEdits(globalModel.get(), &presentation::GlobalConfigurationViewModel::pendingChanged);
  QSignalSpy volumeEdits(volume.get(), &presentation::VolumeConfigurationViewModel::pendingChanged);
  // Each business state owns one widget pair; exercise every language and its
  // return transition on those same live bindings without rebuilding the UI.
  for (const auto& locale : ui::I18n::languages()) {
    const auto initialLanguage = locale.id == ui::I18n::Lang::En ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En;
    for (const auto language : {initialLanguage, locale.id, initialLanguage}) {
      ui::I18n::instance().setLang(language);
      const bool editable = available && elevated && !horm;
      const bool hasOverlay = available && elevated && filter && protectedVolume;
      QCOMPARE(protection->isEnabled(), editable);
      QCOMPARE(commit->isEnabled(), hasOverlay && !horm);
      QCOMPARE(overlay->isEnabled(), hasOverlay);
      const auto commitHint =
          !available ? ui::I18n::tr("UWF is currently unavailable") : ui::I18n::tr("Review available commit operations and their requirements.");
      const auto overlayHint = !available         ? ui::I18n::tr("UWF is currently unavailable")
                               : !elevated        ? ui::I18n::tr("Read-only")
                               : !filter          ? ui::I18n::tr("The UWF filter is disabled; no volume has an overlay to inspect.")
                               : !protectedVolume ? ui::I18n::tr("This volume is not currently protected by UWF, so it has no overlay to inspect.")
                                                  : ui::I18n::tr("Open a diagnostic view of files currently cached in the overlay for this volume.");
      QCOMPARE(commit->toolTip(), commitHint);
      QCOMPARE(overlay->toolTip(), overlayHint);
      auto* fileCommit = actionWithText(&tab, ui::I18n::tr("Commit file changes…"));
      auto* registryCommit = actionWithText(&tab, ui::I18n::tr("Commit registry changes…"));
      QVERIFY(fileCommit && registryCommit);
      QCOMPARE(fileCommit->isEnabled(), editable && filter && protectedVolume);
      QCOMPARE(registryCommit->isEnabled(), editable && filter && protectedVolume);
      QCOMPARE(status->text(), !available ? ui::I18n::tr("Unavailable") : filter ? ui::I18n::tr("Enabled") : ui::I18n::tr("Disabled"));
      QCOMPARE(warning->isEnabled(), editable);
      QCOMPARE(type->isEnabled(), editable && !filter);
      QCOMPARE(maximum->isEnabled(), editable && !filter);
      const auto typeHint =
          !editable ? ui::I18n::tr("Read-only")
          : available && filter
              ? ui::I18n::tr("To change the overlay type or maximum capacity:\n1. Disable UWF.\n2. Restart the device.\n3. Change this setting.")
              : ui::I18n::tr(
                    "RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay "
                    "data across restarts.");
      QCOMPARE(type->toolTip(), typeHint);
      if (!available || !filter) QCOMPARE(usage->text(), ui::I18n::tr(available ? "Filter is disabled" : "Usage unavailable"));
    }
  }
  QCOMPARE(globalEdits.count(), 0);
  QCOMPARE(volumeEdits.count(), 0);
}

void WidgetConfigurationTests::exclusionActionsFollowSelectionAndLanguage_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("registry");
  QTest::addColumn<bool>("readOnly");
  QTest::addColumn<bool>("removed");
  QTest::addColumn<bool>("selected");
  for (const auto& language : uwf::ui::I18n::languages())
    for (const bool registry : {false, true})
      for (const bool readOnly : {false, true})
        for (const bool removed : {false, true})
          for (const bool selected : {false, true})
            QTest::newRow(qUtf8Printable(QStringLiteral("%1/registry-%2/readonly-%3/removed-%4/selected-%5")
                                             .arg(QString(language.code))
                                             .arg(registry)
                                             .arg(readOnly)
                                             .arg(removed)
                                             .arg(selected)))
                << language.id << registry << readOnly << removed << selected;
}

void WidgetConfigurationTests::exclusionActionsFollowSelectionAndLanguage() {
  using namespace uwf;
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, registry);
  QFETCH(bool, readOnly);
  QFETCH(bool, removed);
  QFETCH(bool, selected);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto restore = qScopeGuard([&] { ui::I18n::instance().setLang(previousLanguage); });
  const auto initialLanguage = locale == ui::I18n::Lang::En ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En;
  ui::I18n::instance().setLang(initialLanguage);
  auto model = std::make_shared<presentation::ExclusionListViewModel>(registry ? presentation::ExclusionListViewModel::Kind::Registry
                                                                               : presentation::ExclusionListViewModel::Kind::File);
  const auto path = registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Example") : QStringLiteral("C:\\Data\\state.db");
  model->setDriveLetter(QStringLiteral("C:"));
  model->setBaseline({path}, {path});
  if (removed) QVERIFY(model->removeSelection({{path, std::nullopt}}));
  ui::ExclusionListWidget widget(model, test::localFileDialogs());
  auto* list = widget.findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* filter = widget.findChild<QLineEdit*>(QStringLiteral("filterInput"));
  auto* add = widget.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* remove = widget.findChild<QPushButton*>(QStringLiteral("dangerBtn"));
  QVERIFY(list && filter && add && remove);
  QCOMPARE(list->model()->rowCount(), 1);
  if (selected) list->selectionModel()->select(list->model()->index(0, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
  for (const auto language : {initialLanguage, locale, initialLanguage}) {
    ui::I18n::instance().setLang(language);
    for (const bool locked : {readOnly, !readOnly, readOnly}) {
      widget.setReadOnly(locked);
      QCOMPARE(add->isEnabled(), !locked);
      QCOMPARE(remove->isEnabled(), !locked && selected);
      QVERIFY(list->isEnabled() && filter->isEnabled());
      QCOMPARE(remove->accessibleName(), ui::I18n::tr(removed && selected ? "Undo removal" : "Remove selected"));
      const auto removeHint = locked      ? ui::I18n::tr("Read-only")
                              : !selected ? ui::I18n::tr("Select entries to remove.")
                              : removed   ? ui::I18n::tr("Undo removal")
                                          : ui::I18n::tr("Remove the selected entries from the exclusion list. Takes effect after Apply.");
      QCOMPARE(remove->toolTip(), removeHint);
      QCOMPARE(add->toolTip(), locked     ? ui::I18n::tr("Read-only")
                               : registry ? ui::I18n::tr("Add a registry key to the exclusion list, or enable a persistence switch.")
                                          : ui::I18n::tr("Add a file or folder to the exclusion list."));
      auto* action = actionWithText(&widget, ui::I18n::tr(registry ? "Registry key…" : "File…"));
      QVERIFY(action);
      QCOMPARE(action->isEnabled(), !locked);
    }
  }
}

void WidgetConfigurationTests::stagingActionsFollowStorageSelectionAndLanguage_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("readOnly");
  QTest::addColumn<bool>("removed");
  QTest::addColumn<bool>("selected");
  for (const auto& language : uwf::ui::I18n::languages())
    for (const bool readOnly : {false, true})
      for (const bool removed : {false, true})
        for (const bool selected : {false, true})
          QTest::newRow(
              qUtf8Printable(QStringLiteral("%1/readonly-%2/removed-%3/selected-%4").arg(QString(language.code)).arg(readOnly).arg(removed).arg(selected)))
              << language.id << readOnly << removed << selected;
}

void WidgetConfigurationTests::stagingActionsFollowStorageSelectionAndLanguage() {
  using namespace uwf;
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, readOnly);
  QFETCH(bool, removed);
  QFETCH(bool, selected);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto restore = qScopeGuard([&] { ui::I18n::instance().setLang(previousLanguage); });
  const auto initialLanguage = locale == ui::I18n::Lang::En ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En;
  ui::I18n::instance().setLang(initialLanguage);
  const QString path = QStringLiteral("C:\\Data\\state.db");
  auto store = std::make_shared<test::application::MemoryFileStagingStore>();
  store->entries = {{app::FileStagingKind::File, path}};
  store->readFailure = QStringLiteral("storage unavailable");
  app::FileStagingDraft draft(store);
  auto editor = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, std::make_shared<test::commit::Tree>(false));
  auto model = std::make_shared<presentation::FileStagingViewModel>(editor);
  ui::FileStagingWidget widget(model, test::localFileDialogs());
  QTRY_VERIFY(!model->busy());
  auto* list = uwf::test::stagingEntryList(widget);
  auto* filter = widget.findChild<QLineEdit*>(QStringLiteral("filterInput"));
  auto* add = widget.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* remove = widget.findChild<QPushButton*>(QStringLiteral("dangerBtn"));
  QVERIFY(list && filter && add && remove);
  for (const auto language : {initialLanguage, locale, initialLanguage}) {
    ui::I18n::instance().setLang(language);
    QVERIFY(!list->isEnabled() && !filter->isEnabled() && !add->isEnabled() && !remove->isEnabled());
    const auto hint = ui::I18n::tr("File staging is unavailable: %1").arg(store->readFailure);
    QCOMPARE(add->toolTip(), hint);
    QCOMPARE(remove->toolTip(), hint);
  }
  store->readFailure.clear();
  model->load();
  QTRY_VERIFY(!model->busy());
  QVERIFY(model->storageAvailable());
  QCOMPARE(list->model()->rowCount(), 1);
  if (removed) {
    editor->removePaths({path});
    QCoreApplication::sendPostedEvents(editor.get(), QEvent::MetaCall);
    QCOMPARE(list->model()->rowCount(), 1);
  }
  if (selected) list->selectionModel()->select(list->model()->index(0, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
  for (const auto language : {initialLanguage, locale, initialLanguage}) {
    ui::I18n::instance().setLang(language);
    for (const bool locked : {readOnly, !readOnly, readOnly}) {
      model->setAvailability({.readOnly = locked});
      QVERIFY(list->isEnabled() && filter->isEnabled());
      QCOMPARE(add->isEnabled(), !locked);
      QCOMPARE(remove->isEnabled(), !locked && selected);
      QCOMPARE(remove->accessibleName(), ui::I18n::tr(removed && selected ? "Undo removal" : "Remove selected"));
      QCOMPARE(add->toolTip(), ui::I18n::tr(locked ? "Read-only" : "Add files or a folder to the automatic commit list."));
      const auto removeHint = locked      ? ui::I18n::tr("Read-only")
                              : !selected ? ui::I18n::tr("Select entries to remove.")
                              : removed   ? ui::I18n::tr("Undo removal")
                                          : ui::I18n::tr("Mark selected paths for removal. Takes effect after Apply.");
      QCOMPARE(remove->toolTip(), removeHint);
    }
  }
  QCOMPARE(store->writes, 0);
}

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
    for (auto hints = list->findChildren<ui::ContentHintBox*>(); auto* text : std::as_const(hints))
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
    QTRY_COMPARE(list->verticalScrollBar()->isVisible(), list->verticalScrollBar()->maximum() > 0);
    QTRY_VERIFY_WITH_TIMEOUT(rowsAreContiguous(), 1500);
    auto* row = rootText()->parentWidget();
    ui::StatusBadge* badge = nullptr;
    for (auto* child : row->children())
      if (auto* candidate = dynamic_cast<ui::StatusBadge*>(child)) badge = candidate;
    QVERIFY(badge && badge->isVisibleTo(list));
    QCOMPARE(badge->text(), ui::I18n::tr("Built-in"));
    QTRY_VERIFY(badge->width() >= badge->sizeHint().width());
    ui::TextLabel* description = nullptr;
    for (auto labels = row->findChildren<ui::TextLabel*>(); auto* label : std::as_const(labels))
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
  uwf::presentation::ConfigurationViewModel configuration(8192);
  GlobalStatusPanel panel(configuration.global());
  panel.show();
  QVERIFY(QTest::qWaitForWindowExposed(&panel));
  uwf::core::SessionSnapshot current;
  current.overlay.maximumSizeMb = 4096;
  current.filter.enabled = true;
  configuration.resetSnapshot(uwf::test::application::configurationSnapshot(current, current, {3072, 1024}));
  auto* icon = panel.findChild<QWidget*>(QStringLiteral("currentSessionStateIcon"));
  auto* label = panel.findChild<QLabel*>(QStringLiteral("statusCurLabel"));
  QVERIFY(icon && label);
  const auto enabled = icon->grab().toImage();
  QCOMPARE(label->text(), I18n::tr("Enabled"));
  current.filter.enabled = false;
  configuration.resetSnapshot(uwf::test::application::configurationSnapshot(current, current, {3072, 1024}));
  const auto disabled = icon->grab().toImage();
  QVERIFY(enabled != disabled);
  QCOMPARE(label->text(), I18n::tr("Disabled"));
  auto* usage = panel.findChild<QLabel*>(QStringLiteral("statusCurrent"));
  QVERIFY(usage);
  QCOMPARE(usage->text(), I18n::tr("Filter is disabled"));
  QVERIFY(panel.usageWarningText().isEmpty());
  panel.viewModel().setAvailable(false);
  QVERIFY(uwf::test::changedCoverage(disabled, icon->grab().toImage(), icon->rect()) > 0);
  QCOMPARE(label->text(), I18n::tr("Unavailable"));
  QCOMPARE(usage->text(), I18n::tr("Usage unavailable"));
}

void WidgetConfigurationTests::globalStatusEditsPublishOnlyConstrainedValues() {
  uwf::presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  uwf::ui::GlobalStatusPanel panel(configuration.global());
  uwf::core::SessionSnapshot session;
  session.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  configuration.resetSnapshot(uwf::test::application::configurationSnapshot(session, session));
  auto* maximum = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayMaximumNext"));
  auto* warning = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayWarningNext"));
  auto* critical = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayCriticalNext"));
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
  uwf::presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  uwf::ui::GlobalStatusPanel panel(configuration.global());
  uwf::core::SessionSnapshot session;
  session.overlay = {OverlayType::RAM, 512, 256, 384};
  configuration.resetSnapshot(uwf::test::application::configurationSnapshot(session, session));
  auto* type = panel.findChild<QComboBox*>(QStringLiteral("overlayTypeNext"));
  auto* maximum = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayMaximumNext"));
  auto* warning = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayWarningNext"));
  auto* critical = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayCriticalNext"));
  QVERIFY(type && maximum && warning && critical);
  type->setCurrentIndex(type->findData(static_cast<int>(OverlayType::Disk)));
  QCOMPARE(maximum->minimum(), 1024);
  QCOMPARE(maximum->value(), 1024);
  QCOMPARE(critical->value(), 384);
  QCOMPARE(warning->value(), 256);
  type->setCurrentIndex(type->findData(static_cast<int>(OverlayType::RAM)));
  QCOMPARE(maximum->minimum(), 1);
  QCOMPARE(maximum->value(), 1024);

  // An intermediate imported type must not clamp the final Disk capacity.
  session.overlay = {OverlayType::Disk, 2048, 256, 384};
  configuration.resetSnapshot(uwf::test::application::configurationSnapshot(session, session));
  QCOMPARE((panel.viewModel().importOverlayMaxMb(1000000)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((panel.viewModel().importOverlayType(OverlayType::RAM)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((panel.viewModel().importOverlayType(OverlayType::Disk)).outcome, uwf::app::GlobalEditOutcome::Applied);
  panel.viewModel().finishImport();
  QCOMPARE(maximum->value(), 1000000);
  QCOMPARE(warning->value(), 256);
  QCOMPARE(critical->value(), 384);
  QCOMPARE(panel.viewModel().pendingOverlay().maximumSizeMb, std::optional<uint32_t>(1000000));
  QVERIFY(!panel.viewModel().pendingOverlay().type.has_value());

  QCOMPARE((panel.viewModel().importOverlayMaxMb(512)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((panel.viewModel().importOverlayType(OverlayType::RAM)).outcome, uwf::app::GlobalEditOutcome::Applied);
  panel.viewModel().finishImport();
  QCOMPARE(maximum->value(), 512);
  QCOMPARE(warning->value(), 256);
  QCOMPARE(critical->value(), 384);
}

void WidgetConfigurationTests::globalStatusThresholdEventsCanBeDisabledIndependently() {
  uwf::presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  uwf::ui::GlobalStatusPanel panel(configuration.global());
  uwf::core::SessionSnapshot session;
  auto* maximum = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayMaximumNext"));
  auto* warning = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayWarningNext"));
  auto* critical = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayCriticalNext"));
  QVERIFY(maximum && warning && critical);
  for (const auto [warningValue, criticalValue] :
       {std::pair{512u, 1024u}, std::pair{1536u, 2048u}, std::pair{512u, 0u}, std::pair{0u, 768u}, std::pair{0u, 0u}}) {
    session.overlay = {uwf::core::OverlayType::RAM, 1024, warningValue, criticalValue};
    configuration.resetSnapshot(uwf::test::application::configurationSnapshot(session, session));
    QCOMPARE(maximum->value(), 1024);
    QCOMPARE(warning->value(), static_cast<double>(warningValue));
    QCOMPARE(critical->value(), static_cast<double>(criticalValue));
    QVERIFY(panel.viewModel().pendingOverlay().empty());
    panel.viewModel().finishImport();
    QVERIFY(panel.viewModel().pendingOverlay().empty());
  }
  session.overlay = {uwf::core::OverlayType::RAM, 4096, 512, 768};
  configuration.resetSnapshot(uwf::test::application::configurationSnapshot(session, session));
  critical->setValue(0);
  QCOMPARE(warning->value(), 512);
  QCOMPARE(warning->maximum(), static_cast<double>(std::numeric_limits<uint32_t>::max()));
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

void WidgetConfigurationTests::globalStatusShowsObservedValuesWithoutEditing_data() {
  QTest::addColumn<uint32_t>("maximumValue");
  QTest::addColumn<uint32_t>("warningValue");
  QTest::addColumn<uint32_t>("criticalValue");
  QTest::addColumn<bool>("readOnly");
  struct Observation {
    const char* label;
    uint32_t maximum;
    uint32_t warning;
    uint32_t critical;
  };
  constexpr auto maximum = std::numeric_limits<uint32_t>::max();
  constexpr std::array observations{Observation{"ordinary", 1024, 512, 768},
                                    Observation{"warning-above-critical", 1024, 500, 400},
                                    Observation{"equal-thresholds", 1024, 500, 500},
                                    Observation{"disabled-critical", 1024, maximum, 0},
                                    Observation{"above-int-max", 1024, uint32_t{1} << 31, maximum},
                                    Observation{"complete-uint32", maximum, maximum - 1, maximum}};
  for (const auto& observation : observations)
    for (const bool readOnly : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-readonly-%2").arg(QString::fromLatin1(observation.label)).arg(readOnly)))
          << observation.maximum << observation.warning << observation.critical << readOnly;
}

void WidgetConfigurationTests::globalStatusShowsObservedValuesWithoutEditing() {
  QFETCH(uint32_t, maximumValue);
  QFETCH(uint32_t, warningValue);
  QFETCH(uint32_t, criticalValue);
  QFETCH(bool, readOnly);
  uwf::presentation::ConfigurationViewModel configuration(8192);
  uwf::ui::GlobalStatusPanel panel(configuration.global());
  uwf::core::SessionSnapshot session;
  session.overlay = {uwf::core::OverlayType::Disk, maximumValue, warningValue, criticalValue};
  QSignalSpy pending(&panel.viewModel(), &uwf::presentation::GlobalConfigurationViewModel::pendingChanged);
  configuration.resetSnapshot(uwf::test::application::configurationSnapshot(session, session));
  panel.viewModel().setControlsEnabled(!readOnly);
  const auto* maximum = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayMaximumNext"));
  const auto* warning = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayWarningNext"));
  const auto* critical = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayCriticalNext"));
  QVERIFY(maximum && warning && critical);
  QCOMPARE(maximum->value(), static_cast<double>(maximumValue));
  QCOMPARE(warning->value(), static_cast<double>(warningValue));
  QCOMPARE(critical->value(), static_cast<double>(criticalValue));
  QCOMPARE(panel.viewModel().draft().overlay.maximumSizeMb, maximumValue);
  QCOMPARE(panel.viewModel().draft().overlay.warningThresholdMb, warningValue);
  QCOMPARE(panel.viewModel().draft().overlay.criticalThresholdMb, criticalValue);
  for (const auto* spin : {maximum, warning, critical}) {
    QCOMPARE(spin->decimals(), 0);
    QCOMPARE(spin->isEnabled(), !readOnly);
    QVERIFY(spin->width() >= spin->minimumSizeHint().width());
  }
  QCOMPARE(pending.count(), 0);
  QVERIFY(panel.viewModel().pendingOverlay().empty());
}

void WidgetConfigurationTests::globalStatusEditsCompleteUint32Thresholds() {
  uwf::presentation::ConfigurationViewModel configuration(8192);
  uwf::ui::GlobalStatusPanel panel(configuration.global());
  uwf::core::SessionSnapshot session;
  session.overlay = {uwf::core::OverlayType::RAM, 1024, 512, 0};
  configuration.resetSnapshot(uwf::test::application::configurationSnapshot(session, session));
  auto* warning = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayWarningNext"));
  auto* critical = panel.findChild<QDoubleSpinBox*>(QStringLiteral("overlayCriticalNext"));
  QVERIFY(warning && critical);
  panel.show();
  QVERIFY(QTest::qWaitForWindowExposed(&panel));
  warning->setFocus();
  warning->selectAll();
  QTest::keyClicks(warning, "4294967295");
  QCOMPARE(panel.viewModel().draft().overlay.warningThresholdMb, uint32_t{512});
  QTest::keyClick(warning, Qt::Key_Return);
  constexpr auto maximum = std::numeric_limits<uint32_t>::max();
  QCOMPARE(warning->value(), static_cast<double>(maximum));
  QCOMPARE(panel.viewModel().pendingOverlay().warningThresholdMb, std::optional<uint32_t>{maximum});
  warning->stepDown();
  QCOMPARE(warning->value(), static_cast<double>(maximum - 1));
  QCOMPARE(panel.viewModel().pendingOverlay().warningThresholdMb, std::optional<uint32_t>{maximum - 1});

  critical->setFocus();
  critical->selectAll();
  QTest::keyClicks(critical, "4294967296");
  QTest::keyClick(critical, Qt::Key_Return);
  QCOMPARE(critical->value(), static_cast<double>(maximum));
  QCOMPARE(panel.viewModel().pendingOverlay().criticalThresholdMb, std::optional<uint32_t>{maximum});
  // This is still the existing strict edit policy, independently of display.
  warning->setValue(maximum);
  QCOMPARE(warning->value(), static_cast<double>(maximum - 1));
  QCOMPARE(panel.viewModel().draft().overlay.warningThresholdMb, maximum - 1);
}

void WidgetConfigurationTests::exclusionListDoubleClickCopiesTheDisplayedFullPath() {
  uwf::ui::ExclusionListWidget list(std::make_shared<uwf::presentation::ExclusionListViewModel>(uwf::ui::ExclusionListWidget::Kind::File),
                                    uwf::test::localFileDialogs());
  list.viewModel().setDriveLetter(QStringLiteral("Z:"));
  list.viewModel().setBaseline({QStringLiteral("\\Data\\State.db")}, {QStringLiteral("\\Data\\State.db")});
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
  files.viewModel().setDriveLetter(QStringLiteral("Z:"));
  files.viewModel().setBaseline({QStringLiteral("Z:\\Alpha"), QStringLiteral("Z:\\Beta"), QStringLiteral("Z:\\Gamma")},
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
  QCOMPARE(files.viewModel().pendingRemoved().size(), 2);

  auto registryEditor = std::make_shared<uwf::app::ExclusionEditor>(uwf::app::ExclusionEditor::Kind::Registry);
  uwf::ui::ExclusionListWidget registry(std::make_shared<uwf::presentation::ExclusionListViewModel>(registryEditor), uwf::test::localFileDialogs());
  const QString stagingRegistryRoot = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager");
  bool fileStagingHasData = true;
  registryEditor->setFileStagingDataProbe([&fileStagingHasData] { return fileStagingHasData; });
  registry.viewModel().setBaseline({stagingRegistryRoot}, {stagingRegistryRoot});
  registry.viewModel().setPersistBaseline(true, true, false, false);
  registry.resize(620, 320);
  registry.show();
  QVERIFY(QTest::qWaitForWindowExposed(&registry));
  auto* enableTscal = actionWithText(&registry, QStringLiteral("Terminal Services license persistence"));
  QVERIFY(enableTscal);
  enableTscal->trigger();
  QCOMPARE(registry.viewModel().pendingPersistTSCAL(), std::optional<bool>(true));

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
    QCOMPARE(item.data(Qt::ToolTipRole).toString(), QStringLiteral("File staging relies on this registry exclusion to preserve its list across restarts.\n\n"
                                                                   "Current session: Excluded\nNext session: Excluded"));
    QVERIFY(!item.data(Qt::ToolTipRole).toString().contains(QStringLiteral("Registry:")));
  }
  QVERIFY(stagingRegistryObserved);
  QVERIFY(stagingRegistryItem.isValid());
  registryList->selectionModel()->select(stagingRegistryItem, QItemSelectionModel::Select);
  QVERIFY(!registryRemove->isEnabled());
  QTest::mouseClick(registryRemove, Qt::LeftButton);
  QCOMPARE(registry.viewModel().pendingRemoved().size(), 0);

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
  QVERIFY(!registry.viewModel().pendingPersistTSCAL().has_value());

  registryList->clearSelection();
  for (int row = 0; row < registryList->model()->rowCount(); ++row)
    registryList->selectionModel()->select(registryList->model()->index(row, 0), QItemSelectionModel::Select);
  QTest::mouseClick(registryRemove, Qt::LeftButton);
  QCOMPARE(registry.viewModel().pendingPersistDomainSecretKey(), std::optional<bool>(false));
}

void WidgetConfigurationTests::exclusionFileActionsUseThePlatformBoundaryAndPreserveValidation() {
  FileDialogStub files;
  files.openedFiles = {QStringLiteral("Z:/Data/one.db"), QStringLiteral("Y:/wrong.db"), QString()};
  files.selectedDirectory = QStringLiteral("Z:/Cache");
  uwf::ui::ExclusionListWidget list(std::make_shared<uwf::presentation::ExclusionListViewModel>(uwf::ui::ExclusionListWidget::Kind::File), files);
  list.viewModel().setDriveLetter(QStringLiteral("z:"));
  list.viewModel().setBaseline({}, {});

  auto* addFile = actionWithText(&list, QStringLiteral("File…"));
  auto* addFolder = actionWithText(&list, QStringLiteral("Folder…"));
  QVERIFY(addFile);
  QVERIFY(addFolder);

  bool warningSeen = false;
  QVERIFY(uwf::test::driveModalDialog(
      *this, [&] { addFile->trigger(); },
      [&](QDialog& dialog) {
        warningSeen = true;
        dialog.accept();
      }));
  QVERIFY(warningSeen);
  QCOMPARE(list.viewModel().pendingAdded(), QStringList({QStringLiteral("Z:\\Data\\one.db")}));
  QCOMPARE(files.requests.size(), 1);
  QCOMPARE(files.requests.front().initialPath, QStringLiteral("Z:\\"));

  addFolder->trigger();
  QCOMPARE(list.viewModel().pendingAdded(), QStringList({QStringLiteral("Z:\\Cache"), QStringLiteral("Z:\\Data\\one.db")}));
  QCOMPARE(files.requests.size(), 2);

  list.setReadOnly(true);
  files.openedFiles = {QStringLiteral("Z:/Blocked")};
  addFile->trigger();
  QCOMPARE(files.requests.size(), 2);
  QCOMPARE(list.viewModel().pendingAdded().size(), 2);
}

void WidgetConfigurationTests::importDialogFiltersCommentsAndAppendsBatchReports() {
  QVERIFY(uwf::app::parseErrorMessage(uwf::app::ParseError::MalformedQuoting, {}).contains(QStringLiteral("quote"), Qt::CaseInsensitive));
  QVERIFY(uwf::app::parseErrorMessage(uwf::app::ParseError::UnexpectedArgument, QStringLiteral("extra")).contains(QStringLiteral("extra")));

  auto model = std::make_shared<uwf::presentation::ImportViewModel>(uwf::windows::readBoundedTextFile);
  uwf::ui::ImportDialog dialog(model, uwf::test::localFileDialogs());
  auto* input = dialog.findChild<uwf::ui::CodeEditor*>(QStringLiteral("importTextEdit"));
  auto* importButton = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* report = dialog.findChild<QTableView*>();
  QVERIFY(input);
  QVERIFY(importButton);
  QVERIFY(report);
  QVERIFY(!importButton->isEnabled());

  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* hint = dialog.findChild<uwf::ui::TextLabel*>(QStringLiteral("importDraftHint"));
  auto* hintIcon = dialog.findChild<QWidget*>(QStringLiteral("importDraftHintIcon"));
  QVERIFY(hint && hintIcon);
  dialog.resize(760, 500);
  QTRY_VERIFY(uwf::test::centersAligned(QRectF(hintIcon->mapTo(&dialog, QPoint()), hintIcon->size()),
                                        hint->firstLineInkBounds().translated(hint->mapTo(&dialog, QPoint()))));

  QList<QList<uwf::app::UwfmgrCommand>> batches;
  model->setApplier([&](const QList<uwf::app::UwfmgrCommand>& commands) {
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
  model->setPreparing(true);
  QVERIFY(!importButton->isEnabled());
  QCOMPARE(importButton->text(), uwf::ui::I18n::tr("Reading state…"));
  model->importText();
  QVERIFY(batches.isEmpty());
  QCOMPARE(report->model()->rowCount(), 0);
  model->setPreparing(false);
  QVERIFY(importButton->isEnabled());
  QCOMPARE(importButton->text(), uwf::ui::I18n::tr("Import configuration"));
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
  QVERIFY(uwf::test::driveModalDialog(
      dialog, [&] { QTest::mouseClick(report->viewport(), Qt::LeftButton, Qt::NoModifier, report->visualRect(report->model()->index(0, 2)).center()); },
      [&](QDialog& modal) {
        auto* result = &modal;
        QVERIFY(result != &dialog);
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
      }));
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
  QCOMPARE(report->model()->rowCount(), 4);  // Two initial results, one batch separator and one later result.
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
  QVERIFY(uwf::test::driveModalDialog(*this, [&] { QTest::mouseClick(load, Qt::LeftButton); }, [](QDialog& warning) { warning.accept(); }));
  QCOMPARE(input->toPlainText(), text);

  QTemporaryFile oversized;
  QVERIFY(oversized.open());
  const QByteArray payload(5 * 1024 * 1024 + 1, 'x');
  QCOMPARE(oversized.write(payload), static_cast<qint64>(payload.size()));
  oversized.close();
  files.openedFiles = {oversized.fileName()};
  QVERIFY(uwf::test::driveModalDialog(*this, [&] { QTest::mouseClick(load, Qt::LeftButton); }, [](QDialog& warning) { warning.accept(); }));
  QCOMPARE(input->toPlainText(), text);
}

void WidgetConfigurationTests::recommendedConfigurationSelectionAppendsOnlyChosenGroups() {
  uwf::ui::ImportDialog dialog(std::make_shared<uwf::presentation::ImportViewModel>(uwf::windows::readBoundedTextFile), uwf::test::localFileDialogs());
  auto* input = dialog.findChild<uwf::ui::CodeEditor*>(QStringLiteral("importTextEdit"));
  QPushButton* recommended = nullptr;
  for (auto buttons = dialog.findChildren<QPushButton*>(); auto* button : std::as_const(buttons)) {
    if (button->text() == QStringLiteral("Load recommended configuration")) recommended = button;
  }
  QVERIFY(input);
  QVERIFY(recommended);

  bool chooserStructureValid = false;
  QVERIFY(uwf::test::driveModalDialog(
      *this, [&] { QTest::mouseClick(recommended, Qt::LeftButton); },
      [&chooserStructureValid](QDialog& modal) {
        auto* chooser = &modal;
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
        for (auto buttons = box->buttons(); auto* button : std::as_const(buttons)) {
          if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) accept = qobject_cast<QPushButton*>(button);
        }
        if (!accept || !accept->isEnabled()) {
          chooser->reject();
          return;
        }
        chooserStructureValid = true;
        QTest::mouseClick(accept, Qt::LeftButton);
      }));

  QVERIFY(chooserStructureValid);
  const QString text = input->toPlainText();
  QVERIFY(text.contains(QStringLiteral("Customer Experience Improvement Program")));
  auto commands = uwf::app::parseUwfmgrText(text.toStdString());
  std::erase_if(commands, [](const auto& command) { return command.parseError == uwf::app::ParseError::Comment; });
  QCOMPARE(commands.size(), std::size_t{3});
  for (const auto& command : commands) {
    QCOMPARE(command.parseError, uwf::app::ParseError::None);
    QCOMPARE(command.kind, uwf::app::UwfmgrKind::RegistryAddExclusion);
  }
  QVERIFY(!text.contains(QStringLiteral("Background Intelligent Transfer Service")));
  auto* import = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(import);
  QVERIFY(import->isEnabled());
}

}  // namespace
QTEST_MAIN(WidgetConfigurationTests)
#include "ConfigurationCases.moc"
