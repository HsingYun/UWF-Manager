// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QAbstractScrollArea>
#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPoint>
#include <QProgressBar>
#include <QPushButton>
#include <QRect>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSize>
#include <QString>
#include <QTest>
#include <QTimer>
#include <QWidget>
#include <algorithm>
#include <chrono>
#include <optional>
#include <stdexcept>
#include <utility>

#include "cases/presentation/views/widgets/WidgetUiFixture.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"
#include "presentation/viewmodels/staging/FileStagingResultText.h"
#include "presentation/views/power/PowerActionDialog.h"
#include "support/ui/DialogControls.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

using namespace uwf::test::widgets;

namespace {
class WidgetPowerTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { initializeWidgets(); }
  void powerWarningAlignsWithFirstTextLine_data();
  void powerWarningAlignsWithFirstTextLine();
  void powerConfirmationDefaultsToCancel();
  void powerConfirmationEscapeAlwaysCancels();
  void powerConfirmationCanSkipStagingForDirectRestart();
  void powerConfirmationAcceptsOnlyTheExplicitAction();
  void powerDialogShowsProgressAndFailureInPlace();
  void powerFailureListUsesOneScrollOwner_data();
  void powerFailureListUsesOneScrollOwner();
  void powerFailureActionsMatchTheRecordedResult_data();
  void powerFailureActionsMatchTheRecordedResult();
  void powerLoadingKeepsTheSummaryAndFooterInPlace();
  void powerDialogCompactLayoutKeepsSafeActionsOutsideScrolling_data();
  void powerDialogCompactLayoutKeepsSafeActionsOutsideScrolling();
  void powerAutomaticPauseDisplaysItsReason_data();
  void powerAutomaticPauseDisplaysItsReason();
};

void WidgetPowerTests::powerAutomaticPauseDisplaysItsReason_data() {
  QTest::addColumn<int>("reason");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("chinese");
  using D = uwf::app::AutomaticCommitDisposition;
  for (const auto reason : {D::SkipHormActive, D::SkipHormTransition, D::SkipFullCommit, D::SkipReadOnlyMedia, D::SkipRetainedOverlay})
    for (const bool dark : {false, true})
      for (const bool chinese : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("reason-%1-%2-%3")
                                     .arg(int(reason))
                                     .arg(dark ? QStringLiteral("dark") : QStringLiteral("light"), chinese ? QStringLiteral("zh") : QStringLiteral("en"))))
            << int(reason) << dark << chinese;
}
void WidgetPowerTests::powerAutomaticPauseDisplaysItsReason() {
  using namespace uwf;
  using namespace ui;
  QFETCH(int, reason);
  QFETCH(bool, dark);
  QFETCH(bool, chinese);
  const auto oldLanguage = I18n::instance().lang();
  const auto oldTheme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(oldLanguage);
    ThemeManager::instance().apply(oldTheme);
  });
  I18n::instance().setLang(chinese ? I18n::Lang::Zh_CN : I18n::Lang::En);
  ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  app::AutomaticCommitSummary summary;
  summary.include(static_cast<app::AutomaticCommitDisposition>(reason));
  app::PowerActionDialogRequest request{app::PowerAction::Restart,
                                        [summary]() -> std::optional<app::PowerStagingPreparation> { return app::PowerStagingNotRequired{summary}; }};
  QVERIFY(test::driveModalDialog(
      *this, [&] { QCOMPARE(runPowerActionDialog(nullptr, std::move(request), std::chrono::milliseconds::zero()), app::PowerActionDialogOutcome::Canceled); },
      [&](QDialog& dialog) {
        auto* action = dialog.findChild<QPushButton*>(QStringLiteral("restartBtn"));
        auto* note = dialog.findChild<TextLabel*>(QStringLiteral("powerWarningText"));
        auto* count = dialog.findChild<TextLabel*>(QStringLiteral("powerFileCount"));
        QVERIFY(action && note && count);
        QTRY_VERIFY(action->isEnabled());
        QCOMPARE(count->text(), I18n::tr("Automatic commits paused"));
        QVERIFY(note->text().contains(presentation::automaticCommitDescription(static_cast<app::AutomaticCommitDisposition>(reason))));
        QTRY_VERIFY(note->height() >= note->heightForWidth(note->width()));
        QVERIFY(test::saveScreenshot(dialog, QStringLiteral("power-automatic-pause-%1").arg(QString::fromLatin1(QTest::currentDataTag()))));
        dialog.reject();
      }));
}

void WidgetPowerTests::powerWarningAlignsWithFirstTextLine_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("language");
  QTest::addColumn<int>("width");
  for (const auto& supported : uwf::ui::I18n::languages())
    for (const int width : {480, 588}) {
      const auto language = supported.id;
      const auto tag = QByteArray::number(static_cast<int>(language)) + '-' + QByteArray::number(width);
      QTest::newRow(tag.constData()) << language << width;
    }
}

void WidgetPowerTests::powerWarningAlignsWithFirstTextLine() {
  using namespace uwf::ui;
  QFETCH(I18n::Lang, language);
  QFETCH(int, width);
  const auto previous = I18n::instance().lang();
  const auto restore = qScopeGuard([&] { I18n::instance().setLang(previous); });
  I18n::instance().setLang(language);
  for (const auto action : {uwf::app::PowerAction::Shutdown, uwf::app::PowerAction::Restart}) {
    bool inspected = false;
    QVERIFY(uwf::test::driveModalDialog(
        *this, [&] { QCOMPARE(runPowerActionDialog(nullptr, {action}), uwf::app::PowerActionDialogOutcome::Canceled); },
        [&](QDialog& modal) {
          auto* dialog = &modal;
          dialog->resize(width, dialog->height());
          QVERIFY(QTest::qWaitForWindowExposed(dialog));
          QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
          QTest::qWait(30);
          auto* note = dialog->findChild<QWidget*>(QStringLiteral("powerWarningNote"));
          auto* icon = dialog->findChild<QWidget*>(QStringLiteral("powerWarningIcon"));
          auto* text = dialog->findChild<TextLabel*>(QStringLiteral("powerWarningText"));
          QVERIFY(note && icon && text && icon->isVisible() && text->isVisible());
          const auto renderedInk = [&](QWidget& widget, const QRectF& region) {
            const qreal scale = widget.devicePixelRatioF();
            QImage frame(QSize(qCeil(widget.width() * scale), qCeil(widget.height() * scale)), QImage::Format_ARGB32_Premultiplied);
            frame.setDevicePixelRatio(scale);
            const QColor background(Qt::magenta);
            frame.fill(background);
            widget.render(&frame, QPoint(), QRegion(), QWidget::RenderFlags{});
            return uwf::test::visibleInkBounds(frame, DesignSystem::instance().color(design::Color::Muted), background, region)
                .translated(widget.mapTo(note, QPoint()));
          };
          const auto iconInk = renderedInk(*icon, icon->rect());
          const auto textInk = renderedInk(*text, text->firstLineBoxBounds());
          QVERIFY(uwf::test::centersAligned(iconInk, textInk));
          QVERIFY(text->height() >= text->heightForWidth(text->width()));
          inspected = true;
        }));
    QVERIFY(inspected);
  }
}

void WidgetPowerTests::powerConfirmationDefaultsToCancel() {
  bool inspected = false;
  bool cancelWasDefault = false;
  bool enterRejectedDialog = false;

  QVERIFY(uwf::test::driveModalDialog(
      *this, [&] { QVERIFY(uwf::ui::runPowerActionDialog(nullptr, {uwf::app::PowerAction::Shutdown, {}}) == uwf::app::PowerActionDialogOutcome::Canceled); },
      [&](QDialog& modal) {
        auto* dialog = &modal;
        auto* cancel = uwf::test::buttonByRole(dialog, QDialogButtonBox::RejectRole);
        inspected = cancel != nullptr;
        cancelWasDefault = cancel && cancel->isDefault();
        QSignalSpy rejected(dialog, &QDialog::rejected);
        QTest::keyClick(dialog, Qt::Key_Return);
        enterRejectedDialog = rejected.count() == 1;
        if (dialog->isVisible()) dialog->reject();
      }));
  QVERIFY(inspected);
  QVERIFY(cancelWasDefault);
  QVERIFY(enterRejectedDialog);
}

void WidgetPowerTests::powerConfirmationEscapeAlwaysCancels() {
  bool escapeRejectedDialog = false;

  QVERIFY(uwf::test::driveModalDialog(
      *this, [&] { QVERIFY(uwf::ui::runPowerActionDialog(nullptr, {uwf::app::PowerAction::Restart, {}}) == uwf::app::PowerActionDialogOutcome::Canceled); },
      [&](QDialog& modal) {
        auto* dialog = &modal;
        QSignalSpy rejected(dialog, &QDialog::rejected);
        QTest::keyClick(dialog, Qt::Key_Escape);
        escapeRejectedDialog = rejected.count() == 1;
        if (dialog->isVisible()) dialog->reject();
      }));
  QVERIFY(escapeRejectedDialog);
}

void WidgetPowerTests::powerConfirmationCanSkipStagingForDirectRestart() {
  int preparationPolls = 0;
  bool safeRestartBlocked = false;
  bool directRestartAvailable = false;
  bool restartLabelsCorrect = false;

  uwf::app::PowerActionDialogRequest request{uwf::app::PowerAction::Restart, [&]() -> std::optional<uwf::app::PowerStagingPreparation> {
                                               ++preparationPolls;
                                               return uwf::app::PowerStagingNotRequired{};
                                             }};
  QVERIFY(uwf::test::driveModalDialog(
      *this, [&] { QVERIFY(uwf::ui::runPowerActionDialog(nullptr, std::move(request)) == uwf::app::PowerActionDialogOutcome::DirectConfirmed); },
      [&](QDialog& modal) {
        auto* dialog = &modal;
        auto* safeRestart = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"));
        auto* directRestart = dialog->findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
        safeRestartBlocked = safeRestart && !safeRestart->isEnabled();
        directRestartAvailable = directRestart && directRestart->isEnabled();
        const QString hint = directRestart ? directRestart->toolTip() : QString{};
        restartLabelsCorrect = safeRestart && safeRestart->text() == QStringLiteral("Safe restart") && directRestart &&
                               directRestart->text() == QStringLiteral("Direct restart…") && hint.startsWith(QStringLiteral("Direct restart"));
        if (directRestart) {
          QVERIFY(uwf::test::driveModalDialog(
              *dialog, [&] { QTest::mouseClick(directRestart, Qt::LeftButton); },
              [](QDialog& confirmation) {
                auto* confirm = uwf::test::buttonByRole(&confirmation, QDialogButtonBox::AcceptRole);
                QVERIFY(confirm);
                QTest::mouseClick(confirm, Qt::LeftButton);
              }));
        }
      }));
  QVERIFY(safeRestartBlocked);
  QVERIFY(directRestartAvailable);
  QVERIFY(restartLabelsCorrect);
  QCOMPARE(preparationPolls, 0);

  bool shutdownLabelsCorrect = false;
  QVERIFY(uwf::test::driveModalDialog(
      *this,
      [&] { QVERIFY(uwf::ui::runPowerActionDialog(nullptr, {uwf::app::PowerAction::Shutdown, {}}) == uwf::app::PowerActionDialogOutcome::DirectConfirmed); },
      [&](QDialog& modal) {
        auto* dialog = &modal;
        auto* safeShutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
        auto* directShutdown = dialog->findChild<QPushButton*>(QStringLiteral("directShutdownBtn"));
        const QString hint = directShutdown ? directShutdown->toolTip() : QString{};
        shutdownLabelsCorrect = safeShutdown && safeShutdown->text() == QStringLiteral("Safe shutdown") && directShutdown &&
                                directShutdown->text() == QStringLiteral("Direct shutdown…") && hint.startsWith(QStringLiteral("Direct shutdown"));
        if (directShutdown) {
          QVERIFY(uwf::test::driveModalDialog(
              *dialog, [&] { QTest::mouseClick(directShutdown, Qt::LeftButton); },
              [](QDialog& confirmation) {
                auto* confirm = uwf::test::buttonByRole(&confirmation, QDialogButtonBox::AcceptRole);
                QVERIFY(confirm);
                QTest::mouseClick(confirm, Qt::LeftButton);
              }));
        }
      }));
  QVERIFY(shutdownLabelsCorrect);
}

void WidgetPowerTests::powerConfirmationAcceptsOnlyTheExplicitAction() {
  QString heading;
  bool clicked = false;
  bool stagingSummaryVisible = false;
  bool actionDisabledDuringPreparation = false;
  qint64 preparationDurationMs = -1;
  int preparationPolls = 0;
  QElapsedTimer preparationElapsed;
  preparationElapsed.start();

  QTimer driver;
  driver.setInterval(1);
  connect(&driver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (auto* label = dialog->findChild<QLabel*>(QStringLiteral("dialogTitle"))) heading = label->text();
    if (auto* action = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"))) {
      if (!action->isEnabled()) {
        actionDisabledDuringPreparation = true;
        return;
      }
      preparationDurationMs = preparationElapsed.elapsed();
      for (auto labels = dialog->findChildren<QLabel*>(); const auto* label : std::as_const(labels)) {
        if (label->objectName() == QLatin1String("powerFileCount") && label->text() == QLatin1String("3")) stagingSummaryVisible = true;
      }
      clicked = true;
      QTest::mouseClick(action, Qt::LeftButton);
      driver.stop();
    } else {
      dialog->reject();
      driver.stop();
    }
  });
  driver.start();

  uwf::app::PowerActionDialogRequest request{uwf::app::PowerAction::Restart, [&]() -> std::optional<uwf::app::PowerStagingPreparation> {
                                               ++preparationPolls;
                                               if (preparationPolls == 1) return std::nullopt;
                                               return uwf::app::PowerStagingCommit{
                                                   3, [] { return uwf::app::PowerStagingProgress{uwf::app::PowerStagingState::Completed, 3, 3, {}, {}}; }};
                                             }};
  QVERIFY(uwf::ui::runPowerActionDialog(nullptr, std::move(request)) == uwf::app::PowerActionDialogOutcome::Confirmed);
  QVERIFY(clicked);
  QVERIFY(stagingSummaryVisible);
  QVERIFY(actionDisabledDuringPreparation);
  QVERIFY(preparationDurationMs >= 1000);
  QCOMPARE(preparationPolls, 2);
  QVERIFY(!heading.isEmpty());
}

void WidgetPowerTests::powerDialogShowsProgressAndFailureInPlace() {
  QDialog* initialDialog = nullptr;
  bool actionClicked = false;
  bool failureObserved = false;
  bool failureTextIsCompact = false;
  bool failureTextIsSelectable = false;
  bool sameDialog = false;
  bool cancelWasDefault = false;
  bool escapeIgnoredDuringStaging = false;
  bool progressObserved = false;
  int steps = 0;

  QTimer driver;
  driver.setInterval(1);
  connect(&driver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (!initialDialog) initialDialog = dialog;

    auto* details = dialog->findChild<uwf::ui::ContentHintBox*>(QStringLiteral("powerStagingFailureDetails"));
    if (auto* disclosure = dialog->findChild<uwf::ui::Disclosure*>(QStringLiteral("powerFailureDisclosure"));
        disclosure && disclosure->isVisible() && !disclosure->isOpen()) {
      QTest::mouseClick(disclosure->toggleButton(), Qt::LeftButton);
      return;
    }
    if (details && details->isVisible()) {
      const auto text = details->toPlainText();
      static const QRegularExpression leadingWhitespace(QStringLiteral("(?m)^[\\t ]+"));
      failureObserved = text.contains(QStringLiteral("C:\\locked.bin")) && text.contains(QStringLiteral("Provider rejected the commit."));
      failureTextIsCompact = !text.contains(QStringLiteral("\n\n")) && !leadingWhitespace.match(text).hasMatch();
      failureTextIsSelectable = details->textInteractionFlags().testFlag(Qt::TextSelectableByMouse);
      sameDialog = dialog == initialDialog;
      if (auto* cancel = uwf::test::buttonByRole(dialog, QDialogButtonBox::RejectRole)) {
        cancelWasDefault = cancel->isDefault();
        QTest::mouseClick(cancel, Qt::LeftButton);
        driver.stop();
        return;
      }
      dialog->reject();
      driver.stop();
      return;
    }

    if (!actionClicked) {
      if (auto* action = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"))) {
        if (!action->isEnabled()) return;
        actionClicked = true;
        QTest::mouseClick(action, Qt::LeftButton);
      }
    }
  });
  driver.start();

  uwf::app::PowerActionDialogRequest request{
      uwf::app::PowerAction::Restart, [&]() -> std::optional<uwf::app::PowerStagingPreparation> {
        return uwf::app::PowerStagingCommit{
            2, [&]() {
              ++steps;
              if (steps == 1) {
                auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
                if (dialog) {
                  QTest::keyClick(dialog, Qt::Key_Escape);
                  escapeIgnoredDuringStaging = dialog->isVisible();
                }
                return uwf::app::PowerStagingProgress{uwf::app::PowerStagingState::InProgress, 1, 2, QStringLiteral("C:\\first.bin"), {}};
              }
              auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
              auto* progress = dialog ? dialog->findChild<QProgressBar*>(QStringLiteral("powerStagingProgress")) : nullptr;
              auto* path = dialog ? dialog->findChild<QLineEdit*>(QStringLiteral("powerStagingCurrentPath")) : nullptr;
              progressObserved = progress && progress->isVisible() && progress->value() == 1 && progress->maximum() == 2 && path &&
                                 path->text() == QStringLiteral("C:\\first.bin");
              uwf::app::FileStagingCommitResult result;
              result.discoveredFiles = 2;
              result.committedFiles = 1;
              result.failures = {
                  {QStringLiteral("C:\\locked.bin"), uwf::app::FileStagingCommitFailureKind::ProviderFailure, QStringLiteral("Provider rejected the commit.")}};
              return uwf::app::PowerStagingProgress{
                  uwf::app::PowerStagingState::Failed, 2, 2, QStringLiteral("C:\\locked.bin"), uwf::presentation::renderFileStagingResult(result), result};
            }};
      }};
  QVERIFY(uwf::ui::runPowerActionDialog(nullptr, std::move(request)) == uwf::app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
  QCOMPARE(steps, 2);
  QVERIFY(actionClicked);
  QVERIFY(failureObserved);
  QVERIFY(failureTextIsCompact);
  QVERIFY(failureTextIsSelectable);
  QVERIFY(sameDialog);
  QVERIFY(cancelWasDefault);
  QVERIFY(escapeIgnoredDuringStaging);
  QVERIFY(progressObserved);
}

void WidgetPowerTests::powerFailureListUsesOneScrollOwner_data() {
  QTest::addColumn<int>("failureCount");
  QTest::addColumn<QSize>("windowSize");
  QTest::addColumn<bool>("chinese");
  QTest::addColumn<bool>("dark");
  for (const int failures : {8, 100})
    for (const QSize size : {QSize(588, 500), QSize(480, 420)})
      for (const bool chinese : {false, true})
        for (const bool dark : {false, true}) {
          const auto tag = QStringLiteral("%1-%2x%3-%4-%5")
                               .arg(failures)
                               .arg(size.width())
                               .arg(size.height())
                               .arg(chinese ? QStringLiteral("zh") : QStringLiteral("en"), dark ? QStringLiteral("dark") : QStringLiteral("light"));
          QTest::newRow(qUtf8Printable(tag)) << failures << size << chinese << dark;
        }
}

void WidgetPowerTests::powerFailureListUsesOneScrollOwner() {
  using namespace uwf;
  using namespace uwf::ui;
  QFETCH(int, failureCount);
  QFETCH(QSize, windowSize);
  QFETCH(bool, chinese);
  QFETCH(bool, dark);
  const auto oldLanguage = I18n::instance().lang();
  const auto oldTheme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(oldLanguage);
    ThemeManager::instance().apply(oldTheme);
  });
  I18n::instance().setLang(chinese ? I18n::Lang::Zh_CN : I18n::Lang::En);
  ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  const QString multilineReason = QStringLiteral("The file is in use.\nClose the application editing this file.\nRetry after releasing the file.");
  app::FileStagingCommitResult result;
  result.discoveredFiles = static_cast<std::size_t>(failureCount);
  for (int index = 1; index <= failureCount; ++index) {
    auto path = QStringLiteral("C:\\Data\\unique-failure-%1.json").arg(index, 3, 10, QChar('0'));
    if (index == 1) path += QStringLiteral("\\长目录与完整诊断路径").repeated(12) + QStringLiteral("\\file.json");
    result.failures.append({path, app::FileStagingCommitFailureKind::ProviderFailure, index == 1 ? multilineReason : QStringLiteral("Access denied.")});
  }
  bool inspected = false;
  app::PowerActionDialogRequest request{app::PowerAction::Restart, [result]() -> std::optional<app::PowerStagingPreparation> {
                                          return app::PowerStagingFailure{QStringLiteral("fallback failure text"), result};
                                        }};
  QVERIFY(uwf::test::driveModalDialog(
      *this, [&] { QCOMPARE(runPowerActionDialog(nullptr, std::move(request)), app::PowerActionDialogOutcome::CanceledAfterStagingFailure); },
      [&](QDialog& modal) {
        auto* dialog = &modal;
        dialog->resize(windowSize);
        auto* disclosure = dialog->findChild<Disclosure*>(QStringLiteral("powerFailureDisclosure"));
        auto* details = dialog->findChild<ContentHintBox*>(QStringLiteral("powerStagingFailureDetails"));
        auto* outer = dialog->findChild<QScrollArea*>(QStringLiteral("powerConfirmationScroll"));
        auto* cancel = uwf::test::buttonByRole(dialog, QDialogButtonBox::RejectRole);
        QVERIFY(disclosure && details && outer && cancel);
        QTRY_VERIFY(disclosure->isVisibleTo(dialog));
        disclosure->setOpen(true);
        QTRY_VERIFY(details->isVisibleTo(dialog));
        QTRY_COMPARE(details->verticalScrollBar()->maximum(), 0);
        QCOMPARE(details->horizontalScrollBar()->maximum(), 0);
        QTRY_VERIFY(details->viewport()->height() + 1 >= details->document()->size().height());
        const auto text = details->toPlainText();
        QCOMPARE(text.count(QStringLiteral("unique-failure-")), 8);
        QVERIFY(text.contains(multilineReason));
        for (int index = 0; index < 8; ++index) QVERIFY(text.contains(result.failures[index].path));
        if (failureCount > 8) QVERIFY(!text.contains(result.failures[8].path));
        QVERIFY(!text.contains(QStringLiteral("fallback failure text")));
        const QString summary = I18n::tr("Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5")
                                    .arg(failureCount)
                                    .arg(0)
                                    .arg(0)
                                    .arg(0)
                                    .arg(failureCount);
        bool summaryFound = false;
        bool remainingFound = false;
        const QString remaining = I18n::tr("%1 additional failure(s) were written to the log.").arg(92);
        for (auto labels = dialog->findChildren<TextLabel*>(); auto* label : std::as_const(labels)) {
          summaryFound = summaryFound || (label->isVisibleTo(dialog) && label->text() == summary);
          remainingFound = remainingFound || (label->isVisibleTo(dialog) && label->text() == remaining);
        }
        QVERIFY(summaryFound);
        QCOMPARE(remainingFound, failureCount == 100);
        QTRY_VERIFY(outer->verticalScrollBar()->maximum() > 0);
        int verticalScrollOwners = 0;
        for (auto areas = dialog->findChildren<QAbstractScrollArea*>(); auto* area : std::as_const(areas)) {
          if (!area->isVisibleTo(dialog) || area->verticalScrollBar()->maximum() == 0) continue;
          ++verticalScrollOwners;
          QCOMPARE(area, static_cast<QAbstractScrollArea*>(outer));
        }
        QCOMPARE(verticalScrollOwners, 1);
        outer->verticalScrollBar()->setValue(outer->verticalScrollBar()->maximum());
        QCOMPARE(outer->verticalScrollBar()->value(), outer->verticalScrollBar()->maximum());
        const QRect bottomLine(details->mapTo(outer->viewport(), QPoint(0, details->height() - 1)), QSize(details->width(), 1));
        QVERIFY(outer->viewport()->rect().intersects(bottomLine));
        QVERIFY(!outer->isAncestorOf(cancel));
        QVERIFY(cancel->isDefault());
        QVERIFY(dialog->rect().contains(QRect(cancel->mapTo(dialog, QPoint()), cancel->size())));
        QVERIFY(!cancel->visibleRegion().isEmpty());
        inspected = true;
      }));
  QVERIFY(inspected);
}

void WidgetPowerTests::powerFailureActionsMatchTheRecordedResult_data() {
  QTest::addColumn<bool>("shutdown");
  QTest::addColumn<int>("phase");
  QTest::addColumn<bool>("recordedTargets");
  for (const bool shutdown : {false, true})
    for (int phase = 1; phase < 5; ++phase)
      for (const bool recorded : {false, true}) {
        if (phase >= 3 && recorded) continue;
        const auto name = QStringLiteral("%1-phase-%2-%3").arg(shutdown ? "shutdown" : "restart").arg(phase).arg(recorded ? "targets" : "batch");
        QTest::newRow(qUtf8Printable(name)) << shutdown << phase << recorded;
      }
}
void WidgetPowerTests::powerFailureActionsMatchTheRecordedResult() {
  using namespace uwf;
  using namespace uwf::ui;
  QFETCH(bool, shutdown);
  QFETCH(int, phase);
  QFETCH(bool, recordedTargets);
  const QString detail = QStringLiteral("Original batch error\n完整原始错误详情。失败原因必须完整换行显示，不能丢失第二行。\n").repeated(8);
  app::PowerStagingFailure failure{detail};
  if (recordedTargets) {
    failure.result.emplace();
    failure.result->failures.append({QStringLiteral("C:\\Data\\failed.bin"), app::FileStagingCommitFailureKind::ProviderFailure, detail});
  }
  app::PowerActionDialogRequest request(shutdown ? app::PowerAction::Shutdown : app::PowerAction::Restart);
  request.pollStagingPreparation = [phase, failure]() -> std::optional<app::PowerStagingPreparation> {
    if (phase == 3) throw std::runtime_error(failure.details.toStdString());
    if (phase == 1) return failure;
    return app::PowerStagingCommit{1, [phase, failure] {
                                     if (phase == 4) throw std::runtime_error(failure.details.toStdString());
                                     return app::PowerStagingProgress{app::PowerStagingState::Failed, 0, 1, {}, failure.details, failure.result};
                                   }};
  };
  bool inspected = false;
  bool commitStarted = false;
  QTimer driver;
  driver.setInterval(5);
  connect(&driver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* action = dialog->findChild<QPushButton*>(shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"));
    auto* disclosure = dialog->findChild<Disclosure*>(QStringLiteral("powerFailureDisclosure"));
    if (!action || !disclosure) return;
    if (!disclosure->isVisibleTo(dialog)) {
      if ((phase == 2 || phase == 4) && action->isEnabled() && !commitStarted) {
        commitStarted = true;
        action->click();
      }
      return;
    }
    driver.stop();
    const auto dismiss = qScopeGuard([dialog] {
      if (dialog->isVisible()) dialog->reject();
    });
    dialog->resize(480, 420);
    auto* close = uwf::test::buttonByRole(dialog, QDialogButtonBox::RejectRole);
    auto* details = dialog->findChild<ContentHintBox*>(QStringLiteral("powerStagingFailureDetails"));
    QVERIFY(close && details);
    QCOMPARE(action->isVisibleTo(dialog), recordedTargets);
    QCOMPARE(action->isEnabled(), recordedTargets);
    QCOMPARE(close->text(), recordedTargets ? I18n::tr("Cancel") : I18n::tr("Close"));
    QVERIFY(close->isDefault());
    auto* direct = dialog->findChild<QPushButton*>(shutdown ? QStringLiteral("directShutdownBtn") : QStringLiteral("directRestartBtn"));
    QVERIFY(direct && !direct->isVisibleTo(dialog) && !direct->isEnabled());
    QCOMPARE(disclosure->isOpen(), !recordedTargets);
    disclosure->setOpen(true);
    QTRY_VERIFY(details->isVisibleTo(dialog));
    QVERIFY(details->toPlainText().contains(detail));
    QTRY_VERIFY(details->viewport()->height() + 1 >= details->document()->size().height());
    QCOMPARE(details->verticalScrollBar()->maximum(), 0);
    if (!recordedTargets) {
      action->click();
      QVERIFY(dialog->isVisible());
      close->click();
    } else {
      action->click();
    }
    inspected = true;
  });
  QTimer timeout;
  timeout.setSingleShot(true);
  connect(&timeout, &QTimer::timeout, this, [] {
    if (auto* dialog = QApplication::activeModalWidget()) dialog->close();
  });
  driver.start();
  timeout.start(6000);
  const auto outcome = runPowerActionDialog(nullptr, std::move(request));
  QVERIFY(inspected);
  QCOMPARE(outcome, recordedTargets ? app::PowerActionDialogOutcome::ContinuedAfterStagingFailure : app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
}

void WidgetPowerTests::powerLoadingKeepsTheSummaryAndFooterInPlace() {
  for (const auto action : {uwf::app::PowerAction::Shutdown, uwf::app::PowerAction::Restart}) {
    QSize initial;
    QRect cardBounds, footerBounds;
    bool sawLoading = false, sawReady = false;
    QTimer driver;
    driver.setInterval(10);
    connect(&driver, &QTimer::timeout, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!dialog) return;
      auto* button = dialog->findChild<QPushButton*>(action == uwf::app::PowerAction::Shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"));
      auto* card = dialog->findChild<QWidget*>(QStringLiteral("powerSafetyCard"));
      auto* footer = dialog->findChild<QWidget*>(QStringLiteral("dialogFooter"));
      if (!button || !card || !footer) return;
      const auto bounds = [&](QWidget* widget) { return QRect(widget->mapTo(dialog, QPoint()), widget->size()); };
      if (!button->isEnabled() && !sawLoading) {
        initial = dialog->size();
        cardBounds = bounds(card);
        footerBounds = bounds(footer);
        sawLoading = true;
      } else if (button->isEnabled()) {
        driver.stop();
        sawReady = true;
        const auto reject = qScopeGuard([&] { dialog->reject(); });
        QVERIFY(sawLoading);
        QCOMPARE(dialog->size(), initial);
        QCOMPARE(bounds(card), cardBounds);
        QCOMPARE(bounds(footer), footerBounds);
      }
    });
    QTimer timeout;
    timeout.setSingleShot(true);
    connect(&timeout, &QTimer::timeout, this, [] {
      if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) dialog->reject();
    });
    driver.start();
    timeout.start(4000);
    uwf::app::PowerActionDialogRequest request{action, []() -> std::optional<uwf::app::PowerStagingPreparation> {
                                                 return uwf::app::PowerStagingCommit{58543, [] { return uwf::app::PowerStagingProgress{}; }};
                                               }};
    QCOMPARE(uwf::ui::runPowerActionDialog(nullptr, std::move(request)), uwf::app::PowerActionDialogOutcome::Canceled);
    QVERIFY(sawLoading && sawReady);
  }
}

void WidgetPowerTests::powerDialogCompactLayoutKeepsSafeActionsOutsideScrolling_data() {
  QTest::addColumn<bool>("chinese");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("failure");
  for (const bool chinese : {false, true})
    for (const bool dark : {false, true})
      for (const bool failure : {false, true}) {
        const QByteArray name = QStringLiteral("%1-%2-%3").arg(chinese ? "zh" : "en", dark ? "dark" : "light", failure ? "failure" : "confirmation").toLatin1();
        QTest::newRow(name.constData()) << chinese << dark << failure;
      }
}

void WidgetPowerTests::powerDialogCompactLayoutKeepsSafeActionsOutsideScrolling() {
  using namespace uwf::ui;
  QFETCH(bool, chinese);
  QFETCH(bool, dark);
  QFETCH(bool, failure);
  const auto language = I18n::instance().lang();
  const auto theme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(language);
    ThemeManager::instance().apply(theme);
  });
  I18n::instance().setLang(chinese ? I18n::Lang::Zh_CN : I18n::Lang::En);
  ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  bool inspected = false;
  uwf::app::PowerActionDialogRequest request{uwf::app::PowerAction::Restart};
  if (failure) {
    int screenHeight = 1;
    for (auto screens = QGuiApplication::screens(); const auto* screen : std::as_const(screens))
      screenHeight = std::max(screenHeight, screen->availableGeometry().height());
    const int lineSpacing = std::max(1, QFontMetrics(QApplication::font()).lineSpacing());
    const int lines = 2 * (screenHeight / lineSpacing + 1);
    request.pollStagingPreparation = [lines]() -> std::optional<uwf::app::PowerStagingPreparation> {
      return uwf::app::PowerStagingFailure{QStringLiteral("Mock failure details:\n").repeated(lines)};
    };
  }
  QVERIFY(uwf::test::driveModalDialog(
      *this,
      [&] {
        const auto outcome = runPowerActionDialog(nullptr, std::move(request));
        QCOMPARE(outcome, failure ? uwf::app::PowerActionDialogOutcome::CanceledAfterStagingFailure : uwf::app::PowerActionDialogOutcome::Canceled);
      },
      [&](QDialog& modal) {
        auto* dialog = &modal;
        if (failure) {
          auto* disclosure = dialog->findChild<Disclosure*>(QStringLiteral("powerFailureDisclosure"));
          QVERIFY(disclosure);
          QTRY_VERIFY(disclosure->isVisibleTo(dialog));
        }
        QTRY_VERIFY(dialog->minimumSize() != dialog->maximumSize());
        dialog->resize(760, 420);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
        QCOMPARE(dialog->size(), QSize(760, 420));
        const auto clipping = uwf::test::waitForUnclippedText(*dialog, 2000);
        QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
        auto* scroll = dialog->findChild<QScrollArea*>(QStringLiteral("powerConfirmationScroll"));
        QVERIFY(scroll);
        QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
        if (failure) QTRY_VERIFY(scroll->verticalScrollBar()->maximum() > 0);
        // A user-sized compact window may need scrolling in either state. The
        // complete body stays reachable while actions remain outside its viewport.
        auto* scrollbar = scroll->verticalScrollBar();
        scrollbar->setValue(scrollbar->maximum());
        QCOMPARE(scrollbar->value(), scrollbar->maximum());
        for (auto dialogButtons = dialog->findChildren<QPushButton*>(); auto* button : std::as_const(dialogButtons)) {
          if (!button->isVisibleTo(dialog)) continue;
          QVERIFY(!scroll->isAncestorOf(button));
          QVERIFY(dialog->rect().contains(QRect(button->mapTo(dialog, QPoint()), button->size())));
          QVERIFY(!button->visibleRegion().isEmpty());
        }
        auto* cancel = uwf::test::buttonByRole(dialog, QDialogButtonBox::RejectRole);
        QVERIFY(cancel && cancel->isDefault());
        const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
        if (!screenshots.isEmpty()) {
          QVERIFY(QDir().mkpath(screenshots));
          QVERIFY(uwf::test::screenshot(*dialog).save(QDir(screenshots).filePath("power-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
        }
        inspected = true;
      }));
  QVERIFY(inspected);
}

}  // namespace
QTEST_MAIN(WidgetPowerTests)
#include "PowerCases.moc"
