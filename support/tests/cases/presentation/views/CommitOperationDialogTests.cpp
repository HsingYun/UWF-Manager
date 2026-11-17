// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractScrollArea>
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QScopeGuard>
#include <QSemaphore>
#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <QtMath>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/views/commit/CommitInteractionAdapter.h"
#include "presentation/views/commit/CommitOperationDialog.h"
#include "simulation/ui/FileDialogStub.h"
#include "support/app/CommitScenario.h"
#include "support/ui/DialogControls.h"
#include "support/ui/LayoutSettling.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/visual/AnimationClock.h"
#include "support/visual/InkBounds.h"
#include "support/visual/InlineIconTextProbe.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

namespace {
using namespace uwf;
using Model = presentation::CommitOperationViewModel;
class CommitOperationDialogTests final : public QObject {
  Q_OBJECT
  static QPushButton* button(QWidget& dialog, const char* name) { return dialog.findChild<QPushButton*>(QLatin1String(name)); }
  static void capture(QWidget& dialog, const QString& state, const QString& appearance = {}) {
    const auto directory = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
    if (directory.isEmpty()) return;
    QDir().mkpath(directory);
    const auto backdrop = dialog.property("systemBackdropActive");
    dialog.setProperty("systemBackdropActive", false);
    const auto tag = appearance.isEmpty() ? QString::fromLatin1(QTest::currentDataTag()) : appearance;
    const auto frame = [&] {
      if (state == QStringLiteral("preparing") && test::visualComparisonEnabled()) {
        test::AnimationClock clock;
        clock.advanceBy(400);
        return test::screenshot(dialog);
      }
      return test::screenshot(dialog);
    }();
    dialog.setProperty("systemBackdropActive", backdrop);
    QVERIFY(frame.save(QDir(directory).filePath(QStringLiteral("commit-%1-%2.png").arg(tag, state))));
  }
  static void checkGeometry(ui::CommitOperationDialog& dialog) {
    const auto clipping = test::waitForUnclippedText(dialog, 2000);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    QVERIFY(dialog.findChildren<QAbstractScrollArea*>().isEmpty());
    for (auto buttons = dialog.findChildren<QPushButton*>(); auto* widget : std::as_const(buttons))
      if (widget->isVisible()) {
        QTRY_VERIFY2_WITH_TIMEOUT(dialog.rect().contains(QRect(widget->mapTo(&dialog, QPoint()), widget->size())), qUtf8Printable(widget->objectName()), 2000);
        QTRY_VERIFY2_WITH_TIMEOUT(widget->width() >= widget->fontMetrics().horizontalAdvance(widget->text()) +
                                                         (widget->property("designRole").toString() == QLatin1String("link") ? 0 : 10),
                                  qUtf8Printable(widget->objectName()), 2000);
      }
    for (const auto name : {"commitOperationDescription", "commitOperationCount"}) {
      const auto* text = dialog.findChild<ui::TextLabel*>(QLatin1String(name));
      QVERIFY(text);
      QVERIFY2(text->height() >= text->heightForWidth(text->width()), qUtf8Printable(text->text()));
    }
    const auto* path = dialog.findChild<ui::TextLabel*>(QStringLiteral("commitOperationTarget"));
    QVERIFY(path);
    QCOMPARE(path->elideMode(), Qt::ElideRight);
    QCOMPARE(path->toolTip(), path->text());
    const auto* note = dialog.findChild<ui::TextLabel*>(QStringLiteral("commitOperationNote"));
    QVERIFY(note);
    QCOMPARE(note->elideMode(), Qt::ElideNone);
    if (note->isVisible()) QTRY_VERIFY_WITH_TIMEOUT(note->width() >= note->minimumSizeHint().width(), 2000);
    auto* heading = dialog.findChild<ui::TextLabel*>(QStringLiteral("commitOperationHeading"));
    auto* glyph = dialog.findChild<QWidget*>(QStringLiteral("commitOperationGlyph"));
    QVERIFY(heading && glyph);
    const qreal dpr = heading->devicePixelRatioF();
    QImage pixels(QSize(qCeil(heading->width() * dpr), qCeil(heading->height() * dpr)), QImage::Format_ARGB32_Premultiplied);
    pixels.setDevicePixelRatio(dpr);
    const QColor background(Qt::magenta);
    pixels.fill(background);
    heading->render(&pixels, QPoint(), QRegion(), QWidget::RenderFlags{});
    QRect ink;
    for (auto inkRuns = test::horizontalInkRuns(pixels, ui::DesignSystem::instance().color(ui::design::Color::Text), background, pixels.rect(), 2);
         const auto& run : std::as_const(inkRuns))
      ink = ink.united(run);
    QVERIFY(!ink.isEmpty());
    const auto textBounds = test::logicalBounds(ink, dpr).translated(heading->mapTo(&dialog, QPoint()));
    const QRectF iconBounds(glyph->mapTo(&dialog, QPoint()), glyph->size());
    QVERIFY2(test::centersAligned(textBounds, iconBounds), qUtf8Printable(QStringLiteral("%1: text center %2, icon center %3, icon height %4")
                                                                              .arg(heading->text())
                                                                              .arg(textBounds.center().y())
                                                                              .arg(iconBounds.center().y())
                                                                              .arg(iconBounds.height())));
  }
  static void checkLocalizedState(Model& model, ui::CommitOperationDialog& dialog, const Model::Phase phase, const QString& state) {
    for (const auto& language : app::languages()) {
      ui::I18n::instance().setLang(language.id);
      for (const bool dark : {false, true}) {
        ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
        QVERIFY(test::waitForLayoutSettled(dialog));
        QCOMPARE(model.phase(), phase);
        QCOMPARE(dialog.windowTitle(), model.text().title);
        QCOMPARE(button(dialog, "commitOperationPrimary")->text(), model.text().primary);
        QCOMPARE(button(dialog, "commitOperationCancel")->text(), model.text().close);
        checkGeometry(dialog);
        if (QTest::currentTestFailed()) return;
        const auto alignmentError = test::probeInlineIconText(dialog, QStringLiteral("commit-operation-") + state,
                                                              QStringLiteral("%1-%2").arg(QString(language.code), dark ? "dark" : "light"));
        QVERIFY2(alignmentError.isEmpty(), qUtf8Printable(alignmentError));
        capture(dialog, state, QStringLiteral("%1-%2").arg(QString(language.code), dark ? "dark" : "light"));
      }
    }
  }
 private slots:
  void completedOutcomesStayDistinctAcrossLanguages_data() {
    QTest::addColumn<int>("scenario");
    QTest::newRow("all-succeeded") << 0;
    QTest::newRow("all-failed") << 1;
    QTest::newRow("partially-succeeded") << 2;
    QTest::newRow("not-found-is-failed") << 3;
  }
  void completedOutcomesStayDistinctAcrossLanguages() {
    QFETCH(int, scenario);
    ui::I18n::instance().setLang(app::LanguageId::En);
    auto backend = std::make_shared<test::commit::Backend>(false);
    app::CommitTargetResult ok, failed, notFound;
    ok.succeeded = true;
    failed.failureDetail = QStringLiteral("controlled write failure");
    notFound.failureKind = app::OperationFailureKind::NotFound;
    notFound.failureEffect = app::WriteEffect::Uncertain;
    backend->results = scenario == 0 ? QList{ok, ok} : scenario == 1 ? QList{failed, failed} : scenario == 2 ? QList{ok, failed} : QList{notFound, notFound};
    Model model(backend->request(), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    model.commit();
    QTRY_COMPARE(model.phase(), Model::Phase::Complete);
    uwf::simulation::FileDialogStub files;
    ui::CommitOperationDialog dialog(model, files, {}, nullptr);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    const auto title = scenario == 0 ? "All operations succeeded" : scenario == 2 ? "Some operations succeeded" : "No operations succeeded";
    const auto icon = scenario == 0 ? ":/icons/status_success.svg" : scenario == 2 ? ":/icons/status_warning.svg" : ":/icons/status_error.svg";
    for (const auto& language : app::languages()) {
      ui::I18n::instance().setLang(language.id);
      for (const bool dark : {false, true}) {
        ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
        QVERIFY(test::waitForLayoutSettled(dialog));
        QCOMPARE(dialog.windowTitle(), ui::I18n::tr(title));
        QCOMPARE(model.text().icon, QString::fromLatin1(icon));
        presentation::CommitReportViewModel report(model.rows(), app::CommitOperation::Persist);
        QCOMPARE(report.counts().succeeded, scenario == 0 ? 2 : scenario == 2 ? 1 : 0);
        QCOMPARE(report.counts().failed, scenario == 1 || scenario == 3 ? 2 : scenario == 2 ? 1 : 0);
        QCOMPARE(report.outcome().title, dialog.windowTitle());
        checkGeometry(dialog);
        const auto alignmentError = test::probeInlineIconText(dialog, QStringLiteral("commit-outcome-%1").arg(scenario),
                                                              QStringLiteral("%1-%2").arg(QString(language.code), dark ? "dark" : "light"));
        QVERIFY2(alignmentError.isEmpty(), qUtf8Printable(alignmentError));
        if (language.id == app::LanguageId::Zh_CN || language.id == app::LanguageId::En || language.id == app::LanguageId::Ar)
          capture(dialog, QStringLiteral("result-%1-%2").arg(QString(language.code), dark ? "dark" : "light"));
      }
    }
  }
  void localizedGeometryAndStateBindings() {
    const auto language = ui::I18n::instance().lang();
    const auto theme = ui::ThemeManager::instance().current();
    const auto restore = qScopeGuard([&] {
      ui::I18n::instance().setLang(language);
      ui::ThemeManager::instance().apply(theme);
    });
    auto backend = std::make_shared<test::commit::Backend>(true);
    backend->tree->fail = true;
    // Hold Preparing during locale/theme checks; the scope guard releases traversal after assertion failures.
    auto traversalGate = std::make_shared<QSemaphore>();
    backend->tree->afterChild = [traversalGate] {
      traversalGate->acquire();
      traversalGate->release();
    };
    const auto release = qScopeGuard([traversalGate] { traversalGate->release(); });
    Model model(backend->request(app::CommitOperation::DeleteAndPersist), backend);
    uwf::simulation::FileDialogStub files;
    ui::CommitOperationDialog dialog(model, files, {}, nullptr);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    model.start();
    QTRY_VERIFY(backend->tree->reads.load() > 0);
    QVERIFY(!button(dialog, "commitOperationPrimary")->isEnabled());
    QVERIFY(button(dialog, "commitOperationCancel")->isDefault());
    checkLocalizedState(model, dialog, Model::Phase::Preparing, QStringLiteral("preparing"));
    if (QTest::currentTestFailed()) return;
    traversalGate->release();
    QTRY_COMPARE(model.phase(), Model::Phase::Partial);
    checkLocalizedState(model, dialog, Model::Phase::Partial, QStringLiteral("partial"));
    if (QTest::currentTestFailed()) return;
    QVERIFY(button(dialog, "commitOperationRetry")->isVisible());
    QCOMPARE(button(dialog, "commitOperationPrimary")->text(), ui::I18n::tr("Continue…"));
    button(dialog, "commitOperationPrimary")->click();
    QCOMPARE(model.phase(), Model::Phase::Ready);
    QCOMPARE(backend->writes.load(), 0);
    checkLocalizedState(model, dialog, Model::Phase::Ready, QStringLiteral("ready"));
    if (QTest::currentTestFailed()) return;
    QVERIFY(!button(dialog, "commitOperationPrimary")->icon().isNull());
    button(dialog, "commitOperationPrimary")->click();
    QTRY_COMPARE(model.phase(), Model::Phase::Complete);
    checkLocalizedState(model, dialog, Model::Phase::Complete, QStringLiteral("complete"));
    if (QTest::currentTestFailed()) return;
    QVERIFY(button(dialog, "commitOperationReport")->isVisible());
    button(dialog, "commitOperationCancel")->click();
    QVERIFY(!dialog.isVisible());
  }
  void cancellationKeepsAWriteVisibleUntilItReturns_data() {
    QTest::addColumn<QString>("entry");
    for (const QString& entry : {QStringLiteral("title"), QStringLiteral("footer"), QStringLiteral("escape"), QStringLiteral("window")})
      QTest::newRow(qUtf8Printable(entry)) << entry;
  }
  void cancellationKeepsAWriteVisibleUntilItReturns() {
    QFETCH(QString, entry);
    ui::I18n::instance().setLang(app::LanguageId::En);
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->blockWrite = true;
    const auto release = qScopeGuard([backend] {
      backend->releaseWrite = true;
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    Model model(backend->request(), backend);
    uwf::simulation::FileDialogStub files;
    ui::CommitOperationDialog dialog(model, files, {}, nullptr);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* footer = button(dialog, "commitOperationCancel");
    QVERIFY(footer);
    QSignalSpy rejected(&dialog, &QDialog::rejected);
    QSignalSpy finished(&dialog, &QDialog::finished);
    const auto requestClose = [&] {
      if (entry == QStringLiteral("title"))
        QTest::mouseClick(dialog.closeButton(), Qt::LeftButton);
      else if (entry == QStringLiteral("footer"))
        QTest::mouseClick(footer, Qt::LeftButton);
      else if (entry == QStringLiteral("escape"))
        QTest::keyClick(&dialog, Qt::Key_Escape);
      else
        dialog.close();
    };
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    QVERIFY(dialog.closeButton()->isEnabled());
    QVERIFY(footer->isEnabled());
    button(dialog, "commitOperationPrimary")->click();
    QTRY_COMPARE(backend->writes.load(), 1);
    QVERIFY(dialog.closeButton()->isEnabled());
    QVERIFY(footer->isEnabled());
    requestClose();
    QVERIFY(dialog.isVisible());
    QCOMPARE(model.phase(), Model::Phase::Canceling);
    QVERIFY(!dialog.closeButton()->isEnabled());
    QVERIFY(!footer->isEnabled());
    QCOMPARE(rejected.count(), 0);
    QCOMPARE(finished.count(), 0);
    requestClose();
    QCOMPARE(model.phase(), Model::Phase::Canceling);
    QVERIFY(dialog.isVisible());
    QCOMPARE(rejected.count(), 0);
    QCOMPARE(finished.count(), 0);
    backend->releaseWrite = true;
    QTRY_COMPARE(model.phase(), Model::Phase::Canceled);
    QCOMPARE(model.rows().size(), 1);
    QCOMPARE(backend->writes.load(), 1);
    QVERIFY(dialog.closeButton()->isEnabled());
    QVERIFY(footer->isEnabled());
    checkGeometry(dialog);
    requestClose();
    QVERIFY(!dialog.isVisible());
    QCOMPARE(rejected.count(), 1);
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.front().front().toInt(), static_cast<int>(QDialog::Rejected));
  }
  void interactionsUseTheActiveOwnerAndPreserveLiteralTargets() {
    QWidget owner;
    owner.show();
    QDialog modal(&owner);
    modal.setWindowModality(Qt::WindowModal);
    modal.show();
    QVERIFY(QTest::qWaitForWindowExposed(&modal));
    auto backend = std::make_shared<test::commit::Backend>(false);
    auto request = backend->request();
    request.target.path = QStringLiteral("C:\\A&B<one>\\long-literal-path");
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        owner, [&] { ui::commitInteraction(&owner, {}, uwf::test::localFileDialogs()).run(request, backend); },
        [&](QDialog& active) {
          auto* dialog = qobject_cast<ui::CommitOperationDialog*>(&active);
          QVERIFY(dialog);
          QCOMPARE(dialog->parentWidget(), &modal);
          auto* target = dialog->findChild<ui::TextLabel*>(QStringLiteral("commitOperationTarget"));
          QCOMPARE(target->text(), request.target.path);
          QCOMPARE(target->textFormat(), Qt::PlainText);
          inspected = true;
          dialog->reject();
        }));
    QVERIFY(inspected);
    QCOMPARE(backend->writes.load(), 0);
  }
  void destroyingTheOwnerDuringPreparationDoesNotWaitOrLeaveACallback() {
    auto owner = std::make_unique<QWidget>();
    owner->show();
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->tree->block = true;
    backend->tree->ignoreStop = true;
    const auto finishWork = qScopeGuard([&] {
      backend->tree->release = true;
      QThreadPool::globalInstance()->waitForDone();
    });
    QTimer destroyOwner;
    destroyOwner.setInterval(5);
    connect(&destroyOwner, &QTimer::timeout, this, [&] {
      if (backend->tree->reads == 0) return;
      destroyOwner.stop();
      owner.reset();
    });
    destroyOwner.start();
    QElapsedTimer elapsed;
    elapsed.start();
    ui::commitInteraction(owner.get(), {}, uwf::test::localFileDialogs()).run(backend->request(), backend);
    QVERIFY(elapsed.elapsed() < 1000);
    backend->tree->release = true;
    QVERIFY(QThreadPool::globalInstance()->waitForDone(2000));
    QCOMPARE(backend->writes.load(), 0);
  }
};
}  // namespace
QTEST_MAIN(CommitOperationDialogTests)
#include "CommitOperationDialogTests.moc"
