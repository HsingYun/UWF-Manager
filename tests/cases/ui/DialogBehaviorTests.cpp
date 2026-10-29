// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QAbstractScrollArea>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFocusFrame>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QRegion>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTableView>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QThreadPool>
#include <QTimer>
#include <QTreeView>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "app/commit/CommitBatch.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PageProxyModel.h"
#include "presentation/common/PresentationTiming.h"
#include "support/app/WorkerGate.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/commit/CommitReportDialog.h"
#include "ui/diagnostics/LogViewerDialog.h"
#include "ui/design/DesignSystem.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/dialogs/PanelDialog.h"
#include "ui/overlay/OverlayFilesDialog.h"
#include "ui/registry/RegistryPickerDialog.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/ContentState.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/PagedViewController.h"
#include "ui/widgets/PagedWindowLayout.h"
#include "ui/widgets/RecordInspector.h"
#include "ui/widgets/TextLabel.h"
#include "ui/widgets/VerticalScrollArea.h"
#include "windows/filesystem/AtomicTextFileWriter.h"

namespace {

using uwf::ui::RegistryPickerDialog;

using uwf::test::buttonByRole;
using uwf::test::FileDialogStub;

class MemoryRegistryBrowser final : public uwf::app::RegistryBrowser {
 public:
  std::vector<std::string> roots{"HKEY_LOCAL_MACHINE", "HKEY_CURRENT_USER", "HKEY_CLASSES_ROOT", "HKEY_USERS", "HKEY_CURRENT_CONFIG"};
  std::map<std::string, std::vector<std::string>, std::less<>> children;
  std::map<std::string, std::vector<uwf::app::RegistryValueInfo>, std::less<>> keyValues;
  bool failSubkeyEnumeration = false;
  bool failValues = false;

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

  [[nodiscard]] std::vector<uwf::app::RegistryValueInfo> values(const std::string_view key) const override {
    if (failValues) throw std::runtime_error("registry values unavailable: \xE6\x8B\x92\xE7\xBB\x9D\xE8\xAE\xBF\xE9\x97\xAE");
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

QString labelContaining(const QWidget* parent, const QString& needle) {
  for (const auto labels = parent->findChildren<QLabel*>(); const auto* label : labels) {
    if (label->text().contains(needle, Qt::CaseInsensitive)) return label->text();
  }
  for (const auto texts = parent->findChildren<uwf::ui::ContentHintBox*>(); const auto* text : texts) {
    if (text->toPlainText().contains(needle, Qt::CaseInsensitive)) return text->toPlainText();
  }
  return {};
}

QList<QAbstractScrollArea*> scrollingAncestors(QWidget& content, QWidget& root) {
  QList<QAbstractScrollArea*> owners;
  for (QWidget* ancestor = &content; ancestor && ancestor != &root; ancestor = ancestor->parentWidget())
    if (auto* area = qobject_cast<QAbstractScrollArea*>(ancestor); area && area->verticalScrollBar()->maximum() > 0) owners.append(area);
  return owners;
}

bool textDocumentFits(const uwf::ui::ContentHintBox& text) {
  return text.viewport()->height() >= static_cast<int>(std::ceil(text.document()->documentLayout()->documentSize().height()));
}

bool pagedViewportShowsWholeRows(const QAbstractItemView& view, const int rowHeight) {
  const auto* pages = qobject_cast<const uwf::presentation::PageProxyModel*>(view.model());
  if (!pages || rowHeight <= 0) return false;
  const auto* viewport = view.viewport();
  const int rows = viewport->height() / rowHeight;
  if (rows < 1 || viewport->height() % rowHeight != 0 || pages->pager().pageSize != rows || !viewport->mask().isEmpty()) return false;
  if (pages->rowCount() == 0) return true;
  const auto first = view.indexAt(QPoint(viewport->width() / 2, 0));
  if (!first.isValid() || view.visualRect(first).top() != 0) return false;
  const int lastRow = std::min(pages->rowCount() - 1, first.row() + rows - 1);
  const auto last = view.visualRect(pages->index(lastRow, first.column()));
  return last.height() == rowHeight && last.bottom() < viewport->height() &&
         (view.verticalScrollBar()->value() != view.verticalScrollBar()->maximum() || lastRow == pages->rowCount() - 1);
}

bool textPositionVisible(uwf::ui::ContentHintBox& text, QWidget& root, const qsizetype position) {
  if (position < 0) return false;
  QTextCursor cursor(text.document());
  cursor.setPosition(static_cast<int>(position));
  auto target = text.cursorRect(cursor);
  target.setWidth(1);
  for (QWidget* ancestor = text.viewport(); ancestor; ancestor = ancestor->parentWidget()) {
    if (!ancestor->rect().contains(QRect(text.viewport()->mapTo(ancestor, target.topLeft()), target.size()))) return false;
    if (ancestor == &root) return true;
  }
  return false;
}

bool exposeTextPosition(uwf::ui::ContentHintBox& text, QAbstractScrollArea& owner, QWidget& root, const qsizetype position) {
  if (position < 0) return false;
  QTextCursor cursor(text.document());
  cursor.setPosition(static_cast<int>(position));
  const auto inOwner = text.viewport()->mapTo(owner.viewport(), text.cursorRect(cursor).center());
  auto* bar = owner.verticalScrollBar();
  bar->setValue(bar->value() + inOwner.y() - owner.viewport()->height() / 2);
  return textPositionVisible(text, root, position);
}

QModelIndex findTreeItem(const QAbstractItemModel* model, const QModelIndex& parent, const QString& text) {
  for (int row = 0; row < model->rowCount(parent); ++row) {
    const auto index = model->index(row, 0, parent);
    if (index.data().toString() == text) return index;
    const auto found = findTreeItem(model, index, text);
    if (found.isValid()) return found;
  }
  return {};
}
QModelIndex findTreeItem(QTreeView* tree, const QString& text) { return findTreeItem(tree->model(), {}, text); }
QModelIndex findTreeItem(const QModelIndex& parent, const QString& text) { return findTreeItem(parent.model(), parent, text); }

enum class RejectionEntry { Title, Cancel, Close, Escape, WindowClose, Reject, Done };

const std::pair<const char*, RejectionEntry> rejectionEntries[] = {{"title", RejectionEntry::Title},
                                                                   {"cancel", RejectionEntry::Cancel},
                                                                   {"close", RejectionEntry::Close},
                                                                   {"escape", RejectionEntry::Escape},
                                                                   {"window-close", RejectionEntry::WindowClose},
                                                                   {"reject", RejectionEntry::Reject},
                                                                   {"done-rejected", RejectionEntry::Done}};

const char* const rejectionStates[] = {"enabled", "disabled", "re-enabled"};

class RejectionProbeDialog final : public uwf::ui::PanelDialog {
 public:
  RejectionProbeDialog() {
    setWindowTitle(QStringLiteral("Rejection policy"));
    auto* frame = new uwf::ui::DialogLayout(this, uwf::ui::DialogContent::Form, QSize(560, 240));
    input = new QLineEdit(this);
    frame->body()->addWidget(input);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Close, this);
    cancel = buttons->button(QDialogButtonBox::Cancel);
    footerClose = buttons->button(QDialogButtonBox::Close);
    acceptButton = buttons->button(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    frame->setActions(buttons);
  }

  QLineEdit* input = nullptr;
  QPushButton* cancel = nullptr;
  QPushButton* footerClose = nullptr;
  QPushButton* acceptButton = nullptr;
  int rejectionRequests = 0;
  bool completeRejection = true;

 protected:
  bool requestRejection() override {
    ++rejectionRequests;
    return completeRejection;
  }
};

void setRejectionState(uwf::ui::DialogWindow& dialog, const int state) {
  if (state != 0) dialog.setRejectionEnabled(false);
  if (state == 2) dialog.setRejectionEnabled(true);
}

void requestRejection(RejectionProbeDialog& dialog, const RejectionEntry entry) {
  switch (entry) {
    case RejectionEntry::Title:
      QTest::mouseClick(dialog.closeButton(), Qt::LeftButton);
      break;
    case RejectionEntry::Cancel:
      QTest::mouseClick(dialog.cancel, Qt::LeftButton);
      break;
    case RejectionEntry::Close:
      QTest::mouseClick(dialog.footerClose, Qt::LeftButton);
      break;
    case RejectionEntry::Escape:
      QTest::keyClick(dialog.input, Qt::Key_Escape);
      break;
    case RejectionEntry::WindowClose:
      dialog.close();
      break;
    case RejectionEntry::Reject:
      static_cast<QDialog&>(dialog).reject();
      break;
    case RejectionEntry::Done:
      static_cast<QDialog&>(dialog).done(QDialog::Rejected);
      break;
  }
}

class DialogBehaviorTests final : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase() {
    uwf::test::initializeVisualComparison();
    uwf::ui::I18n::instance().setLang(uwf::ui::I18n::Lang::En);
  }
  void rejectionPolicyCoversEveryCloseEntry_data();
  void contentDetailsShrinkAndRegrowWithOneScrollOwner_data();
  void contentDetailsShrinkAndRegrowWithOneScrollOwner();
  void rejectionPolicyCoversEveryCloseEntry();
  void rejectionPolicyAllowsBusinessCompletion_data();
  void rejectionPolicyAllowsBusinessCompletion();
  void rejectionHookCanDeferClosure_data();
  void rejectionHookCanDeferClosure();
  void rejectionPolicyAppliesWhenActionsAreRegistered();
  void overlayFilesNormalizesMergesAndClassifiesMockRows();
  void overlayFilesSizeAggregationSaturatesWithoutWrapping();
  void overlayFilesPaginatesWithoutReloadingTheSource();
  void recordSelectionKeepsPagedGeometry_data();
  void recordSelectionKeepsPagedGeometry();
  void overlayFilesExportWritesTheCompleteSnapshotAndHonorsCancellation();
  void overlayFilesFailureKeepsActionsUnavailable();
  void overlayFilesLongErrorUsesOneReachableScrollOwner_data();
  void overlayFilesLongErrorUsesOneReachableScrollOwner();
  void overlayFilesDestructionCancelsWithoutWaitingForTheActiveLoader();
  void overlayFilesRejectsMissingLoader();
  void overlayFilesContextMenuCommitsOnlyOrdinaryEntries();
  void overlayMessagesPreserveScopeAndConfirmationActions_data();
  void overlayMessagesPreserveScopeAndConfirmationActions();
  void registryPickerBrowsesSnapshotAndFiltersPersistentHives();
  void registryPickerFailedEnumerationCanBeRetriedWithoutPartialChildren();
  void registryPickerAccessFailuresStayInTheDetailsPane_data();
  void registryPickerAccessFailuresStayInTheDetailsPane();
  void registryValuePreviewCoversNumericStringMultiStringAndBinaryBoundaries();
  void registryPickerReturnsNamedValueAndWholeKeyIntents();
  void registryPickerCancellationClearsSharedSelection_data();
  void registryPickerCancellationClearsSharedSelection();
  void registryAddressBarNavigatesShorthandAndClearResetsSelection();
  void pastingARegistryAddressDoesNotEnableTheNavigationFocusRing_data();
  void pastingARegistryAddressDoesNotEnableTheNavigationFocusRing();
  void registryAddressKeyboardFocusOutlineIsFullyVisible_data();
  void registryAddressKeyboardFocusOutlineIsFullyVisible();
  void registryExclusionDisablesValueSelection();
  void commitReportPaginatesAndCopiesTheCompleteDeleteReport();
  void commitReportKeepsPersistColumnsAndEmptyStateConsistent();
  void commitReportDetailsGrowForWrappedTextAndPreserveCopying();
  void dataDialogsKeepCompactGeometryAndReadableColumns_data();
  void dataDialogsKeepCompactGeometryAndReadableColumns();
};

void DialogBehaviorTests::contentDetailsShrinkAndRegrowWithOneScrollOwner_data() {
  QTest::addColumn<bool>("selfManaged");
  QTest::addColumn<bool>("rtl");
  for (const bool selfManaged : {false, true})
    for (const bool rtl : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(selfManaged ? "self" : "host", rtl ? "rtl" : "ltr"))) << selfManaged << rtl;
}
void DialogBehaviorTests::contentDetailsShrinkAndRegrowWithOneScrollOwner() {
  QFETCH(bool, selfManaged);
  QFETCH(bool, rtl);
  const auto previousClipboard = QApplication::clipboard()->text();
  const auto restoreClipboard = qScopeGuard([&] { QApplication::clipboard()->setText(previousClipboard); });
  QWidget host;
  host.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
  auto* layout = new QVBoxLayout(&host);
  layout->setContentsMargins(0, 0, 0, 0);
  auto* state = new uwf::ui::ContentState(nullptr, selfManaged ? uwf::ui::ContentState::ScrollMode::SelfManaged
                                                           : uwf::ui::ContentState::ScrollMode::HostManaged);
  state->setText(QStringLiteral("Unable to read these files"), QStringLiteral("Inspect the complete error and retry after fixing access permissions."));
  state->setAction(QStringLiteral("Retry"));
  auto* details = new uwf::ui::ContentHintBox(state);
  const auto original = QStringLiteral("C:\\Managed application data\\<literal & complete>\\Protected directory\n") +
                        QStringLiteral("多行原始错误必须完整显示并可复制。 The complete original diagnostic must wrap and remain reachable.\n").repeated(8);
  details->setPlainText(original);
  state->setDetails(details);
  QScrollArea* owner = nullptr;
  if (selfManaged) {
    layout->addWidget(state);
    owner = state->findChild<QScrollArea*>();
  } else {
    auto* scroll = new uwf::ui::VerticalScrollArea(&host);
    scroll->setWidget(state);
    layout->addWidget(scroll);
    owner = scroll;
  }
  QVERIFY(owner);
  host.resize(700, 260);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  for (const int width : {700, 360, 700, 360}) {
    host.resize(width, 260);
    QTRY_COMPARE(host.width(), width);
    QTRY_VERIFY(details->height() >= details->heightForWidth(details->width()));
    QTRY_VERIFY(owner->widget()->width() <= owner->viewport()->width());
    QVERIFY(details->minimumWidth() < 360);
    QVERIFY(details->width() <= owner->viewport()->width());
    if (width == 700) QVERIFY(details->width() > 350);
    QCOMPARE(owner->horizontalScrollBar()->maximum(), 0);
    QCOMPARE(details->horizontalScrollBar()->maximum(), 0);
    QCOMPARE(details->verticalScrollBar()->maximum(), 0);
    QTRY_VERIFY(owner->verticalScrollBar()->maximum() > 0);
    int scrollingAncestors = 0;
    for (auto* ancestor = details->parentWidget(); ancestor; ancestor = ancestor->parentWidget())
      if (auto* scroll = qobject_cast<QAbstractScrollArea*>(ancestor); scroll && scroll->verticalScrollBar()->maximum() > 0) ++scrollingAncestors;
    QCOMPARE(scrollingAncestors, 1);
    owner->verticalScrollBar()->setValue(owner->verticalScrollBar()->maximum());
    const auto bottom = owner->viewport()->mapFromGlobal(details->mapToGlobal(details->rect().bottomRight()));
    QVERIFY(bottom.y() < owner->viewport()->height());
    QVERIFY(bottom.y() >= 0);
    details->setFocus();
    details->selectAll();
    QTest::keyClick(details, Qt::Key_C, Qt::ControlModifier);
    QCOMPARE(QApplication::clipboard()->text(), original);
  }
}

void DialogBehaviorTests::rejectionPolicyCoversEveryCloseEntry_data() {
  QTest::addColumn<int>("state");
  QTest::addColumn<int>("entry");
  for (int state = 0; state < 3; ++state)
    for (const auto& [name, entry] : rejectionEntries)
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QLatin1String(rejectionStates[state]), QLatin1String(name))))
          << state << static_cast<int>(entry);
}

void DialogBehaviorTests::rejectionPolicyCoversEveryCloseEntry() {
  QFETCH(int, state);
  QFETCH(int, entry);
  RejectionProbeDialog dialog;
  setRejectionState(dialog, state);
  const bool enabled = state != 1;
  QCOMPARE(dialog.rejectionEnabled(), enabled);
  QCOMPARE(dialog.closeButton()->isEnabled(), enabled);
  QCOMPARE(dialog.cancel->isEnabled(), enabled);
  QCOMPARE(dialog.footerClose->isEnabled(), enabled);
  QVERIFY(dialog.acceptButton->isEnabled());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy finished(&dialog, &QDialog::finished);
  QSignalSpy rejected(&dialog, &QDialog::rejected);

  requestRejection(dialog, static_cast<RejectionEntry>(entry));

  QCOMPARE(dialog.rejectionRequests, enabled ? 1 : 0);
  QCOMPARE(dialog.isVisible(), !enabled);
  QCOMPARE(finished.count(), enabled ? 1 : 0);
  QCOMPARE(rejected.count(), enabled ? 1 : 0);
  if (enabled) QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
}

void DialogBehaviorTests::rejectionPolicyAllowsBusinessCompletion_data() {
  QTest::addColumn<int>("state");
  QTest::addColumn<int>("entry");
  const char* const entries[] = {"accept-button", "accept", "done-accepted"};
  for (int state = 0; state < 3; ++state)
    for (int entry = 0; entry < 3; ++entry)
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QLatin1String(rejectionStates[state]), QLatin1String(entries[entry])))) << state << entry;
}

void DialogBehaviorTests::rejectionPolicyAllowsBusinessCompletion() {
  QFETCH(int, state);
  QFETCH(int, entry);
  RejectionProbeDialog dialog;
  dialog.completeRejection = false;
  setRejectionState(dialog, state);
  QVERIFY(dialog.acceptButton->isEnabled());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy finished(&dialog, &QDialog::finished);
  QSignalSpy accepted(&dialog, &QDialog::accepted);
  QSignalSpy rejected(&dialog, &QDialog::rejected);

  if (entry == 0)
    QTest::mouseClick(dialog.acceptButton, Qt::LeftButton);
  else if (entry == 1)
    static_cast<QDialog&>(dialog).accept();
  else
    static_cast<QDialog&>(dialog).done(QDialog::Accepted);

  QCOMPARE(dialog.rejectionRequests, 0);
  QVERIFY(!dialog.isVisible());
  QCOMPARE(dialog.result(), static_cast<int>(QDialog::Accepted));
  QCOMPARE(finished.count(), 1);
  QCOMPARE(accepted.count(), 1);
  QCOMPARE(rejected.count(), 0);
}

void DialogBehaviorTests::rejectionHookCanDeferClosure_data() {
  QTest::addColumn<int>("entry");
  for (const auto& [name, entry] : rejectionEntries) QTest::newRow(name) << static_cast<int>(entry);
}

void DialogBehaviorTests::rejectionHookCanDeferClosure() {
  QFETCH(int, entry);
  RejectionProbeDialog dialog;
  dialog.completeRejection = false;
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy finished(&dialog, &QDialog::finished);
  QSignalSpy rejected(&dialog, &QDialog::rejected);

  requestRejection(dialog, static_cast<RejectionEntry>(entry));

  QCOMPARE(dialog.rejectionRequests, 1);
  QVERIFY(dialog.isVisible());
  QCOMPARE(finished.count(), 0);
  QCOMPARE(rejected.count(), 0);
  dialog.completeRejection = true;
  requestRejection(dialog, static_cast<RejectionEntry>(entry));
  QCOMPARE(dialog.rejectionRequests, 2);
  QVERIFY(!dialog.isVisible());
  QCOMPARE(finished.count(), 1);
  QCOMPARE(rejected.count(), 1);
}

void DialogBehaviorTests::rejectionPolicyAppliesWhenActionsAreRegistered() {
  uwf::ui::PanelDialog dialog;
  dialog.setRejectionEnabled(false);
  auto* frame = new uwf::ui::DialogLayout(&dialog, uwf::ui::DialogContent::Form, QSize(560, 240));
  frame->body()->addWidget(new QLabel(QStringLiteral("Actions follow the current rejection policy."), &dialog));
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  auto* auxiliary = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  frame->setActions(buttons, auxiliary);
  QVERIFY(!dialog.closeButton()->isEnabled());
  QVERIFY(!buttons->button(QDialogButtonBox::Cancel)->isEnabled());
  QVERIFY(!auxiliary->button(QDialogButtonBox::Close)->isEnabled());
  QVERIFY(buttons->button(QDialogButtonBox::Ok)->isEnabled());

  dialog.setRejectionEnabled(true);

  QVERIFY(dialog.closeButton()->isEnabled());
  QVERIFY(buttons->button(QDialogButtonBox::Cancel)->isEnabled());
  QVERIFY(auxiliary->button(QDialogButtonBox::Close)->isEnabled());
  QVERIFY(buttons->button(QDialogButtonBox::Ok)->isEnabled());
}

void DialogBehaviorTests::overlayFilesNormalizesMergesAndClassifiesMockRows() {
  std::atomic<int> loads{0};
  QString observedDriveLetter;
  uwf::ui::OverlayFilesDialog dialog(std::make_shared<uwf::presentation::OverlayFilesViewModel>(
                                         QStringLiteral("c"),
                                         [&loads, &observedDriveLetter](const QString& driveLetter, std::stop_token) {
                                           observedDriveLetter = driveLetter;
                                           ++loads;
                                           return QVector<uwf::app::OverlayFileEntry>{
                                               {QStringLiteral("\\Users\\Alice\\note.txt:$DATA"), {}, false, false, 100},
                                               {QStringLiteral("\\Users\\Alice\\note.txt:history:$DATA"), {}, false, false, 50},
                                               {QStringLiteral("\\Folder:$INDEX_ALLOCATION"), {}, false, false, 4096},
                                               {QStringLiteral("\\$Extend\\$Deleted\\42:$DATA"), {}, false, false, 32},
                                           };
                                         },
                                         uwf::windows::writeUtf8TextFile),
                                     uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* list = dialog.findChild<QTableView*>(QStringLiteral("overlayFilesTable"));
  QVERIFY(list);
  QTRY_COMPARE_WITH_TIMEOUT(loads.load(), 1, 2500);
  QCOMPARE(observedDriveLetter, QStringLiteral("C:"));
  QTRY_COMPARE_WITH_TIMEOUT(list->model()->rowCount(), 3, 2500);
  QTRY_VERIFY_WITH_TIMEOUT(labelContaining(&dialog, QStringLiteral("3 items")).size() > 0, 2500);

  QMap<QString, QModelIndex> byPath;
  for (int row = 0; row < list->model()->rowCount(); ++row)
    byPath.insert(list->model()->index(row, 0).data(Qt::UserRole).toString(), list->model()->index(row, 0));
  QVERIFY(byPath.contains(QStringLiteral("C:\\Users\\Alice\\note.txt")));
  QCOMPARE(byPath.value(QStringLiteral("C:\\Users\\Alice\\note.txt")).siblingAtColumn(2).data(uwf::presentation::FullTextRole).toString(),
           QStringLiteral("150 B"));
  QVERIFY(byPath.value(QStringLiteral("C:\\Folder")).data(Qt::UserRole + 1).toBool());
  QVERIFY(byPath.value(QStringLiteral("C:\\$Extend\\$Deleted\\42")).data(Qt::UserRole + 2).toBool());
  auto* exportButton = buttonByText(&dialog, QStringLiteral("Export list"));
  QVERIFY(exportButton);
  QVERIFY(exportButton->isEnabled());
}

void DialogBehaviorTests::overlayFilesSizeAggregationSaturatesWithoutWrapping() {
  FileDialogStub files;
  uwf::ui::OverlayFilesDialog dialog(std::make_shared<uwf::presentation::OverlayFilesViewModel>(
                                         QStringLiteral("C:"),
                                         [](const QString&, std::stop_token) {
                                           return QVector<uwf::app::OverlayFileEntry>{
                                               {QStringLiteral("\\Data\\huge.bin:$DATA"), {}, false, false, std::numeric_limits<qulonglong>::max()},
                                               {QStringLiteral("\\Data\\huge.bin:extra:$DATA"), {}, false, false, 1},
                                               {QStringLiteral("\\Data\\other.bin:$DATA"), {}, false, false, std::numeric_limits<qulonglong>::max()},
                                           };
                                         },
                                         uwf::windows::writeUtf8TextFile),
                                     files);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* list = dialog.findChild<QTableView*>(QStringLiteral("overlayFilesTable"));
  auto* exportButton = buttonByText(&dialog, QStringLiteral("Export list"));
  QVERIFY(list);
  QVERIFY(exportButton);
  QTRY_COMPARE_WITH_TIMEOUT(list->model()->rowCount(), 2, 2500);
  for (int row = 0; row < list->model()->rowCount(); ++row) QVERIFY(!list->model()->index(row, 0).data().toString().contains(QStringLiteral("0 B")));

  QTemporaryDir outputDirectory;
  QVERIFY(outputDirectory.isValid());
  files.savedFile = outputDirectory.filePath(QStringLiteral("saturated.txt"));
  QTimer::singleShot(0, this, [] {
    if (auto* information = qobject_cast<QDialog*>(QApplication::activeModalWidget())) information->accept();
  });
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QFile written(files.savedFile);
  QVERIFY(written.open(QIODevice::ReadOnly | QIODevice::Text));
  const QString exported = QString::fromUtf8(written.readAll());
  QCOMPARE(exported.count(QStringLiteral("\t18446744073709551615\t")), 2);
  QVERIFY(!exported.contains(QStringLiteral("\t0\t0 B\t")));
}

void DialogBehaviorTests::recordSelectionKeepsPagedGeometry_data() {
  QTest::addColumn<bool>("logs");
  QTest::addColumn<uwf::ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const bool logs : {false, true})
    for (const auto locale : {uwf::ui::I18n::Lang::En, uwf::ui::I18n::Lang::Zh_CN, uwf::ui::I18n::Lang::Ar})
      for (const bool dark : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(logs ? "logs" : "overlay").arg(static_cast<int>(locale)).arg(dark ? "dark" : "light")))
            << logs << locale << dark;
}

void DialogBehaviorTests::recordSelectionKeepsPagedGeometry() {
  QFETCH(bool, logs);
  QFETCH(uwf::ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  auto& language = uwf::ui::I18n::instance();
  auto& theme = uwf::ui::ThemeManager::instance();
  const auto originalLanguage = language.lang();
  const auto originalTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(originalLanguage);
    theme.apply(originalTheme);
  });
  language.setLang(locale);
  theme.apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  std::unique_ptr<QDialog> dialog;
  if (logs) {
    class Records final : public uwf::app::LogSource {
     public:
      std::vector<std::string> lines;
      std::vector<std::string> recentLines() const override { return lines; }
      void clear() override { lines.clear(); }
    };
    auto source = std::make_shared<Records>();
    source->lines = {"[01:02:03.456 I module] selection-short",
                     "[01:02:03.456 I module] selection-long " + QStringLiteral("long diagnostic text 中文\n").repeated(100).toStdString(),
                     "[01:02:03.456 I " + std::string(1000, 'm') + "] selection-module", "[01:02:03.456 I module] selection-last"};
    for (int i = 0; i < 80; ++i) source->lines.push_back("[01:02:03.456 I module] filler " + std::to_string(i));
    dialog = std::make_unique<uwf::ui::LogViewerDialog>(std::make_shared<uwf::presentation::LogViewModel>(source));
  } else {
    dialog = std::make_unique<uwf::ui::OverlayFilesDialog>(
        std::make_shared<uwf::presentation::OverlayFilesViewModel>(
            QStringLiteral("C:"),
            [](const QString&, std::stop_token) {
              QVector<uwf::app::OverlayFileEntry> entries{
                  {QStringLiteral("\\selection-short.txt:$DATA"), {}, false, false, 1},
                  {QStringLiteral("\\selection-long\\") + QStringLiteral("deep-directory-中文\\").repeated(60) + QStringLiteral("file.txt:$DATA"),
                   {},
                   false,
                   false,
                   1},
                  {QStringLiteral("\\selection-folder:$INDEX_ALLOCATION"), {}, false, false, 1},
                  {QStringLiteral("\\$Extend\\$Deleted\\42:$DATA"), {}, false, false, 1}};
              for (int i = 0; i < 80; ++i) entries.append({QStringLiteral("\\filler-%1.txt:$DATA").arg(i), {}, false, false, 1});
              return entries;
            }),
        uwf::test::localFileDialogs());
  }
  dialog->show();
  QVERIFY(QTest::qWaitForWindowExposed(dialog.get()));
  auto* table = dialog->findChild<QTableView*>(logs ? QStringLiteral("logTable") : QStringLiteral("overlayFilesTable"));
  auto* details = dialog->findChild<uwf::ui::ContentHintBox*>(logs ? QStringLiteral("logMessageDetails") : QStringLiteral("overlaySelectedPath"));
  QVERIFY(table && details);
  QTRY_COMPARE(table->model()->rowCount(), 84);
  QTRY_VERIFY(details->isVisible());
  QWidget* inspector = details;
  while (inspector && !dynamic_cast<uwf::ui::RecordInspector*>(inspector)) inspector = inspector->parentWidget();
  auto* summary = dialog->findChild<QWidget*>(QStringLiteral("paginationSummary"));
  auto* pages = qobject_cast<uwf::presentation::PageProxyModel*>(table->model());
  QVERIFY(inspector && summary && pages);
  auto* pagination = summary->parentWidget();
  QList<int> rows;
  int shortRow = -1;
  int longRow = -1;
  for (int row = 0; row < table->model()->rowCount(); ++row) {
    const auto index = table->model()->index(row, logs ? 3 : 0);
    const auto text = index.data(logs ? uwf::presentation::FullTextRole : uwf::presentation::PathRole).toString();
    if (text.contains(QStringLiteral("selection-")) || (!logs && index.data(uwf::presentation::SystemMetadataRole).toBool())) rows.append(row);
    if (text.contains(QStringLiteral("selection-short"))) shortRow = row;
    if (text.contains(QStringLiteral("selection-long"))) longRow = row;
  }
  QCOMPARE(rows.size(), 4);
  QVERIFY(shortRow >= 0 && longRow >= 0);
  const auto selectRecord = [&](const int row) {
    const auto owners = scrollingAncestors(*details, *dialog);
    for (auto* owner : owners) owner->verticalScrollBar()->setValue(owner->verticalScrollBar()->minimum());
    const auto index = table->model()->index(row, logs ? 3 : 0);
    table->scrollTo(index);
    QTRY_VERIFY(table->viewport()->rect().contains(table->visualRect(index).center()));
    QTest::mouseClick(table->viewport(), Qt::LeftButton, Qt::NoModifier, table->visualRect(index).center());
    QTRY_COMPARE(table->currentIndex().row(), row);
    QTRY_COMPARE(details->toPlainText(), index.data(logs ? uwf::presentation::FullTextRole : uwf::presentation::PathRole).toString());
    QTRY_COMPARE(details->verticalScrollBar()->maximum(), 0);
    QTRY_COMPARE(details->horizontalScrollBar()->maximum(), 0);
    QTRY_VERIFY(textDocumentFits(*details));
    QTRY_COMPARE(details->height(), std::max(details->minimumHeight(), details->heightForWidth(details->width())));
  };
  const auto gapAfterTable = [&] { return inspector->mapTo(dialog.get(), QPoint()).y() - table->mapTo(dialog.get(), QPoint(0, table->height())).y(); };
  const auto gapAfterDetails = [&] {
    return pagination->mapTo(dialog.get(), QPoint()).y() - inspector->mapTo(dialog.get(), QPoint(0, inspector->height())).y();
  };
  for (const QSize size : {QSize(1096, 754), QSize(760, 754), QSize(720, 500)}) {
    dialog->resize(size);
    selectRecord(shortRow);
    // Consume the requested outer resize before recording its resulting
    // geometry. Wrapped content can reflow afterwards; the visible row mask
    // and page capacity, rather than physical viewport padding, are the contract.
    auto& windowLayout = uwf::ui::PagedWindowLayout::forWindow(dialog.get());
    windowLayout.fit();
    QTRY_VERIFY(pagedViewportShowsWholeRows(*table, table->verticalHeader()->defaultSectionSize()));
    QTRY_VERIFY(textDocumentFits(*details));
    QTRY_COMPARE(details->height(), std::max(details->minimumHeight(), details->heightForWidth(details->width())));
    const auto windowGeometry = dialog->geometry();
    const int shortDetailsHeight = details->height();
    const int rowsWithShortDetails = table->viewport()->height() / table->verticalHeader()->defaultSectionSize();
    const auto selectionCycle = rows + rows;
    for (const int row : selectionCycle) {
      selectRecord(row);
      QVERIFY2(dialog->geometry() == windowGeometry, qUtf8Printable(QStringLiteral("Selecting row %1 (%2) changed the window from %3x%4 to %5x%6")
                                                                        .arg(row)
                                                                        .arg(details->toPlainText().left(100))
                                                                        .arg(windowGeometry.width())
                                                                        .arg(windowGeometry.height())
                                                                        .arg(dialog->width())
                                                                        .arg(dialog->height())));
      QTRY_VERIFY(pagedViewportShowsWholeRows(*table, table->verticalHeader()->defaultSectionSize()));
      QVERIFY(gapAfterTable() > 0 && gapAfterDetails() > 0);
      const auto owners = scrollingAncestors(*details, *dialog);
      QVERIFY(owners.size() <= 1);
      if (row == longRow) {
        QTRY_VERIFY(details->height() > shortDetailsHeight);
        QTRY_VERIFY(table->viewport()->height() / table->verticalHeader()->defaultSectionSize() < rowsWithShortDetails || rowsWithShortDetails == 1);
        if (owners.isEmpty()) {
          QTRY_VERIFY(textPositionVisible(*details, *dialog, 0));
          QTRY_VERIFY(textPositionVisible(*details, *dialog, details->toPlainText().size() - 1));
        } else {
          auto* owner = owners.front();
          QVERIFY(owner != details);
          QTRY_VERIFY(exposeTextPosition(*details, *owner, *dialog, 0));
          QTRY_VERIFY(exposeTextPosition(*details, *owner, *dialog, details->toPlainText().size() - 1));
        }
      }
    }
    selectRecord(shortRow);
    QTRY_COMPARE(details->height(), shortDetailsHeight);
    QTRY_COMPARE(table->viewport()->height() / table->verticalHeader()->defaultSectionSize(), rowsWithShortDetails);
  }
}

void DialogBehaviorTests::overlayFilesPaginatesWithoutReloadingTheSource() {
  const auto originalLanguage = uwf::ui::I18n::instance().lang();
  const auto originalTheme = uwf::ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    uwf::ui::I18n::instance().setLang(originalLanguage);
    uwf::ui::ThemeManager::instance().apply(originalTheme);
  });
  std::atomic<int> loads{0};
  uwf::ui::OverlayFilesDialog dialog(
      std::make_shared<uwf::presentation::OverlayFilesViewModel>(
          QStringLiteral("C:"),
          [&loads](const QString&, std::stop_token) {
            ++loads;
            QVector<uwf::app::OverlayFileEntry> rows;
            for (int index = 0; index < 80; ++index) {
              rows.append({QStringLiteral("\\Files\\item-%1.txt:$DATA").arg(index, 3, 10, QChar('0')), {}, false, false, static_cast<qulonglong>(index + 1)});
            }
            return rows;
          }),
      uwf::test::localFileDialogs());
  dialog.resize(760, 420);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* list = dialog.findChild<QTableView*>(QStringLiteral("overlayFilesTable"));
  auto* last = uwf::test::buttonByAccessibleName(&dialog, QStringLiteral("Last page"));
  QVERIFY(list);
  QVERIFY(last);
  QTRY_VERIFY_WITH_TIMEOUT(last->isEnabled(), 2500);
  QTRY_VERIFY(pagedViewportShowsWholeRows(*list, list->verticalHeader()->defaultSectionSize()));
  const auto originalSize = dialog.size();
  const QString firstPagePath = list->model()->index(0, 0).data(Qt::UserRole).toString();
  QTest::mouseClick(last, Qt::LeftButton);
  QCOMPARE(list->model()->rowCount(), 80);
  QTRY_COMPARE(list->verticalScrollBar()->value(), list->verticalScrollBar()->maximum());
  QVERIFY(list->model()->index(list->rowAt(0), 0).data(Qt::UserRole).toString() != firstPagePath);
  QCOMPARE(dialog.size(), originalSize);
  QCOMPARE(loads.load(), 1);
  const QPersistentModelIndex selected(list->model()->index(79, 0));
  list->selectionModel()->setCurrentIndex(selected, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  auto* detail = dialog.findChild<uwf::ui::ContentHintBox*>(QStringLiteral("overlaySelectedPath"));
  QVERIFY(detail && !detail->toPlainText().isEmpty());
  auto cursor = detail->textCursor();
  cursor.setPosition(3);
  cursor.setPosition(9, QTextCursor::KeepAnchor);
  detail->setTextCursor(cursor);
  const auto selection = cursor.selectedText();
  QSignalSpy resets(list->model(), &QAbstractItemModel::modelReset);
  for (const auto language : {uwf::ui::I18n::Lang::Zh_CN, uwf::ui::I18n::Lang::Ar, uwf::ui::I18n::Lang::En}) {
    const auto theme = uwf::ui::ThemeManager::instance().isLight() ? uwf::app::Theme::Dark : uwf::app::Theme::Light;
    uwf::ui::I18n::instance().requestLang(language);
    uwf::ui::ThemeManager::instance().requestToggle();
    QTRY_COMPARE(uwf::ui::I18n::instance().lang(), language);
    QTRY_COMPARE(uwf::ui::ThemeManager::instance().current(), theme);
    QTRY_VERIFY(dialog.findChildren<QWidget*>(QStringLiteral("appearanceTransition")).isEmpty());
    QCOMPARE(dialog.windowTitle(), uwf::ui::I18n::tr("View overlay"));
    QCOMPARE(last->accessibleName(), uwf::ui::I18n::tr("Last page"));
    QCOMPARE(list->verticalScrollBar()->value(), list->verticalScrollBar()->maximum());
    QVERIFY(selected.isValid() && list->selectionModel()->isSelected(selected));
    QCOMPARE(detail->textCursor().selectedText(), selection);
    QCOMPARE(resets.count(), 0);
    QCOMPARE(loads.load(), 1);
  }
}

void DialogBehaviorTests::overlayFilesExportWritesTheCompleteSnapshotAndHonorsCancellation() {
  FileDialogStub files;
  auto model = std::make_shared<uwf::presentation::OverlayFilesViewModel>(
      QStringLiteral("C:"),
      [](const QString&, std::stop_token) {
        return QVector<uwf::app::OverlayFileEntry>{{QStringLiteral("\\Data\\note.txt:$DATA"), {}, false, false, 12},
                                                   {QStringLiteral("\\Cache:$INDEX_ALLOCATION"), {}, false, false, 4096}};
      },
      uwf::windows::writeUtf8TextFile);
  uwf::ui::OverlayFilesDialog dialog(model, files);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* exportButton = buttonByText(&dialog, QStringLiteral("Export list"));
  QVERIFY(exportButton);
  QTRY_VERIFY_WITH_TIMEOUT(exportButton->isEnabled(), 2500);

  QTest::mouseClick(exportButton, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 1);
  QVERIFY(files.requests.front().initialPath.contains(QStringLiteral("overlay-files-C-")));

  QTemporaryDir outputDirectory;
  QVERIFY(outputDirectory.isValid());
  files.savedFile = outputDirectory.filePath(QStringLiteral("overlay-files.txt"));
  {
    QFile previous(files.savedFile);
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

  QFile written(files.savedFile);
  QVERIFY(written.open(QIODevice::ReadOnly | QIODevice::Text));
  const QString text = QString::fromUtf8(written.readAll());
  QVERIFY(text.contains(QStringLiteral("# UWF overlay files for C:")));
  QVERIFY(text.contains(QStringLiteral("file\t12\t")));
  QVERIFY(text.contains(QStringLiteral("\tC:\\Data\\note.txt")));
  QVERIFY(text.contains(QStringLiteral("dir\t4096\t")));
  QVERIFY(text.contains(QStringLiteral("\tC:\\Cache")));
  QCOMPARE(files.requests.size(), 2);

  files.savedFile = QDir::tempPath();
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
  const uwf::app::wmi::WmiInfrastructureError failure(static_cast<int32_t>(uwf::app::wmi::WmiErrorCode::RpcServerFault), "read overlay files",
                                                      "mock provider failure\n\noriginal provider detail");
  std::atomic<bool> reject = true;
  auto model = std::make_shared<uwf::presentation::OverlayFilesViewModel>(QStringLiteral("D:"), [&, failure](const QString&, std::stop_token) {
    if (reject.load()) throw failure;
    return QVector<uwf::app::OverlayFileEntry>{};
  });
  QSignalSpy failures(model.get(), &uwf::presentation::OverlayFilesViewModel::loadFailed);
  uwf::ui::OverlayFilesDialog dialog(model, uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* progress = dialog.findChild<QProgressBar*>();
  auto* exportButton = buttonByText(&dialog, QStringLiteral("Export list"));
  QVERIFY(progress);
  QVERIFY(exportButton);
  QTRY_VERIFY_WITH_TIMEOUT(!progress->isVisible(), 2500);
  QTRY_COMPARE(failures.count(), 1);
  QTRY_VERIFY_WITH_TIMEOUT(labelContaining(&dialog, QStringLiteral("mock provider failure")).size() > 0, 2500);
  QVERIFY(!exportButton->isEnabled());
  auto* list = dialog.findChild<QTableView*>(QStringLiteral("overlayFilesTable"));
  QVERIFY(list);
  QCOMPARE(list->model()->rowCount(), 0);
  auto* disclosure = dialog.findChild<uwf::ui::Disclosure*>(QStringLiteral("overlayErrorDisclosure"));
  auto* hint = dialog.findChild<uwf::ui::TextLabel*>(QStringLiteral("overlayErrorHint"));
  auto* detail = dialog.findChild<uwf::ui::ContentHintBox*>(QStringLiteral("overlayErrorDetails"));
  QVERIFY(disclosure && hint && detail);
  QVERIFY(!hint->text().isEmpty());
  QCOMPARE(hint->text(), model->errorHint());
  QCOMPARE(detail->toPlainText(), QStringLiteral("Raw error: %1").arg(QString::fromUtf8(failure.what())));
  QCOMPARE(model->errorText(), hint->text() + QStringLiteral("\n\n") + detail->toPlainText());
  QCOMPARE(failures.front().front().toString(), model->errorText());
  QVERIFY(detail->textInteractionFlags().testFlag(Qt::TextSelectableByMouse));
  disclosure->setOpen(true);
  QTRY_VERIFY(hint->isVisible() && detail->isVisible());
  for (const QSize size : {QSize(1096, 754), QSize(720, 600)}) {
    dialog.resize(size);
    QTRY_VERIFY(detail->y() > hint->y() + hint->height());
    QTRY_VERIFY(hint->height() >= hint->heightForWidth(hint->width()));
    QTRY_VERIFY(detail->height() >= detail->heightForWidth(detail->width()));
    QTRY_COMPARE(detail->verticalScrollBar()->maximum(), 0);
  }

  reject = false;
  model->startLoading();
  QVERIFY(model->errorHint().isEmpty());
  QVERIFY(model->errorDetail().isEmpty());
  QTRY_COMPARE(model->loadState(), uwf::presentation::ContentLoadState::Empty);
  QVERIFY(!disclosure->isVisible());
  QVERIFY(detail->toPlainText().isEmpty());
}

void DialogBehaviorTests::overlayFilesLongErrorUsesOneReachableScrollOwner_data() {
  QTest::addColumn<QSize>("size");
  QTest::newRow("minimum-window") << QSize(720, 500);
  QTest::newRow("normal-window") << QSize(720, 600);
}

void DialogBehaviorTests::overlayFilesLongErrorUsesOneReachableScrollOwner() {
  QFETCH(QSize, size);
  QString providerDetail = QStringLiteral("BEGIN provider diagnostic\n");
  for (int index = 1; index <= 24; ++index)
    providerDetail +=
        QStringLiteral("Diagnostic %1: C:\\ProgramData\\Application\\LongProviderDiagnosticDirectory\\PendingFile-%1.json could not be read.\n").arg(index);
  providerDetail += QStringLiteral("END provider diagnostic");
  const uwf::app::wmi::WmiInfrastructureError failure(static_cast<int32_t>(uwf::app::wmi::WmiErrorCode::RpcServerFault), "read overlay files",
                                                      providerDetail.toStdString());
  auto model = std::make_shared<uwf::presentation::OverlayFilesViewModel>(
      QStringLiteral("C:"), [failure](const QString&, std::stop_token) -> QVector<uwf::app::OverlayFileEntry> { throw failure; });
  uwf::ui::OverlayFilesDialog dialog(model, uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  dialog.resize(size);
  QTRY_COMPARE(dialog.size(), size);
  QTRY_COMPARE(model->loadState(), uwf::presentation::ContentLoadState::Failed);
  auto* disclosure = dialog.findChild<uwf::ui::Disclosure*>(QStringLiteral("overlayErrorDisclosure"));
  auto* detail = dialog.findChild<uwf::ui::ContentHintBox*>(QStringLiteral("overlayErrorDetails"));
  auto* close = buttonByRole(&dialog, QDialogButtonBox::RejectRole);
  QVERIFY(disclosure && detail && close);
  disclosure->setOpen(true);
  QTRY_VERIFY(detail->isVisible());
  QCOMPARE(detail->toPlainText(), QStringLiteral("Raw error: %1").arg(QString::fromUtf8(failure.what())));
  QVERIFY(detail->toPlainText().contains(providerDetail));

  // A hidden scrollbar still has a range when its document is clipped. Verify
  // actual content geometry as well as the number of scrolling ancestors.
  QTRY_COMPARE(detail->verticalScrollBar()->maximum(), 0);
  QTRY_COMPARE(detail->horizontalScrollBar()->maximum(), 0);
  QTRY_VERIFY(textDocumentFits(*detail));
  QTRY_COMPARE(scrollingAncestors(*detail, dialog).size(), 1);
  auto* owner = scrollingAncestors(*detail, dialog).constFirst();
  QVERIFY(owner != detail);
  QVERIFY(detail->document()->documentLayout()->documentSize().height() > owner->viewport()->height());
  QTRY_VERIFY(owner->verticalScrollBar()->isVisible());

  QTRY_VERIFY(exposeTextPosition(*detail, *owner, dialog, detail->toPlainText().indexOf(QStringLiteral("BEGIN provider diagnostic"))));
  QTRY_VERIFY(exposeTextPosition(*detail, *owner, dialog, detail->toPlainText().indexOf(QStringLiteral("END provider diagnostic"))));
  QTRY_VERIFY(owner->verticalScrollBar()->value() > owner->verticalScrollBar()->minimum());
  QVERIFY(!close->visibleRegion().isEmpty());
  QVERIFY(dialog.rect().contains(QRect(close->mapTo(&dialog, QPoint()), close->size())));
  QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("overlay-long-error-single-scroll")));
}

void DialogBehaviorTests::overlayFilesDestructionCancelsWithoutWaitingForTheActiveLoader() {
  auto gate = std::make_shared<uwf::test::WorkerGate>();
  const auto release = qScopeGuard([gate] { gate->release(); });
  QElapsedTimer elapsed;
  {
    uwf::ui::OverlayFilesDialog dialog(std::make_shared<uwf::presentation::OverlayFilesViewModel>(QStringLiteral("C:"),
                                                                                                  [gate](const QString&, const std::stop_token stopToken) {
                                                                                                    gate->wait(stopToken);
                                                                                                    return QVector<uwf::app::OverlayFileEntry>{};
                                                                                                  }),
                                       uwf::test::localFileDialogs());
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_COMPARE_WITH_TIMEOUT(gate->started.load(), 1, 2500);
    elapsed.start();
  }
  QVERIFY(elapsed.elapsed() < 100);
  QTRY_COMPARE_WITH_TIMEOUT(gate->canceled.load(), 1, 2000);
  QVERIFY(QThreadPool::globalInstance()->waitForDone(2000));
}

void DialogBehaviorTests::overlayFilesRejectsMissingLoader() {
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, uwf::ui::OverlayFilesDialog(std::make_shared<uwf::presentation::OverlayFilesViewModel>(
                                                                                  QStringLiteral("C:"), uwf::app::OverlayFileLoader{}),
                                                                              uwf::test::localFileDialogs()));
}

void DialogBehaviorTests::overlayFilesContextMenuCommitsOnlyOrdinaryEntries() {
  const auto previousClipboard = QApplication::clipboard()->text();
  const auto restoreClipboard = qScopeGuard([&] { QApplication::clipboard()->setText(previousClipboard); });
  uwf::ui::OverlayFilesDialog dialog(
      std::make_shared<uwf::presentation::OverlayFilesViewModel>(QStringLiteral("C:"),
                                                                 [](const QString&, std::stop_token) {
                                                                   return QVector<uwf::app::OverlayFileEntry>{
                                                                       {QStringLiteral("\\Users\\Alice\\note.txt:$DATA"), {}, false, false, 100},
                                                                       {QStringLiteral("\\$MFT:$DATA"), {}, false, false, 50},
                                                                   };
                                                                 }),
      uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* list = dialog.findChild<QTableView*>(QStringLiteral("overlayFilesTable"));
  QVERIFY(list);
  QTRY_COMPARE_WITH_TIMEOUT(list->model()->rowCount(), 2, 2500);
  QSignalSpy commitSpy(&dialog, &uwf::ui::OverlayFilesDialog::commitFileRequested);

  auto openContextMenu = [list](const int row, const std::function<void(QMenu*)>& inspect) {
    list->scrollTo(list->model()->index(row, 0));
    const QPoint local = list->visualRect(list->model()->index(row, 0)).center();
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
  for (int row = 0; row < list->model()->rowCount(); ++row) {
    if (list->model()->index(row, 0).data(Qt::UserRole + 2).toBool())
      metadataRow = row;
    else
      ordinaryRow = row;
  }
  QVERIFY(ordinaryRow >= 0);
  QVERIFY(metadataRow >= 0);
  const QString ordinaryPath = list->model()->index(ordinaryRow, 0).data(Qt::UserRole).toString();
  openContextMenu(ordinaryRow, [](QMenu* menu) {
    QCOMPARE(menu->actions().size(), 4);
    for (const auto actions = menu->actions(); auto* action : actions)
      if (!action->isSeparator()) QVERIFY(!action->icon().isNull());
    menu->actions().at(3)->trigger();
  });
  QCOMPARE(commitSpy.count(), 1);
  QCOMPARE(commitSpy.at(0).at(0).toString(), ordinaryPath);

  openContextMenu(metadataRow, [](QMenu* menu) {
    QCOMPARE(menu->actions().size(), 2);
    for (const auto actions = menu->actions(); auto* action : actions) QVERIFY(!action->icon().isNull());
    menu->actions().at(1)->trigger();
    menu->close();
  });
  QCOMPARE(QApplication::clipboard()->text(), list->model()->index(metadataRow, 0).data(Qt::UserRole).toString());
  QCOMPARE(commitSpy.count(), 1);
}

void DialogBehaviorTests::overlayMessagesPreserveScopeAndConfirmationActions_data() {
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("chinese");
  QTest::newRow("en-light") << false << false;
  QTest::newRow("en-dark") << true << false;
  QTest::newRow("zh-light") << false << true;
  QTest::newRow("zh-dark") << true << true;
}

void DialogBehaviorTests::overlayMessagesPreserveScopeAndConfirmationActions() {
  using namespace uwf;
  QFETCH(bool, dark);
  QFETCH(bool, chinese);
  const auto previousTheme = ui::ThemeManager::instance().current();
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto restore = qScopeGuard([&] {
    ui::ThemeManager::instance().apply(previousTheme);
    ui::I18n::instance().setLang(previousLanguage);
  });
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  ui::I18n::instance().setLang(chinese ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En);
  QWidget owner;
  owner.resize(1100, 760);
  owner.show();
  FileDialogStub files;
  ui::OverlayFilesDialog overlay(std::make_shared<presentation::OverlayFilesViewModel>(
                                     QStringLiteral("C:"),
                                     [](const QString&, std::stop_token) {
                                       return QVector<app::OverlayFileEntry>{{QStringLiteral("\\Users\\Alice\\note.txt:$DATA"), {}, false, false, 100}};
                                     }),
                                 files, &owner);
  overlay.setWindowModality(Qt::WindowModal);
  overlay.show();
  QVERIFY(QTest::qWaitForWindowExposed(&overlay));
  auto* scope = buttonByText(&overlay, ui::I18n::tr("About this snapshot"));
  QVERIFY(scope);
  bool scopeInspected = false;
  QTimer::singleShot(0, &overlay, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(dialog && dialog != &overlay);
    const auto close = qScopeGuard([&] { dialog->reject(); });
    QCOMPARE(dialog->objectName(), QStringLiteral("overlaySnapshotScopeDialog"));
    QCOMPARE(dialog->width(), 520);
    QCOMPARE(dialog->windowTitle(), ui::I18n::tr("Overlay snapshot scope"));
    for (const auto& heading : {"NTFS volumes only", "Small files may be missing", "Does not indicate pending commits"})
      QVERIFY(!labelContaining(dialog, ui::I18n::tr(heading)).isEmpty());
    auto* dismiss = buttonByRole(dialog, QDialogButtonBox::RejectRole);
    QVERIFY(dismiss && dismiss->isVisible());
    QCOMPARE(dismiss->text(), ui::I18n::tr("Close"));
    scopeInspected = true;
  });
  QTest::mouseClick(scope, Qt::LeftButton);
  QVERIFY(scopeInspected);
  QTRY_COMPARE(QApplication::activeModalWidget(), &overlay);
}

void DialogBehaviorTests::registryPickerBrowsesSnapshotAndFiltersPersistentHives() {
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::CommitValue, browser),
                              QStringLiteral("Registry snapshot"));
  dialog.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* tree = dialog.findChild<QTreeView*>();
  QVERIFY(tree);
  const auto hklm = findTreeItem(tree, QStringLiteral("HKEY_LOCAL_MACHINE"));
  QVERIFY(hklm.isValid());
  tree->expand(hklm);
  QCoreApplication::processEvents();
  QVERIFY(findTreeItem(hklm, QStringLiteral("SOFTWARE")).isValid());
  QVERIFY(findTreeItem(hklm, QStringLiteral("SYSTEM")).isValid());
  QVERIFY(!findTreeItem(hklm, QStringLiteral("PERSISTENT_SOFTWARE")).isValid());

  dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  QCOMPARE(tree->currentIndex().data().toString(), QStringLiteral("Product"));
  auto* values = dialog.findChild<QTableView*>();
  QVERIFY(values);
  QCOMPARE(values->model()->rowCount(), 4);
  QCOMPARE(values->model()->index(0, 0).data().toString(), QStringLiteral("(Default)"));
  QVERIFY(!(values->model()->index(0, 0).flags() & Qt::ItemIsSelectable));
  QCOMPARE(values->model()->index(1, 0).data().toString(), QStringLiteral("Name"));
  QCOMPARE(values->model()->index(1, 1).data().toString(), QStringLiteral("REG_SZ"));
  QCOMPARE(values->model()->index(1, 2).data().toString(), QStringLiteral("UWF Manager"));
  QCOMPARE(values->model()->index(2, 2).data().toString(), QStringLiteral("0x00000001 (1)"));
  QVERIFY(values->model()->index(3, 2).data().toString().contains(QStringLiteral("unavailable"), Qt::CaseInsensitive));
}

void DialogBehaviorTests::registryPickerFailedEnumerationCanBeRetriedWithoutPartialChildren() {
  auto browser = registrySnapshot();
  browser.failSubkeyEnumeration = true;
  RegistryPickerDialog dialog(std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::CommitValue, browser),
                              QStringLiteral("Registry retry"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QVERIFY(!QApplication::activeModalWidget());
  auto* error = dialog.findChild<QWidget*>(QStringLiteral("registryAccessError"));
  QVERIFY(error && error->isVisible());
  QVERIFY(!labelContaining(error, QStringLiteral("mock registry enumeration failure")).isEmpty());
  auto* values = dialog.findChild<QTableView*>();
  QVERIFY(values && !values->isVisible());
  auto* tree = dialog.findChild<QTreeView*>();
  QVERIFY(tree);
  const auto hklm = findTreeItem(tree, QStringLiteral("HKEY_LOCAL_MACHINE"));
  QVERIFY(hklm.isValid());
  QVERIFY(!findTreeItem(hklm, QStringLiteral("SOFTWARE")).isValid());

  browser.failSubkeyEnumeration = false;
  tree->expand(hklm);
  QCoreApplication::processEvents();
  QVERIFY(findTreeItem(hklm, QStringLiteral("SOFTWARE")).isValid());
  QVERIFY(findTreeItem(hklm, QStringLiteral("SYSTEM")).isValid());
  QVERIFY(!error->isVisible());
  QVERIFY(values->isVisible());
}

void DialogBehaviorTests::registryPickerAccessFailuresStayInTheDetailsPane_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("language");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<RegistryPickerDialog::Mode>("mode");
  for (const auto language : {uwf::ui::I18n::Lang::En, uwf::ui::I18n::Lang::Zh_CN})
    for (const bool dark : {false, true})
      for (const auto mode : {RegistryPickerDialog::Mode::CommitValue, RegistryPickerDialog::Mode::DeleteValue, RegistryPickerDialog::Mode::Exclusion}) {
        const auto name = QByteArray::number(static_cast<int>(language)) + (dark ? "-dark-" : "-light-") + QByteArray::number(static_cast<int>(mode));
        QTest::newRow(name.constData()) << language << dark << mode;
      }
}

void DialogBehaviorTests::registryPickerAccessFailuresStayInTheDetailsPane() {
  QFETCH(uwf::ui::I18n::Lang, language);
  QFETCH(bool, dark);
  QFETCH(RegistryPickerDialog::Mode, mode);
  const auto previousLanguage = uwf::ui::I18n::instance().lang();
  const auto previousTheme = uwf::ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    uwf::ui::I18n::instance().setLang(previousLanguage);
    uwf::ui::ThemeManager::instance().apply(previousTheme);
  });
  uwf::ui::I18n::instance().setLang(language);
  uwf::ui::ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(std::make_shared<uwf::presentation::RegistryPickerViewModel>(mode, browser), QStringLiteral("Registry access"));
  dialog.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  dialog.resize(720, 480);
  auto* table = dialog.findChild<QTableView*>();
  auto* error = dialog.findChild<QWidget*>(QStringLiteral("registryAccessError"));
  auto* path = dialog.findChild<uwf::ui::ContentHintBox*>(QStringLiteral("registryAccessErrorPath"));
  auto* detail = dialog.findChild<uwf::ui::ContentHintBox*>(QStringLiteral("registryAccessErrorDetail"));
  QVERIFY(table && error && path && detail);
  QVERIFY(table->isVisible());
  QVERIFY(!error->isVisible());
  QCOMPARE(table->model()->rowCount(), 4);
  browser.failValues = true;
  dialog.preselectKey(QStringLiteral("HKLM\\SYSTEM"));
  QVERIFY(!QApplication::activeModalWidget());
  QVERIFY(error->isVisible());
  QVERIFY(!table->isVisible());
  QCOMPARE(table->model()->rowCount(), 0);
  QCOMPARE(path->toPlainText(), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM"));
  QVERIFY(detail->toPlainText().contains(QStringLiteral("拒绝访问")));
  QVERIFY(!detail->toPlainText().contains(QChar::ReplacementCharacter));
  QTRY_VERIFY2(uwf::test::clippedText(dialog).isEmpty(), qUtf8Printable(uwf::test::clippedText(dialog)));
  QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("registry-access-error")));

  browser.failValues = false;
  dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  QVERIFY(table->isVisible());
  QVERIFY(!error->isVisible());
  QCOMPARE(table->model()->rowCount(), 4);
  QVERIFY(path->toPlainText().isEmpty());
  QVERIFY(detail->toPlainText().isEmpty());
  QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("registry-access-recovered")));
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
  RegistryPickerDialog dialog(std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::CommitValue, browser),
                              QStringLiteral("Registry values"));
  dialog.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* values = dialog.findChild<QTableView*>();
  QVERIFY(values);

  auto preview = [values](const QString& name) {
    for (int row = 0; row < values->model()->rowCount(); ++row) {
      if (values->model()->index(row, 0).data().toString() == name) return values->model()->index(row, 2).data().toString();
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
  RegistryPickerDialog named(std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::CommitValue, browser),
                             QStringLiteral("Commit registry"));
  named.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  named.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  named.show();
  QVERIFY(QTest::qWaitForWindowExposed(&named));
  auto* values = named.findChild<QTableView*>();
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

  RegistryPickerDialog wholeKey(std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::DeleteValue, browser),
                                QStringLiteral("Delete registry"));
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

void DialogBehaviorTests::registryPickerCancellationClearsSharedSelection_data() {
  QTest::addColumn<int>("entry");
  QTest::newRow("title") << static_cast<int>(RejectionEntry::Title);
  QTest::newRow("cancel") << static_cast<int>(RejectionEntry::Cancel);
  QTest::newRow("escape") << static_cast<int>(RejectionEntry::Escape);
  QTest::newRow("window-close") << static_cast<int>(RejectionEntry::WindowClose);
}

void DialogBehaviorTests::registryPickerCancellationClearsSharedSelection() {
  QFETCH(int, entry);
  auto browser = registrySnapshot();
  auto model = std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::CommitValue, browser);
  model->accept(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product"), QStringLiteral("Name"));
  RegistryPickerDialog dialog(model, QStringLiteral("Cancel registry selection"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QVERIFY(model->selection().has_value());
  QVERIFY(dialog.selection().has_value());
  QCOMPARE(model->selection()->valueName, QStringLiteral("Name"));
  QSignalSpy finished(&dialog, &QDialog::finished);
  QSignalSpy rejected(&dialog, &QDialog::rejected);

  switch (static_cast<RejectionEntry>(entry)) {
    case RejectionEntry::Title:
      QTest::mouseClick(dialog.closeButton(), Qt::LeftButton);
      break;
    case RejectionEntry::Cancel: {
      auto* cancel = buttonByRole(&dialog, QDialogButtonBox::RejectRole);
      QVERIFY(cancel);
      QTest::mouseClick(cancel, Qt::LeftButton);
      break;
    }
    case RejectionEntry::Escape: {
      auto* address = dialog.findChild<QLineEdit*>(QStringLiteral("registryAddress"));
      QVERIFY(address);
      QTest::keyClick(address, Qt::Key_Escape);
      break;
    }
    case RejectionEntry::WindowClose:
      dialog.close();
      break;
    default:
      QFAIL("Unexpected registry rejection entry");
  }

  QVERIFY(!dialog.isVisible());
  QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
  QCOMPARE(finished.count(), 1);
  QCOMPARE(rejected.count(), 1);
  QVERIFY(!model->selection().has_value());
  QVERIFY(!dialog.selection().has_value());
}

void DialogBehaviorTests::registryAddressBarNavigatesShorthandAndClearResetsSelection() {
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::CommitValue, browser),
                              QStringLiteral("Registry address"));
  dialog.setAvailabilityChecker([](const QString&) { return RegistryPickerDialog::KeyAvailability::Selectable; });
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* address = dialog.findChild<QLineEdit*>();
  auto* tree = dialog.findChild<QTreeView*>();
  auto* values = dialog.findChild<QTableView*>();
  auto* ok = buttonByRole(&dialog, QDialogButtonBox::AcceptRole);
  QVERIFY(address && tree && values && ok);

  address->setText(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  QTest::keyClick(address, Qt::Key_Return);
  QVERIFY(tree->currentIndex().isValid());
  QCOMPARE(tree->currentIndex().data().toString(), QStringLiteral("Product"));
  QCOMPARE(address->text(), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product"));
  QCOMPARE(values->model()->rowCount(), 4);
  QVERIFY(dialog.isVisible());
  QVERIFY(!dialog.selection().has_value());
  QTest::keyClick(address, Qt::Key_Enter);
  QVERIFY(dialog.isVisible());
  QVERIFY(!dialog.selection().has_value());

  address->clear();
  QCOMPARE(tree->selectionModel()->selectedIndexes().size(), 0);
  QCOMPARE(values->selectionModel()->selectedIndexes().size(), 0);
  QVERIFY(!ok->isEnabled());
  QCOMPARE(tree->model()->rowCount(), 5);
}

void DialogBehaviorTests::pastingARegistryAddressDoesNotEnableTheNavigationFocusRing_data() {
  QTest::addColumn<bool>("dark");
  QTest::newRow("light") << false;
  QTest::newRow("dark") << true;
}

void DialogBehaviorTests::pastingARegistryAddressDoesNotEnableTheNavigationFocusRing() {
  using namespace uwf;
  QFETCH(bool, dark);
  const auto previousTheme = ui::ThemeManager::instance().current();
  const auto previousClipboard = QApplication::clipboard()->text();
  const auto restore = qScopeGuard([&] {
    ui::ThemeManager::instance().apply(previousTheme);
    QApplication::clipboard()->setText(previousClipboard);
  });
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(std::make_shared<presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::Exclusion, browser),
                              QStringLiteral("Registry exclusion"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  auto* address = dialog.findChild<QLineEdit*>(QStringLiteral("registryAddress"));
  QVERIFY(address);
  QTest::mouseClick(address, Qt::LeftButton);
  QTRY_VERIFY(address->hasFocus());
  const QString pasted = QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product");
  QApplication::clipboard()->setText(pasted);
  QTest::keyClick(dialog.windowHandle(), Qt::Key_V, Qt::ControlModifier);
  QCOMPARE(address->text(), pasted);
  const auto frames = dialog.findChildren<QFocusFrame*>();
  for (auto* frame : frames) QVERIFY(!frame->isVisible() || frame->widget() != address);
  QVERIFY(test::saveScreenshot(dialog, QStringLiteral("registry-pasted-address")));
}

void DialogBehaviorTests::registryAddressKeyboardFocusOutlineIsFullyVisible_data() {
  QTest::addColumn<bool>("dark");
  QTest::newRow("light") << false;
  QTest::newRow("dark") << true;
}

void DialogBehaviorTests::registryAddressKeyboardFocusOutlineIsFullyVisible() {
  using namespace uwf;
  QFETCH(bool, dark);
  const auto previousTheme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] { ui::ThemeManager::instance().apply(previousTheme); });
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(std::make_shared<presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::Exclusion, browser),
                              QStringLiteral("Registry exclusion"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  auto* address = dialog.findChild<QLineEdit*>(QStringLiteral("registryAddress"));
  QVERIFY(address);
  QTest::mouseClick(address, Qt::LeftButton);
  QTest::keyClick(dialog.windowHandle(), Qt::Key_Tab);
  QTest::keyClick(dialog.windowHandle(), Qt::Key_Tab, Qt::ShiftModifier);
  QTRY_VERIFY(address->hasFocus());
  const auto focusOutline = [&]() -> QFocusFrame* {
    const auto frames = dialog.findChildren<QFocusFrame*>();
    for (auto* frame : frames)
      if (frame->isVisible() && frame->widget() == address) return frame;
    return nullptr;
  };
  QTRY_VERIFY(focusOutline());
  auto* outline = focusOutline();
  QTRY_COMPARE_WITH_TIMEOUT(outline->visibleRegion(), QRegion(outline->rect()), 1500);
  const auto upperOutlinePixels = [&] {
    const auto image = dialog.grab().toImage().scaled(dialog.size());
    const auto origin = outline->mapTo(&dialog, QPoint());
    const auto top = address->mapTo(&dialog, QPoint()).y();
    const QRect upperBand(origin.x() + 10, origin.y(), outline->width() - 20, top - origin.y());
    const auto accent = ui::DesignSystem::instance().color(ui::design::Color::Accent);
    int pixels = 0;
    for (int y = upperBand.top(); y <= upperBand.bottom(); ++y)
      for (int x = upperBand.left(); x <= upperBand.right(); ++x)
        if (image.rect().contains(x, y) && image.pixelColor(x, y) == accent) ++pixels;
    return pixels;
  };
  QTRY_VERIFY_WITH_TIMEOUT(upperOutlinePixels() > 20, 1500);
  QVERIFY(test::saveScreenshot(dialog, QStringLiteral("registry-keyboard-focus-outline")));
}

void DialogBehaviorTests::registryExclusionDisablesValueSelection() {
  auto browser = registrySnapshot();
  RegistryPickerDialog dialog(std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::Exclusion, browser),
                              QStringLiteral("Registry exclusion"));
  dialog.setAvailabilityChecker([](const QString& key) {
    return key.endsWith(QStringLiteral("Product")) ? RegistryPickerDialog::KeyAvailability::Selectable : RegistryPickerDialog::KeyAvailability::ContainerOnly;
  });
  dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* values = dialog.findChild<QTableView*>();
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
  QList<uwf::presentation::CommitReportRow> rows;
  for (int index = 0; index < 205; ++index) {
    rows.append({index % 2 == 0 ? uwf::presentation::CommitRowStatus::Succeeded : uwf::presentation::CommitRowStatus::Failed,
                 QStringLiteral("C:\\Files\\item-%1.txt").arg(index), index % 2 == 0 ? QStringLiteral("-") : QStringLiteral("0x80041001"),
                 index % 2 == 0 ? QStringLiteral("-") : QStringLiteral("Mock failure"), true, index % 2 == 0 ? std::optional<bool>(false) : std::nullopt});
  }

  bool inspected = false;
  bool structureValid = false;
  bool paginationValid = false;
  bool copyValid = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* table = dialog->findChild<QTableView*>();
    if (!table) {
      dialog->reject();
      return;
    }
    auto* pages = dialog->findChild<uwf::presentation::PageProxyModel*>();
    structureValid = pages && table->model()->columnCount() == 6 && pages->totalRows() == 205 && table->model()->rowCount() > 0;
    auto* last = uwf::test::buttonByAccessibleName(dialog, QStringLiteral("Last page"));
    if (last && last->isEnabled()) QTest::mouseClick(last, Qt::LeftButton);
    paginationValid =
        pages && !pages->pager().hasNext(pages->totalRows()) && table->model()->rowCount() > 0 &&
        table->model()->index(table->model()->rowCount() - 1, 1).data(uwf::presentation::FullTextRole).toString() == QStringLiteral("C:\\Files\\item-204.txt");
    auto* copyAll = dialog->findChild<QPushButton*>(QStringLiteral("commitReportCopyAllButton"));
    if (copyAll) QTest::mouseClick(copyAll, Qt::LeftButton);
    copyValid = QApplication::clipboard()->text().contains(QStringLiteral("item-204.txt"));
    inspected = true;
    if (auto* accept = buttonByRole(dialog, QDialogButtonBox::AcceptRole))
      accept->click();
    else
      dialog->accept();
  });

  uwf::ui::showCommitReport(nullptr, rows, uwf::app::CommitOperation::DeleteAndPersist, {uwf::presentation::CommitCompletion::Cause::Canceled, 3});
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
    auto* table = dialog->findChild<QTableView*>();
    structureValid = table && table->model()->columnCount() == 4 && table->model()->rowCount() == 0 &&
                     !labelContaining(dialog, QStringLiteral("Succeeded")).isEmpty() && !labelContaining(dialog, QStringLiteral("0 items")).isEmpty();
    inspected = true;
    if (auto* accept = buttonByRole(dialog, QDialogButtonBox::AcceptRole))
      accept->click();
    else
      dialog->accept();
  });

  uwf::ui::showCommitReport(nullptr, {}, uwf::app::CommitOperation::Persist);
  QVERIFY(inspected);
  QVERIFY(structureValid);
}

void DialogBehaviorTests::commitReportDetailsGrowForWrappedTextAndPreserveCopying() {
  const auto previousClipboard = QApplication::clipboard()->text();
  const auto restoreClipboard = qScopeGuard([&] { QApplication::clipboard()->setText(previousClipboard); });
  const QString longPath = QStringLiteral(
      "C:\\Users\\User\\Documents\\项目资料\\年度归档\\设备配置与运行记录\\"
      "需要在重启前保存的现场诊断数据与说明\\设备配置备份与现场工作记录.json");
  QString reason = QStringLiteral(
      "The file is being used by another process.\nClose the application editing the file and try again.\n"
      "The complete path and failure reason remain available for copying.");
  for (int line = 1; line <= 12; ++line)
    reason += QStringLiteral("\nAdditional provider diagnostic %1 remains available without an internal scrollbar.").arg(line);
  const QString expectedDetails = longPath + QStringLiteral("\n0x80070020\n") + reason;
  QList<uwf::presentation::CommitReportRow> rows{
      {uwf::presentation::CommitRowStatus::Succeeded, QStringLiteral("C:\\Data\\saved.json"), QStringLiteral("-"), QStringLiteral("-"), true, false},
      {uwf::presentation::CommitRowStatus::Failed, longPath, QStringLiteral("0x80070020"), reason, true, true}};
  for (int index = 0; index < 20; ++index)
    rows.append({uwf::presentation::CommitRowStatus::Succeeded, QStringLiteral("C:\\Data\\item-%1.json").arg(index), QStringLiteral("-"), QStringLiteral("-"),
                 true, false});

  bool inspected = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(dialog);
    const auto close = qScopeGuard([dialog] { dialog->reject(); });
    auto* table = dialog->findChild<QTableView*>(QStringLiteral("commitReportTable"));
    auto* details = dialog->findChild<uwf::ui::ContentHintBox*>(QStringLiteral("commitReportDetails"));
    auto* copy = dialog->findChild<QPushButton*>(QStringLiteral("commitReportCopyRowButton"));
    auto* done = buttonByRole(dialog, QDialogButtonBox::RejectRole);
    QVERIFY(table && details && copy && done);
    dialog->resize(900, 690);
    QTRY_COMPARE(details->toPlainText(), rows.front().path);
    QTRY_COMPARE(details->height(), std::max(details->minimumHeight(), details->heightForWidth(details->width())));
    QTRY_COMPARE(table->viewport()->height() % table->verticalHeader()->defaultSectionSize(), 0);
    const int shortHeight = details->height();
    const int rowsWithShortDetails = table->viewport()->height() / table->verticalHeader()->defaultSectionSize();
    table->selectRow(1);
    QTRY_COMPARE(details->toPlainText(), expectedDetails);
    QTRY_VERIFY(details->height() > shortHeight);
    QTRY_COMPARE(details->verticalScrollBar()->maximum(), 0);
    QTRY_VERIFY(table->viewport()->height() / table->verticalHeader()->defaultSectionSize() < rowsWithShortDetails);
    for (const QSize size : {QSize(900, 690), QSize(720, 600), QSize(720, 500)}) {
      dialog->resize(size);
      QTRY_COMPARE(dialog->size(), size);
      QTRY_COMPARE(details->verticalScrollBar()->maximum(), 0);
      QCOMPARE(details->horizontalScrollBar()->maximum(), 0);
      QTRY_VERIFY(textDocumentFits(*details));
      QTRY_COMPARE(scrollingAncestors(*details, *dialog).size(), 1);
      auto* owner = scrollingAncestors(*details, *dialog).constFirst();
      QVERIFY(owner != details);
      QTRY_VERIFY(exposeTextPosition(*details, *owner, *dialog, 0));
      QTRY_VERIFY(exposeTextPosition(*details, *owner, *dialog, expectedDetails.size() - 1));
      QTRY_VERIFY(pagedViewportShowsWholeRows(*table, table->verticalHeader()->defaultSectionSize()));
      QVERIFY(details->textInteractionFlags().testFlag(Qt::TextSelectableByMouse));
      copy->click();
      QCOMPARE(QApplication::clipboard()->text(), expectedDetails);
      QVERIFY(!done->visibleRegion().isEmpty());
      QVERIFY(dialog->rect().contains(QRect(done->mapTo(dialog, QPoint()), done->size())));
    }
    table->selectRow(0);
    QTRY_COMPARE(details->toPlainText(), rows.front().path);
    QTRY_COMPARE(details->height(), shortHeight);
    inspected = true;
  });
  uwf::ui::showCommitReport(nullptr, rows, uwf::app::CommitOperation::Persist);
  QVERIFY(inspected);
}

void DialogBehaviorTests::dataDialogsKeepCompactGeometryAndReadableColumns_data() {
  QTest::addColumn<bool>("chinese");
  QTest::addColumn<bool>("dark");
  for (const bool chinese : {false, true})
    for (const bool dark : {false, true}) QTest::newRow(chinese ? (dark ? "zh-dark" : "zh-light") : (dark ? "en-dark" : "en-light")) << chinese << dark;
}

void DialogBehaviorTests::dataDialogsKeepCompactGeometryAndReadableColumns() {
  using namespace uwf::ui;
  QFETCH(bool, chinese);
  QFETCH(bool, dark);
  const auto language = I18n::instance().lang();
  const auto theme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(language);
    ThemeManager::instance().apply(theme);
  });
  I18n::instance().setLang(chinese ? I18n::Lang::Zh_CN : I18n::Lang::En);
  ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  const auto inspect = [&](QDialog& dialog, const QString& name, const bool fitContent = false) {
    QTRY_VERIFY(dialog.minimumSize() != dialog.maximumSize());
    const QSize requested = QSize(1000, 600).expandedTo(dialog.minimumSize());
    dialog.resize(requested);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCOMPARE(dialog.size(), requested);
    if (!screenshots.isEmpty()) {
      QVERIFY(QDir().mkpath(screenshots));
      QVERIFY(uwf::test::screenshot(dialog).save(
          QDir(screenshots).filePath(QStringLiteral("data-dialog-") + name + "-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
    }
    if (fitContent)
      for (const auto areas = dialog.findChildren<QAbstractScrollArea*>(); const auto* area : areas) {
        if (!area->isVisibleTo(&dialog)) continue;
        if (const int rowHeight = uwf::test::displayedRowHeight(area); rowHeight > 0) {
          const auto* view = qobject_cast<const QAbstractItemView*>(area);
          QVERIFY(view);
          QTRY_VERIFY(pagedViewportShowsWholeRows(*view, rowHeight));
          QCOMPARE(area->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        } else {
          if (area->property("dialogScrollHost").toBool() && area->verticalScrollBar()->maximum() > 0) {
            const auto* scroll = qobject_cast<const QScrollArea*>(area);
            qWarning() << name << "overflow" << dialog.size() << area->viewport()->size() << scroll->widget()->size() << scroll->widget()->minimumSizeHint()
                       << scroll->widget()->heightForWidth(area->viewport()->width());
            const auto* body = scroll->widget()->layout();
            for (int i = 0; i < body->count(); ++i) {
              const auto* item = body->itemAt(i);
              qWarning() << i << item->geometry() << item->sizeHint() << item->minimumSize() << item->hasHeightForWidth()
                         << item->heightForWidth(scroll->widget()->width());
            }
          }
          QTRY_VERIFY2(area->verticalScrollBar()->maximum() == 0,
                       qUtf8Printable(name + ": " + area->objectName() + " · " + QString::number(area->verticalScrollBar()->maximum())));
        }
        QTRY_COMPARE(area->horizontalScrollBar()->maximum(), 0);
      }
    QTRY_VERIFY2_WITH_TIMEOUT(uwf::test::clippedText(dialog).isEmpty(), qUtf8Printable(uwf::test::clippedText(dialog)), 2000);
    for (const auto buttons = dialog.findChildren<QPushButton*>(); auto* button : buttons) {
      if (!button->isVisible()) continue;
      QVERIFY2(dialog.rect().contains(QRect(button->mapTo(&dialog, QPoint()), button->size())), qUtf8Printable(name + ": " + button->text()));
      QVERIFY2(!button->visibleRegion().isEmpty(), qUtf8Printable(name + ": " + button->objectName() + " · " + button->text()));
    }
    for (const auto tables = dialog.findChildren<QTableView*>(); auto* table : tables) {
      QVERIFY(table->horizontalHeader()->defaultAlignment().testFlag(Qt::AlignLeft));
      QVERIFY(table->viewport()->width() > 0 && table->viewport()->height() > 0);
    }
  };
  {
    auto browser = registrySnapshot();
    const std::string longKey = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product\\" + std::string(180, 'x');
    const std::string longValue(180, 'v');
    const std::string veryLongValue(16383, 'v');
    browser.children["HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product"] = {std::string(180, 'x')};
    browser.keyValues[longKey] = {{longValue, 1u, utf16Le(u"Mock data, no registry access")}, {veryLongValue, 1u, utf16Le(u"Mock long value")}};
    RegistryPickerDialog dialog(std::make_shared<uwf::presentation::RegistryPickerViewModel>(RegistryPickerDialog::Mode::CommitValue, browser),
                                I18n::tr("Select registry key / value"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* address = dialog.findChild<QLineEdit*>();
    QVERIFY(address);
    address->setText(QStringLiteral("HKLM\\SOFTWARE\\Vendor\\Product"));
    QTest::keyClick(address, Qt::Key_Return);
    auto* table = dialog.findChild<QTableView*>();
    QVERIFY(table);
    QTRY_COMPARE(table->model()->rowCount(), 4);
    inspect(dialog, QStringLiteral("registry-picker"));
    address->setText(QString::fromStdString(longKey));
    QTest::keyClick(address, Qt::Key_Return);
    // 即使数据源没有默认值，选择器也必须保留不可选择的默认值占位行。
    QTRY_COMPARE(table->model()->rowCount(), 3);
    QVERIFY(!table->model()->index(0, 0).flags().testFlag(Qt::ItemIsSelectable));
    QCOMPARE(table->model()->index(0, 0).data(Qt::UserRole).toString(), QString());
    table->setCurrentIndex(table->model()->index(1, 0));
    QTest::mouseClick(table->viewport(), Qt::LeftButton, Qt::NoModifier, table->visualRect(table->model()->index(1, 0)).center());
    QTRY_VERIFY2(uwf::test::clippedText(dialog).isEmpty(), qUtf8Printable(uwf::test::clippedText(dialog)));
    auto* selectedPath = dialog.findChild<QLabel*>(QStringLiteral("registrySelectionPath"));
    auto* selectedScope = dialog.findChild<QLabel*>(QStringLiteral("registrySelectionScope"));
    QVERIFY(selectedPath && selectedScope);
    QCOMPARE(selectedPath->text(), QString::fromStdString(longKey));
    QCOMPARE(selectedPath->toolTip(), QString::fromStdString(longKey));
    QVERIFY(selectedScope->text().contains(QString::fromStdString(longValue)));
    inspect(dialog, QStringLiteral("registry-picker-long-selection"));
    table->selectRow(2);
    QVERIFY(selectedScope->text().contains(QString::fromStdString(veryLongValue)));
    QVERIFY(selectedScope->toolTip().contains(QString::fromStdString(veryLongValue)));
    inspect(dialog, QStringLiteral("registry-picker-long-value"));
    QTimer::singleShot(0, this, [] {
      if (auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget())) {
        menu->actions().constFirst()->trigger();
        menu->close();
      }
    });
    QContextMenuEvent copySelection(QContextMenuEvent::Mouse, selectedScope->rect().center(), selectedScope->mapToGlobal(selectedScope->rect().center()));
    QApplication::sendEvent(selectedScope, &copySelection);
    QCOMPARE(QApplication::clipboard()->text(), QString::fromStdString(longKey) + u'\n' + QString::fromStdString(veryLongValue));
    auto* ok = buttonByRole(&dialog, QDialogButtonBox::AcceptRole);
    QVERIFY(ok && ok->isEnabled());
    QTest::mouseClick(ok, Qt::LeftButton);
    QVERIFY(dialog.selection().has_value());
    QCOMPARE(dialog.selection()->key, QString::fromStdString(longKey));
    QCOMPARE(dialog.selection()->valueName, QString::fromStdString(veryLongValue));
  }
  {
    FileDialogStub files;
    OverlayFilesDialog dialog(std::make_shared<uwf::presentation::OverlayFilesViewModel>(
                                  QStringLiteral("C:"),
                                  [](const QString&, std::stop_token) {
                                    QVector<uwf::app::OverlayFileEntry> entries;
                                    for (int i = 0; i < 100; ++i)
                                      entries.append({QStringLiteral("\\Application Data\\用户数据\\document-%1.txt:$DATA").arg(i), {}, false, false, 4096});
                                    return entries;
                                  }),
                              files);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* list = dialog.findChild<QAbstractItemView*>();
    QVERIFY(list);
    QTRY_VERIFY_WITH_TIMEOUT(list->model()->rowCount() > 0, 2000);
    inspect(dialog, QStringLiteral("overlay-files"));
    QVERIFY(list->viewport()->width() > 0 && list->viewport()->height() > 0);
    QVERIFY(files.requests.isEmpty());
  }
  for (const auto operation : {uwf::app::CommitOperation::Persist, uwf::app::CommitOperation::DeleteAndPersist}) {
    bool inspected = false;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!dialog) return;
      inspect(*dialog, operation == uwf::app::CommitOperation::Persist ? QStringLiteral("commit-report") : QStringLiteral("deletion-report"), true);
      inspected = true;
      dialog->reject();
    });
    const QList<uwf::presentation::CommitReportRow> rows{
        {uwf::presentation::CommitRowStatus::Succeeded, QStringLiteral("C:\\Application Data\\用户数据\\Saved State.json"), QStringLiteral("-"),
         QStringLiteral("-"), true, false},
        {uwf::presentation::CommitRowStatus::Failed, QStringLiteral("C:\\Application Data\\用户数据\\Locked State.json"), QStringLiteral("0x80041003"),
         QStringLiteral("Mock access denied; no system operation was performed."), true, true}};
    showCommitReport(nullptr, rows, operation);
    QVERIFY(inspected);
  }
}

}  // namespace

QTEST_MAIN(DialogBehaviorTests)

#include "DialogBehaviorTests.moc"
