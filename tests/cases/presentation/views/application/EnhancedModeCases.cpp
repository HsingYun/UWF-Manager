// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollArea>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLayout>
#include <QToolButton>
#include <QtMath>
#include <algorithm>
#include <array>
#include <initializer_list>
#include <memory>
#include <utility>

#include "application/enhanced/EnhancedModeManager.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "cases/presentation/views/application/ApplicationUiFixture.h"
#include "cases/presentation/views/application/ApplicationUiTest.h"
#include "composition/desktop/MainWindowComposition.h"
#include "composition/desktop/ViewModelComposition.h"
#include "domain/uwf/model/UwfModel.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/feedback/StatusBadge.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"
#include "presentation/viewmodels/enhanced/EnhancedModeViewModel.h"
#include "presentation/views/enhanced/EnhancedModeDialog.h"
#include "presentation/views/shell/MainWindow.h"
#include "support/app/ConfigurationFixture.h"
#include "support/ui/DesktopPlatformFixture.h"
#include "support/ui/DialogControls.h"
#include "support/ui/LayoutSettling.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/uwf/FakeUwfTrace.h"
#include "support/visual/InlineIconTextProbe.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

class ApplicationEnhancedModeTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void enhancedModeDialogReflectsDisabledEnabledAndRepairStates();
  void enhancedModeInstallsTheLocalizedServiceDescription_data();
  void enhancedModeInstallsTheLocalizedServiceDescription();
  void enhancedModeUsesPlainChineseForDisabledState();
  void enhancedModeDialogUpdatesAutomaticCommitPause_data();
  void enhancedModeDialogUpdatesAutomaticCommitPause();
  void enhancedModeInlineGraphicsFollowTheFirstVisibleLine_data();
  void enhancedModeInlineGraphicsFollowTheFirstVisibleLine();
  void enhancedModeHeroAndActionsUseTheirRenderedCenters_data();
  void enhancedModeHeroAndActionsUseTheirRenderedCenters();
  void enhancedModePathsShowSelectionAndCopyCompleteText_data();
  void enhancedModePathsShowSelectionAndCopyCompleteText();
  void mainWindowEnhancedModeActionTracksTheServiceLifecycle();
  void mainWindowEnhancedModeIconUsesStatusColors_data();
  void mainWindowEnhancedModeIconUsesStatusColors();
  void mainWindowCreatesAndOwnsTheInjectedAgent();
  void mainWindowRetriesTransientEnhancedModeStatusFailures();
  void mainWindowTreatsAuthenticatedServiceConnectionAsStatusOnly();
};

using namespace uwf;
using namespace uwf::test::application;

namespace {
app::AutomaticCommitSummary automaticSummary(const std::initializer_list<app::AutomaticCommitDisposition> dispositions) {
  app::AutomaticCommitSummary summary;
  for (const auto disposition : dispositions) summary.include(disposition);
  return summary;
}
}  // namespace

void ApplicationEnhancedModeTests::enhancedModeHeroAndActionsUseTheirRenderedCenters_data() { enhancedModeInlineGraphicsFollowTheFirstVisibleLine_data(); }

void ApplicationEnhancedModeTests::enhancedModeHeroAndActionsUseTheirRenderedCenters() {
  QFETCH(ui::I18n::Lang, language);
  QFETCH(bool, dark);
  QFETCH(int, width);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto previousTheme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([previousLanguage, previousTheme] {
    ui::I18n::instance().setLang(previousLanguage);
    ui::ThemeManager::instance().apply(previousTheme);
  });
  ui::I18n::instance().setLang(language);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  auto system = std::make_shared<test::FakeUwfBackend>();
  app::EnhancedModeManager manager(service, app::uwfMaintenanceRepository(system), app::UwfCapability::Unavailable);
  auto model = composition::createViewModelFactory(system, test::localDesktopPorts())->enhancedMode(manager);
  ui::EnhancedModeDialog dialog(model);
  dialog.resize(test::boundedWindowSize(dialog, QSize(width, 690)));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QTRY_VERIFY(!model->busy());
  auto* badge = dynamic_cast<ui::StatusBadge*>(dialog.findChild<QWidget*>(QStringLiteral("enhancedModeStatus")));
  auto* heroIcon = dialog.findChild<QWidget*>(QStringLiteral("enhancedModeHeroIcon"));
  auto* heroTitle = dialog.findChild<ui::TextLabel*>(QStringLiteral("enhancedModeHeroTitle"));
  auto* heroDescription = dialog.findChild<ui::TextLabel*>(QStringLiteral("enhancedModeHeroDescription"));
  auto* issue = dialog.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(badge && heroIcon && heroTitle && heroDescription && issue);
  QList<ui::IconView*> issueGraphics;
  for (auto children = issue->findChildren<QWidget*>(); auto* child : std::as_const(children))
    if (auto* graphic = dynamic_cast<ui::IconView*>(child)) issueGraphics.append(graphic);
  const auto issueLabels = issue->findChildren<ui::TextLabel*>();
  const auto auditDirectory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR");
  const QString dataTag = QString::fromLatin1(QTest::currentDataTag());
  if (!auditDirectory.isEmpty()) QVERIFY(QDir().mkpath(auditDirectory));
  QJsonArray measurements;
  for (int state = 0; state < 5; ++state) {
    service->queryFailure.clear();
    service->current = completeEnhancedModeStatus();
    if (state == 1) service->current = {};
    if (state == 2) {
      service->current.state = app::EnhancedModeState::Stopped;
      service->current.running = service->current.preshutdownAccepted = false;
    }
    if (state == 3) {
      service->current.state = app::EnhancedModeState::RepairRequired;
      service->current.executableMatches = false;
    }
    if (state == 4) service->queryFailure = QStringLiteral("Pixel audit query failure");
    model->refreshStatus();
    QTRY_VERIFY(!model->busy());
    const auto status = model->statusText();
    QTRY_COMPARE(badge->text(), status.label);
    QVERIFY(test::waitForLayoutSettled(dialog));
    const auto frame = dialog.grab().toImage();
    const qreal dpr = frame.devicePixelRatio();
    const auto graphicInk = test::renderedInlineInk(frame, QRectF(heroIcon->mapTo(&dialog, QPoint()), heroIcon->size()));
    const auto titleInk = test::renderedInlineInk(frame, QRectF(heroTitle->mapTo(&dialog, QPoint()), heroTitle->size()));
    const auto descriptionInk = test::renderedInlineInk(frame, QRectF(heroDescription->mapTo(&dialog, QPoint()), heroDescription->size()));
    const auto textInk = titleInk.united(descriptionInk);
    const qreal heroDelta = qAbs(qRound((graphicInk.center().y() - textInk.center().y()) * dpr * 2)) / 2.0;
    const bool heroFits = heroIcon->parentWidget()->rect().contains(heroIcon->geometry()) &&
                          heroIcon->parentWidget()->parentWidget()->rect().contains(heroIcon->parentWidget()->geometry());
    if (!auditDirectory.isEmpty())
      measurements.append(QJsonObject{{"resource", ":/icons/enhanced.svg"},
                                      {"widget", "enhancedModeHeroIcon"},
                                      {"rule", "visible-block"},
                                      {"state", state},
                                      {"dpr", dpr},
                                      {"graphicCenterPhysical", graphicInk.center().y() * dpr},
                                      {"textCenterPhysical", textInk.center().y() * dpr},
                                      {"centerDifferencePhysical", heroDelta},
                                      {"graphicFitsHost", heroFits},
                                      {"nonempty", !graphicInk.isEmpty() && !textInk.isEmpty()}});
    QString errors;
    if (!heroFits || graphicInk.isEmpty() || textInk.isEmpty() || heroDelta > 1)
      errors += QStringLiteral("Enhanced hero differs by %1 physical pixels (fits host %2)\n").arg(heroDelta).arg(heroFits);
    if (!status.icon.isEmpty()) {
      const auto runs = test::inlineInkRuns(frame, badge->rect().adjusted(3, 3, -3, -3).translated(badge->mapTo(&dialog, QPoint())), qCeil(3 * dpr));
      if (runs.size() < 2) {
        errors += QStringLiteral("Cannot separate actual Enhanced status badge icon/text\n");
      } else {
        const bool rtl = badge->layoutDirection() == Qt::RightToLeft;
        const auto icon = rtl ? runs.back() : runs.front();
        QRectF text;
        for (qsizetype run = rtl ? 0 : 1; run < runs.size() - (rtl ? 1 : 0); ++run) text = text.united(runs.at(run));
        const qreal delta = qAbs(qRound((icon.center().y() - text.center().y()) * dpr * 2)) / 2.0;
        if (!auditDirectory.isEmpty())
          measurements.append(QJsonObject{{"resource", status.icon},
                                          {"widget", "enhancedModeStatus"},
                                          {"rule", "visible-center"},
                                          {"state", state},
                                          {"dpr", dpr},
                                          {"graphicCenterPhysical", icon.center().y() * dpr},
                                          {"textCenterPhysical", text.center().y() * dpr},
                                          {"centerDifferencePhysical", delta},
                                          {"nonempty", true}});
        if (delta > 1) errors += QStringLiteral("Enhanced status badge differs by %1 physical pixels\n").arg(delta);
      }
    }
    if (issue->isVisibleTo(&dialog)) {
      QWidget* graphic = nullptr;
      for (auto* candidate : issueGraphics)
        if (candidate->isVisibleTo(&dialog)) graphic = candidate;
      ui::TextLabel* reference = nullptr;
      for (auto* label : issueLabels)
        if (label->isVisibleTo(&dialog) && !label->text().isEmpty() &&
            (!reference || label->mapTo(&dialog, QPoint()).y() < reference->mapTo(&dialog, QPoint()).y()))
          reference = label;
      if (!graphic || !reference) {
        errors += QStringLiteral("Missing actual Enhanced issue icon/text\n");
      } else {
        const auto icon = test::renderedInlineInk(frame, QRectF(graphic->mapTo(&dialog, QPoint()), graphic->size()));
        const auto text = test::renderedInlineInk(frame, reference->firstLineBoxBounds().translated(reference->mapTo(&dialog, QPoint())));
        const qreal delta = qAbs(qRound((icon.center().y() - text.center().y()) * dpr * 2)) / 2.0;
        if (!auditDirectory.isEmpty())
          measurements.append(QJsonObject{{"widget", "enhancedModeIssue"},
                                          {"rule", "first-visible-line"},
                                          {"state", state},
                                          {"dpr", dpr},
                                          {"graphicCenterPhysical", icon.center().y() * dpr},
                                          {"textCenterPhysical", text.center().y() * dpr},
                                          {"centerDifferencePhysical", delta},
                                          {"nonempty", !icon.isEmpty() && !text.isEmpty()}});
        if (icon.isEmpty() || text.isEmpty() || delta > 1) errors += QStringLiteral("Enhanced issue icon differs by %1 physical pixels\n").arg(delta);
      }
    }
    const QString tag = QStringLiteral("%1-state-%2").arg(dataTag).arg(state);
    errors += test::probeInlineIconText(dialog, frame, QStringLiteral("enhanced-lifecycle"), tag);
    if (!auditDirectory.isEmpty()) {
      QVERIFY(frame.save(QDir(auditDirectory).filePath(QStringLiteral("enhanced-lifecycle-%1.png").arg(tag))));
      QFile output(QDir(auditDirectory).filePath(QStringLiteral("enhanced-lifecycle-%1.json").arg(dataTag)));
      QVERIFY(output.open(QIODevice::WriteOnly));
      QVERIFY(output.write(QJsonDocument(measurements).toJson()) > 0);
    }
    QVERIFY2(errors.isEmpty(), qUtf8Printable(errors));
  }
  QCOMPARE(service->installs + service->starts + service->disables + service->removals, 0);
}

void ApplicationEnhancedModeTests::enhancedModeInlineGraphicsFollowTheFirstVisibleLine_data() {
  QTest::addColumn<ui::I18n::Lang>("language");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<int>("width");
  for (const auto& language : app::languages())
    for (const bool dark : {false, true})
      for (const int width : {540, 740})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(QString(language.code), dark ? "dark" : "light").arg(width)))
            << language.id << dark << width;
}

void ApplicationEnhancedModeTests::enhancedModeInlineGraphicsFollowTheFirstVisibleLine() {
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
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  const auto system = std::make_shared<test::FakeUwfBackend>();
  app::EnhancedModeManager manager(service, app::uwfMaintenanceRepository(system), app::UwfCapability::Unavailable);
  auto model = composition::createViewModelFactory(system, test::localDesktopPorts())->enhancedMode(manager);
  ui::EnhancedModeDialog dialog(model);
  dialog.resize(test::boundedWindowSize(dialog, QSize(width, 690)));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* serviceState = dialog.findChild<ui::TextLabel*>(QStringLiteral("enhancedModeServiceState"));
  QVERIFY(serviceState);
  QTRY_COMPARE(serviceState->text(), model->statusText().service);
  struct StatusRow {
    ui::TextLabel* title;
    QWidget* icon;
    ui::TextLabel* value;
    QString resource;
  };
  std::array<StatusRow, 2> rows;
  int rowIndex = 0;
  for (const auto& name : {QStringLiteral("enhancedModeService"), QStringLiteral("enhancedModeStaging")}) {
    auto& row = rows[static_cast<std::size_t>(rowIndex++)];
    row.title = dialog.findChild<ui::TextLabel*>(name + QStringLiteral("Title"));
    row.icon = dialog.findChild<QWidget*>(name + QStringLiteral("Icon"));
    row.value = dialog.findChild<ui::TextLabel*>(name + QStringLiteral("State"));
    row.resource = name == QStringLiteral("enhancedModeService") ? QStringLiteral(":/icons/service.svg") : QStringLiteral(":/icons/file_staging.svg");
    QVERIFY(row.title && row.icon && row.value);
  }
  auto* fallbackIcon = dialog.findChild<QWidget*>(QStringLiteral("enhancedModeFallbackIcon"));
  auto* fallbackText = dialog.findChild<ui::TextLabel*>(QStringLiteral("enhancedModeFallback"));
  QVERIFY(fallbackIcon && fallbackText);
  const auto auditDirectory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR");
  const auto auditName = QStringLiteral("enhanced-%1").arg(QString::fromLatin1(QTest::currentDataTag()));
  if (!auditDirectory.isEmpty()) QVERIFY(QDir().mkpath(auditDirectory));
  QJsonArray measurements;
  using Disposition = app::AutomaticCommitDisposition;
  int automaticIndex = 0;
  for (const auto& automatic : {automaticSummary({Disposition::CheckDurability}), automaticSummary({Disposition::SkipRetainedOverlay}),
                                automaticSummary({Disposition::SkipRetainedOverlay, Disposition::CheckDurability})}) {
    model->setAutomaticCommitSummary(automatic);
    QVERIFY(test::waitForLayoutSettled(dialog));
    const auto clipping = test::waitForUnclippedText(dialog);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    const auto frame = dialog.grab().toImage();
    const qreal dpr = frame.devicePixelRatio();
    const auto& colors = ui::DesignSystem::instance();
    // The icon and state value share a title. Reuse its measured ink while the
    // frame and sampled background are unchanged.
    QHash<ui::TextLabel*, std::pair<QRgb, QRectF>> referenceInk;
    const auto sample = [&](QWidget* graphic, ui::TextLabel* reference, const QString& resource, const QColor& graphicColor, const bool graphicIsText) -> bool {
      const bool contained = graphic->parentWidget()->rect().contains(graphic->geometry());
      if (!contained) {
        qWarning() << graphic->objectName() << "graphic geometry exceeds its alignment host" << graphic->geometry() << graphic->parentWidget()->rect();
      }
      const QRectF graphicRegion(graphic->mapTo(&dialog, QPoint()), graphic->size());
      const auto textRegion = reference->firstLineBoxBounds().translated(reference->mapTo(&dialog, QPoint()));
      const auto corner = graphicRegion.topLeft() * dpr;
      const auto background = frame.pixelColor(qFloor(corner.x()), qFloor(corner.y()));
      const auto graphicInk = test::visibleInkBounds(frame, graphicColor, background, graphicRegion);
      QRectF textInk;
      if (const auto cached = referenceInk.constFind(reference); cached != referenceInk.cend() && cached->first == background.rgba())
        textInk = cached->second;
      else {
        const auto textColor = colors.color(reference == fallbackText ? ui::design::Color::Muted : ui::design::Color::Text);
        textInk = test::visibleInkBounds(frame, textColor, background, textRegion);
        referenceInk.insert(reference, {background.rgba(), textInk});
      }
      const auto graphicCenter = static_cast<qreal>(qRound64(graphicInk.center().y() * dpr * 2)) / 2.0;
      const auto textCenter = static_cast<qreal>(qRound64(textInk.center().y() * dpr * 2)) / 2.0;
      const qreal difference = qAbs(graphicCenter - textCenter);
      if (!auditDirectory.isEmpty())
        measurements.append(QJsonObject{
            {"resource", resource},
            {"widget", graphic->objectName()},
            {"reference", reference->objectName()},
            {"rule", "first-visible-line"},
            {"automaticState", automaticIndex},
            {"dpr", dpr},
            {"graphicCenterPhysical", graphicCenter},
            {"textCenterPhysical", textCenter},
            {"centerDifferencePhysical", difference},
            {"referencePredictedCenterPhysical", (reference->firstLineInkBounds().center().y() + reference->mapTo(&dialog, QPoint()).y()) * dpr},
            {"graphicPredictedCenterPhysical",
             graphicIsText ? (qobject_cast<ui::TextLabel*>(graphic)->firstLineInkBounds().center().y() + graphic->mapTo(&dialog, QPoint()).y()) * dpr : 0},
            {"graphicVerticalOffset", graphicIsText ? qobject_cast<ui::TextLabel*>(graphic)->verticalAlignmentOffset() : 0},
            {"nonempty", !graphicInk.isEmpty() && !textInk.isEmpty()},
            {"graphicIsText", graphicIsText},
            {"graphicFitsHost", contained}});
      if (graphicInk.isEmpty() || textInk.isEmpty() || difference > 1)
        qWarning().noquote() << QStringLiteral("%1 / %2: graphic=%3, text=%4, delta=%5 physical pixels")
                                    .arg(graphic->objectName(), reference->text())
                                    .arg(graphicInk.center().y() * dpr)
                                    .arg(textInk.center().y() * dpr)
                                    .arg(difference);
      return contained && !graphicInk.isEmpty() && !textInk.isEmpty() && difference <= 1;
    };
    bool aligned = true;
    const auto status = model->statusText();
    for (const auto& row : rows) {
      aligned = sample(row.icon, row.title, row.resource, colors.color(ui::design::Color::Muted), false) && aligned;
      const auto valueColor = row.value == serviceState && status.statusTone == presentation::EnhancedModeViewModel::FeedbackTone::Success
                                  ? ui::design::Color::Success
                                  : ui::design::Color::Muted;
      aligned = sample(row.value, row.title, {}, colors.color(valueColor), true) && aligned;
    }
    const bool fallbackVisible = model->statusText().showFallbackCommitHint;
    QCOMPARE(fallbackIcon->isVisible(), fallbackVisible);
    QCOMPARE(fallbackText->isVisible(), fallbackVisible);
    if (fallbackVisible)
      aligned = sample(fallbackIcon, fallbackText, QStringLiteral(":/icons/info.svg"), colors.color(ui::design::Color::Muted), false) && aligned;
    if (!auditDirectory.isEmpty()) {
      QVERIFY(frame.save(QDir(auditDirectory).filePath(auditName + QStringLiteral("-%1.png").arg(automaticIndex))));
      QFile output(QDir(auditDirectory).filePath(auditName + QStringLiteral(".json")));
      QVERIFY(output.open(QIODevice::WriteOnly));
      QVERIFY(output.write(QJsonDocument(measurements).toJson()) > 0);
    }
    QVERIFY(aligned);
    ++automaticIndex;
  }
  QCOMPARE(service->installs, 0);
  QCOMPARE(service->disables, 0);
}

void ApplicationEnhancedModeTests::enhancedModeInstallsTheLocalizedServiceDescription_data() {
  QTest::addColumn<ui::I18n::Lang>("language");
  QTest::addColumn<QString>("description");
  QTest::newRow("english") << ui::I18n::Lang::En << QStringLiteral("UWF Manager enhanced mode helper service");
  QTest::newRow("simplified-chinese") << ui::I18n::Lang::Zh_CN << QStringLiteral("UWF Manager 增强模式辅助服务");
}

void ApplicationEnhancedModeTests::enhancedModeInstallsTheLocalizedServiceDescription() {
  QFETCH(ui::I18n::Lang, language);
  QFETCH(QString, description);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto restoreLanguage = qScopeGuard([previousLanguage] { ui::I18n::instance().setLang(previousLanguage); });
  ui::I18n::instance().setLang(language);
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  app::EnhancedModeManager manager(service, app::uwfMaintenanceRepository(std::make_shared<test::FakeUwfBackend>()), app::UwfCapability::Unavailable);
  auto model = composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), test::localDesktopPorts())->enhancedMode(manager);
  model->refreshStatus();
  model->applyRequestedState();
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(service->installs, 1);
  QCOMPARE(service->installedDescription, description);
}

void ApplicationEnhancedModeTests::enhancedModeUsesPlainChineseForDisabledState() {
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto restoreLanguage = qScopeGuard([previousLanguage] { ui::I18n::instance().setLang(previousLanguage); });
  ui::I18n::instance().setLang(ui::I18n::Lang::Zh_CN);
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  app::EnhancedModeManager manager(service, app::uwfMaintenanceRepository(std::make_shared<test::FakeUwfBackend>()), app::UwfCapability::Unavailable);
  auto model = composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), test::localDesktopPorts())->enhancedMode(manager);
  model->refreshStatus();
  model->applyRequestedState();
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(model->statusText().label, QStringLiteral("已停用"));
  QCOMPARE(model->statusText().startup, QStringLiteral("已禁用"));
  QCOMPARE(model->statusText().action, QStringLiteral("启用增强模式"));
  QVERIFY(model->statusText().issues.isEmpty());
  QVERIFY(model->statusText().canRemove);
}

void ApplicationEnhancedModeTests::enhancedModeDialogUpdatesAutomaticCommitPause_data() {
  QTest::addColumn<ui::I18n::Lang>("language");
  QTest::newRow("english") << ui::I18n::Lang::En;
  QTest::newRow("simplified-chinese") << ui::I18n::Lang::Zh_CN;
}

void ApplicationEnhancedModeTests::enhancedModeDialogUpdatesAutomaticCommitPause() {
  QFETCH(ui::I18n::Lang, language);
  using Disposition = app::AutomaticCommitDisposition;
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto restoreLanguage = qScopeGuard([previousLanguage] { ui::I18n::instance().setLang(previousLanguage); });
  ui::I18n::instance().setLang(language);
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  const auto system = std::make_shared<test::FakeUwfBackend>();
  app::EnhancedModeManager manager(service, app::uwfMaintenanceRepository(system), app::UwfCapability::Unavailable);
  auto model = composition::createViewModelFactory(system, test::localDesktopPorts())->enhancedMode(manager);
  const auto retained = automaticSummary({Disposition::SkipRetainedOverlay});
  model->setAutomaticCommitSummary(retained);
  ui::EnhancedModeDialog dialog(model);
  dialog.resize(test::boundedWindowSize(dialog, QSize(540, 690)));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* state = dialog.findChild<QLabel*>(QStringLiteral("enhancedModeStagingState"));
  auto* hint = dialog.findChild<QLabel*>(QStringLiteral("enhancedModeStagingDescription"));
  auto* change = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* remove = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  auto* fallback = dialog.findChild<QWidget*>(QStringLiteral("enhancedModeFallbackRow"));
  QVERIFY(state && hint && change && remove && fallback);
  QCOMPARE(state->text(), presentation::automaticCommitPresentation(retained).status);
  QCOMPARE(hint->text(), presentation::automaticCommitPresentation(retained).description);
  QVERIFY(change->isEnabled());
  QVERIFY(remove->isEnabled());
  QVERIFY(!fallback->isVisible());

  const auto partial = automaticSummary({Disposition::SkipRetainedOverlay, Disposition::CheckDurability});
  model->setAutomaticCommitSummary(partial);
  QCOMPARE(state->text(), presentation::automaticCommitPresentation(partial).status);
  QCOMPARE(hint->text(), presentation::automaticCommitPresentation(partial).description);
  QTRY_VERIFY(hint->height() >= hint->heightForWidth(hint->width()));
  QVERIFY(change->isEnabled());
  QVERIFY(remove->isEnabled());
  QVERIFY(!fallback->isVisible());

  const auto ready = automaticSummary({Disposition::CheckDurability});
  model->setAutomaticCommitSummary(ready);
  QCOMPARE(state->text(), presentation::automaticCommitPresentation(ready).status);
  QCOMPARE(hint->text(), presentation::automaticCommitPresentation(ready).description);
  QVERIFY(fallback->isVisible());
  model->setAutomaticCommitSummary(automaticSummary({Disposition::SkipHormTransition}));
  QVERIFY(!fallback->isVisible());
  QVERIFY(change->isEnabled());
  QVERIFY(remove->isEnabled());
  QVERIFY(model->statusText().issues.isEmpty());
  model->setAutomaticCommitSummary(automaticSummary({Disposition::Unavailable}));
  QVERIFY(!fallback->isVisible());
  model->setAutomaticCommitSummary(automaticSummary({Disposition::CheckDurability}));
  QVERIFY(fallback->isVisible());
  model->setWriteRestriction(app::RuntimeWriteRestriction::HormUnknown);
  QVERIFY(!fallback->isVisible());
  model->setWriteRestriction(app::RuntimeWriteRestriction::None);
  QVERIFY(fallback->isVisible());
  const auto persistent = automaticSummary({Disposition::AlreadyPersistent});
  model->setAutomaticCommitSummary(persistent);
  QCOMPARE(state->text(), ui::I18n::tr("No files to commit"));
  QCOMPARE(hint->text(), presentation::automaticCommitPresentation(persistent).description);
  QVERIFY(!fallback->isVisible());
  const auto mixed =
      automaticSummary({Disposition::SkipRetainedOverlay, Disposition::CheckDurability, Disposition::AlreadyPersistent, Disposition::Unavailable});
  model->setAutomaticCommitSummary(mixed);
  QCOMPARE(state->text(), presentation::automaticCommitPresentation(mixed).status);
  QCOMPARE(hint->text(), presentation::automaticCommitPresentation(mixed).description);
  QVERIFY(hint->text().contains(presentation::automaticCommitDescription(Disposition::SkipRetainedOverlay)));
  QVERIFY(hint->text().contains(presentation::automaticCommitDescription(Disposition::Unavailable)));
  QVERIFY(!fallback->isVisible());
  QVERIFY(change->isEnabled());
  QVERIFY(remove->isEnabled());
  QVERIFY(test::waitForUnclippedText(dialog).isEmpty());
  QVERIFY(service->current.serviceContractSatisfied());
  QCOMPARE(service->installs, 0);
  QCOMPARE(service->disables, 0);
}

namespace {
QToolButton* commandButtonForAction(QWidget& window, QAction* action) {
  for (auto buttons = window.findChildren<QToolButton*>(); auto* button : std::as_const(buttons))
    if (button->defaultAction() == action) return button;
  return nullptr;
}

bool renderedIconHasColor(QToolButton& button, const QColor& expected) {
  const auto size = button.iconSize();
  const QRect bounds(QPoint((button.width() - size.width()) / 2, (button.height() - size.height()) / 2), size);
  const auto pixels = button.grab(bounds).toImage();
  constexpr qreal minimumIconCoverage = .01;
  return test::colorCoverage(pixels, expected, test::logicalBounds(pixels.rect(), pixels.devicePixelRatio())) >= minimumIconCoverage;
}
}  // namespace

void ApplicationEnhancedModeTests::enhancedModePathsShowSelectionAndCopyCompleteText_data() {
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("wrapped");
  for (const bool dark : {false, true})
    for (const bool wrapped : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(dark ? "dark" : "light", wrapped ? "wrapped" : "single-line"))) << dark << wrapped;
}

void ApplicationEnhancedModeTests::enhancedModePathsShowSelectionAndCopyCompleteText() {
  QFETCH(bool, dark);
  QFETCH(bool, wrapped);
  auto& theme = ui::ThemeManager::instance();
  const auto originalTheme = theme.current();
  const auto originalClipboard = QApplication::clipboard()->text();
  const auto originalProgramFiles = qgetenv("ProgramFiles");
  const bool hadProgramFiles = qEnvironmentVariableIsSet("ProgramFiles");
  const auto restore = qScopeGuard([&] {
    theme.apply(originalTheme);
    QApplication::clipboard()->setText(originalClipboard);
    if (hadProgramFiles)
      qputenv("ProgramFiles", originalProgramFiles);
    else
      qunsetenv("ProgramFiles");
  });
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  const QString programFiles = wrapped ? QStringLiteral("C:/Program Files/Enterprise application deployment/Managed security tools/Long installation directory")
                                       : QStringLiteral("C:/Apps");
  qputenv("ProgramFiles", programFiles.toUtf8());
  const auto expected = QDir::toNativeSeparators(QDir(programFiles).filePath(QStringLiteral("UWF Manager/Service/UWF.exe")));
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto serviceControl = std::make_shared<MemoryEnhancedModeServiceControl>();
  app::EnhancedModeManager manager(serviceControl, uwf::app::uwfMaintenanceRepository(system), app::UwfCapability::Unavailable);
  ui::EnhancedModeDialog dialog(
      composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())->enhancedMode(manager));
  dialog.resize(test::boundedWindowSize(dialog, QSize(wrapped ? 540 : 740, 690)));
  auto* safety = dialog.findChild<ui::Disclosure*>(QStringLiteral("enhancedModeSafety"));
  auto* details = dialog.findChild<ui::Disclosure*>(QStringLiteral("enhancedServiceDetails"));
  auto* scroll = dialog.findChild<QScrollArea*>(QStringLiteral("enhancedModeScroll"));
  auto* location = dialog.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeSuggestedLocation"));
  QVERIFY(safety && details && scroll && location);
  safety->setOpen(true);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QTRY_VERIFY(location->height() >= location->heightForWidth(location->width()));
  scroll->ensureWidgetVisible(location);
  QCoreApplication::processEvents();
  QCOMPARE(location->toPlainText(), expected);
  QCOMPARE(location->font().pixelSize(), 12);
  QCOMPARE(location->font().families(), ui::DesignSystem::monospaceFamilies());
  QCOMPARE(location->focusPolicy(), Qt::ClickFocus);
  QVERIFY(!location->textInteractionFlags().testFlag(Qt::TextEditable));
  auto* layout = location->document()->firstBlock().layout();
  QVERIFY(layout);
  QVERIFY(wrapped ? layout->lineCount() > 1 : layout->lineCount() == 1);
  location->setFocus();
  const auto before = location->viewport()->grab().toImage();
  QTextCursor first(location->document());
  QTextCursor last(location->document());
  last.movePosition(QTextCursor::End);
  const auto start = location->cursorRect(first).center();
  const auto end = location->cursorRect(last).center() + QPoint(1, 0);
  QTest::mousePress(location->viewport(), Qt::LeftButton, Qt::NoModifier, start);
  QTest::mouseMove(location->viewport(), end, 20);
  QTest::mouseRelease(location->viewport(), Qt::LeftButton, Qt::NoModifier, end);
  QTRY_COMPARE(location->textCursor().selectedText(), expected);
  const auto selected = location->viewport()->grab().toImage();
  const auto highlight = location->palette().color(QPalette::Highlight);
  constexpr qreal minimumSelectionCoverage = .10;
  for (int line = 0; line < layout->lineCount(); ++line) {
    QTextCursor lineStart(location->document());
    lineStart.setPosition(layout->lineAt(line).textStart());
    const auto bounds = location->cursorRect(lineStart);
    const QRectF lineRegion(bounds.left(), bounds.top(), layout->lineAt(line).naturalTextWidth(), bounds.height());
    const auto coverage = test::colorCoverage(selected, highlight, lineRegion);
    QVERIFY2(coverage >= minimumSelectionCoverage && coverage > test::colorCoverage(before, highlight, lineRegion),
             qUtf8Printable(QStringLiteral("No painted selection on wrapped line %1").arg(line)));
  }
  QVERIFY(test::copySelectionToClipboard(*location, expected));
  QCOMPARE(QApplication::clipboard()->text(), expected);
  QVERIFY(test::saveScreenshot(dialog, QStringLiteral("enhanced-mode-selected-path")));
  safety->setOpen(false);
  details->setOpen(true);
  for (int index = 0; index < 5; ++index) {
    auto* value = dialog.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeServiceValue%1").arg(index));
    QVERIFY(value && !value->toPlainText().isEmpty());
    QCOMPARE(value->focusPolicy(), Qt::ClickFocus);
    value->selectAll();
    const auto text = value->toPlainText();
    QCOMPARE(value->textCursor().selectedText(), text);
    value->copy();
    QCOMPARE(QApplication::clipboard()->text(), text);
  }
}

void ApplicationEnhancedModeTests::enhancedModeDialogReflectsDisabledEnabledAndRepairStates() {
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  app::EnhancedModeManager manager(serviceControlOwner, uwf::app::uwfMaintenanceRepository(system), uwf::app::UwfCapability::Unavailable);
  ui::EnhancedModeDialog dialog(
      uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())->enhancedMode(manager));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* status = dialog.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* change = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* remove = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  QVERIFY(status && change && remove);
  QCOMPARE(status->text(), QStringLiteral("Disabled"));
  QCOMPARE(change->text(), QStringLiteral("Enable enhanced mode"));
  QVERIFY(!remove->isEnabled());
  QTest::mouseClick(change, Qt::LeftButton);
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(serviceControl.installs, 1);
  QCOMPARE(serviceControl.installedDescription, QStringLiteral("UWF Manager enhanced mode helper service"));
  QCOMPARE(status->text(), QStringLiteral("Enabled"));
  QCOMPARE(change->text(), QStringLiteral("Disable enhanced mode"));
  QVERIFY(remove->isEnabled());

  const int queriesBeforeAgentStateChanges = serviceControl.queries;
  manager.setAgentState(app::EnhancedModeAgentState::Connecting);
  auto* issue = dialog.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(issue);
  QCOMPARE(issue->text(), QStringLiteral("Waiting for the UI agent to complete authentication."));

  manager.setAgentState(app::EnhancedModeAgentState::Disconnected);
  QCOMPARE(status->text(), QStringLiteral("Enabled"));
  QCOMPARE(change->text(), QStringLiteral("Disable enhanced mode"));
  QCOMPARE(issue->text(), QStringLiteral("The service is running, but no authenticated UI agent is connected."));

  manager.setAgentState(app::EnhancedModeAgentState::Connected);
  QVERIFY(!issue->isVisible());
  QCOMPARE(serviceControl.queries, queriesBeforeAgentStateChanges);

  QTest::mouseClick(change, Qt::LeftButton);
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(serviceControl.disables, 1);
  QCOMPARE(serviceControl.removals, 0);
  QCOMPARE(status->text(), QStringLiteral("Disabled"));
  QCOMPARE(change->text(), QStringLiteral("Enable enhanced mode"));
  QVERIFY(serviceControl.current.serviceExists);
  QVERIFY(serviceControl.current.serviceRegistryPresent);
  QVERIFY(serviceControl.current.startupDisabled);
  QVERIFY(!serviceControl.current.automaticStart);
  QVERIFY(!serviceControl.current.running);
  QVERIFY(!issue->isVisible());
  QVERIFY(remove->isEnabled());
  auto* startup = dialog.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeServiceValue2"));
  QVERIFY(startup);
  QCOMPARE(startup->toPlainText(), QStringLiteral("Disabled"));

  QTest::mouseClick(change, Qt::LeftButton);
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(serviceControl.installs, 2);
  QCOMPARE(status->text(), QStringLiteral("Enabled"));
  QVERIFY(serviceControl.current.automaticStart);
  QVERIFY(!serviceControl.current.startupDisabled);

  QTest::mouseClick(remove, Qt::LeftButton);
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(serviceControl.removals, 1);
  QCOMPARE(serviceControl.disables, 1);
  QCOMPARE(status->text(), QStringLiteral("Disabled"));
  QVERIFY(!serviceControl.current.serviceExists);
  QCOMPARE(serviceControl.activeDeletionBarriers, 0);
  QVERIFY(!serviceControl.queriedDuringDeletion);
  QVERIFY(!remove->isEnabled());

  serviceControl.current = {};
  serviceControl.current.state = app::EnhancedModeState::Stopped;
  serviceControl.current.serviceExists = true;
  serviceControl.current.ownProcess = true;
  serviceControl.current.automaticStart = true;
  serviceControl.current.localSystemAccount = true;
  serviceControl.current.executableMatches = true;
  serviceControl.current.preshutdownTimeoutConfigured = true;
  serviceControl.current.requiredPrivilegesConfigured = true;
  serviceControl.current.serviceRegistryPresent = true;
  ui::EnhancedModeDialog stopped(
      uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())->enhancedMode(manager));
  auto* start = stopped.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* stoppedStatus = stopped.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* stoppedIssue = stopped.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(start && stoppedStatus && stoppedIssue);
  QCOMPARE(stoppedStatus->text(), QStringLiteral("Service stopped"));
  QCOMPARE(start->text(), QStringLiteral("Start service"));
  QVERIFY(!stoppedIssue->isHidden());
  QVERIFY(stoppedIssue->text().contains(QStringLiteral("installed but stopped")));
  QTest::mouseClick(start, Qt::LeftButton);
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(serviceControl.starts, 1);
  QCOMPARE(serviceControl.installs, 2);
  QCOMPARE(stoppedStatus->text(), QStringLiteral("Enabled"));

  serviceControl.current.running = true;
  serviceControl.current.state = app::EnhancedModeState::RepairRequired;
  serviceControl.current.executableMatches = false;
  ui::EnhancedModeDialog repairRequired(
      uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())->enhancedMode(manager));
  auto* repair = repairRequired.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* repairStatus = repairRequired.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* repairIssue = repairRequired.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(repair && repairStatus && repairIssue);
  QCOMPARE(repairStatus->text(), QStringLiteral("Repair required"));
  QCOMPARE(repair->text(), QStringLiteral("Repair enhanced mode"));
  QVERIFY(repairIssue->text().contains(QStringLiteral("executable path or startup arguments")));

  serviceControl.current = {};
  serviceControl.current.state = app::EnhancedModeState::RepairRequired;
  serviceControl.current.serviceRegistryPresent = true;
  ui::EnhancedModeDialog residual(
      uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())->enhancedMode(manager));
  auto* residualRemove = residual.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  auto* residualIssue = residual.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(residualRemove && residualIssue);
  QVERIFY(residualRemove->isEnabled());
  QCOMPARE(residualIssue->text(), QStringLiteral("The service is absent, but its service registry data remains."));

  serviceControl.queryFailure = QStringLiteral("SCM unavailable");
  ui::EnhancedModeDialog unavailable(
      uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())->enhancedMode(manager));
  auto* unavailableStatus = unavailable.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* unavailableChange = unavailable.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* unavailableRemove = unavailable.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  auto* unavailableIssue = unavailable.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(unavailableStatus && unavailableChange && unavailableRemove && unavailableIssue);
  QCOMPARE(unavailableStatus->text(), QStringLiteral("Status unavailable"));
  QVERIFY(unavailableIssue->text().contains(QStringLiteral("service status could not be read")));
  bool hasDiagnostic = false;
  for (auto texts = unavailable.findChildren<ui::ContentHintBox*>(); const auto* text : std::as_const(texts))
    hasDiagnostic = hasDiagnostic || text->toPlainText().contains(serviceControl.queryFailure);
  QVERIFY(hasDiagnostic);
  QVERIFY(unavailableChange->isEnabled());
  QCOMPARE(unavailableChange->text(), QStringLiteral("Retry"));
  QVERIFY(!unavailableRemove->isEnabled());
  serviceControl.queryFailure.clear();
  serviceControl.current = completeEnhancedModeStatus();
  const int queriesBeforeRetry = serviceControl.queries;
  QTest::mouseClick(unavailableChange, Qt::LeftButton);
  QCOMPARE(serviceControl.queries, queriesBeforeRetry + 1);
  QCOMPARE(unavailableStatus->text(), QStringLiteral("Enabled"));
  QCOMPARE(unavailableChange->text(), QStringLiteral("Disable enhanced mode"));
  QVERIFY(unavailableRemove->isEnabled());
}

void ApplicationEnhancedModeTests::mainWindowEnhancedModeActionTracksTheServiceLifecycle() {
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  ui::MainWindow window(uwf::composition::mainWindowServices(system, source, stagingOwner, uwf::test::localDesktopPorts(), serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  window.resize(test::boundedWindowSize(window, QSize(ui::design::smallMainWidth, ui::design::smallMainHeight)));
  auto* enhancedAction = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(enhancedAction);
  QVERIFY(!enhancedAction->icon().isNull());
  QCOMPARE(enhancedAction->toolTip(), QStringLiteral("Coordinate automatic file staging with Windows shutdown and restart."));
  auto* enhancedButton = commandButtonForAction(window, enhancedAction);
  QVERIFY(enhancedButton);
  QTRY_COMPARE(enhancedButton->toolButtonStyle(), Qt::ToolButtonIconOnly);
  auto& theme = ui::ThemeManager::instance();
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Fg)));

  const auto changeService = [&](const QString& initialStatus, auto verifyChange) {
    return test::driveModalDialog(
        window, [enhancedAction] { enhancedAction->trigger(); },
        [&](QDialog& dialog) {
          QVERIFY2(dialog.objectName() == QStringLiteral("enhancedModeDialog"), qUtf8Printable(dialog.objectName()));
          auto* status = dialog.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
          auto* issue = dialog.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
          auto* change = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
          QVERIFY(status && issue && change);
          QTRY_COMPARE(status->text(), initialStatus);
          change->click();
          verifyChange(*status, *issue);
        });
  };
  const bool enableDialogObserved = changeService(QStringLiteral("Disabled"), [](QLabel&, ui::Notice& issue) {
    QTRY_COMPARE(issue.text(), QStringLiteral("Waiting for the UI agent to complete authentication."));
  });
  QVERIFY(enableDialogObserved);
  QCOMPARE(serviceControl.installs, 1);
  QCOMPARE(serviceControl.current.state, app::EnhancedModeState::Enabled);
  QCOMPARE(agent.starts, 1);
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Accent)));

  const bool disableDialogObserved =
      changeService(QStringLiteral("Enabled"), [](QLabel& status, ui::Notice&) { QTRY_COMPARE(status.text(), QStringLiteral("Disabled")); });
  QVERIFY(disableDialogObserved);
  QCOMPARE(serviceControl.disables, 1);
  QCOMPARE(serviceControl.removals, 0);
  QCOMPARE(serviceControl.current.state, app::EnhancedModeState::Disabled);
  QVERIFY(serviceControl.current.serviceExists);
  QVERIFY(serviceControl.current.startupDisabled);
  QCOMPARE(agent.stops, 1);
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Fg)));
}

void ApplicationEnhancedModeTests::mainWindowEnhancedModeIconUsesStatusColors_data() {
  QTest::addColumn<bool>("dark");
  QTest::addColumn<int>("serviceState");
  QTest::addColumn<int>("agentState");
  QTest::addColumn<int>("expectedColor");
  const auto row = [](const char* name, const app::EnhancedModeState service, const app::EnhancedModeAgentState agent, const ui::Sem color) {
    for (const bool dark : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString::fromLatin1(name), dark ? "dark" : "light")))
          << dark << static_cast<int>(service) << static_cast<int>(agent) << static_cast<int>(color);
  };
  row("disabled", app::EnhancedModeState::Disabled, app::EnhancedModeAgentState::Unobserved, ui::Sem::Fg);
  row("connecting", app::EnhancedModeState::Enabled, app::EnhancedModeAgentState::Connecting, ui::Sem::Accent);
  row("connected", app::EnhancedModeState::Enabled, app::EnhancedModeAgentState::Connected, ui::Sem::AddOk);
  row("disconnected", app::EnhancedModeState::Enabled, app::EnhancedModeAgentState::Disconnected, ui::Sem::Accent);
  row("stopped", app::EnhancedModeState::Stopped, app::EnhancedModeAgentState::Unobserved, ui::Sem::Warn);
  row("repair-required", app::EnhancedModeState::RepairRequired, app::EnhancedModeAgentState::Unobserved, ui::Sem::Warn);
}

void ApplicationEnhancedModeTests::mainWindowEnhancedModeIconUsesStatusColors() {
  QFETCH(bool, dark);
  QFETCH(int, serviceState);
  QFETCH(int, agentState);
  QFETCH(int, expectedColor);
  auto& theme = ui::ThemeManager::instance();
  const auto originalTheme = theme.current();
  const auto restore = qScopeGuard([&] { theme.apply(originalTheme); });
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);

  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto staging = std::make_shared<MemoryFileStagingStore>();
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  const auto state = static_cast<app::EnhancedModeState>(serviceState);
  if (state != app::EnhancedModeState::Disabled) service->current = completeEnhancedModeStatus();
  service->current.state = state;
  if (state == app::EnhancedModeState::Stopped) service->current.running = false;
  if (state == app::EnhancedModeState::RepairRequired) service->current.executableMatches = false;
  MemoryEnhancedModeAgent agent;
  agent.connectOnStart = agentState == static_cast<int>(app::EnhancedModeAgentState::Connected);
  agent.canStart = agentState != static_cast<int>(app::EnhancedModeAgentState::Disconnected);
  MutableApplicationStateSource source;
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;
  ui::MainWindow window(uwf::composition::mainWindowServices(system, source, staging, uwf::test::localDesktopPorts(), service, &agent),
                        {.uwfCapability = app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_VERIFY(refreshCompleted(window));
  window.resize(test::boundedWindowSize(window, QSize(ui::design::smallMainWidth, ui::design::smallMainHeight)));
  auto* action = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(action);
  auto* button = commandButtonForAction(window, action);
  QVERIFY(button);
  QTRY_COMPARE(button->toolButtonStyle(), Qt::ToolButtonIconOnly);
  QVERIFY(button->isEnabled());
  QVERIFY(renderedIconHasColor(*button, theme.color(static_cast<ui::Sem>(expectedColor))));

  if (state == app::EnhancedModeState::Enabled) {
    agent.publishConnectionState(true);
    QVERIFY(renderedIconHasColor(*button, theme.color(ui::Sem::AddOk)));
    agent.publishConnectionState(false);
    QVERIFY(renderedIconHasColor(*button, theme.color(ui::Sem::Accent)));
  }
  theme.toggle();
  const auto finalColor = state == app::EnhancedModeState::Enabled ? ui::Sem::Accent : static_cast<ui::Sem>(expectedColor);
  QVERIFY(renderedIconHasColor(*button, theme.color(finalColor)));
}

void ApplicationEnhancedModeTests::mainWindowCreatesAndOwnsTheInjectedAgent() {
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  MutableApplicationStateSource source;
  int created = 0;
  QPointer<MemoryEnhancedModeAgent> agent;
  auto services = uwf::composition::mainWindowServices(system, source, stagingOwner, uwf::test::localDesktopPorts(), serviceControlOwner, nullptr, [&] {
    ++created;
    auto owned = std::make_unique<MemoryEnhancedModeAgent>();
    agent = owned.get();
    return owned;
  });
  {
    ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Unavailable});
    QCOMPARE(created, 1);
    QVERIFY(agent);
    QVERIFY(uwf::test::writeRequests(*system).empty());
  }
  QVERIFY(agent.isNull());
}

void ApplicationEnhancedModeTests::mainWindowRetriesTransientEnhancedModeStatusFailures() {
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  MemoryEnhancedModeAgent agent;
  serviceControl.queryFailure = QStringLiteral("SCM temporarily unavailable");
  MutableApplicationStateSource source;
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  ui::MainWindow window(uwf::composition::mainWindowServices(system, source, stagingOwner, uwf::test::localDesktopPorts(), serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  window.resize(test::boundedWindowSize(window, QSize(ui::design::smallMainWidth, ui::design::smallMainHeight)));
  auto* enhancedAction = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(enhancedAction);
  auto* enhancedButton = commandButtonForAction(window, enhancedAction);
  QVERIFY(enhancedButton);
  QTRY_COMPARE(enhancedButton->toolButtonStyle(), Qt::ToolButtonIconOnly);
  auto& theme = ui::ThemeManager::instance();
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Warn)));
  QCOMPARE(serviceControl.queries, 1);

  serviceControl.queryFailure.clear();
  QTRY_VERIFY_WITH_TIMEOUT(serviceControl.queries >= 2, 2500);
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Fg)));
}

void ApplicationEnhancedModeTests::mainWindowTreatsAuthenticatedServiceConnectionAsStatusOnly() {
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\pending.bin")}};
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  serviceControl.current = completeEnhancedModeStatus();
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  auto services = uwf::composition::mainWindowServices(system, source, stagingOwner, uwf::test::localDesktopPorts(), serviceControlOwner, &agent);
  const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
  services.workflows = workflows;
  ui::MainWindow window(std::move(services), {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  QCOMPARE(agent.starts, 1);
  QVERIFY(workflows->observed);
  const auto maintenance = workflows->observed->overlayMaintenance();
  QVERIFY(maintenance);
  // The main refresh also starts an independent maintenance observation.
  // Authentication must be compared with its completed startup baseline.
  QTRY_VERIFY(maintenance->hasSnapshot() && !maintenance->busy());

  const int queriesBeforeAuthentication = serviceControl.queries;
  const int loadsBeforeAuthentication = staging.loads;
  const int readsBeforeAuthentication = source.reads;
  const auto requestsBeforeAuthentication = system->trace().size();
  agent.publishConnectionState(true);
  QCoreApplication::processEvents();
  QCOMPARE(workflows->observed->enhancedModeStatus().agentState, app::EnhancedModeAgentState::Connected);
  QCOMPARE(serviceControl.queries, queriesBeforeAuthentication);
  QCOMPARE(staging.loads, loadsBeforeAuthentication);
  QCOMPARE(source.reads, readsBeforeAuthentication);
  QVERIFY(!maintenance->busy());
  QCOMPARE(system->trace().size(), requestsBeforeAuthentication);
}

QTEST_MAIN(ApplicationEnhancedModeTests)
#include "EnhancedModeCases.moc"
