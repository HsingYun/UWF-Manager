// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTabBar>
#include <QTabWidget>
#include <QTest>
#include <QTranslator>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/localization/OverlayTypeText.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"
#include "presentation/views/configuration/global/GlobalStatusPanel.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDialog.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/OverlayPolicyFixture.h"
#include "support/ui/DialogControls.h"
#include "support/ui/LayoutSettling.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/WindowActivation.h"
#include "support/visual/InkBounds.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

namespace {
using namespace uwf;
using Model = presentation::OverlayAdvancedViewModel;
class Actions final : public app::ConfigurationActions {
 public:
  int legacy = 0, mixed = 0, completed = 0;
  app::ConfigurationPlan received;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    ++legacy;
    throw std::logic_error("overlay review must preserve its complete plan");
  }
  ApplyOperation prepareApply(const app::ConfigurationPlan& plan) override {
    ++mixed;
    received = plan;
    return [operations = plan.operations()] {
      app::ConfigurationApplyResult result;
      for (const auto& operation : operations)
        result.steps.push_back({"Applied", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed, operation});
      return result;
    };
  }
  void completeApply(const app::ConfigurationApplyResult&) override { ++completed; }
};
struct Fixture {
  core::UwfSnapshot snapshot = test::application::editableSnapshot();
  app::OverlayPolicySnapshot policy = test::application::diskPersistentPolicy();
  presentation::ConfigurationViewModel configuration{16384};
  Fixture() {
    snapshot.current.filter.enabled = false;
    policy.current.filterEnabled.value = false;
    snapshot.current.overlay.type = snapshot.next.overlay.type = core::OverlayType::Disk;
    configuration.resetSnapshot(snapshot, policy);
  }
  std::shared_ptr<Model> model() { return configuration.overlayPolicy(); }
};
class MaintenanceTitleFacts final : public app::OverlayMaintenanceActions {
 public:
  bool failObservation = false;
  bool volumesReadable = false;
  std::atomic_int reads{0};
  std::optional<app::OverlayMaintenanceSnapshot> snapshot() const override {
    app::OverlayMaintenanceSnapshot snapshot;
    snapshot.filterEnabled = snapshot.hormEnabled = snapshot.overlayCommit = false;
    snapshot.currentOverlayType = snapshot.nextOverlayType = 1;
    snapshot.currentFlags = snapshot.nextFlags = app::OverlayFlags{3};
    snapshot.hibernationAvailable = snapshot.allVolumesProtected = true;
    snapshot.romDisposition = 0;
    snapshot.volumesReadable = volumesReadable;
    return snapshot;
  }
  app::OverlayMaintenanceSnapshot observe(std::stop_token) override {
    ++reads;
    if (failObservation) throw std::runtime_error("Cannot read maintenance state. 无法读取维护状态，请稍后重试。");
    volumesReadable = true;
    return *snapshot();
  }
  app::OverlayMaintenanceResult fullCommit(std::stop_token) override { throw std::logic_error("Layout tests must not write"); }
  app::OverlayMaintenanceResult createSwap(const QString&, std::stop_token) override { throw std::logic_error("Layout tests must not write"); }
  app::OverlayMaintenanceResult setHorm(bool, std::stop_token) override { throw std::logic_error("Layout tests must not write"); }
};
class DuplicateAliasTranslator final : public QTranslator {
 public:
  bool isEmpty() const override { return false; }
  QString translate(const char* context, const char* source, const char*, int) const override {
    if (!context || !source || qstrcmp(context, "uwf") != 0) return {};
    if (qstrcmp(source, "Freespace Passthrough") == 0) return QStringLiteral("  fREESPACE pASSTHROUGH  ");
    if (qstrcmp(source, "Persistent Overlay") == 0) return QStringLiteral("  persistent OVERLAY  ");
    if (qstrcmp(source, "Read-Only Media mode") == 0) return QStringLiteral("  read-only MEDIA MODE  ");
    if (qstrcmp(source, "Configure storage location") == 0) return QStringLiteral("  CREATE SWAPFILE  ");
    if (qstrcmp(source, "Hibernate once / resume many") == 0) return QStringLiteral("  hOrM  ");
    return {};
  }
};
QRectF paintedInk(const QImage& image, QWidget& widget, const QWidget& parent, const bool firstLine = false) {
  if (image.isNull()) return {};
  const qreal scale = image.devicePixelRatio();
  const auto* label = qobject_cast<ui::TextLabel*>(&widget);
  const auto local = firstLine && label ? label->firstLineBoxBounds() : QRectF(widget.rect());
  // Isolate the real widget paint at its window offset: an adjacent keyboard
  // focus frame can overlap this rectangle and share the icon's exact color.
  return test::renderedInkBounds(widget, scale, local).translated(widget.mapTo(&parent, QPoint()));
}
bool rasterCentersAligned(const QRectF& first, const QRectF& second, const qreal scale) {
  return !first.isEmpty() && !second.isEmpty() && qRound(std::abs(first.center().y() - second.center().y()) * scale * 2) <= 2;
}
void revealWidget(QWidget& widget) {
  for (auto* ancestor = widget.parentWidget(); ancestor; ancestor = ancestor->parentWidget())
    if (auto* scroll = qobject_cast<QScrollArea*>(ancestor); scroll && scroll->widget() && scroll->widget()->isAncestorOf(&widget))
      scroll->ensureWidgetVisible(&widget, 0, 0);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCoreApplication::processEvents();
}
ui::SwitchButton* control(QWidget& dialog, const char* name) { return dialog.findChild<ui::SwitchButton*>(QString::fromLatin1(name)); }
QPushButton* confirm(QWidget& dialog) { return dialog.findChild<QPushButton*>(QStringLiteral("overlayAdvancedConfirm")); }
struct Feature {
  Model::Field field;
  const char* name;
};
constexpr std::array features{Feature{Model::Field::Passthrough, "overlayPassthrough"}, Feature{Model::Field::Persistent, "overlayPersistent"},
                              Feature{Model::Field::Reset, "overlayResetPersistent"}, Feature{Model::Field::ReadOnlyMedia, "overlayReadOnlyMedia"}};
QAbstractButton* action(QWidget& dialog, const Feature& feature, const char* suffix) {
  return dialog.findChild<QAbstractButton*>(QString::fromLatin1(feature.name) + QString::fromLatin1(suffix));
}
QStringList visibleText(QWidget& window) {
  QStringList result;
  for (auto labels = window.findChildren<QLabel*>(); auto* label : std::as_const(labels))
    if (label->window() == &window && label->isVisibleTo(&window)) result.append(label->text());
  return result;
}
bool expose(QDialog& dialog) {
  dialog.show();
  return QTest::qWaitForWindowExposed(&dialog) && test::activateWindowForInput(dialog);
}
enum class DetailClose { Back, Escape, WindowClose };
bool inspectDetails(ui::OverlayAdvancedDialog& parent, QAbstractButton& opener, const DetailClose close, const std::function<void(QDialog&)>& inspect,
                    const bool keyboard = false) {
  bool inspected = false;
  opener.setFocus(Qt::TabFocusReason);
  const bool observed = test::driveModalDialog(
      parent,
      [&] {
        if (keyboard)
          QTest::keyClick(&opener, Qt::Key_Return);
        else
          opener.click();
      },
      [&](QDialog& dialog) {
        auto* details = &dialog;
        QCOMPARE(details->objectName(), QStringLiteral("overlayAdvancedDetailsDialog"));
        QCOMPARE(details->parentWidget(), static_cast<QWidget*>(&parent));
        QVERIFY(details->isModal());
        inspect(*details);
        switch (close) {
          case DetailClose::Back: {
            auto* back = details->findChild<QAbstractButton*>(QStringLiteral("overlayDetailsBack"));
            QVERIFY(back && back->isVisible() && back->isEnabled());
            back->click();
            break;
          }
          case DetailClose::Escape:
            QTest::keyClick(details, Qt::Key_Escape);
            break;
          case DetailClose::WindowClose:
            details->close();
            break;
        }
        inspected = true;
      });
  // Establish the active input surface before checking the focus widget
  // remembered by production code after the nested dialog closes.
  return observed && inspected && test::activateWindowForInput(parent);
}
void checkDetailLayout(QDialog& dialog, const int width) {
  dialog.resize(std::min(width, 500), dialog.height());
  const auto clipping = test::waitForUnclippedText(dialog);
  QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
  for (auto scrollers = dialog.findChildren<QScrollArea*>(); auto* scroll : std::as_const(scrollers)) QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
}
bool badgeHasSemanticColors(QWidget& badge, QLabel& caption, const QColor& foreground, const QColor& background) {
  const qreal dpr = caption.devicePixelRatioF();
  if (caption.text().isEmpty() || caption.size().isEmpty() || !(dpr > 0)) return false;
  QImage glyphs(QSize(qCeil(caption.width() * dpr), qCeil(caption.height() * dpr)), QImage::Format_ARGB32_Premultiplied);
  glyphs.setDevicePixelRatio(dpr);
  glyphs.fill(Qt::transparent);
  caption.render(&glyphs, QPoint(), QRegion(), QWidget::RenderFlags{});
  QRect ink;
  qreal coverage = 0, matching = 0;
  for (int y = 0; y < glyphs.height(); ++y)
    for (int x = 0; x < glyphs.width(); ++x) {
      const auto pixel = glyphs.pixelColor(x, y);
      if (pixel.alphaF() < test::kVisualColorRatio) continue;
      ink = ink.united(QRect(x, y, 1, 1));
      coverage += pixel.alphaF();
      // Unpremultiplied color preserves the semantic foreground at AA edges.
      if (std::abs(pixel.redF() - foreground.redF()) <= test::kVisualColorRatio && std::abs(pixel.greenF() - foreground.greenF()) <= test::kVisualColorRatio &&
          std::abs(pixel.blueF() - foreground.blueF()) <= test::kVisualColorRatio)
        matching += pixel.alphaF();
    }
  if (ink.isEmpty() || coverage <= 0 || matching / coverage < test::kVisualMinimumCoverage) return false;
  const auto textBounds = test::logicalBounds(ink, dpr);
  const QRectF content = caption.contentsRect();
  const auto visible = textBounds.intersected(content);
  if (visible.width() * visible.height() < textBounds.width() * textBounds.height() * test::kVisualMinimumCoverage ||
      textBounds.width() < caption.fontMetrics().horizontalAdvance(caption.text()) * test::kVisualMinimumCoverage ||
      textBounds.height() < caption.fontMetrics().xHeight() * test::kVisualMinimumCoverage || !test::centersAligned(textBounds, content))
    return false;
  const auto badgeInk = textBounds.translated(caption.mapTo(&badge, QPoint()));
  const QRectF clearBand(0, 0, badge.width(), badgeInk.top());
  return test::colorCoverage(badge.grab().toImage(), background, test::interiorRegion(clearBand)) >= test::kVisualMinimumCoverage;
}
QRect bounds(const QWidget& widget, const QWidget& root) { return {widget.mapTo(&root, QPoint()), widget.size()}; }
void checkMainLayout(ui::OverlayAdvancedDialog& dialog, const std::shared_ptr<Model>& model) {
  const auto clipping = test::waitForUnclippedText(dialog);
  QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
  auto* featureHeading = dialog.findChild<QLabel*>(QStringLiteral("overlayFeatureHeading"));
  auto* currentHeading = dialog.findChild<QLabel*>(QStringLiteral("overlayCurrentSessionHeading"));
  auto* nextHeading = dialog.findChild<QLabel*>(QStringLiteral("overlayNextSessionHeading"));
  QVERIFY(featureHeading && currentHeading && nextHeading);
  QCOMPARE(featureHeading->text(), ui::I18n::tr("Feature"));
  QCOMPARE(currentHeading->text(), ui::I18n::tr("Current session"));
  QCOMPARE(nextHeading->text(), ui::I18n::tr("Next session"));
  const auto rowSeparationError = [&]() -> QString {
    const QWidget* previous = featureHeading;
    for (const auto& feature : features) {
      const auto name = QString::fromLatin1(feature.name) + QStringLiteral("Name");
      const auto* label = dialog.findChild<QLabel*>(name);
      if (!label) return QStringLiteral("missing row label: %1").arg(name);
      const auto* row = label->parentWidget();
      const auto rowBounds = bounds(*row, dialog);
      const auto previousBounds = bounds(*previous, dialog);
      if (rowBounds.top() <= previousBounds.bottom()) {
        QString diagnostic;
        QDebug(&diagnostic).noquote().nospace() << name << " row=" << rowBounds << " preceding=" << previousBounds << " dialog=" << dialog.size();
        return diagnostic;
      }
      previous = row;
    }
    return {};
  };
  // Responsive wrapping queues a content-height fit. Wait for the same strict
  // row separation contract before sampling the complete three-column layout.
  QTRY_VERIFY2(rowSeparationError().isEmpty(), qUtf8Printable(rowSeparationError()));
  const auto currentColumn = bounds(*currentHeading, dialog).center().x();
  const auto nextColumn = bounds(*nextHeading, dialog).center().x();
  QVERIFY(dialog.isRightToLeft() ? currentColumn > nextColumn : currentColumn < nextColumn);
  const auto mainText = visibleText(dialog);
  const auto edit = model->beginEdit();
  const QWidget* previousRow = featureHeading;
  for (const auto& feature : features) {
    const auto name = QString::fromLatin1(feature.name);
    auto* label = dialog.findChild<QLabel*>(name + QStringLiteral("Name"));
    auto* current = dialog.findChild<QLabel*>(name + QStringLiteral("Current"));
    auto* badge = dialog.findChild<QWidget*>(name + QStringLiteral("CurrentBadge"));
    auto* info = action(dialog, feature, "Info");
    auto* button = control(dialog, feature.name);
    QVERIFY(label && current && badge && info && button);
    revealWidget(*label->parentWidget());
    QCOMPARE(label->text(), Model::fieldName(feature.field));
    QCOMPARE(current->text(), model->currentText(feature.field));
    QVERIFY(current->width() >= current->sizeHint().width());
    QVERIFY(info->isVisible() && info->isEnabled());
    QVERIFY(info->focusPolicy() != Qt::NoFocus);
    const auto featureBounds = bounds(*label->parentWidget(), dialog);
    const auto badgeBounds = bounds(*badge, dialog);
    const auto controlBounds = bounds(*button, dialog);
    QVERIFY(featureBounds.top() > bounds(*previousRow, dialog).bottom());
    previousRow = label->parentWidget();
    QVERIFY(!featureBounds.intersects(badgeBounds));
    QVERIFY(!badgeBounds.intersects(controlBounds));
    QVERIFY(!bounds(*label, dialog).intersects(bounds(*info, dialog)));
    QVERIFY(test::centersAligned(badgeBounds, bounds(*currentHeading, dialog), Qt::Horizontal));
    QVERIFY(test::centersAligned(controlBounds, bounds(*nextHeading, dialog), Qt::Horizontal));
    QVERIFY(test::centersAligned(badgeBounds, controlBounds));
    const auto rowFrame = dialog.grab().toImage();
    const auto titleInk = paintedInk(rowFrame, *label, dialog, true);
    const auto infoInk = paintedInk(rowFrame, *info, dialog);
    auto* changed = dialog.findChild<QWidget*>(name + QStringLiteral("Changed"));
    QVERIFY(changed);
    const auto changedInk = changed->isVisibleTo(&dialog) ? paintedInk(rowFrame, *changed, dialog) : QRectF{};
    QVERIFY2(rasterCentersAligned(titleInk, infoInk, dialog.devicePixelRatioF()),
             qUtf8Printable(QStringLiteral("%1: title center=%2 info center=%3").arg(name).arg(titleInk.center().y()).arg(infoInk.center().y())));
    auto* indicator = dialog.findChild<QWidget*>(name + QStringLiteral("CurrentIndicator"));
    QVERIFY(indicator);
    const auto indicatorInk = paintedInk(rowFrame, *indicator, dialog);
    const auto currentInk = paintedInk(rowFrame, *current, dialog);
    QVERIFY2(rasterCentersAligned(indicatorInk, currentInk, dialog.devicePixelRatioF()),
             qUtf8Printable(QStringLiteral("%1 current state indicator: %2 / %3").arg(name).arg(indicatorInk.center().y()).arg(currentInk.center().y())));
    if (changed->isVisibleTo(&dialog))
      QVERIFY2(rasterCentersAligned(changedInk, titleInk, dialog.devicePixelRatioF()), qUtf8Printable(name + QStringLiteral(" changed indicator")));
    if (dialog.isRightToLeft()) {
      QCOMPARE(featureBounds.right(), bounds(*featureHeading, dialog).right());
      QVERIFY(featureBounds.left() > badgeBounds.right());
    } else {
      QCOMPARE(featureBounds.left(), bounds(*featureHeading, dialog).left());
      QVERIFY(featureBounds.right() < badgeBounds.left());
    }
    QVERIFY(!mainText.contains(Model::description(feature.field)));
    const auto reason = model->disabledReason(edit, feature.field);
    if (!reason.isEmpty()) QVERIFY(!mainText.contains(reason));
    auto* alias = dialog.findChild<QLabel*>(name + QStringLiteral("OfficialName"));
    QVERIFY(!alias || !alias->isVisibleTo(&dialog));
  }
  for (auto scrollers = dialog.findChildren<QScrollArea*>(); auto* scroll : std::as_const(scrollers)) QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QVERIFY(confirm(dialog)->isVisible());
  auto* arrow = dialog.findChild<QWidget*>(QStringLiteral("overlayAdvancedTypeArrow"));
  QVERIFY(arrow);
  revealWidget(*arrow);
  const auto contextFrame = dialog.grab().toImage();
  for (const auto& name : {QStringLiteral("overlayAdvancedCurrentType"), QStringLiteral("overlayAdvancedNextType")}) {
    auto* type = dialog.findChild<ui::TextLabel*>(name);
    QVERIFY(type);
    QVERIFY2(rasterCentersAligned(paintedInk(contextFrame, *arrow, dialog), paintedInk(contextFrame, *type, dialog), dialog.devicePixelRatioF()),
             qUtf8Printable(type->text()));
  }
  auto* filter = dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayAdvancedFilterState"));
  auto* filterIndicator = dialog.findChild<QWidget*>(QStringLiteral("overlayAdvancedFilterIndicator"));
  QVERIFY(filter && filterIndicator);
  QVERIFY(rasterCentersAligned(paintedInk(contextFrame, *filter, dialog), paintedInk(contextFrame, *filterIndicator, dialog), dialog.devicePixelRatioF()));
  const auto frame = dialog.grab().toImage();

  const auto output = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!output.isEmpty()) {
    QVERIFY(QDir().mkpath(output));
    const auto tag = QString::fromLatin1(QTest::currentDataTag());
    const auto* pending = dialog.findChild<QWidget*>(QStringLiteral("overlayPassthroughChanged"));
    const auto variant = pending && pending->isVisibleTo(&dialog) ? QStringLiteral("changed") : QStringLiteral("current");
    QVERIFY(frame.save(QDir(output).filePath(QStringLiteral("overlay-inline-%1-%2.png").arg(tag, variant))));
  }
}
class OverlayAdvancedTests final : public QObject {
  Q_OBJECT
 private slots:
  void init() {
    ui::I18n::instance().setLang(ui::I18n::Lang::Zh_CN);
    ui::ThemeManager::instance().apply(app::Theme::Light);
  }
  void maintenanceDetailsAreReadOnlyWhenTheInventoryIsMissing_data() {
    QTest::addColumn<bool>("detailsAlreadyRead");
    QTest::newRow("published-configuration-only") << false;
    QTest::newRow("empty-inventory-already-read") << true;
  }
  void maintenanceDetailsAreReadOnlyWhenTheInventoryIsMissing() {
    QFETCH(bool, detailsAlreadyRead);
    Fixture f;
    auto facts = std::make_shared<MaintenanceTitleFacts>();
    auto maintenance = std::make_shared<presentation::OverlayMaintenanceViewModel>(facts, f.configuration.maintenanceEditor());
    maintenance->setInteractionContext(true, false);
    if (detailsAlreadyRead) {
      maintenance->refresh();
      QTRY_VERIFY(!maintenance->busy());
    } else {
      maintenance->adoptPublishedState();
      QVERIFY(!maintenance->busy());
      QCOMPARE(facts->reads.load(), 0);
    }
    QVERIFY(maintenance->hasSnapshot());
    QCOMPARE(maintenance->snapshot().volumesReadable, detailsAlreadyRead);
    QVERIFY(maintenance->snapshot().volumes.empty());
    {
      ui::OverlayAdvancedDialog dialog(f.model(), maintenance);
      dialog.show();
      QTRY_VERIFY(!maintenance->busy() && maintenance->snapshot().volumesReadable);
      QCoreApplication::processEvents();
      QCOMPARE(facts->reads.load(), 1);
      dialog.reject();
    }
    ui::OverlayAdvancedDialog reopened(f.model(), maintenance);
    reopened.show();
    QCoreApplication::processEvents();
    QVERIFY(!maintenance->busy());
    QCOMPARE(facts->reads.load(), 1);
    QVERIFY(!maintenance->hasPendingChanges());
    reopened.reject();
  }
  void flagControlsUseCurrentUwfRatherThanTheNextSession_data() {
    QTest::addColumn<bool>("currentFilter");
    QTest::addColumn<bool>("nextFilter");
    QTest::addColumn<bool>("ram");
    for (const bool current : {false, true})
      for (const bool next : {false, true})
        for (const bool ram : {false, true}) {
          if (current && ram) continue;
          QTest::newRow(qUtf8Printable(QStringLiteral("current-%1-next-%2-%3").arg(current).arg(next).arg(ram ? "ram" : "disk"))) << current << next << ram;
        }
  }
  void flagControlsUseCurrentUwfRatherThanTheNextSession() {
    QFETCH(bool, currentFilter);
    QFETCH(bool, nextFilter);
    QFETCH(bool, ram);
    ui::I18n::instance().setLang(ui::I18n::Lang::En);
    Fixture f;
    f.snapshot.current.filter.enabled = currentFilter;
    f.snapshot.next.filter.enabled = nextFilter;
    f.snapshot.next.overlay.type = ram ? core::OverlayType::RAM : core::OverlayType::Disk;
    f.policy.current.filterEnabled.value = currentFilter;
    f.policy.next.filterEnabled.value = nextFilter;
    f.policy.next.overlayType.value = ram ? 0 : 1;
    f.policy.next.flags.value = app::OverlayFlags{ram ? 1u : 3u};
    f.configuration.resetSnapshot(f.snapshot, f.policy);
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    auto* passthrough = control(dialog, "overlayPassthrough");
    auto* persistent = control(dialog, "overlayPersistent");
    QVERIFY(passthrough && persistent);
    QCOMPARE(passthrough->isEnabled(), !currentFilter);
    QCOMPARE(persistent->isEnabled(), !currentFilter && !ram);
    const auto edit = f.model()->beginEdit();
    for (const auto field : {Model::Field::Passthrough, Model::Field::Persistent}) {
      const QString expected = currentFilter ? QStringLiteral("UWF is enabled in the current session. Disable UWF and restart before changing this setting.")
                               : ram && field == Model::Field::Persistent
                                   ? QStringLiteral("Persistent Overlay is only available for Disk; the next overlay is RAM.")
                                   : QString{};
      QCOMPARE(f.model()->disabledReason(edit, field), expected);
      auto* reason = action(dialog, features[field == Model::Field::Passthrough ? 0 : 1], "Reason");
      if (!expected.isEmpty()) {
        QVERIFY(reason && reason->isVisible() && reason->isEnabled());
        QVERIFY(inspectDetails(dialog, *reason, DetailClose::Back, [&](QDialog& details) {
          auto* text = details.findChild<QLabel*>(QStringLiteral("restrictionReason"));
          QVERIFY(text);
          QCOMPARE(text->text(), expected);
          checkDetailLayout(details, 500);
        }));
        QTRY_VERIFY(reason->hasFocus());
      } else {
        QVERIFY(!reason || !reason->isVisible());
      }
      auto* button = field == Model::Field::Passthrough ? passthrough : persistent;
      const bool checked = button->isChecked();
      button->click();
      QCOMPARE(button->isChecked(), button->isEnabled() ? !checked : checked);
    }
    QVERIFY(f.configuration.pendingConfiguration().empty());
  }
  void featureDetailsExposeTheirExplanationAndConditions_data() {
    QTest::addColumn<int>("featureIndex");
    for (std::size_t index = 0; index < features.size(); ++index) QTest::newRow(features[index].name) << static_cast<int>(index);
  }
  void featureDetailsExposeTheirExplanationAndConditions() {
    QFETCH(int, featureIndex);
    const auto feature = features.at(static_cast<std::size_t>(featureIndex));
    Fixture f;
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    control(dialog, "overlayPassthrough")->click();
    auto* info = action(dialog, feature, "Info");
    QVERIFY(info && info->isVisible() && info->isEnabled());
    QVERIFY(info->focusPolicy() != Qt::NoFocus);
    QVERIFY(!info->accessibleName().isEmpty());
    const auto expected = Model::featureDetails(feature.field);
    QVERIFY(inspectDetails(dialog, *info, DetailClose::Back, [&](QDialog& details) {
      QCOMPARE(details.windowTitle(), Model::fieldName(feature.field));
      auto* official = details.findChild<QLabel*>(QStringLiteral("officialName"));
      auto* description = details.findChild<QLabel*>(QStringLiteral("detailDescription"));
      QVERIFY(official && description);
      QCOMPARE(official->text(), expected.officialName);
      QVERIFY(official->isVisible());
      QCOMPARE(description->text(), expected.description);
      for (const auto& condition : expected.conditions) {
        QVERIFY(visibleText(details).contains(condition.title));
        QVERIFY(visibleText(details).contains(condition.text));
      }
      checkDetailLayout(details, 500);
    }));
    QTRY_VERIFY(info->hasFocus());
    QVERIFY(dialog.isVisible());
    QVERIFY(!control(dialog, "overlayPassthrough")->isChecked());
    QVERIFY(f.configuration.pendingConfiguration().empty());
  }
  void secondaryCloseReturnsToTheSameLocalDraft_data() {
    QTest::addColumn<bool>("reason");
    QTest::addColumn<int>("closeEntry");
    QTest::addColumn<bool>("keyboard");
    for (const bool reason : {false, true})
      for (const auto close : {DetailClose::Back, DetailClose::Escape, DetailClose::WindowClose})
        for (const bool keyboard : {false, true}) {
          const auto name = close == DetailClose::Back ? "back" : close == DetailClose::Escape ? "escape" : "window-close";
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(reason ? "restriction" : "feature", name, keyboard ? "keyboard" : "pointer")))
              << reason << static_cast<int>(close) << keyboard;
        }
  }
  void secondaryCloseReturnsToTheSameLocalDraft() {
    QFETCH(bool, reason);
    QFETCH(int, closeEntry);
    QFETCH(bool, keyboard);
    Fixture f;
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    QSignalSpy parentFinished(&dialog, &QDialog::finished);
    QSignalSpy pendingChanged(&f.configuration, &presentation::ConfigurationViewModel::pendingChanged);
    control(dialog, "overlayPassthrough")->click();
    auto* changed = dialog.findChild<QWidget*>(QStringLiteral("overlayPassthroughChanged"));
    QVERIFY(changed && changed->isVisible());
    const auto feature = features[reason ? 3 : 1];
    auto* opener = action(dialog, feature, reason ? "Reason" : "Info");
    QVERIFY(opener && opener->isVisible() && opener->isEnabled());
    QVERIFY(opener->focusPolicy() != Qt::NoFocus);
    QVERIFY(!opener->accessibleName().isEmpty());
    QVERIFY(inspectDetails(
        dialog, *opener, static_cast<DetailClose>(closeEntry),
        [&](QDialog& details) {
          auto* text = details.findChild<QLabel*>(QString::fromLatin1(reason ? "restrictionReason" : "detailDescription"));
          QVERIFY(text);
          QCOMPARE(text->text(), reason ? f.model()->disabledReason(f.model()->beginEdit(), feature.field) : Model::description(feature.field));
        },
        keyboard));
    QTRY_VERIFY(opener->hasFocus());
    QVERIFY(dialog.isVisible());
    QCOMPARE(parentFinished.count(), 0);
    QCOMPARE(pendingChanged.count(), 0);
    QVERIFY(!control(dialog, "overlayPassthrough")->isChecked());
    QVERIFY(changed->isVisible());
    QVERIFY(f.configuration.pendingConfiguration().empty());
    dialog.reject();
    QVERIFY(f.configuration.pendingConfiguration().empty());
  }
  void languageChangesPreserveEditingAndRefreshSecondaryText() {
    Fixture f;
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    control(dialog, "overlayPassthrough")->click();
    for (const auto language : {ui::I18n::Lang::En, ui::I18n::Lang::Ja}) {
      ui::I18n::instance().setLang(language);
      const auto* alias = dialog.findChild<QLabel*>(QStringLiteral("overlayPassthroughOfficialName"));
      QVERIFY(!alias || !alias->isVisibleTo(&dialog));
      auto* info = action(dialog, features.front(), "Info");
      QVERIFY(info);
      QVERIFY(inspectDetails(dialog, *info, DetailClose::Back, [&](QDialog& details) {
        auto* description = details.findChild<QLabel*>(QStringLiteral("detailDescription"));
        auto* official = details.findChild<QLabel*>(QStringLiteral("officialName"));
        QVERIFY(description && official);
        QCOMPARE(description->text(), Model::description(Model::Field::Passthrough));
        QCOMPARE(official->text(), Model::officialName(Model::Field::Passthrough));
        QCOMPARE(official->isVisible(), language != ui::I18n::Lang::En);
      }));
      QVERIFY(!control(dialog, "overlayPassthrough")->isChecked());
      auto* current = dialog.findChild<QLabel*>(QStringLiteral("overlayPassthroughCurrent"));
      QVERIFY(current);
      QCOMPARE(current->text(), f.model()->currentText(Model::Field::Passthrough));
      QVERIFY(f.configuration.pendingConfiguration().empty());
    }
  }
  void currentStatesKeepTheirBareTextAndSemanticColors_data() {
    QTest::addColumn<bool>("dark");
    QTest::addColumn<int>("state");
    for (const bool dark : {false, true})
      for (const auto state : {Model::ValueState::Off, Model::ValueState::On, Model::ValueState::Unknown}) {
        const auto name = state == Model::ValueState::Off ? "off" : state == Model::ValueState::On ? "on" : "unknown";
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(dark ? "dark" : "light", name))) << dark << static_cast<int>(state);
      }
  }
  void currentStatesKeepTheirBareTextAndSemanticColors() {
    QFETCH(bool, dark);
    QFETCH(int, state);
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    const auto expected = static_cast<Model::ValueState>(state);
    Fixture f;
    if (expected == Model::ValueState::Unknown)
      f.policy.current.flags = {app::PolicyObservationStatus::Failed};
    else
      f.policy.current.flags.value = app::OverlayFlags{expected == Model::ValueState::On ? 3u : 0u};
    f.configuration.resetSnapshot(f.snapshot, f.policy);
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    auto* current = dialog.findChild<QLabel*>(QStringLiteral("overlayPassthroughCurrent"));
    QVERIFY(current);
    QCOMPARE(f.model()->currentState(Model::Field::Passthrough), expected);
    QCOMPARE(current->text(), f.model()->currentText(Model::Field::Passthrough));
    using Color = ui::design::Color;
    const auto foreground = expected == Model::ValueState::On ? Color::Success : expected == Model::ValueState::Off ? Color::Accent : Color::Warning;
    const auto background = expected == Model::ValueState::On ? Color::SuccessBg : expected == Model::ValueState::Off ? Color::Selected : Color::WarningBg;
    auto* badge = dialog.findChild<QWidget*>(QStringLiteral("overlayPassthroughCurrentBadge"));
    QVERIFY(badge);
    const auto clipping = test::waitForUnclippedText(dialog);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    QTRY_VERIFY(badgeHasSemanticColors(*badge, *current, ui::DesignSystem::instance().color(foreground), ui::DesignSystem::instance().color(background)));
  }
  void resetCurrentStateIsSeparateFromTheNextStartupRequest_data() {
    QTest::addColumn<int>("selector");
    QTest::newRow("preserve") << 0;
    QTest::newRow("clear-at-next-startup") << 1;
    QTest::newRow("saved-mode-unknown") << 255;
  }
  void resetCurrentStateIsSeparateFromTheNextStartupRequest() {
    QFETCH(int, selector);
    Fixture f;
    f.policy.resetPersistentState.value = selector;
    f.configuration.resetSnapshot(f.snapshot, f.policy);
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    auto* current = dialog.findChild<QLabel*>(QStringLiteral("overlayResetPersistentCurrent"));
    auto* next = control(dialog, "overlayResetPersistent");
    QVERIFY(current && next);
    QCOMPARE(current->text(), ui::I18n::tr(selector == 255 ? "Unknown" : "Off"));
    QCOMPARE(f.model()->currentState(Model::Field::Reset), selector == 255 ? Model::ValueState::Unknown : Model::ValueState::Off);
    QCOMPARE(next->isUnknown(), selector == 255);
    QCOMPARE(next->isChecked(), selector == 1);
    QVERIFY(f.configuration.pendingConfiguration().empty());
  }
  void mainOverlayTypeTracksLanguageWithoutChangingItsValue_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<QString>("disk");
    QTest::newRow("english") << static_cast<int>(ui::I18n::Lang::En) << QStringLiteral("Disk");
    QTest::newRow("chinese") << static_cast<int>(ui::I18n::Lang::Zh_CN) << QStringLiteral("磁盘");
  }
  void mainOverlayTypeTracksLanguageWithoutChangingItsValue() {
    QFETCH(int, language);
    QFETCH(QString, disk);
    const auto initial = static_cast<ui::I18n::Lang>(language);
    ui::I18n::instance().setLang(initial);
    Fixture f;
    ui::GlobalStatusPanel panel(f.configuration.global(), f.model());
    auto* combo = panel.findChild<QComboBox*>(QStringLiteral("overlayTypeNext"));
    QVERIFY(combo);
    const int diskIndex = combo->findData(static_cast<int>(core::OverlayType::Disk));
    QVERIFY(diskIndex >= 0);
    QCOMPARE(combo->itemText(diskIndex), disk);
    QCOMPARE(combo->itemText(diskIndex), app::overlayTypeText(core::OverlayType::Disk));
    const auto selected = combo->currentData();
    ui::I18n::instance().setLang(initial == ui::I18n::Lang::En ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En);
    QCOMPARE(combo->itemText(diskIndex), app::overlayTypeText(core::OverlayType::Disk));
    QCOMPARE(combo->currentData(), selected);
    ui::I18n::instance().setLang(initial);
    QCOMPARE(combo->itemText(diskIndex), disk);
    QCOMPARE(combo->currentData(), selected);
    QVERIFY(f.configuration.pendingConfiguration().empty());
  }
  void cancelDiscardsLocalEditingAndConfirmStagesOnly() {
    Fixture f;
    {
      ui::OverlayAdvancedDialog dialog(f.model());
      dialog.show();
      control(dialog, "overlayPassthrough")->click();
      dialog.reject();
    }
    QVERIFY(f.configuration.pendingConfiguration().empty());
    ui::OverlayAdvancedDialog dialog(f.model());
    dialog.show();
    QSignalSpy changed(&f.configuration, &presentation::ConfigurationViewModel::pendingChanged);
    control(dialog, "overlayPassthrough")->click();
    auto* current = dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayPassthroughCurrent"));
    QCOMPARE(current->text(), f.model()->currentText(Model::Field::Passthrough));
    confirm(dialog)->setFocus(Qt::TabFocusReason);
    QTest::keyClick(confirm(dialog), Qt::Key_Return);
    QCOMPARE(dialog.result(), QDialog::Accepted);
    QCOMPARE(f.configuration.pendingConfiguration().count(), std::size_t{1});
    QCOMPARE(changed.count(), 1);
    QCOMPARE(f.policy.next.flags.value->raw, std::uint32_t{3});
    ui::OverlayAdvancedDialog reopened(f.model());
    QVERIFY(!control(reopened, "overlayPassthrough")->isChecked());
  }
  void saveUsesOnlyChangesSinceOpening_data() {
    QTest::addColumn<bool>("overlayPending");
    QTest::addColumn<bool>("maintenancePending");
    for (const bool overlay : {false, true})
      for (const bool maintenance : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("overlay-%1-maintenance-%2").arg(overlay).arg(maintenance))) << overlay << maintenance;
  }
  void saveUsesOnlyChangesSinceOpening() {
    QFETCH(bool, overlayPending);
    QFETCH(bool, maintenancePending);
    Fixture f;
    // An unrelated main-window draft is not a local change in this dialog.
    f.configuration.global()->editWarningThreshold(1024);
    if (overlayPending) {
      auto edit = f.model()->beginEdit();
      Model::setValue(edit, Model::Field::Passthrough, false);
      QVERIFY(f.model()->commit(edit));
    }
    auto maintenance =
        std::make_shared<presentation::OverlayMaintenanceViewModel>(std::make_shared<MaintenanceTitleFacts>(), f.configuration.maintenanceEditor());
    maintenance->setInteractionContext(true, false);
    maintenance->refresh();
    QTRY_VERIFY(!maintenance->busy());
    if (maintenancePending) QVERIFY(maintenance->stageAllVolumesProtection(true));
    const auto openingOverlay = f.configuration.pendingConfiguration();
    const auto openingMaintenance = maintenance->draft();
    ui::OverlayAdvancedDialog dialog(f.model(), maintenance);
    QVERIFY(expose(dialog));
    auto* save = confirm(dialog);
    auto* passthrough = control(dialog, "overlayPassthrough");
    QVERIFY(save && passthrough && passthrough->isEnabled());
    QVERIFY(!save->isEnabled());
    const bool openingValue = passthrough->isChecked();
    passthrough->click();
    QVERIFY(save->isEnabled());
    passthrough->click();
    QCOMPARE(passthrough->isChecked(), openingValue);
    QVERIFY(!save->isEnabled());
    QCOMPARE(f.configuration.pendingConfiguration().overlayPolicy, openingOverlay.overlayPolicy);
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);
    QVERIFY(!save->isEnabled());
    auto* protect = dialog.findChild<QAction*>(QStringLiteral("protectAllVolumes"));
    auto* unprotect = dialog.findChild<QAction*>(QStringLiteral("unprotectAllVolumes"));
    QVERIFY(protect && unprotect);
    auto* change = maintenancePending ? unprotect : protect;
    QVERIFY(change->isEnabled());
    change->trigger();
    QVERIFY(save->isEnabled());
    QCOMPARE(maintenance->draft(), openingMaintenance);
    if (maintenancePending) {
      protect->trigger();
      QVERIFY(!save->isEnabled());
      unprotect->trigger();
      QVERIFY(save->isEnabled());
    }
    QSignalSpy overlayChanged(f.model().get(), &Model::pendingChanged);
    save->click();
    QCOMPARE(dialog.result(), QDialog::Accepted);
    QCOMPARE(maintenance->draft().allVolumesProtected, std::optional{!maintenancePending});
    QCOMPARE(f.configuration.pendingConfiguration().overlayPolicy, openingOverlay.overlayPolicy);
    QCOMPARE(f.configuration.pendingConfiguration().settings.setOverlay.warningThresholdMb, openingOverlay.settings.setOverlay.warningThresholdMb);
    QCOMPARE(overlayChanged.count(), 0);
  }
  void togglingPersistentBackAfterRevertDoesNotRestoreItForFutureStorage() {
    Fixture f;
    const auto volume = QStringLiteral("66666666-7777-8888-9999-aaaaaaaaaaaa");
    auto facts = std::make_shared<MaintenanceTitleFacts>();
    auto maintenance = std::make_shared<presentation::OverlayMaintenanceViewModel>(facts, f.configuration.maintenanceEditor());
    auto observed = *facts->snapshot();
    observed.maximumOverlaySize = f.snapshot.next.overlay.maximumSizeMb;
    observed.volumesReadable = true;
    observed.swapCapability.support = app::OverlayMaintenanceSupport::Supported;
    observed.volumes = {{volume, QStringLiteral("D:"), QStringLiteral("Storage"), std::uint64_t{16} * 1024 * 1024 * 1024, true, {}, {}}};
    maintenance->setInteractionContext(true, false);
    auto editor = maintenance->editor();
    const auto observation = editor->beginObservation();
    QVERIFY(observation.has_value());
    editor->finishObservation(*observation, observed);
    QVERIFY(maintenance->stageStorageLocation(volume));

    ui::OverlayAdvancedDialog dialog(f.model(), maintenance);
    QVERIFY(expose(dialog));
    auto* persistent = control(dialog, "overlayPersistent");
    auto* revert = dialog.findChild<QAbstractButton*>(QStringLiteral("overlayAdvancedRevert"));
    auto* save = confirm(dialog);
    QVERIFY(persistent && persistent->isEnabled() && !persistent->isChecked());
    QVERIFY(revert && revert->isVisibleTo(&dialog));
    QVERIFY(save && !save->isEnabled());
    revert->click();
    QVERIFY(persistent->isChecked());
    QVERIFY(save->isEnabled());
    persistent->click();
    QVERIFY(!persistent->isChecked());
    persistent->click();
    QVERIFY(persistent->isChecked());
    QVERIFY(save->isEnabled());
    save->click();
    QCOMPARE(dialog.result(), QDialog::Accepted);
    QVERIFY(f.configuration.pendingConfiguration().empty());
    QVERIFY(f.model()->beginEdit().intent.empty());

    QVERIFY(maintenance->stageStorageLocation(volume));
    ui::OverlayAdvancedDialog reopened(f.model(), maintenance);
    QVERIFY(expose(reopened));
    auto* nextPersistent = control(reopened, "overlayPersistent");
    QVERIFY(nextPersistent && nextPersistent->isEnabled());
    QVERIFY(!nextPersistent->isChecked());
    QVERIFY(f.model()->beginEdit().intent.empty());
    reopened.reject();
  }
  void sharedModelUpdatesRebasePristineEditingAndCancelPreservesThem() {
    Fixture f;
    auto edit = f.model()->beginEdit();
    Model::setValue(edit, Model::Field::Passthrough, false);
    QVERIFY(f.model()->commit(edit));
    auto maintenance =
        std::make_shared<presentation::OverlayMaintenanceViewModel>(std::make_shared<MaintenanceTitleFacts>(), f.configuration.maintenanceEditor());
    maintenance->setInteractionContext(true, false);
    maintenance->refresh();
    QTRY_VERIFY(!maintenance->busy());
    ui::OverlayAdvancedDialog dialog(f.model(), maintenance);
    QVERIFY(expose(dialog));
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);
    QVERIFY(!confirm(dialog)->isEnabled());
    // Another editor's changes update a pristine session's baseline. They are
    // not user edits in this window and Cancel must never undo them.
    edit = f.model()->beginEdit();
    Model::setValue(edit, Model::Field::Passthrough, true);
    QVERIFY(f.model()->commit(edit));
    QVERIFY(!confirm(dialog)->isEnabled());
    QVERIFY(maintenance->stageAllVolumesProtection(true));
    const auto externalMaintenance = maintenance->draft();
    QVERIFY(!confirm(dialog)->isEnabled());
    tabs->setCurrentIndex(0);
    auto* passthrough = control(dialog, "overlayPassthrough");
    QVERIFY(passthrough && passthrough->isChecked());
    passthrough->click();
    QVERIFY(!passthrough->isChecked());
    QVERIFY(confirm(dialog)->isEnabled());
    passthrough->click();
    QVERIFY(!confirm(dialog)->isEnabled());
    passthrough->click();
    QVERIFY(confirm(dialog)->isEnabled());
    dialog.reject();
    QCOMPARE(maintenance->draft(), externalMaintenance);
    QCOMPARE(Model::value(f.model()->beginEdit(), Model::Field::Passthrough), std::optional{true});
  }
  void knownRestrictionsDisableControlsAndConflictsCanBeRepaired() {
    Fixture f;
    f.snapshot.current.filter.enabled = false;
    f.policy.next.flags.value = app::OverlayFlags{app::OverlayFlags::Passthrough};
    f.configuration.resetSnapshot(f.snapshot, f.policy);
    QList<app::UwfmgrCommand> commands;
    for (const auto& command : app::parseUwfmgrText("uwfmgr overlay set-persistent on")) commands.append(command);
    const auto imported = f.configuration.importCommands(commands);
    QCOMPARE(imported.size(), 1);
    QCOMPARE(imported.front().status, app::ImportResult::Status::Success);
    f.configuration.global()->editOverlayType(core::OverlayType::RAM);
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    QVERIFY(!confirm(dialog)->isEnabled());
    auto* details = dialog.findChild<QAbstractButton*>(QStringLiteral("overlayAdvancedConflictDetails"));
    QVERIFY(details && details->isVisible() && details->isEnabled());
    QVERIFY(inspectDetails(dialog, *details, DetailClose::Back, [&](QDialog& child) {
      auto* list = child.findChild<QWidget*>(QStringLiteral("overlayConflictList"));
      QVERIFY(list);
      const auto conflicts = f.model()->conflicts(f.model()->beginEdit());
      QVERIFY(!conflicts.isEmpty());
      for (const auto& conflict : conflicts) QVERIFY(visibleText(child).contains(conflict));
    }));
    QTRY_VERIFY(details->hasFocus());
    QVERIFY(control(dialog, "overlayPersistent")->isEnabled());
    control(dialog, "overlayPersistent")->click();
    QVERIFY(confirm(dialog)->isEnabled());
    QVERIFY(!control(dialog, "overlayPersistent")->isEnabled());
    // Current UWF is stopped and the next overlay is RAM, so ROM can be enabled.
    QVERIFY(control(dialog, "overlayReadOnlyMedia")->isEnabled());
    QVERIFY(control(dialog, "overlayPassthrough")->isEnabled());
    confirm(dialog)->click();
    // Repair removes the imported Persistent-on intent; only the type edit remains.
    QCOMPARE(f.configuration.pendingConfiguration().count(), std::size_t{1});
    QVERIFY(!f.configuration.pendingConfiguration().overlayPolicy.persistent);
  }
  void revertingAnImportedConflictIsLocalUntilConfirmed() {
    Fixture f;
    QList<app::UwfmgrCommand> commands;
    for (const auto& command : app::parseUwfmgrText("uwfmgr.exe overlay set-rom-mode on")) commands.append(command);
    QVERIFY(!commands.isEmpty());
    const auto importResults = f.configuration.importCommands(commands);
    QCOMPARE(importResults.size(), 1);
    QCOMPARE(importResults.front().status, app::ImportResult::Status::Success);
    QCOMPARE(f.configuration.pendingConfiguration().overlayPolicy.readOnlyMedia, std::optional{true});
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    auto* details = dialog.findChild<QAbstractButton*>(QStringLiteral("overlayAdvancedConflictDetails"));
    auto* revert = dialog.findChild<QAbstractButton*>(QStringLiteral("overlayAdvancedRevert"));
    auto* changed = dialog.findChild<QWidget*>(QStringLiteral("overlayReadOnlyMediaChanged"));
    QVERIFY(details && revert && changed);
    QVERIFY(!confirm(dialog)->isEnabled());
    QVERIFY(control(dialog, "overlayReadOnlyMedia")->isChecked());
    QVERIFY(changed->isVisible());
    const auto conflicts = f.model()->conflicts(f.model()->beginEdit());
    QVERIFY(!conflicts.isEmpty());
    QSignalSpy pendingChanged(&f.configuration, &presentation::ConfigurationViewModel::pendingChanged);
    QVERIFY(inspectDetails(dialog, *details, DetailClose::Escape, [&](QDialog& child) {
      auto* list = child.findChild<QWidget*>(QStringLiteral("overlayConflictList"));
      QVERIFY(list);
      QStringList reasons;
      for (auto labels = list->findChildren<QLabel*>(); auto* label : std::as_const(labels))
        if (conflicts.contains(label->text())) reasons.append(label->text());
      QCOMPARE(reasons, conflicts);
      checkDetailLayout(child, 440);
    }));
    QTRY_VERIFY(details->hasFocus());
    QCOMPARE(pendingChanged.count(), 0);
    QVERIFY(control(dialog, "overlayReadOnlyMedia")->isChecked());
    QVERIFY(changed->isVisible());
    revert->click();
    QVERIFY(!control(dialog, "overlayReadOnlyMedia")->isChecked());
    QVERIFY(!changed->isVisible());
    QVERIFY(!details->isVisible());
    QVERIFY(!revert->isVisible());
    QVERIFY(confirm(dialog)->isEnabled());
    QCOMPARE(pendingChanged.count(), 0);
    QCOMPARE(f.configuration.pendingConfiguration().overlayPolicy.readOnlyMedia, std::optional{true});
    confirm(dialog)->click();
    QCOMPARE(dialog.result(), QDialog::Accepted);
    QCOMPARE(pendingChanged.count(), 1);
    QVERIFY(f.configuration.pendingConfiguration().empty());
  }
  void dllAvailabilityControlsGearAndReadonlyStillOpens() {
    Fixture f;
    ui::GlobalStatusPanel panel(f.configuration.global(), f.model());
    panel.show();
    auto* gear = panel.findChild<QPushButton*>(QStringLiteral("overlayAdvancedSettings"));
    QVERIFY(gear && !gear->isHidden());
    f.snapshot.elevated = false;
    f.configuration.resetSnapshot(f.snapshot, f.policy);
    QVERIFY(gear->isEnabled());
    ui::OverlayAdvancedDialog dialog(f.model());
    dialog.show();
    QVERIFY(confirm(dialog)->isHidden());
    QVERIFY(!control(dialog, "overlayPassthrough")->isEnabled());
    f.policy.transportAvailable = false;
    f.configuration.resetSnapshot(f.snapshot, f.policy);
    QVERIFY(gear->isHidden());
  }
  void partialReadFailureDoesNotDisableOtherFieldsOrDisplayFalse() {
    Fixture f;
    f.policy.resetPersistentState = {app::PolicyObservationStatus::Failed};
    f.configuration.resetSnapshot(f.snapshot, f.policy);
    ui::OverlayAdvancedDialog dialog(f.model());
    dialog.show();
    QVERIFY(control(dialog, "overlayResetPersistent")->isUnknown());
    QVERIFY(!control(dialog, "overlayResetPersistent")->isEnabled());
    QVERIFY(control(dialog, "overlayPassthrough")->isEnabled());
    control(dialog, "overlayPassthrough")->click();
    confirm(dialog)->click();
    QVERIFY(!f.configuration.pendingConfiguration().overlayPolicy.reset);
    QCOMPARE(f.configuration.pendingConfiguration().overlayPolicy.passthrough, std::optional{false});
  }
  void normalDialogKeepsTheCompactThreeColumnLayout() {
    Fixture f;
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    QCOMPARE(dialog.width(), 660);
    checkMainLayout(dialog, f.model());
    auto* scroll = dialog.findChild<QScrollArea*>();
    QVERIFY(scroll);
    QCOMPARE(scroll->verticalScrollBar()->maximum(), 0);
    QVERIFY(dialog.height() < 640);
    auto* hint = dialog.findChild<QLabel*>(QStringLiteral("overlayAdvancedHint"));
    QVERIFY(hint && !hint->isVisible());
    QVERIFY(hint->text().isEmpty());
    QVERIFY(!dialog.findChild<QAbstractButton*>(QStringLiteral("overlayAdvancedRevert"))->isVisible());
    QVERIFY(!dialog.findChild<QAbstractButton*>(QStringLiteral("overlayAdvancedConflictDetails"))->isVisible());
  }
  void unifiedReviewPreservesCspDraftAndExecutesTheReviewedPlan() {
    Fixture f;
    auto edit = f.model()->beginEdit();
    Model::setValue(edit, Model::Field::Passthrough, false);
    Model::setValue(edit, Model::Field::Persistent, false);
    QVERIFY(f.model()->commit(edit));
    auto actions = std::make_shared<Actions>();
    const auto plan = app::reviewApplicationConfiguration(f.configuration.pendingConfiguration(), f.snapshot, f.policy);
    presentation::ApplyPlanViewModel review(plan, actions);
    QCOMPARE(review.changeCount(), std::size_t{2});
    QCOMPARE(review.groups().size(), 1);
    QCOMPARE(review.groups().front().items.size(), 2);
    QVERIFY(review.changeCommandsText().contains(QStringLiteral("set-passthrough off")));
    QVERIFY(review.changeCommandsText().contains(QStringLiteral("set-persistent off")));
    QVERIFY(!review.changeCommandsText().contains(QStringLiteral("reset-persistentstate")));
    QVERIFY(!review.changeCommandsText().contains(QStringLiteral("255")));
    QCOMPARE(actions->mixed, 0);
    review.apply();
    QTRY_COMPARE(actions->completed, 1);
    QCOMPARE(actions->legacy, 0);
    QCOMPARE(actions->received.overlayPolicy, plan.overlayPolicy);
    QVERIFY(!review.canApply());
    QVERIFY(review.canRestart());
  }
  void invalidImportedPlanRemainsReviewableWithoutExecutableCommands() {
    Fixture f;
    auto changes = f.configuration.pendingConfiguration();
    changes.overlayPolicy.readOnlyMedia = true;
    auto actions = std::make_shared<Actions>();
    presentation::ApplyPlanViewModel review(app::reviewApplicationConfiguration(changes, f.snapshot, f.policy), actions);
    QVERIFY(!review.canApply());
    QVERIFY(!review.warning().isEmpty());
    QCOMPARE(review.changeCount(), std::size_t{1});
    QVERIFY(review.exportCommands().empty());
    review.apply();
    QCOMPARE(actions->mixed, 0);
  }
  void onlyCurrentMediaModeBlocksIndividualCommits_data() {
    QTest::addColumn<bool>("flagsKnown");
    QTest::addColumn<bool>("currentMediaMode");
    QTest::addColumn<bool>("nextMediaMode");
    for (const bool known : {false, true})
      for (const bool current : {false, true})
        for (const bool next : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("known-%1/current-%2/next-%3").arg(known).arg(current).arg(next))) << known << current << next;
  }
  void onlyCurrentMediaModeBlocksIndividualCommits() {
    QFETCH(bool, flagsKnown);
    QFETCH(bool, currentMediaMode);
    QFETCH(bool, nextMediaMode);
    Fixture f;
    f.snapshot.current.filter.enabled = true;
    f.policy.current.filterEnabled.value = true;
    f.snapshot.current.overlay.type = f.snapshot.next.overlay.type = core::OverlayType::RAM;
    f.policy.current.overlayType.value = f.policy.next.overlayType.value = 0;
    f.policy.next.flags.value = app::OverlayFlags{nextMediaMode ? app::OverlayFlags::ReadOnlyMedia : 0u};
    auto volume = f.configuration.addVolume({"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 0, 0, core::VolumeSupport::Supported}, true);
    f.policy.current.flags = {flagsKnown ? app::PolicyObservationStatus::Available : app::PolicyObservationStatus::Failed,
                              app::OverlayFlags{currentMediaMode ? app::OverlayFlags::ReadOnlyMedia : 0u}};
    f.configuration.resetSnapshot(f.snapshot, f.policy);
    const auto availability = volume->commitAvailability();
    const bool commitsAllowed = !(flagsKnown && currentMediaMode);
    QCOMPARE(availability.files, commitsAllowed);
    QCOMPARE(availability.registry, commitsAllowed);
    QCOMPARE(volume->files()->readOnly(), flagsKnown && currentMediaMode);
    QCOMPARE(volume->registry()->readOnly(), flagsKnown && currentMediaMode);
    QVERIFY(availability.overlay);
    QVERIFY(f.configuration.global()->controlsEnabled());
    QVERIFY(!availability.commitHint.isEmpty());
    if (!commitsAllowed) QCOMPARE(availability.commitHint, ui::I18n::tr("File and registry commits are unavailable while Read-Only Media mode is active."));
    // An unreadable optional extension must not silently turn off legacy WMI capabilities.
    f.configuration.resetSnapshot(f.snapshot, {});
    QVERIFY(volume->commitAvailability().files);
    QVERIFY(volume->commitAvailability().registry);
  }
  void reviewUsesTheSameClearEligibilityAsTheEditor() {
    Fixture f;
    f.policy.current.flags.value = app::OverlayFlags{1};
    auto changes = f.configuration.pendingConfiguration();
    changes.overlayPolicy.reset = app::PersistentReset::Clear;
    auto actions = std::make_shared<Actions>();
    presentation::ApplyPlanViewModel review(app::reviewApplicationConfiguration(changes, f.snapshot, f.policy), actions);
    QVERIFY(!review.canApply());
    QCOMPARE(review.warning(), Model::conflictText(app::OverlayPolicyIssue::CurrentPersistentRequired));
    QVERIFY(review.exportCommands().empty());
  }
  void allLanguagesFitTheirTextAtNormalAndNarrowWidths_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<int>("width");
    QTest::addColumn<bool>("dark");
    for (const auto& language : ui::I18n::languages())
      for (const int width : {660, 440})
        for (const bool dark : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(QString(language.code)).arg(width).arg(dark ? "dark" : "light")))
              << static_cast<int>(language.id) << width << dark;
  }
  void allLanguagesFitTheirTextAtNormalAndNarrowWidths() {
    QFETCH(int, language);
    QFETCH(int, width);
    QFETCH(bool, dark);
    const auto requestedLanguage = static_cast<ui::I18n::Lang>(language);
    ui::I18n::instance().setLang(requestedLanguage);
    QCOMPARE(ui::I18n::instance().lang(), requestedLanguage);
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    Fixture f;
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    dialog.resize(width, dialog.height());
    QTRY_COMPARE(dialog.width(), width);
    checkMainLayout(dialog, f.model());
    control(dialog, "overlayPassthrough")->click();
    checkMainLayout(dialog, f.model());
    auto* changed = dialog.findChild<QWidget*>(QStringLiteral("overlayPassthroughChanged"));
    auto* revert = dialog.findChild<QAbstractButton*>(QStringLiteral("overlayAdvancedRevert"));
    QVERIFY(changed && changed->isVisible());
    QVERIFY(revert && revert->isVisible());
    for (const bool restriction : {false, true}) {
      const auto feature = features[restriction ? 3 : 1];
      auto* opener = action(dialog, feature, restriction ? "Reason" : "Info");
      QVERIFY(opener && opener->isVisible() && opener->isEnabled());
      QVERIFY(inspectDetails(dialog, *opener, DetailClose::Back, [&](QDialog& details) {
        auto* text = details.findChild<QLabel*>(QString::fromLatin1(restriction ? "restrictionReason" : "detailDescription"));
        QVERIFY(text);
        QCOMPARE(text->text(), restriction ? f.model()->disabledReason(f.model()->beginEdit(), feature.field) : Model::description(feature.field));
        checkDetailLayout(details, width);
      }));
      QTRY_VERIFY(opener->hasFocus());
      QVERIFY(!control(dialog, "overlayPassthrough")->isChecked());
    }
    QVERIFY(f.configuration.pendingConfiguration().empty());
  }
  void minimumWidthKeepsAllThreeColumns_data() {
    QTest::addColumn<int>("language");
    QTest::newRow("english") << static_cast<int>(ui::I18n::Lang::En);
    QTest::newRow("chinese") << static_cast<int>(ui::I18n::Lang::Zh_CN);
    QTest::newRow("arabic") << static_cast<int>(ui::I18n::Lang::Ar);
  }
  void minimumWidthKeepsAllThreeColumns() {
    QFETCH(int, language);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    Fixture f;
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    dialog.resize(360, dialog.height());
    QTRY_COMPARE(dialog.width(), 360);
    checkMainLayout(dialog, f.model());
    auto* reason = action(dialog, features.back(), "Reason");
    QVERIFY(reason && reason->isVisible() && reason->isEnabled());
    QVERIFY(inspectDetails(dialog, *reason, DetailClose::Back, [&](QDialog& details) { checkDetailLayout(details, 360); }));
    QTRY_VERIFY(reason->hasFocus());
  }
  void englishFeatureAliasesHideOnlyDuplicateNames_data() {
    QTest::addColumn<bool>("normalizedSpelling");
    QTest::newRow("original-spelling") << false;
    QTest::newRow("whitespace-and-case") << true;
  }
  void englishFeatureAliasesHideOnlyDuplicateNames() {
    QFETCH(bool, normalizedSpelling);
    ui::I18n::instance().setLang(ui::I18n::Lang::En);
    DuplicateAliasTranslator translator;
    if (normalizedSpelling) QVERIFY(QCoreApplication::installTranslator(&translator));
    const auto restore = qScopeGuard([&] {
      if (normalizedSpelling) QCoreApplication::removeTranslator(&translator);
    });
    Fixture f;
    ui::OverlayAdvancedDialog dialog(f.model());
    QVERIFY(expose(dialog));
    for (const auto& feature : features) {
      auto* info = action(dialog, feature, "Info");
      QVERIFY(info);
      QVERIFY(inspectDetails(dialog, *info, DetailClose::Back, [&](QDialog& details) {
        auto* alias = details.findChild<ui::TextLabel*>(QStringLiteral("officialName"));
        QVERIFY(alias);
        QCOMPARE(alias->isVisibleTo(&details), feature.field == Model::Field::Reset);
        QCOMPARE(alias->text(), Model::officialName(feature.field));
      }));
    }
  }
  void maintenanceAliasesHideDuplicateSpellingAndReappearWhenDistinct() {
    ui::I18n::instance().setLang(ui::I18n::Lang::En);
    DuplicateAliasTranslator translator;
    QVERIFY(QCoreApplication::installTranslator(&translator));
    bool installed = true;
    const auto restore = qScopeGuard([&] {
      if (installed) QCoreApplication::removeTranslator(&translator);
    });
    Fixture f;
    auto maintenance =
        std::make_shared<presentation::OverlayMaintenanceViewModel>(std::make_shared<MaintenanceTitleFacts>(), f.configuration.maintenanceEditor());
    maintenance->setInteractionContext(true, false);
    maintenance->refresh();
    QTRY_VERIFY(!maintenance->busy());
    ui::OverlayAdvancedDialog dialog(f.model(), maintenance);
    QVERIFY(expose(dialog));
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);
    for (const auto& name : {QStringLiteral("overlaySwap"), QStringLiteral("overlayHorm")}) {
      auto* alias = dialog.findChild<ui::TextLabel*>(name + QStringLiteral("Alias"));
      QVERIFY(alias);
      QVERIFY(!alias->isVisibleTo(&dialog));
    }
    QCoreApplication::removeTranslator(&translator);
    installed = false;
    QVERIFY(QMetaObject::invokeMethod(&ui::I18n::events(), "changed", Qt::DirectConnection));
    for (const auto& name : {QStringLiteral("overlaySwap"), QStringLiteral("overlayHorm")}) {
      auto* alias = dialog.findChild<ui::TextLabel*>(name + QStringLiteral("Alias"));
      QVERIFY(alias && alias->isVisibleTo(&dialog));
    }
    QVERIFY(f.configuration.pendingConfiguration().empty());
    QVERIFY(!maintenance->hasPendingChanges());
  }
  void switchingAdvancedTabsDoesNotPaintTransientScrollbars_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<bool>("dark");
    for (const auto language : {ui::I18n::Lang::Zh_CN, ui::I18n::Lang::En})
      for (const bool dark : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(language == ui::I18n::Lang::Zh_CN ? "chinese" : "english", dark ? "dark" : "light")))
            << static_cast<int>(language) << dark;
  }
  void switchingAdvancedTabsDoesNotPaintTransientScrollbars() {
    QFETCH(int, language);
    QFETCH(bool, dark);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    Fixture f;
    auto maintenance =
        std::make_shared<presentation::OverlayMaintenanceViewModel>(std::make_shared<MaintenanceTitleFacts>(), f.configuration.maintenanceEditor());
    maintenance->setInteractionContext(true, false);
    maintenance->refresh();
    QTRY_VERIFY(!maintenance->busy());
    ui::OverlayAdvancedDialog dialog(f.model(), maintenance);
    QVERIFY(expose(dialog));
    QVERIFY(test::waitForLayoutSettled(dialog));
    QCOMPARE(dialog.width(), 660);
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    auto* scroll = dialog.findChild<QScrollArea*>();
    QVERIFY(tabs && scroll && scroll->widget());
    QCOMPARE(tabs->currentIndex(), 0);
    QCOMPARE(scroll->verticalScrollBar()->maximum(), 0);

    class TabPaintProbe final : public QObject {
     public:
      TabPaintProbe(QDialog& dialog, QTabWidget& tabs, QScrollArea& scroll) : dialog(dialog), tabs(tabs), scroll(scroll) {}
      QDialog& dialog;
      QTabWidget& tabs;
      QScrollArea& scroll;
      std::array<int, 2> pagePaints{};
      int contentPaints = 0;
      QStringList violations;
      struct TextGap {
        struct Sample {
          QSize descriptionSize;
          QSize statusSize;
          int widgetGap;
          qreal firstLineDistance;
          qreal inkGap;
          bool operator==(const Sample&) const = default;
          QString description() const {
            return QStringLiteral("description=%1x%2 status=%3x%4 widgetGap=%5 firstLineDistance=%6 inkGap=%7")
                .arg(descriptionSize.width())
                .arg(descriptionSize.height())
                .arg(statusSize.width())
                .arg(statusSize.height())
                .arg(widgetGap)
                .arg(firstLineDistance)
                .arg(inkGap);
          }
        };
        QString name;
        ui::TextLabel* description = nullptr;
        ui::TextLabel* status = nullptr;
        std::vector<Sample> paints;
        Sample measure() const {
          const int distance = status->y() - description->y();
          return {description->size(), status->size(), distance - description->height(),
                  distance + status->firstLineBoxBounds().top() - description->firstLineBoxBounds().top(),
                  distance + status->firstLineInkBounds().top() - description->inkBounds().bottom()};
        }
      };
      std::array<TextGap, 3> textGaps;

      QString state(const QString& reason) const {
        const auto* content = scroll.widget();
        const auto* layout = content->layout();
        return QStringLiteral("%1: tab=%2 dialog=%3 viewport=%4 content=%5 minimum=%6 layoutMinimum=%7 range=%8 policy=%9")
            .arg(reason)
            .arg(tabs.currentIndex())
            .arg(dialog.height())
            .arg(scroll.viewport()->height())
            .arg(content->height())
            .arg(content->minimumHeight())
            .arg(layout ? layout->minimumSize().height() : -1)
            .arg(scroll.verticalScrollBar()->maximum())
            .arg(static_cast<int>(scroll.verticalScrollBarPolicy()));
      }

     protected:
      bool eventFilter(QObject* receiver, QEvent* event) override {
        if (event->type() != QEvent::Paint) return false;
        if (receiver == scroll.verticalScrollBar()) violations.append(state(QStringLiteral("vertical scrollbar painted")));
        if (receiver == scroll.widget() || receiver == scroll.viewport()) ++contentPaints;
        for (int page = 0; page < 2; ++page)
          if (receiver == tabs.widget(page)) {
            ++pagePaints[static_cast<std::size_t>(page)];
            if (scroll.verticalScrollBar()->isVisibleTo(&dialog)) violations.append(state(QStringLiteral("page painted with a vertical scrollbar")));
          }
        for (auto& gap : textGaps)
          if ((receiver == gap.description || receiver == gap.status) && tabs.currentIndex() == 1 && gap.description->isVisibleTo(&dialog) &&
              gap.status->isVisibleTo(&dialog))
            gap.paints.push_back(gap.measure());
        return false;
      }
    } probe(dialog, *tabs, *scroll);
    scroll->verticalScrollBar()->installEventFilter(&probe);
    scroll->viewport()->installEventFilter(&probe);
    scroll->widget()->installEventFilter(&probe);
    tabs->widget(0)->installEventFilter(&probe);
    tabs->widget(1)->installEventFilter(&probe);
    std::size_t gapIndex = 0;
    for (const auto& name : {QStringLiteral("overlaySwap"), QStringLiteral("overlayHorm"), QStringLiteral("overlayVolumes")}) {
      auto* status = dialog.findChild<ui::TextLabel*>(name + QStringLiteral("Status"));
      QVERIFY(status && status->parentWidget()->layout());
      auto* copy = status->parentWidget()->layout();
      QVERIFY(copy->count() > 1);
      auto* description = qobject_cast<ui::TextLabel*>(copy->itemAt(1)->widget());
      QVERIFY(description);
      auto& gap = probe.textGaps[gapIndex++];
      gap.name = name;
      gap.description = description;
      gap.status = status;
      description->installEventFilter(&probe);
      status->installEventFilter(&probe);
    }
    for (int round = 0; round < 3; ++round)
      for (int page : {1, 0}) {
        const auto pageIndex = static_cast<std::size_t>(page);
        const int previousPaints = probe.pagePaints[pageIndex];
        QTest::mouseClick(tabs->tabBar(), Qt::LeftButton, Qt::NoModifier, tabs->tabBar()->tabRect(page).center());
        QCOMPARE(tabs->currentIndex(), page);
        // Observe natural paint delivery: activating layouts or grabbing the
        // dialog here could repair an invalid intermediate frame before it paints.
        QTRY_VERIFY(probe.pagePaints[pageIndex] > previousPaints);
        QTRY_COMPARE(scroll->verticalScrollBar()->maximum(), 0);
      }
    QVERIFY(probe.contentPaints > 0);
    for (const auto& gap : probe.textGaps) {
      QVERIFY2(!gap.paints.empty(), qUtf8Printable(gap.name + QStringLiteral(": no maintenance text painted")));
      const auto& final = gap.paints.back();
      for (std::size_t paint = 0; paint < gap.paints.size(); ++paint)
        if (gap.paints[paint] != final)
          probe.violations.append(QStringLiteral("%1 paint %2 changed the description/status gap: %3; final %4")
                                      .arg(gap.name)
                                      .arg(paint)
                                      .arg(gap.paints[paint].description(), final.description()));
    }
    QVERIFY2(probe.violations.isEmpty(), qUtf8Printable(probe.violations.join(u'\n')));
  }
  void maintenanceAliasesFollowTheirTitlesAndWrapAtNarrowWidths_data() {
    QTest::addColumn<int>("language");
    QTest::addColumn<int>("width");
    QTest::addColumn<bool>("dark");
    for (const auto language : {ui::I18n::Lang::Zh_CN, ui::I18n::Lang::En, ui::I18n::Lang::Pl})
      for (const int width : {660, 360})
        for (const bool dark : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("language-%1-width-%2-%3").arg(static_cast<int>(language)).arg(width).arg(dark ? "dark" : "light")))
              << static_cast<int>(language) << width << dark;
  }
  void maintenanceAliasesFollowTheirTitlesAndWrapAtNarrowWidths() {
    QFETCH(int, language);
    QFETCH(int, width);
    QFETCH(bool, dark);
    ui::I18n::instance().setLang(static_cast<ui::I18n::Lang>(language));
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    Fixture f;
    auto facts = std::make_shared<MaintenanceTitleFacts>();
    auto maintenance = std::make_shared<presentation::OverlayMaintenanceViewModel>(facts, f.configuration.maintenanceEditor());
    maintenance->setInteractionContext(true, false);
    maintenance->refresh();
    QTRY_VERIFY(!maintenance->busy());
    ui::OverlayAdvancedDialog dialog(f.model(), maintenance);
    QVERIFY(expose(dialog));
    dialog.resize(width, dialog.height());
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);
    QTRY_COMPARE(dialog.width(), width);
    QCoreApplication::processEvents();
    auto* hint = dialog.findChild<ui::TextLabel*>(QStringLiteral("overlayAdvancedHint"));
    QVERIFY(hint && !hint->isVisibleTo(&dialog));
    QVERIFY(hint->text().isEmpty());
    for (const auto& name : {QStringLiteral("overlaySwap"), QStringLiteral("overlayHorm")}) {
      auto* title = dialog.findChild<ui::TextLabel*>(name + QStringLiteral("Name"));
      auto* alias = dialog.findChild<ui::TextLabel*>(name + QStringLiteral("Alias"));
      auto* line = dialog.findChild<QWidget*>(name + QStringLiteral("TitleLine"));
      QVERIFY(title && alias && line);
      revealWidget(*line);
      QVERIFY(title->isVisibleTo(&dialog) && alias->isVisibleTo(&dialog));
      QCOMPARE(title->parentWidget(), line);
      QCOMPARE(alias->parentWidget(), line);
      QVERIFY(alias->font().pixelSize() < title->font().pixelSize());
      const auto titleBounds = QRect(title->mapTo(line, QPoint()), title->size());
      const auto aliasBounds = QRect(alias->mapTo(line, QPoint()), alias->size());
      if (width == 660) {
        QVERIFY(aliasBounds.left() > titleBounds.right());
        QVERIFY(test::centersAligned(aliasBounds, titleBounds));
        const auto aliasFrame = dialog.grab().toImage();
        const auto titleInk = paintedInk(aliasFrame, *title, *line);
        const auto aliasInk = paintedInk(aliasFrame, *alias, *line);
        QVERIFY(!titleInk.isEmpty() && !aliasInk.isEmpty());
        QVERIFY2(rasterCentersAligned(aliasInk, titleInk, dialog.devicePixelRatioF()),
                 qUtf8Printable(QStringLiteral("%1 glyph centers: title=%2 alias=%3 titleHeight=%4")
                                    .arg(name)
                                    .arg(titleInk.center().y())
                                    .arg(aliasInk.center().y())
                                    .arg(titleInk.height())));
      } else {
        // A short alias may fit beside its translated title even at compact
        // widths. The flow must keep it readable after or below the title.
        QVERIFY(!aliasBounds.intersects(titleBounds));
        if (aliasBounds.top() <= titleBounds.bottom()) {
          QVERIFY(aliasBounds.left() > titleBounds.right());
          QVERIFY(test::centersAligned(aliasBounds, titleBounds));
        } else {
          QVERIFY(aliasBounds.top() > titleBounds.bottom());
        }
      }
      QVERIFY(aliasBounds.right() < line->width());
      QCOMPARE(alias->text(), name == QStringLiteral("overlaySwap") ? QStringLiteral("create swapfile") : QStringLiteral("HORM"));
    }
    for (const auto& name : {QStringLiteral("overlaySwap"), QStringLiteral("overlayHorm"), QStringLiteral("overlayVolumes")}) {
      auto* title = dialog.findChild<ui::TextLabel*>(name + QStringLiteral("Name"));
      auto* icon = dialog.findChild<QWidget*>(name + QStringLiteral("Icon"));
      QVERIFY(title && icon);
      revealWidget(*title);
      revealWidget(*icon);
      const auto maintenanceFrame = dialog.grab().toImage();
      const auto firstLine = paintedInk(maintenanceFrame, *title, dialog, true);
      const auto iconInk = paintedInk(maintenanceFrame, *icon, dialog);
      QVERIFY2(rasterCentersAligned(iconInk, firstLine, dialog.devicePixelRatioF()),
               qUtf8Printable(QStringLiteral("%1: title center=%2 icon center=%3").arg(name).arg(firstLine.center().y()).arg(iconInk.center().y())));
    }
    const auto clipping = test::waitForUnclippedText(dialog);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    for (auto scrollers = dialog.findChildren<QScrollArea*>(); auto* scroll : std::as_const(scrollers)) QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    const auto output = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
    if (!output.isEmpty()) {
      QVERIFY(QDir().mkpath(output));
      QVERIFY(dialog.grab().save(QDir(output).filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral("-maintenance.png"))));
      tabs->setCurrentIndex(0);
      QCoreApplication::processEvents();
      QVERIFY(dialog.grab().save(QDir(output).filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral("-configuration.png"))));
    }
    tabs->setCurrentIndex(1);
    facts->failObservation = true;
    maintenance->refresh();
    QTRY_VERIFY(!maintenance->busy());
    auto* issue = dialog.findChild<ui::Notice*>(QStringLiteral("overlayMaintenanceIssue"));
    QVERIFY(issue && issue->isVisibleTo(&dialog));
    revealWidget(*issue);
    ui::IconView* warning = nullptr;
    for (auto widgets = issue->findChildren<QWidget*>(); auto* widget : std::as_const(widgets))
      if (auto* icon = dynamic_cast<ui::IconView*>(widget)) warning = icon;
    ui::TextLabel* body = nullptr;
    for (auto labels = issue->findChildren<ui::TextLabel*>(); auto* label : std::as_const(labels))
      if (label->isVisibleTo(issue) && !label->text().isEmpty()) body = label;
    QVERIFY(warning && body);
    const auto warningFrame = dialog.grab().toImage();
    QVERIFY(rasterCentersAligned(paintedInk(warningFrame, *warning, dialog), paintedInk(warningFrame, *body, dialog, true), dialog.devicePixelRatioF()));
    QVERIFY(warning->parentWidget()->rect().contains(warning->geometry()));
    if (!output.isEmpty())
      QVERIFY(dialog.grab().save(QDir(output).filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral("-maintenance-warning.png"))));
  }
};
}  // namespace
QTEST_MAIN(OverlayAdvancedTests)
#include "OverlayAdvancedTests.moc"
