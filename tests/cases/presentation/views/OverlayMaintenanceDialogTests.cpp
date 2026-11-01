// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPixmap>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTabBar>
#include <QTest>
#include <QVBoxLayout>
#include <QtMath>
#include <array>
#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/BusyIndicator.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"
#include "presentation/views/configuration/overlay/HormUsageDialog.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDetailsDialog.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDialog.h"
#include "presentation/views/configuration/overlay/OverlayMaintenanceDialogs.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/OverlayPolicyFixture.h"
#include "support/ui/DialogControls.h"
#include "support/ui/LayoutSettling.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/visual/InkBounds.h"
#include "support/visual/InlineIconTextProbe.h"
#include "support/visual/VisualGeometry.h"

namespace {
using namespace uwf;
using Model = presentation::OverlayMaintenanceViewModel;
class Actions final : public app::OverlayMaintenanceActions {
 public:
  app::OverlayMaintenanceSnapshot state;
  std::atomic_int commits = 0, swaps = 0, hormWrites = 0;
  bool confirmCommit = true;
  std::atomic_bool holdObservation = false;
  Actions() {
    state.filterEnabled = true;
    state.overlayCommit = false;
    state.hormEnabled = false;
    state.currentOverlayType = state.nextOverlayType = 0;
    state.maximumOverlaySize = 4096;
    state.currentFlags = state.nextFlags = app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia};
    state.volumesReadable = true;
    state.fullCommitCapability.support = state.swapCapability.support = state.hormCapability.support = app::OverlayMaintenanceSupport::Supported;
    state.hibernationAvailable = state.allVolumesProtected = true;
    state.romDisposition = 0;
    state.systemDrive = QStringLiteral("C:");
    state.systemVolumeId = QStringLiteral("volume-c");
    state.currentStorage.origin = state.nextStorage.origin = app::OverlayStorageOrigin::NotUsed;
    state.volumes = {{QStringLiteral("volume-c"), QStringLiteral("C:"), QStringLiteral("System"), 256ULL << 30, true, 0, 0},
                     {QStringLiteral("volume-d"), QStringLiteral("D:"), QStringLiteral("Data"), 608ULL << 30, true, 0, 0}};
  }
  app::OverlayMaintenanceSnapshot observe(const std::stop_token stop) override {
    while (holdObservation && !stop.stop_requested()) std::this_thread::sleep_for(std::chrono::milliseconds{1});
    return state;
  }
  std::optional<app::OverlayMaintenanceSnapshot> snapshot() const override { return state; }
  app::OverlayMaintenanceResult fullCommit(std::stop_token) override {
    ++commits;
    if (confirmCommit) state.overlayCommit = true;
    app::OverlayMaintenanceResult result;
    result.snapshot = state;
    result.completionConfirmed = confirmCommit;
    result.effect = app::WriteEffect::Confirmed;
    return result;
  }
  app::OverlayMaintenanceResult createSwap(const QString& id, std::stop_token) override {
    ++swaps;
    for (auto& volume : state.volumes) volume.nextMarker = volume.id == id ? 1U : 0U;
    state.nextOverlayType = 1;
    app::OverlayMaintenanceResult result;
    result.snapshot = state;
    result.completionConfirmed = true;
    return result;
  }
  app::OverlayMaintenanceResult setHorm(const bool enabled, std::stop_token) override {
    ++hormWrites;
    state.hormEnabled = enabled;
    app::OverlayMaintenanceResult result;
    result.snapshot = state;
    result.completionConfirmed = true;
    return result;
  }
};
struct Fixture {
  std::shared_ptr<Actions> actions = std::make_shared<Actions>();
  std::shared_ptr<Model> model = std::make_shared<Model>(actions);
  Fixture() { model->setInteractionContext(true, false); }
};
QPushButton* button(QWidget& dialog, const char* name) { return dialog.findChild<QPushButton*>(QString::fromLatin1(name)); }
bool expose(QDialog& dialog) {
  dialog.show();
  dialog.activateWindow();
  if (!QTest::qWaitForWindowExposed(&dialog) || !QTest::qWaitForWindowActive(&dialog)) return false;
  auto* activity = dialog.findChild<QWidget*>(QStringLiteral("maintenanceActivity"));
  return !activity || QTest::qWaitFor([&] { return !activity->isVisibleTo(&dialog); });
}
void checkLayout(QDialog& dialog) {
  dialog.resize(360, dialog.height());
  const auto clipping = test::waitForUnclippedText(dialog);
  QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
  for (auto scrollers = dialog.findChildren<QScrollArea*>(); auto* scroll : std::as_const(scrollers)) QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
}
QRectF paintedTextBounds(ui::TextLabel& label, const QWidget& owner) {
  const qreal dpr = label.devicePixelRatioF();
  QImage pixels(QSize(qCeil(label.width() * dpr), qCeil(label.height() * dpr)), QImage::Format_ARGB32_Premultiplied);
  pixels.setDevicePixelRatio(dpr);
  pixels.fill(Qt::transparent);
  label.render(&pixels, QPoint(), QRegion(), QWidget::RenderFlags{});
  QRect bounds;
  // Measure glyph coverage independently of the theme's text and background
  // colors, using the same 30% mask as the browser comparison.
  for (int y = 0; y < pixels.height(); ++y)
    for (int x = 0; x < pixels.width(); ++x)
      if (qAlpha(pixels.pixel(x, y)) >= 77) bounds = bounds.united(QRect(x, y, 1, 1));
  if (bounds.isEmpty()) return {};
  return test::logicalBounds(bounds, dpr).translated(label.mapTo(&owner, QPoint()));
}
class MaintenanceFrame final {
 public:
  explicit MaintenanceFrame(QDialog& dialog) : m_dialog(dialog), m_scroll(dialog.findChild<QScrollArea*>()) {}
  void invalidate() { m_image = {}; }
  bool show(QWidget& target, const int xmargin = 50, const int ymargin = 50) {
    if (m_image.isNull() && !test::waitForLayoutSettled(m_dialog)) return false;
    const auto before = scrollPosition();
    if (m_scroll) m_scroll->ensureWidgetVisible(&target, xmargin, ymargin);
    if (scrollPosition() != before && !test::waitForLayoutSettled(m_dialog)) return false;
    const auto position = scrollPosition();
    if (m_image.isNull() || position != m_position) {
      m_image = m_dialog.grab().toImage();
      m_position = position;
      ++m_revision;
    }
    return !m_image.isNull();
  }
  const QImage& image() const { return m_image; }
  int revision() const { return m_revision; }

 private:
  QPoint scrollPosition() const { return m_scroll ? QPoint(m_scroll->horizontalScrollBar()->value(), m_scroll->verticalScrollBar()->value()) : QPoint{}; }
  QDialog& m_dialog;
  QScrollArea* m_scroll;
  QImage m_image;
  QPoint m_position;
  int m_revision = 0;
};
bool writeAlignmentAudit(const QString& directory, const QString& filename, const QJsonArray& measurements) {
  if (directory.isEmpty()) return true;
  QFile output(QDir(directory).filePath(filename));
  const auto contents = QJsonDocument(measurements).toJson();
  return output.open(QIODevice::WriteOnly) && output.write(contents) == contents.size() && output.flush();
}
class OverlayMaintenanceDialogTests final : public QObject {
  Q_OBJECT
 private slots:
  void init() {
    ui::I18n::instance().setLang(ui::I18n::Lang::En);
    ui::ThemeManager::instance().apply(app::Theme::Light);
  }
  void conflictBulletsFollowTheFirstRenderedLine_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<bool>("dark");
    QTest::addColumn<int>("width");
    for (const auto& language : app::languages())
      for (const bool dark : {false, true})
        for (const int width : {360, 660})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(QString(language.code), dark ? "dark" : "light").arg(width)))
              << static_cast<int>(language.id) << dark << width;
  }
  void conflictBulletsFollowTheFirstRenderedLine() {
    QFETCH(int, language);
    QFETCH(bool, dark);
    QFETCH(int, width);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    presentation::ConfigurationViewModel configuration{16384};
    configuration.resetSnapshot(test::application::editableSnapshot(), test::application::diskPersistentPolicy());
    auto model = configuration.overlayPolicy();
    auto edit = model->beginEdit();
    presentation::OverlayAdvancedViewModel::setValue(edit, presentation::OverlayAdvancedViewModel::Field::ReadOnlyMedia, true);
    QVERIFY(!model->conflicts(edit).isEmpty());
    ui::OverlayAdvancedDetailsDialog dialog(model, edit, ui::OverlayAdvancedDetailsDialog::Purpose::Conflicts);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    dialog.resize(width, dialog.height());
    QVERIFY(test::waitForLayoutSettled(dialog));
    const auto frame = dialog.grab().toImage();
    const auto dpr = frame.devicePixelRatio();
    const auto color = ui::DesignSystem::instance().color(ui::design::Color::Muted);
    const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR");
    const auto filename = QStringLiteral("conflict-bullets-%1").arg(QString::fromLatin1(QTest::currentDataTag()));
    if (!directory.isEmpty()) {
      QVERIFY(QDir().mkpath(directory));
      QVERIFY(frame.save(QDir(directory).filePath(filename + QStringLiteral(".png"))));
    }
    QJsonArray measurements;
    const auto bullets = dialog.findChildren<ui::TextLabel*>(QStringLiteral("overlayConflictBullet"));
    QVERIFY(!bullets.isEmpty());
    for (auto* bullet : bullets) {
      QVERIFY2(bullet->parentWidget()->rect().contains(bullet->geometry()), "A first-line bullet must fit its alignment host without clipping");
      auto* reason = bullet->parentWidget()->parentWidget()->findChild<ui::TextLabel*>(QStringLiteral("overlayConflictReason"));
      QVERIFY(reason);
      const QRectF graphic(bullet->mapTo(&dialog, QPoint()), bullet->size());
      const auto line = reason->firstLineBoxBounds().translated(reason->mapTo(&dialog, QPoint()));
      const auto background = frame.pixelColor(qFloor(graphic.left() * dpr), qFloor(graphic.top() * dpr));
      const auto bulletInk = test::visibleInkBounds(frame, color, background, graphic);
      const auto textInk = test::visibleInkBounds(frame, color, background, line);
      const auto bulletCenter = static_cast<qreal>(qRound64(bulletInk.center().y() * dpr * 2)) / 2.0;
      const auto textCenter = static_cast<qreal>(qRound64(textInk.center().y() * dpr * 2)) / 2.0;
      const auto difference = qAbs(bulletCenter - textCenter);
      const auto rawBullet = bullet->firstLineInkBounds().translated(bullet->mapTo(&dialog, QPoint())).center().y() * dpr;
      const auto rawText = reason->firstLineInkBounds().translated(reason->mapTo(&dialog, QPoint())).center().y() * dpr;
      measurements.append(QJsonObject{{"resource", "literal:bullet"},
                                      {"widget", "overlayConflictBullet"},
                                      {"rule", "first-visible-line"},
                                      {"dpr", dpr},
                                      {"graphicCenterPhysical", bulletCenter},
                                      {"textCenterPhysical", textCenter},
                                      {"centerDifferencePhysical", difference},
                                      {"rawGraphicCenterPhysical", rawBullet},
                                      {"rawTextCenterPhysical", rawText}});
      if (!directory.isEmpty()) {
        QFile output(QDir(directory).filePath(filename + QStringLiteral(".json")));
        QVERIFY(output.open(QIODevice::WriteOnly));
        QVERIFY(output.write(QJsonDocument(measurements).toJson()) > 0);
      }
      QVERIFY(!bulletInk.isEmpty() && !textInk.isEmpty());
      QVERIFY2(difference <= 1, qUtf8Printable(QStringLiteral("%1: bullet %2 (raw %3), first line %4 (raw %5), delta %6 physical pixels")
                                                   .arg(reason->text())
                                                   .arg(bulletCenter)
                                                   .arg(rawBullet)
                                                   .arg(textCenter)
                                                   .arg(rawText)
                                                   .arg(difference)));
    }
    if (!directory.isEmpty()) {
      QVERIFY(QDir().mkpath(directory));
      QVERIFY(frame.save(QDir(directory).filePath(filename + QStringLiteral(".png"))));
      QFile output(QDir(directory).filePath(filename + QStringLiteral(".json")));
      QVERIFY(output.open(QIODevice::WriteOnly));
      QVERIFY(output.write(QJsonDocument(measurements).toJson()) > 0);
    }
  }
  void hormChecklistUsesTheSameExactConfigurationAsSubmission_data() {
    QTest::addColumn<quint32>("flags");
    QTest::addColumn<bool>("ready");
    QTest::newRow("exact-rom") << quint32{4} << true;
    QTest::newRow("opaque-low-bit") << quint32{12} << false;
    QTest::newRow("opaque-high-bit") << quint32{0x80000004} << false;
  }
  void hormChecklistUsesTheSameExactConfigurationAsSubmission() {
    QFETCH(quint32, flags);
    QFETCH(bool, ready);
    Fixture f;
    f.actions->state.currentFlags = app::OverlayFlags{flags};
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* submit = button(dialog, "hormConfirm");
    auto* rom = dialog.findChild<ui::TextLabel*>(QStringLiteral("hormRequirementState1"));
    QVERIFY(submit && rom);
    QCOMPARE(rom->text(), ready ? ui::I18n::tr("Met") : ui::I18n::tr("Not met"));
    QCOMPARE(submit->isEnabled(), ready);
    QCOMPARE(submit->toolTip().isEmpty(), ready);
  }
  void storageTypeAndFailuresUseTheSelectedLanguage() {
    Fixture f;
    f.actions->state.currentOverlayType = f.actions->state.nextOverlayType = 1;
    f.actions->state.filterEnabled = false;
    f.actions->state.nextStorage.issue = app::OverlayStorageIssue::MultipleHosts;
    f.actions->state.nextStorage.diagnostics = QStringLiteral("Provider 0x80041001");
    ui::I18n::instance().setLang(ui::I18n::Lang::Zh_CN);
    ui::SwapDialog dialog(f.model);
    QVERIFY(expose(dialog));
    QCOMPARE(f.model->currentOverlayTypeText(), QStringLiteral("磁盘"));
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("swapCurrentOverlayType"))->text(), QStringLiteral("磁盘"));
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("swapTargetOverlayType"))->text(), QStringLiteral("磁盘"));
    QVERIFY(f.model->storageLocationError(f.model->snapshot().nextStorage).startsWith(QStringLiteral("多个")));
    QVERIFY(f.model->storageLocationError(f.model->snapshot().nextStorage).endsWith(f.actions->state.nextStorage.diagnostics));
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("swapPreviousHost"))->toolTip(), f.model->storageLocationError(f.model->snapshot().nextStorage));
    f.actions->state.nextOverlayType = 0;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    QCOMPARE(f.model->storageTargetTypeText(), QStringLiteral("RAM → 磁盘"));
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("swapTargetOverlayType"))->text(), QStringLiteral("RAM → 磁盘"));
    ui::I18n::instance().setLang(ui::I18n::Lang::En);
    QCOMPARE(f.model->storageTargetTypeText(), QStringLiteral("RAM → Disk"));
  }
  void fullCommitConfirmationOnlyAddsADraft() {
    Fixture f;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::FullCommitDialog canceled(f.model);
    QVERIFY(expose(canceled));
    QVERIFY(button(canceled, "fullCommitCancel")->isDefault());
    button(canceled, "fullCommitCancel")->click();
    QVERIFY(!f.model->hasPendingChanges());
    ui::FullCommitDialog dialog(f.model);
    QVERIFY(expose(dialog));
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    button(dialog, "fullCommitStart")->click();
    QCOMPARE(accepted.size(), 1);
    QVERIFY(f.model->draft().fullCommit);
    QCOMPARE(f.model->snapshot().overlayCommit, std::optional<bool>{false});
    QCOMPARE(f.actions->commits.load(), 0);
    f.actions->state.overlayCommit = true;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::FullCommitDialog observed(f.model);
    QVERIFY(expose(observed));
    QVERIFY(!button(observed, "fullCommitStart")->isVisible());
    QVERIFY(!f.model->draft().fullCommit);
    QCOMPARE(f.actions->commits.load(), 0);
  }
  void storageDraftCanJoinOtherChangesWithoutWriting() {
    Fixture f;
    f.actions->state.filterEnabled = false;
    f.actions->state.currentFlags = f.actions->state.nextFlags = app::OverlayFlags{0};
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    f.model->setInteractionContext(true, false);
    ui::SwapDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* volumes = dialog.findChild<QComboBox*>(QStringLiteral("swapVolume"));
    QVERIFY(volumes);
    volumes->setCurrentIndex(volumes->findData(QStringLiteral("volume-d")));
    QVERIFY(button(dialog, "swapCreate")->isEnabled());
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("swapCapacitySummary"))->text(), ui::I18n::tr("%1 MB").arg(4096));
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    button(dialog, "swapCreate")->click();
    QCOMPARE(accepted.size(), 1);
    QCOMPARE(f.model->draft().storageVolume, std::optional<QString>{QStringLiteral("volume-d")});
    QCOMPARE(f.actions->swaps.load(), 0);
    QCOMPARE(f.model->snapshot().volumes[1].nextMarker, std::optional<std::uint32_t>{0});
    QCOMPARE(f.model->snapshot().nextOverlayType, std::optional<std::int64_t>{0});
    ui::SwapDialog reopened(f.model);
    QVERIFY(expose(reopened));
    QCOMPARE(reopened.findChild<QComboBox*>(QStringLiteral("swapVolume"))->currentData().toString(), QStringLiteral("volume-d"));
  }
  void storageDefaultsToSystemDriveAndDoesNotInventAnObservedHost() {
    Fixture f;
    f.actions->state.filterEnabled = false;
    f.actions->state.volumes.insert(f.actions->state.volumes.begin(),
                                    {QStringLiteral("volume-f"), QStringLiteral("F:"), QStringLiteral("Other"), 100ULL << 30, true, 0, 0});
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::SwapDialog dialog(f.model);
    QVERIFY(expose(dialog));
    QCOMPARE(dialog.findChild<QComboBox*>(QStringLiteral("swapVolume"))->currentData().toString(), QStringLiteral("volume-c"));
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("swapTargetSummary"))->text(), QStringLiteral("C:"));
    QCOMPARE(f.model->snapshot().nextStorage.origin, app::OverlayStorageOrigin::NotUsed);
    QVERIFY(!f.model->hasPendingChanges());
    QCOMPARE(f.actions->swaps.load(), 0);
  }
  void storageSelectionSurvivesRetranslationAndRefreshWithoutLeakingIntoAnotherDialog() {
    Fixture f;
    f.actions->state.filterEnabled = false;
    ui::SwapDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* volumes = dialog.findChild<QComboBox*>(QStringLiteral("swapVolume"));
    QVERIFY(volumes);
    volumes->setCurrentIndex(volumes->findData(QStringLiteral("volume-d")));
    ui::I18n::instance().setLang(ui::I18n::Lang::Zh_CN);
    QCOMPARE(volumes->currentData().toString(), QStringLiteral("volume-d"));
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("swapTargetSummary"))->text(), QStringLiteral("D:"));
    f.actions->state.nextStorage.volumeId = QStringLiteral("volume-c");
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    QCOMPARE(volumes->currentData().toString(), QStringLiteral("volume-d"));
    QVERIFY(button(dialog, "swapCreate")->isEnabled());
    dialog.reject();
    QVERIFY(!f.model->hasPendingChanges());
    ui::SwapDialog reopened(f.model);
    QVERIFY(expose(reopened));
    QCOMPARE(reopened.findChild<QComboBox*>(QStringLiteral("swapVolume"))->currentData().toString(), QStringLiteral("volume-c"));
    QCOMPARE(f.actions->swaps.load(), 0);
  }
  void storageHeroKeepsTheTechnicalNameBelowTheMainText() {
    Fixture f;
    ui::SwapDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* heading = dialog.findChild<ui::TextLabel*>(QStringLiteral("maintenanceHeroHeading"));
    auto* technical = dialog.findChild<ui::TextLabel*>(QStringLiteral("maintenanceHeroTechnical"));
    auto* mark = dialog.findChild<QWidget*>(QStringLiteral("maintenanceHeaderIcon"));
    QVERIFY(heading && technical && mark);
    QCOMPARE(heading->text(), ui::I18n::tr("Choose where to store the disk overlay"));
    QCOMPARE(technical->text(), QStringLiteral("create swapfile"));
    QCOMPARE(heading->font().pixelSize(), 14);
    QCOMPARE(technical->font().pixelSize(), 11);
    for (const int width : {660, 360}) {
      dialog.resize(width, dialog.height());
      QTRY_VERIFY(heading->mapTo(&dialog, heading->rect().bottomLeft()).y() < technical->mapTo(&dialog, QPoint()).y());
      QTRY_COMPARE(heading->mapTo(&dialog, QPoint()).x(), technical->mapTo(&dialog, QPoint()).x());
      QVERIFY(mark->mapTo(&dialog, mark->rect().topRight()).x() < heading->mapTo(&dialog, QPoint()).x());
      const auto clipping = test::waitForUnclippedText(dialog);
      QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    }
    QCOMPARE(f.actions->swaps.load(), 0);
  }
  void storageWarningHasASeparateFormGapAndALabelColon() {
    Fixture f;
    ui::SwapDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* feedback = dialog.findChild<QWidget*>(QStringLiteral("maintenanceFeedback"));
    auto* label = dialog.findChild<ui::TextLabel*>(QStringLiteral("swapVolumeLabel"));
    auto* gap = dialog.findChild<QWidget*>(QStringLiteral("swapFeedbackSpacing"));
    QVERIFY(feedback && label && gap);
    QVERIFY(feedback->isVisibleTo(&dialog));
    QCOMPARE(label->text(), ui::I18n::tr("Storage volume:"));
    QVERIFY(label->mapTo(&dialog, QPoint()).y() - feedback->mapTo(&dialog, QPoint(0, feedback->height())).y() >= 16);
    dialog.reject();
    f.actions->state.filterEnabled = false;
    ui::SwapDialog ready(f.model);
    QVERIFY(expose(ready));
    QVERIFY(!ready.findChild<QWidget*>(QStringLiteral("swapFeedbackSpacing"))->isVisibleTo(&ready));
  }
  void hormChecksAreReadBeforeEnablingAndCanBeReadAgain() {
    Fixture f;
    f.actions->state.hibernationAvailable.reset();
    f.actions->state.hibernationError = QStringLiteral("Cannot read the hibernation file");
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    QVERIFY(!button(dialog, "hormConfirm")->isEnabled());
    QVERIFY(dialog.findChild<QWidget*>(QStringLiteral("hormRequirementState2"))->toolTip().contains(f.actions->state.hibernationError));
    f.actions->state.hibernationAvailable = true;
    f.actions->state.hibernationError.clear();
    dialog.reject();
    ui::HormDialog reopened(f.model);
    QVERIFY(expose(reopened));
    QVERIFY(button(reopened, "hormConfirm")->isEnabled());
    QVERIFY(!button(reopened, "hormRefresh"));
    QVERIFY(!f.model->hasPendingChanges());
    QCOMPARE(f.actions->hormWrites.load(), 0);
  }
  void hormHeroesKeepTheTechnicalNameBelowTheMainText() {
    Fixture f;
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    dialog.resize(660, dialog.height());
    QCOMPARE(dialog.windowTitle(), ui::I18n::tr("Enable saved-state startup?"));
    auto* copy = dialog.findChild<QWidget*>(QStringLiteral("maintenanceHeroCopy"));
    auto* technical = dialog.findChild<ui::TextLabel*>(QStringLiteral("maintenanceHeroTechnical"));
    auto* heading = dialog.findChild<ui::TextLabel*>(QStringLiteral("maintenanceHeroHeading"));
    auto* mark = dialog.findChild<QWidget*>(QStringLiteral("maintenanceHeaderIcon"));
    QVERIFY(copy && technical && heading && mark);
    QCOMPARE(copy->findChildren<ui::TextLabel*>().size(), 2);
    QCOMPARE(heading->text(), ui::I18n::tr("Resume the same hibernation state repeatedly"));
    QCOMPARE(technical->text(), QStringLiteral("HORM"));
    QCOMPARE(heading->font().pixelSize(), 14);
    QCOMPARE(technical->font().pixelSize(), 11);
    QVERIFY(heading->alignment().testFlag(Qt::AlignVCenter));
    QVERIFY(technical->alignment().testFlag(Qt::AlignVCenter));
    for (const int width : {660, 360}) {
      dialog.resize(width, dialog.height());
      QTRY_VERIFY(heading->mapTo(&dialog, heading->rect().bottomLeft()).y() < technical->mapTo(&dialog, QPoint()).y());
      QTRY_COMPARE(heading->mapTo(&dialog, QPoint()).x(), technical->mapTo(&dialog, QPoint()).x());
      QVERIFY(mark->mapTo(&dialog, mark->rect().topRight()).x() < heading->mapTo(&dialog, QPoint()).x());
      const auto clipping = test::waitForUnclippedText(dialog);
      QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    }
    dialog.reject();
    ui::HormUsageDialog usage;
    QVERIFY(expose(usage));
    usage.resize(660, usage.height());
    auto* hero = usage.findChild<QWidget*>(QStringLiteral("hormUsageHeading"));
    auto* official = usage.findChild<ui::TextLabel*>(QStringLiteral("officialName"));
    auto* usageTitle = usage.findChild<ui::TextLabel*>(QStringLiteral("hormUsageHeroTitle"));
    auto* icon = usage.findChild<QWidget*>(QStringLiteral("hormUsageIcon"));
    QVERIFY(hero && official && usageTitle && icon);
    QCOMPARE(hero->findChildren<ui::TextLabel*>().size(), 2);
    QCOMPARE(usageTitle->text(), ui::I18n::tr("Setup and operation"));
    QCOMPARE(official->text(), technical->text());
    QCOMPARE(usageTitle->font().pixelSize(), 14);
    QCOMPARE(official->font().pixelSize(), 11);
    QVERIFY(usageTitle->alignment().testFlag(Qt::AlignVCenter));
    QVERIFY(official->alignment().testFlag(Qt::AlignVCenter));
    for (const int width : {660, 360}) {
      usage.resize(width, usage.height());
      QTRY_VERIFY(usageTitle->mapTo(&usage, usageTitle->rect().bottomLeft()).y() < official->mapTo(&usage, QPoint()).y());
      QTRY_COMPARE(usageTitle->mapTo(&usage, QPoint()).x(), official->mapTo(&usage, QPoint()).x());
      QVERIFY(icon->mapTo(&usage, icon->rect().topRight()).x() < usageTitle->mapTo(&usage, QPoint()).x());
      const auto clipping = test::waitForUnclippedText(usage);
      QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    }
    for (const auto* owner : {static_cast<QWidget*>(&dialog), static_cast<QWidget*>(&usage)})
      for (auto labels = owner->findChildren<ui::TextLabel*>(); const auto* text : std::as_const(labels))
        QVERIFY(text->text() != ui::I18n::tr("Restore the saved work state"));
    QCOMPARE(f.actions->hormWrites.load(), 0);
  }
  void hormFailuresUsePartitionRowsWithoutRepeatedWarnings() {
    Fixture f;
    f.actions->state.currentFlags = app::OverlayFlags{0};
    f.actions->state.allVolumesProtected = false;
    f.actions->state.blockedVolumes = {{QStringLiteral("aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa"), QStringLiteral("D:\\Mounted{cache}")},
                                       {QStringLiteral("bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb"), QStringLiteral("F:")},
                                       {QStringLiteral("11111111-1111-1111-1111-111111111111"), {}},
                                       {QStringLiteral("22222222-2222-2222-2222-222222222222"), {}}};
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* table = dialog.findChild<QWidget*>(QStringLiteral("hormProtectionDetails"));
    QVERIFY(table && table->isVisibleTo(&dialog));
    QVERIFY(!dialog.findChild<QWidget*>(QStringLiteral("hormDiagnostics")));
    QStringList entries;
    for (auto labels = table->findChildren<ui::TextLabel*>(); const auto* item : std::as_const(labels)) entries.append(item->text());
    QVERIFY(entries.contains(QStringLiteral("D:\\Mounted{cache}")));
    QVERIFY(entries.contains(QStringLiteral("F:")));
    QCOMPARE(entries.count(ui::I18n::tr("Partition")), 3);  // Header and two unnamed partitions.
    QVERIFY(!entries.contains(ui::I18n::tr("Partition %1").arg(1)));
    QVERIFY(!entries.contains(ui::I18n::tr("Partition %1").arg(2)));
    const auto identifiers = table->findChildren<ui::TextLabel*>(QStringLiteral("hormPartitionGuid"));
    QCOMPARE(identifiers.size(), 4);
    QCOMPARE(identifiers.front()->text(), QStringLiteral("aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa"));
    QCOMPARE(identifiers[2]->text(), QStringLiteral("11111111-1111-1111-1111-111111111111"));
    QCOMPARE(identifiers.back()->text(), QStringLiteral("22222222-2222-2222-2222-222222222222"));
    MaintenanceFrame capture(dialog);
    const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR");
    if (!directory.isEmpty()) QVERIFY(QDir().mkpath(directory));
    QJsonArray alignment;
    bool auditFlushed = false;
    const auto flushAudit = [&] {
      auditFlushed = true;
      return writeAlignmentAudit(directory, QStringLiteral("horm-partitions.json"), alignment);
    };
    const auto auditOnExit = qScopeGuard([&] {
      if (!auditFlushed && !flushAudit()) QTest::qFail("Cannot save partition alignment audit", __FILE__, __LINE__);
    });
    int savedRevision = 0;
    for (auto titles = table->findChildren<ui::TextLabel*>(QStringLiteral("hormPartitionName")); auto* title : std::as_const(titles)) {
      auto* icon = title->parentWidget()->findChild<QWidget*>(QStringLiteral("hormPartitionIcon"));
      QVERIFY(icon && icon->parentWidget()->rect().contains(icon->geometry()));
      QVERIFY(capture.show(*title));
      const auto& image = capture.image();
      const auto scale = image.devicePixelRatio();
      const QRectF graphic(icon->mapTo(&dialog, QPoint()), icon->size());
      const QRectF copy(title->mapTo(&dialog, QPoint()), title->size());
      const auto background = image.pixelColor(qFloor(graphic.left() * scale), qFloor(graphic.top() * scale));
      const auto& colors = ui::DesignSystem::instance();
      const auto glyph = test::visibleInkBounds(image, colors.color(ui::design::Color::Muted), background, graphic);
      const auto text = test::visibleInkBounds(image, colors.color(ui::design::Color::Text), background, copy);
      const auto graphicCenter = static_cast<qreal>(qRound64(glyph.center().y() * scale * 2)) / 2;
      const auto textCenter = static_cast<qreal>(qRound64(text.center().y() * scale * 2)) / 2;
      const auto difference = qAbs(graphicCenter - textCenter);
      if (!directory.isEmpty())
        alignment.append(QJsonObject{{"resource", ":/icons/disk.svg"},
                                     {"widget", "hormPartitionIcon"},
                                     {"reference", "hormPartitionName"},
                                     {"rule", "visible-center"},
                                     {"dpr", scale},
                                     {"graphicCenterPhysical", graphicCenter},
                                     {"textCenterPhysical", textCenter},
                                     {"centerDifferencePhysical", difference}});
      if (!directory.isEmpty() && savedRevision != capture.revision()) {
        QVERIFY(image.save(QDir(directory).filePath(QStringLiteral("horm-partitions-%1.png").arg(capture.revision() - 1))));
        savedRevision = capture.revision();
      }
      QVERIFY(!glyph.isEmpty() && !text.isEmpty());
      QVERIFY2(difference <= 1,
               qUtf8Printable(
                   QStringLiteral("%1: disk %2, name %3, delta %4 physical pixels").arg(title->text()).arg(graphicCenter).arg(textCenter).arg(difference)));
    }
    QVERIFY(flushAudit());
    QVERIFY(!button(dialog, "hormConfirm")->isEnabled());
    auto* restriction = dialog.findChild<ui::Notice*>(QStringLiteral("maintenanceFeedback"));
    QVERIFY(restriction && restriction->isVisibleTo(&dialog));
    QCOMPARE(restriction->text(), f.model->hormDisabledReason(true));
    for (auto labels = dialog.findChildren<ui::TextLabel*>(); const auto* item : std::as_const(labels))
      if (item->isVisibleTo(&dialog)) QVERIFY(!item->text().contains(QStringLiteral("Volume{")));
    int probedRevision = 0;
    for (int row = 0; row < 4; ++row) {
      auto* caption = dialog.findChild<ui::TextLabel*>(QStringLiteral("hormRequirementLabel%1").arg(row));
      QVERIFY(caption);
      QVERIFY(capture.show(*caption));
      if (probedRevision != capture.revision()) {
        const auto errors =
            test::probeInlineIconText(dialog, capture.image(), QStringLiteral("horm-failures-row-%1").arg(row), QStringLiteral("partition-failures"));
        QVERIFY2(errors.isEmpty(), qUtf8Printable(errors));
        probedRevision = capture.revision();
      }
    }
    // An inactive filter has no active control-device prerequisites. Keep the
    // known corrective action visible instead of replacing it with Refresh.
    f.actions->state.filterEnabled = false;
    f.actions->state.romDisposition.reset();
    f.actions->state.hibernationAvailable.reset();
    f.actions->state.allVolumesProtected.reset();
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    QTRY_VERIFY(restriction->isVisibleTo(&dialog));
    QCOMPARE(restriction->text(), ui::I18n::tr("Enable UWF and restart before enabling HORM."));
    QVERIFY(!button(dialog, "hormConfirm")->isEnabled());
    QCOMPARE(f.actions->hormWrites.load(), 0);
  }
  void protectingAllFromHormStagesWithoutPretendingCurrentIsReady() {
    Fixture f;
    f.actions->state.currentFlags = app::OverlayFlags{0};
    f.actions->state.systemVolumeProtected = true;
    f.actions->state.allVolumesProtected = false;
    f.actions->state.allVolumesNextProtected = false;
    f.actions->state.allVolumesNextUnprotected = false;
    f.actions->state.blockedVolumes = {{QStringLiteral("11111111-1111-1111-1111-111111111111"), QStringLiteral("D:")}};
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* protect = button(dialog, "hormProtectAll");
    QVERIFY(protect && protect->isVisibleTo(&dialog) && protect->isEnabled());
    auto* table = dialog.findChild<QWidget*>(QStringLiteral("hormProtectionDetails"));
    QVERIFY(table);
    QCOMPARE(protect->height(), 34);
    QCOMPARE(protect->font().pixelSize(), 12);
    QCOMPARE(protect->mapTo(&dialog, protect->rect().topRight()).x(), table->mapTo(&dialog, table->rect().topRight()).x());
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    protect->click();
    QCOMPARE(accepted.size(), 0);
    QVERIFY(dialog.isVisible());
    QVERIFY(!protect->isEnabled());
    QCOMPARE(protect->text(), ui::I18n::tr("Protect all"));
    auto* confirm = button(dialog, "hormConfirm");
    QVERIFY(confirm->isEnabled());
    QCOMPARE(confirm->text(), ui::I18n::tr("OK"));
    QVERIFY(!f.model->draft().allVolumesProtected.has_value());
    QCOMPARE(f.model->snapshot().allVolumesProtected, std::optional<bool>{false});
    QVERIFY(!f.model->canSetHorm(true));
    QCOMPARE(f.actions->hormWrites.load(), 0);
    confirm->click();
    QCOMPARE(accepted.size(), 1);
    QCOMPARE(f.model->draft().allVolumesProtected, std::optional<bool>{true});
    QVERIFY(!f.model->draft().hormEnabled.has_value());
    ui::HormDialog reopened(f.model);
    QVERIFY(expose(reopened));
    auto* pending = button(reopened, "hormProtectAll");
    QVERIFY(pending && pending->isVisibleTo(&reopened) && !pending->isEnabled());
    QCOMPARE(pending->text(), ui::I18n::tr("Protect all"));
    QVERIFY(button(reopened, "hormConfirm")->isEnabled());
  }
  void usageGuideHasThreeStepsAndNeverWrites() {
    Fixture f;
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* usage = button(dialog, "hormUsage");
    QVERIFY(usage && usage->isVisibleTo(&dialog));
    QCOMPARE(usage->font().pixelSize(), 12);
    QVERIFY(!usage->icon().isNull());
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        dialog, [&] { usage->click(); },
        [&](QDialog& modal) {
          auto* guide = qobject_cast<ui::HormUsageDialog*>(&modal);
          QVERIFY(guide);
          for (const auto* name : {"hormUsagePrepare", "hormUsageVerify", "hormUsageEnable"})
            QVERIFY(guide->findChild<ui::TextLabel*>(QString::fromLatin1(name)));
          QCOMPARE(guide->findChild<ui::TextLabel*>(QStringLiteral("hormUsageHibernateCommand"))->text(), QStringLiteral("shutdown /h"));
          QVERIFY(guide->height() <= 720);
          auto* close = button(*guide, "hormUsageClose");
          QVERIFY(close && close->isDefault());
          inspected = true;
          close->click();
        }));
    QVERIFY(inspected);
    QVERIFY(!f.model->hasPendingChanges());
    QCOMPARE(f.actions->commits.load() + f.actions->swaps.load() + f.actions->hormWrites.load(), 0);
  }
  void stackedHeroInkIsCenteredBesideTheIcon_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<bool>("dark");
    QTest::addColumn<int>("width");
    for (const auto language : {ui::I18n::Lang::En, ui::I18n::Lang::Zh_CN})
      for (const bool dark : {false, true})
        for (const int width : {660, 360})
          QTest::newRow(qPrintable(QStringLiteral("%1-%2-%3").arg(static_cast<int>(language)).arg(dark ? "dark" : "light").arg(width)))
              << static_cast<int>(language) << dark << width;
  }
  void stackedHeroInkIsCenteredBesideTheIcon() {
    QFETCH(int, language);
    QFETCH(bool, dark);
    QFETCH(int, width);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    Fixture f;
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    dialog.resize(width, dialog.height());
    auto* heading = dialog.findChild<ui::TextLabel*>(QStringLiteral("maintenanceHeroHeading"));
    auto* technical = dialog.findChild<ui::TextLabel*>(QStringLiteral("maintenanceHeroTechnical"));
    auto* icon = dialog.findChild<QWidget*>(QStringLiteral("maintenanceHeaderIcon"));
    QVERIFY(heading && technical && icon);
    QVERIFY(test::waitForLayoutSettled(dialog));
    const auto clipping = test::clippedText(dialog);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    const auto textBounds = paintedTextBounds(*heading, dialog).united(paintedTextBounds(*technical, dialog));
    QVERIFY(!textBounds.isEmpty());
    const QRectF iconBounds(icon->mapTo(&dialog, QPoint()), icon->size());
    QVERIFY2(test::centersAligned(textBounds, iconBounds),
             qPrintable(QStringLiteral("Title/technical ink center %1 must align with icon center %2 relative to icon height %3")
                            .arg(textBounds.center().y())
                            .arg(iconBounds.center().y())
                            .arg(iconBounds.height())));
    QCOMPARE(f.actions->hormWrites.load(), 0);
  }
  void maintenanceAndInformationDialogsShareTheStandardHero_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<bool>("dark");
    QTest::addColumn<int>("width");
    for (const auto& language : app::languages())
      for (const bool dark : {false, true})
        for (const int width : {360, 660})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(QString(language.code), dark ? "dark" : "light").arg(width)))
              << static_cast<int>(language.id) << dark << width;
  }
  void maintenanceAndInformationDialogsShareTheStandardHero() {
    QFETCH(int, language);
    QFETCH(bool, dark);
    QFETCH(int, width);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    Fixture f;
    presentation::ConfigurationViewModel configuration{16384};
    configuration.resetSnapshot(test::application::editableSnapshot(), test::application::diskPersistentPolicy());
    const auto model = configuration.overlayPolicy();
    ui::FullCommitDialog full(f.model);
    ui::SwapDialog swap(f.model);
    ui::HormDialog horm(f.model);
    ui::HormUsageDialog usage;
    ui::OverlayAdvancedDetailsDialog details(model, model->beginEdit(), ui::OverlayAdvancedDetailsDialog::Purpose::Feature,
                                             presentation::OverlayAdvancedViewModel::Field::Passthrough);
    ui::OverlayAdvancedDetailsDialog restriction(model, model->beginEdit(), ui::OverlayAdvancedDetailsDialog::Purpose::Restriction,
                                                 presentation::OverlayAdvancedViewModel::Field::Passthrough);
    auto edit = model->beginEdit();
    presentation::OverlayAdvancedViewModel::setValue(edit, presentation::OverlayAdvancedViewModel::Field::ReadOnlyMedia, true);
    ui::OverlayAdvancedDetailsDialog conflicts(model, edit, ui::OverlayAdvancedDetailsDialog::Purpose::Conflicts);
    const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR");
    if (!directory.isEmpty()) QVERIFY(QDir().mkpath(directory));
    QJsonArray measurements;
    bool auditFlushed = false;
    const auto flushAudit = [&] {
      auditFlushed = true;
      return writeAlignmentAudit(directory, QStringLiteral("configuration-heroes-%1.json").arg(QString::fromLatin1(QTest::currentDataTag())), measurements);
    };
    const auto auditOnExit = qScopeGuard([&] {
      if (!auditFlushed && !flushAudit()) QTest::qFail("Cannot save configuration hero alignment audit", __FILE__, __LINE__);
    });
    int ordinal = 0;
    const std::array resources{":/icons/disk.svg", ":/icons/disk.svg", ":/icons/restart.svg",       ":/icons/info.svg",
                               ":/icons/info.svg", ":/icons/lock.svg", ":/icons/status_warning.svg"};
    for (auto* dialog : std::array<QDialog*, 7>{&full, &swap, &horm, &usage, &details, &restriction, &conflicts}) {
      QVERIFY(expose(*dialog));
      dialog->resize(width, dialog->height());
      QVERIFY(test::waitForLayoutSettled(*dialog));
      QList<QWidget*> heroes;
      for (auto children = dialog->findChildren<QWidget*>(); auto* child : std::as_const(children))
        if (child->property("dialogHero").toBool()) heroes.append(child);
      QCOMPARE(heroes.size(), 1);
      auto* row = qobject_cast<QHBoxLayout*>(heroes.front()->layout());
      QVERIFY(row);
      QCOMPARE(row->count(), 2);
      QCOMPARE(row->spacing(), 12);
      auto* iconHost = row->itemAt(0)->widget();
      auto* copy = row->itemAt(1)->widget();
      ui::IconView* glyph = nullptr;
      for (auto children = iconHost->findChildren<QWidget*>(); auto* child : std::as_const(children))
        if (auto* candidate = dynamic_cast<ui::IconView*>(child)) glyph = candidate;
      QVERIFY(glyph);
      auto* icon = glyph->parentWidget();
      QVERIFY(icon && copy && icon->layout());
      QCOMPARE(icon->size(), QSize(40, 40));
      QCOMPARE(icon->layout()->itemAt(0)->widget()->sizeHint(), QSize(21, 21));
      auto* lines = qobject_cast<QVBoxLayout*>(copy->layout());
      QVERIFY(lines);
      QCOMPARE(lines->count(), 2);
      auto* heading = qobject_cast<ui::TextLabel*>(lines->itemAt(0)->widget());
      auto* technical = qobject_cast<ui::TextLabel*>(lines->itemAt(1)->widget());
      QVERIFY(heading && technical);
      QCOMPARE(heading->font().pixelSize(), 14);
      QCOMPARE(technical->font().pixelSize(), 11);
      if (technical->isVisibleTo(dialog)) {
        QTRY_VERIFY(heading->mapTo(dialog, heading->rect().bottomLeft()).y() < technical->mapTo(dialog, QPoint()).y());
        QCOMPARE(heading->mapTo(dialog, QPoint()).x(), technical->mapTo(dialog, QPoint()).x());
      }
      const auto frame = dialog->grab().toImage();
      const qreal dpr = frame.devicePixelRatio();
      const auto& colors = ui::DesignSystem::instance();
      const auto ink = [&](QWidget& widget, const ui::design::Color tone) {
        const QRectF region(widget.mapTo(dialog, QPoint()), widget.size());
        const auto background = frame.pixelColor(qFloor(region.left() * dpr), qFloor(region.top() * dpr));
        return test::visibleInkBounds(frame, colors.color(tone), background, region);
      };
      const auto glyphInk = ink(*glyph, ordinal >= 5 ? ui::design::Color::Warning : ui::design::Color::Text);
      auto textInk = ink(*heading, ui::design::Color::Text);
      if (technical->isVisibleTo(dialog)) textInk = textInk.united(ink(*technical, ui::design::Color::Muted));
      const qreal difference = qAbs(qRound((glyphInk.center().y() - textInk.center().y()) * dpr * 2)) / 2.0;
      const bool fitsHost = icon->parentWidget()->rect().contains(icon->geometry()) && icon->rect().contains(glyph->geometry());
      const QString capture = QStringLiteral("configuration-hero-%1-%2").arg(QString::fromLatin1(QTest::currentDataTag())).arg(ordinal);
      if (!directory.isEmpty())
        measurements.append(QJsonObject{{"resource", QString::fromLatin1(resources[static_cast<std::size_t>(ordinal)])},
                                        {"dialog", dialog->objectName()},
                                        {"reference", heading->text() + u'\n' + technical->text()},
                                        {"rule", "visible-block"},
                                        {"dpr", dpr},
                                        {"graphicCenterPhysical", glyphInk.center().y() * dpr},
                                        {"textCenterPhysical", textInk.center().y() * dpr},
                                        {"centerDifferencePhysical", difference},
                                        {"graphicFitsHost", fitsHost},
                                        {"nonempty", !glyphInk.isEmpty() && !textInk.isEmpty()},
                                        {"capture", capture}});
      if (!directory.isEmpty()) {
        QVERIFY(frame.save(QDir(directory).filePath(capture + QStringLiteral(".png"))));
      }
      QVERIFY(fitsHost && !glyphInk.isEmpty() && !textInk.isEmpty());
      QVERIFY2(
          difference <= 1,
          qUtf8Printable(QStringLiteral("%1: hero glyph and actual title/technical ink differ by %2 physical pixels").arg(heading->text()).arg(difference)));
      const auto inlineErrors =
          test::probeInlineIconText(*dialog, frame, QStringLiteral("configuration-hero-%1").arg(ordinal), QString::fromLatin1(QTest::currentDataTag()));
      QVERIFY2(inlineErrors.isEmpty(), qUtf8Printable(inlineErrors));
      dialog->reject();
      ++ordinal;
    }
    QVERIFY(flushAudit());
    QCOMPARE(full.findChild<ui::TextLabel*>(QStringLiteral("maintenanceHeroHeading"))->text(), ui::I18n::tr("Save changes on all protected volumes"));
    QCOMPARE(full.findChild<ui::TextLabel*>(QStringLiteral("maintenanceHeroTechnical"))->text(), QStringLiteral("Full Commit"));
    QCOMPARE(f.actions->commits.load() + f.actions->swaps.load() + f.actions->hormWrites.load(), 0);
  }
  void informationIconsUseTheSameNeutralSurfaceInBothThemes() {
    const auto hasNeutralSurface = [](QWidget& surface) {
      const auto* layout = surface.layout();
      const auto* glyph = layout && layout->count() == 1 ? layout->itemAt(0)->widget() : nullptr;
      if (!glyph) return false;
      auto* window = surface.window();
      if (!test::waitForLayoutSettled(*window)) return false;
      // The glyph keeps its natural size inside a canvas that fills the icon surface.
      const qreal fillWidth = (surface.width() - glyph->sizeHint().width()) / 2.0;
      const QRectF fillBand(surface.mapTo(window, QPoint()), QSizeF(fillWidth, surface.height()));
      return test::colorCoverage(window->grab().toImage(), ui::DesignSystem::instance().color(ui::design::Color::SurfaceSoft),
                                 test::interiorRegion(fillBand)) >= test::kVisualMinimumCoverage;
    };
    for (const auto theme : {app::Theme::Light, app::Theme::Dark}) {
      ui::ThemeManager::instance().apply(theme);
      presentation::ConfigurationViewModel configuration{16384};
      configuration.resetSnapshot(test::application::editableSnapshot(), test::application::diskPersistentPolicy());
      const auto model = configuration.overlayPolicy();
      ui::OverlayAdvancedDetailsDialog details(model, model->beginEdit(), ui::OverlayAdvancedDetailsDialog::Purpose::Feature,
                                               presentation::OverlayAdvancedViewModel::Field::Passthrough);
      QVERIFY(expose(details));
      auto* surface = details.findChild<QWidget*>(QStringLiteral("overlayDetailIcon"));
      QVERIFY(surface);
      QVERIFY(hasNeutralSurface(*surface));
      details.reject();
      ui::HormUsageDialog guide;
      QVERIFY(expose(guide));
      surface = guide.findChild<QWidget*>(QStringLiteral("hormUsageIcon"));
      QVERIFY(surface);
      QVERIFY(hasNeutralSurface(*surface));
    }
  }
  void hormProtectionShortcutAppearsOnlyForAnUnmetDiskCheckAndCancelLeavesItsParentUnchanged() {
    Fixture f;
    f.actions->state.currentFlags = app::OverlayFlags{0};
    f.actions->state.systemVolumeProtected = true;
    ui::HormDialog ready(f.model);
    QVERIFY(expose(ready));
    QVERIFY(!button(ready, "hormProtectAll")->isVisibleTo(&ready));
    ready.reject();
    f.actions->state.allVolumesProtected.reset();
    ui::HormDialog unknown(f.model);
    QVERIFY(expose(unknown));
    QVERIFY(!button(unknown, "hormProtectAll")->isVisibleTo(&unknown));
    unknown.reject();
    f.actions->state.allVolumesProtected = false;
    QVERIFY(f.model->stageAllVolumesProtection(false));
    ui::HormDialog unmet(f.model);
    QVERIFY(expose(unmet));
    auto* protect = button(unmet, "hormProtectAll");
    QVERIFY(protect->isVisibleTo(&unmet) && protect->isEnabled());
    protect->click();
    QCOMPARE(f.model->draft().allVolumesProtected, std::optional<bool>{false});
    unmet.reject();
    QCOMPARE(f.model->draft().allVolumesProtected, std::optional<bool>{false});
    QVERIFY(!f.model->draft().hormEnabled.has_value());
  }
  void bulkProtectionDirectionsShareTheParentsCancelAndSaveFlow_data() {
    QTest::addColumn<bool>("save");
    QTest::newRow("cancel") << false;
    QTest::newRow("save") << true;
  }
  void bulkProtectionDirectionsShareTheParentsCancelAndSaveFlow() {
    QFETCH(bool, save);
    Fixture f;
    f.actions->state.currentFlags = app::OverlayFlags{0};
    f.actions->state.systemVolumeProtected = true;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    auto snapshot = test::application::editableSnapshot();
    auto policy = test::application::diskPersistentPolicy();
    presentation::ConfigurationViewModel configuration{16384, {}, {}, {}, {}, nullptr, f.model->editor()};
    configuration.resetSnapshot(snapshot, policy);
    ui::OverlayAdvancedDialog dialog(configuration.overlayPolicy(), f.model);
    QVERIFY(expose(dialog));
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);
    auto* protect = dialog.findChild<QAction*>(QStringLiteral("protectAllVolumes"));
    auto* unprotect = dialog.findChild<QAction*>(QStringLiteral("unprotectAllVolumes"));
    QVERIFY(protect && unprotect && protect->isEnabled() && unprotect->isEnabled());
    protect->trigger();
    QVERIFY(!f.model->hasPendingChanges());
    QVERIFY(!protect->isEnabled() && unprotect->isEnabled());
    unprotect->trigger();
    QVERIFY(!f.model->hasPendingChanges());
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayAdvancedPendingCount"))->text(), ui::I18n::tr("%1 changes").arg(1));
    QVERIFY(protect->isEnabled() && !unprotect->isEnabled());
    if (save) {
      button(dialog, "overlayAdvancedConfirm")->click();
      QCOMPARE(dialog.result(), static_cast<int>(QDialog::Accepted));
      QCOMPARE(f.model->draft().allVolumesProtected, std::optional<bool>{false});
    } else {
      dialog.reject();
      QVERIFY(!f.model->hasPendingChanges());
    }
    QCOMPARE(f.actions->commits.load() + f.actions->swaps.load() + f.actions->hormWrites.load(), 0);
  }
  void hormDisableDoesNotDependOnReadingEnablePrerequisites() {
    Fixture f;
    f.actions->state.hormEnabled = true;
    f.actions->state.filterEnabled.reset();
    f.actions->state.hibernationAvailable.reset();
    f.actions->state.allVolumesProtected.reset();
    f.actions->state.romDisposition.reset();
    f.actions->state.readError = QStringLiteral("Current configuration is unavailable");
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    QVERIFY(button(dialog, "hormConfirm")->isEnabled());
    button(dialog, "hormConfirm")->click();
    QCOMPARE(f.model->draft().hormEnabled, std::optional<bool>{false});
    QCOMPARE(f.actions->hormWrites.load(), 0);
  }
  void longHormPartitionListsScrollWithActionsVisible() {
    Fixture f;
    f.actions->state.allVolumesProtected = false;
    for (int index = 0; index < 24; ++index)
      f.actions->state.blockedVolumes.push_back({QStringLiteral("11111111-1111-1111-1111-%1").arg(index, 12, 10, QLatin1Char('0')),
                                                 index < 2 ? QStringLiteral("%1:").arg(QChar('D' + index)) : QString{}});
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::HormDialog dialog(f.model);
    QVERIFY(expose(dialog));
    auto* scroll = dialog.findChild<QScrollArea*>();
    QVERIFY(scroll);
    QTRY_VERIFY(scroll->verticalScrollBar()->maximum() > 0);
    QVERIFY(dialog.height() <= 720);
    QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    for (const auto* action : {button(dialog, "hormCancel"), button(dialog, "hormConfirm")}) {
      QVERIFY(action && action->isVisibleTo(&dialog));
      QVERIFY(dialog.rect().contains(QRect(action->mapTo(&dialog, QPoint()), action->size())));
    }
  }
  void maintenanceHeroAndRuleFollowDesignAtWrappedWidths_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<bool>("dark");
    for (const auto& language : app::languages())
      for (const bool dark : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString(language.code), dark ? "dark" : "light"))) << static_cast<int>(language.id) << dark;
  }
  void maintenanceHeroAndRuleFollowDesignAtWrappedWidths() {
    QFETCH(int, language);
    QFETCH(bool, dark);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    Fixture f;
    f.actions->holdObservation = true;
    ui::HormDialog dialog(f.model);
    const auto release = qScopeGuard([&] { f.actions->holdObservation = false; });
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    MaintenanceFrame capture(dialog);
    const auto directory = qEnvironmentVariable("UWF_ALIGNMENT_AUDIT_DIR");
    if (!directory.isEmpty()) QVERIFY(QDir().mkpath(directory));
    QJsonArray measurements;
    bool auditFlushed = false;
    const auto flushAudit = [&] {
      auditFlushed = true;
      return writeAlignmentAudit(directory, QStringLiteral("horm-firstline-%1.json").arg(QString::fromLatin1(QTest::currentDataTag())), measurements);
    };
    const auto auditOnExit = qScopeGuard([&] {
      if (!auditFlushed && !flushAudit()) QTest::qFail("Cannot save HORM first-line alignment audit", __FILE__, __LINE__);
    });
    int savedRevision = 0;
    QRectF captionInk;
    const auto captureCaption = [&](ui::TextLabel& caption) {
      if (!capture.show(caption, 0, 16)) return false;
      captionInk = test::renderedInlineInk(capture.image(), caption.firstLineBoxBounds().translated(caption.mapTo(&dialog, QPoint())));
      if (!directory.isEmpty() && savedRevision != capture.revision()) {
        const auto filename = QStringLiteral("horm-firstline-%1-%2.png").arg(QString::fromLatin1(QTest::currentDataTag())).arg(capture.revision() - 1);
        if (!capture.image().save(QDir(directory).filePath(filename))) return false;
        savedRevision = capture.revision();
      }
      return true;
    };
    const auto capturePair = [&](QWidget& graphic, ui::TextLabel& caption, const QString& resource, const QString& variant) {
      const auto& frame = capture.image();
      const qreal dpr = frame.devicePixelRatio();
      const auto icon = test::renderedInlineInk(frame, QRectF(graphic.mapTo(&dialog, QPoint()), graphic.size()));
      const auto& text = captionInk;
      const qreal delta = qAbs(qRound((icon.center().y() - text.center().y()) * dpr * 2)) / 2.0;
      const bool contained = graphic.parentWidget()->rect().contains(graphic.geometry());
      const auto filename = QStringLiteral("horm-firstline-%1-%2").arg(QString::fromLatin1(QTest::currentDataTag())).arg(capture.revision() - 1);
      if (!directory.isEmpty())
        measurements.append(QJsonObject{{"resource", resource},
                                        {"widget", graphic.objectName()},
                                        {"reference", caption.objectName()},
                                        {"variant", variant},
                                        {"rule", "first-visible-line"},
                                        {"dpr", dpr},
                                        {"graphicCenterPhysical", icon.center().y() * dpr},
                                        {"textCenterPhysical", text.center().y() * dpr},
                                        {"centerDifferencePhysical", delta},
                                        {"graphicFitsHost", contained},
                                        {"nonempty", !icon.isEmpty() && !text.isEmpty()},
                                        {"capture", filename}});
      if (!contained || icon.isEmpty() || text.isEmpty() || delta > 1)
        qWarning().noquote()
            << QStringLiteral("%1: %2 / %3 differ by %4 physical pixels; fits=%5").arg(variant, graphic.objectName(), caption.text()).arg(delta).arg(contained);
      return contained && !icon.isEmpty() && !text.isEmpty() && delta <= 1;
    };
    auto* activity = dialog.findChild<QWidget*>(QStringLiteral("maintenanceActivity"));
    QVERIFY(activity && activity->isVisibleTo(&dialog));
    auto* spinner = activity->findChild<ui::BusyIndicator*>();
    auto* activityText = activity->findChild<ui::TextLabel*>();
    QVERIFY(spinner && activityText);
    spinner->setObjectName(QStringLiteral("maintenanceReadingSpinner"));
    QVERIFY(captureCaption(*activityText));
    QVERIFY(capturePair(*spinner, *activityText, QStringLiteral("vector:BusyIndicator-full-ring"), QStringLiteral("reading")));
    struct Requirement {
      QWidget* icon;
      ui::TextLabel* caption;
      ui::TextLabel* state;
    };
    std::array<Requirement, 4> requirements;
    for (int row = 0; row < 4; ++row) {
      const auto suffix = QString::number(row);
      auto* icon = dialog.findChild<QWidget*>(QStringLiteral("hormRequirementIcon") + suffix);
      auto* caption = dialog.findChild<ui::TextLabel*>(QStringLiteral("hormRequirementLabel") + suffix);
      auto* state = dialog.findChild<ui::TextLabel*>(QStringLiteral("hormRequirementState") + suffix);
      QVERIFY(icon && caption && state);
      requirements[static_cast<std::size_t>(row)] = {icon, caption, state};
      QVERIFY(captureCaption(*caption));
      QVERIFY(capturePair(*icon, *caption, QStringLiteral(":/icons/info.svg"), QStringLiteral("reading-icon-%1").arg(row)));
      QVERIFY(capturePair(*state, *caption, {}, QStringLiteral("reading-state-%1").arg(row)));
    }
    f.actions->holdObservation = false;
    QTRY_VERIFY(!activity->isVisibleTo(&dialog));
    for (const int width : {520, 360}) {
      dialog.resize(width, dialog.height());
      capture.invalidate();
      auto* mark = dialog.findChild<QWidget*>(QStringLiteral("maintenanceHeaderIcon"));
      auto* copy = dialog.findChild<QWidget*>(QStringLiteral("maintenanceHeroCopy"));
      QVERIFY(mark && copy);
      auto* heading = dialog.findChild<QWidget*>(QStringLiteral("maintenanceHeroHeading"));
      auto* technical = dialog.findChild<QWidget*>(QStringLiteral("maintenanceHeroTechnical"));
      QVERIFY(heading && technical);
      const auto rules = dialog.findChildren<QWidget*>(QStringLiteral("maintenanceEffectRule"));
      QCOMPARE(rules.size(), 1);
      QVERIFY(!rules.front()->isVisibleTo(&dialog));
      const auto captureDirectory = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
      if (!captureDirectory.isEmpty()) {
        QVERIFY(QDir().mkpath(captureDirectory));
        QVERIFY(capture.show(*heading, 0, 16));
        QVERIFY(capture.image().save(QDir(captureDirectory).filePath(QStringLiteral("maintenance-firstline-%1-%2.png").arg(width).arg(language))));
      }
      for (int row = 0; row < 4; ++row) {
        const auto [icon, caption, state] = requirements[static_cast<std::size_t>(row)];
        QVERIFY(captureCaption(*caption));
        QVERIFY(capturePair(*icon, *caption, QStringLiteral(":/icons/check.svg"), QStringLiteral("ready-icon-%1-width-%2").arg(row).arg(width)));
        QVERIFY(capturePair(*state, *caption, {}, QStringLiteral("ready-state-%1-width-%2").arg(row).arg(width)));
      }
    }
    QVERIFY(flushAudit());
  }
  void savedStateStartupStagesAndCanUndoItsDraft() {
    Fixture f;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::HormDialog enabling(f.model);
    QVERIFY(expose(enabling));
    QVERIFY(button(enabling, "hormCancel")->isDefault());
    QSignalSpy enabled(&enabling, &QDialog::accepted);
    button(enabling, "hormConfirm")->click();
    QCOMPARE(enabled.size(), 1);
    QCOMPARE(f.model->draft().hormEnabled, std::optional<bool>{true});
    QCOMPARE(f.model->snapshot().hormEnabled, std::optional<bool>{false});
    QVERIFY(!f.model->restartRequired());
    QCOMPARE(f.actions->hormWrites.load(), 0);
    ui::HormDialog undo(f.model);
    QVERIFY(expose(undo));
    button(undo, "hormConfirm")->click();
    QVERIFY(!f.model->draft().hormEnabled.has_value());
    QCOMPARE(f.actions->hormWrites.load(), 0);
  }
  void storageCapacityShowsAndTracksTheGlobalDraft_data() {
    QTest::addColumn<bool>("saveChild");
    QTest::newRow("cancel-child") << false;
    QTest::newRow("save-child-cancel-parent") << true;
  }
  void storageCapacityShowsAndTracksTheGlobalDraft() {
    QFETCH(bool, saveChild);
    Fixture f;
    f.actions->state.filterEnabled = false;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = false;
    snapshot.next.overlay = {core::OverlayType::Disk, 4096, 2048, 3072};
    auto policy = test::application::diskPersistentPolicy();
    policy.current.filterEnabled.value = false;
    presentation::ConfigurationViewModel configuration{16384, {}, {}, {}, {}, nullptr, f.model->editor()};
    configuration.resetSnapshot(snapshot, policy, false);
    configuration.global()->editMaximumSize(8192);
    ui::OverlayAdvancedDialog dialog(configuration.overlayPolicy(), f.model);
    QVERIFY(expose(dialog));
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        dialog, [&] { button(dialog, "overlaySwapManage")->click(); },
        [&](QDialog& modal) {
          auto* child = qobject_cast<ui::SwapDialog*>(&modal);
          QVERIFY(child);
          QTRY_VERIFY(button(*child, "swapCancel")->isEnabled());
          auto* capacity = child->findChild<ui::TextLabel*>(QStringLiteral("swapCapacitySummary"));
          QVERIFY(capacity);
          QCOMPARE(capacity->text(), ui::I18n::tr("%1 MB").arg(8192));
          configuration.global()->editMaximumSize(12288);
          QTRY_COMPARE(capacity->text(), ui::I18n::tr("%1 MB").arg(12288));
          inspected = true;
          if (saveChild) {
            QTRY_VERIFY(button(*child, "swapCreate")->isEnabled());
            button(*child, "swapCreate")->click();
          } else {
            child->reject();
          }
        }));
    QVERIFY(inspected);
    QCOMPARE(tabs->currentIndex(), 1);
    QVERIFY(f.model->draft().empty());
    QCOMPARE(dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayAdvancedPendingCount"))->text(),
             ui::I18n::tr("%1 changes").arg(saveChild ? 1 : 0));
    dialog.reject();
    QVERIFY(f.model->draft().empty());
    QCOMPARE(configuration.global()->pendingOverlay().maximumSizeMb, std::optional<std::uint32_t>{12288});
    QCOMPARE(f.actions->commits.load() + f.actions->swaps.load() + f.actions->hormWrites.load(), 0);
  }
  void advancedCancelDiscardsPrivateDraftsAndChildrenReturnToMaintenance() {
    Fixture f;
    f.actions->state.filterEnabled = false;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = false;
    auto policy = test::application::diskPersistentPolicy();
    policy.current.filterEnabled.value = false;
    presentation::ConfigurationViewModel configuration{16384, {}, {}, {}, {}, nullptr, f.model->editor()};
    configuration.resetSnapshot(snapshot, policy);
    ui::OverlayAdvancedDialog dialog(configuration.overlayPolicy(), f.model);
    QVERIFY(expose(dialog));
    auto* passthrough = dialog.findChild<ui::SwitchButton*>(QStringLiteral("overlayPassthrough"));
    QVERIFY(passthrough);
    passthrough->click();
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        dialog, [&] { button(dialog, "overlaySwapManage")->click(); },
        [&](QDialog& modal) {
          auto* child = qobject_cast<ui::SwapDialog*>(&modal);
          QVERIFY(child);
          QTRY_VERIFY(button(*child, "swapCreate")->isEnabled());
          inspected = true;
          button(*child, "swapCreate")->click();
        }));
    QVERIFY(inspected);
    QCOMPARE(tabs->currentIndex(), 1);
    QVERIFY(!f.model->hasPendingChanges());
    QVERIFY(dialog.findChild<ui::TextLabel*>(QStringLiteral("overlaySwapStatus"))->text().contains(ui::I18n::tr("Draft: %1 · apply required").arg("C:")));
    QCOMPARE(f.actions->swaps.load(), 0);
    QVERIFY(configuration.pendingConfiguration().empty());
    tabs->setCurrentIndex(0);
    QVERIFY(!passthrough->isChecked());
    dialog.reject();
    QVERIFY(!f.model->hasPendingChanges());
    QVERIFY(configuration.pendingConfiguration().empty());
  }
  void activeHormDisableDraftCanBeSavedFromTheParent() {
    Fixture f;
    f.actions->state.hormEnabled = true;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = true;
    auto policy = test::application::diskPersistentPolicy();
    policy.current.filterEnabled.value = true;
    policy.current.overlayType.value = 0;
    policy.current.flags.value = app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia};
    presentation::ConfigurationViewModel configuration{16384, {}, {}, {}, {}, nullptr, f.model->editor()};
    configuration.resetSnapshot(snapshot, policy, true);
    QVERIFY(!configuration.overlayPolicy()->canCommit(configuration.overlayPolicy()->beginEdit()));
    ui::OverlayAdvancedDialog dialog(configuration.overlayPolicy(), f.model);
    QVERIFY(expose(dialog));
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        dialog, [&] { button(dialog, "overlayHormManage")->click(); },
        [&](QDialog& modal) {
          auto* child = qobject_cast<ui::HormDialog*>(&modal);
          QVERIFY(child);
          QTRY_VERIFY(button(*child, "hormConfirm")->isEnabled());
          inspected = true;
          button(*child, "hormConfirm")->click();
        }));
    QVERIFY(inspected);
    QVERIFY(!f.model->draft().hormEnabled.has_value());
    QVERIFY(dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayHormStatus"))
                ->text()
                .contains(ui::I18n::tr("Draft: %1 · apply required").arg(ui::I18n::tr("Disabled"))));
    QVERIFY(f.model->hormActive());
    auto* save = button(dialog, "overlayAdvancedConfirm");
    QVERIFY(save->isVisible() && save->isEnabled());
    QSignalSpy accepted(&dialog, &QDialog::accepted);
    save->click();
    QCOMPARE(accepted.size(), 1);
    QCOMPARE(f.model->draft().hormEnabled, std::optional<bool>{false});
    QVERIFY(f.model->hormActive());
    QCOMPARE(f.actions->hormWrites.load(), 0);
  }
  void advancedContextIsSharedAboveBothTabsAndRefreshes_data() {
    QTest::addColumn<int>("width");
    QTest::addColumn<bool>("dark");
    QTest::newRow("standard-light") << 660 << false;
    QTest::newRow("standard-dark") << 660 << true;
    QTest::newRow("narrow-light") << 360 << false;
    QTest::newRow("narrow-dark") << 360 << true;
  }
  void advancedContextIsSharedAboveBothTabsAndRefreshes() {
    QFETCH(int, width);
    QFETCH(bool, dark);
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    Fixture f;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = false;
    snapshot.current.overlay.type = core::OverlayType::RAM;
    snapshot.next.overlay.type = core::OverlayType::Disk;
    auto policy = test::application::diskPersistentPolicy();
    policy.current.filterEnabled.value = false;
    policy.current.overlayType.value = 0;
    policy.current.flags.value = app::OverlayFlags{0};
    presentation::ConfigurationViewModel configuration{16384, {}, {}, {}, {}, nullptr, f.model->editor()};
    configuration.resetSnapshot(snapshot, policy);
    ui::OverlayAdvancedDialog dialog(configuration.overlayPolicy(), f.model);
    QVERIFY(expose(dialog));
    dialog.resize(width, dialog.height());
    QTRY_COMPARE(dialog.width(), width);
    auto* context = dialog.findChild<QWidget*>(QStringLiteral("overlayAdvancedContext"));
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    auto* filter = dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayAdvancedFilterState"));
    auto* current = dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayAdvancedCurrentType"));
    auto* next = dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayAdvancedNextType"));
    QVERIFY(context && tabs && filter && current && next);
    QVERIFY(!tabs->isAncestorOf(context));
    QCOMPARE(context->parentWidget(), tabs->parentWidget());
    for (int page : {0, 1}) {
      tabs->setCurrentIndex(page);
      QTRY_VERIFY(context->isVisibleTo(&dialog));
      QTRY_VERIFY(!context->visibleRegion().isEmpty());
      const auto contextBottom = context->mapToGlobal(QPoint(0, context->height())).y();
      QVERIFY(contextBottom <= tabs->tabBar()->mapToGlobal(QPoint()).y());
      QCOMPARE(filter->text(), QStringLiteral("UWF Disabled"));
      QCOMPARE(current->text(), QStringLiteral("RAM"));
      QCOMPARE(next->text(), QStringLiteral("Disk"));
      const auto clipping = test::waitForUnclippedText(dialog);
      QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
      for (auto scrollers = dialog.findChildren<QScrollArea*>(); auto* scroll : std::as_const(scrollers)) QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    }
    auto* alias = dialog.findChild<ui::TextLabel*>(QStringLiteral("overlaySwapAlias"));
    QVERIFY(alias && alias->isVisible());
    QCOMPARE(alias->text(), QStringLiteral("create swapfile"));
    QVERIFY(alias->wordWrap());
    snapshot.current.filter.enabled = true;
    snapshot.current.overlay.type = core::OverlayType::Disk;
    snapshot.next.overlay.type = core::OverlayType::RAM;
    policy.current.filterEnabled.value = true;
    policy.current.overlayType.value = 1;
    policy.next.overlayType.value = 0;
    policy.next.flags.value = app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia};
    configuration.resetSnapshot(snapshot, policy);
    QTRY_COMPARE(filter->text(), QStringLiteral("UWF Enabled"));
    QCOMPARE(current->text(), QStringLiteral("Disk"));
    QCOMPARE(next->text(), QStringLiteral("RAM"));
    QVERIFY(context->isVisibleTo(&dialog));
    tabs->setCurrentIndex(0);
    QVERIFY(context->isVisibleTo(&dialog));
    QCOMPARE(f.actions->commits.load() + f.actions->swaps.load() + f.actions->hormWrites.load(), 0);
  }
  void advancedRevertIsPrivateUntilSavedAndCancelKeepsTheParentDraft_data() {
    QTest::addColumn<bool>("save");
    QTest::newRow("revert-then-save-clears-existing-pending") << true;
    QTest::newRow("revert-then-cancel-keeps-existing-pending") << false;
  }
  void advancedRevertIsPrivateUntilSavedAndCancelKeepsTheParentDraft() {
    QFETCH(bool, save);
    Fixture f;
    f.actions->state.filterEnabled = false;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    QVERIFY(f.model->stageStorageLocation(QStringLiteral("volume-c")));
    const auto openingDraft = f.model->draft();
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = false;
    auto policy = test::application::diskPersistentPolicy();
    policy.current.filterEnabled.value = false;
    presentation::ConfigurationViewModel configuration{16384, {}, {}, {}, {}, nullptr, f.model->editor()};
    configuration.resetSnapshot(snapshot, policy);
    ui::OverlayAdvancedDialog dialog(configuration.overlayPolicy(), f.model);
    QVERIFY(expose(dialog));
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    auto* passthrough = dialog.findChild<ui::SwitchButton*>(QStringLiteral("overlayPassthrough"));
    auto* count = dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayAdvancedPendingCount"));
    auto* revert = button(dialog, "overlayAdvancedRevert");
    QVERIFY(tabs && passthrough && count && revert);
    QCOMPARE(count->text(), ui::I18n::tr("%1 changes").arg(1));
    QVERIFY(count->isVisible() && revert->isVisible());
    QVERIFY(passthrough->isChecked());
    passthrough->click();
    QVERIFY(!passthrough->isChecked());
    tabs->setCurrentIndex(1);
    bool storageEdited = false;
    QVERIFY(test::driveModalDialog(
        dialog, [&] { button(dialog, "overlaySwapManage")->click(); },
        [&](QDialog& modal) {
          auto* child = qobject_cast<ui::SwapDialog*>(&modal);
          QVERIFY(child);
          auto* volumes = child->findChild<QComboBox*>(QStringLiteral("swapVolume"));
          QVERIFY(volumes);
          QTRY_VERIFY(button(*child, "swapCreate")->isEnabled());
          volumes->setCurrentIndex(volumes->findData(QStringLiteral("volume-d")));
          button(*child, "swapCreate")->click();
          storageEdited = child->result() == QDialog::Accepted;
        }));
    QVERIFY(storageEdited);
    QCOMPARE(f.model->draft(), openingDraft);
    for (int page : {0, 1}) {
      tabs->setCurrentIndex(page);
      QCOMPARE(count->text(), ui::I18n::tr("%1 changes").arg(2));
      QVERIFY(count->isVisible() && revert->isVisible());
    }
    revert->click();
    QCOMPARE(f.model->draft(), openingDraft);
    for (int page : {1, 0}) {
      tabs->setCurrentIndex(page);
      QVERIFY(!count->isVisible());
      QVERIFY(!revert->isVisible());
    }
    QVERIFY(passthrough->isChecked());
    if (save) {
      button(dialog, "overlayAdvancedConfirm")->click();
      QCOMPARE(dialog.result(), static_cast<int>(QDialog::Accepted));
      QVERIFY(!f.model->hasPendingChanges());
    } else {
      dialog.reject();
      QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
      QVERIFY(f.model->draft() == openingDraft);
    }
    const auto pending = configuration.pendingConfiguration();
    QVERIFY(pending.settings.empty());
    QVERIFY(pending.overlayPolicy.empty());
    QCOMPARE(pending.maintenance, f.model->draft());
    QCOMPARE(f.actions->commits.load() + f.actions->swaps.load() + f.actions->hormWrites.load(), 0);
  }
  void dialogsFitBothThemesAtNarrowWidths_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<bool>("dark");
    QTest::newRow("english-light") << static_cast<int>(ui::I18n::Lang::En) << false;
    QTest::newRow("chinese-dark") << static_cast<int>(ui::I18n::Lang::Zh_CN) << true;
  }
  void dialogsFitBothThemesAtNarrowWidths() {
    QFETCH(int, language);
    QFETCH(bool, dark);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    Fixture f;
    f.model->refresh();
    QTRY_VERIFY(!f.model->busy());
    ui::FullCommitDialog full(f.model);
    QVERIFY(expose(full));
    checkLayout(full);
    full.reject();
    ui::SwapDialog swap(f.model);
    QVERIFY(expose(swap));
    checkLayout(swap);
    swap.reject();
    ui::HormDialog horm(f.model);
    QVERIFY(expose(horm));
    checkLayout(horm);
  }
};
}  // namespace
QTEST_MAIN(OverlayMaintenanceDialogTests)
#include "OverlayMaintenanceDialogTests.moc"
