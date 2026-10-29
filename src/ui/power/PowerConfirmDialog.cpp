// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PowerConfirmDialog.h"

#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLocale>
#include <QProgressBar>
#include <QPushButton>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QVBoxLayout>
#include <algorithm>
#include <limits>
#include <utility>

#include "presentation/power/PowerDialogViewModel.h"
#include "presentation/staging/FileStagingResultText.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/DesignSystem.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/dialogs/PanelDialog.h"
#include "ui/widgets/BusyIndicator.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/Notice.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TextLabel.h"
#include "ui/widgets/VerticalScrollArea.h"

namespace uwf::ui {
namespace {
int progressValue(const std::size_t value) { return static_cast<int>(std::min(value, static_cast<std::size_t>(std::numeric_limits<int>::max()))); }
QString countText(const std::size_t value) { return QLocale(QLocale::English).toString(static_cast<qulonglong>(value)); }
void renderStagingFailureDetails(ContentHintBox& details, TextLabel& summary, TextLabel& remaining, const app::PowerStagingProgress& status) {
  summary.hide();
  remaining.hide();
  if (!status.failureResult || status.failureResult->succeeded()) {
    details.setPlainText(status.failureDetails.isEmpty() ? I18n::tr("No additional error details were provided.") : status.failureDetails);
    return;
  }

  const auto view = presentation::fileStagingResultText(*status.failureResult);
  summary.setText(view.summary);
  summary.show();
  remaining.setText(view.remainingFailures);
  remaining.setVisible(!view.remainingFailures.isEmpty());
  details.clear();
  QTextCursor cursor(details.document());
  cursor.beginEditBlock();
  bool first = true;
  const auto addParagraph = [&](const QString& text, const int gap, const bool path) {
    QTextBlockFormat block;
    block.setTopMargin(gap);
    block.setBottomMargin(0);
    QTextCharFormat character;
    auto font = DesignSystem::defaultFont();
    font.setPixelSize(12);
    if (path) font.setFamilies(DesignSystem::monospaceFamilies());
    character.setFont(font);
    character.setForeground(DesignSystem::instance().color(path ? design::Color::Text : design::Color::Muted));
    if (first) {
      cursor.setBlockFormat(block);
      cursor.setCharFormat(character);
      first = false;
    } else {
      cursor.insertBlock(block, character);
    }
    cursor.insertText(text);
  };
  for (const auto& failure : view.failures) {
    addParagraph(failure.target, first ? 0 : 12, true);
    addParagraph(failure.reason, 4, false);
  }
  cursor.endEditBlock();
  cursor.movePosition(QTextCursor::Start);
  details.setTextCursor(cursor);
}

bool confirmDirectPower(QWidget* parent, const bool shutdown) {
  return dialogs::confirm(parent, {shutdown ? I18n::tr("Direct shutdown?") : I18n::tr("Direct restart?"),
                                   shutdown ? I18n::tr("File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.")
                                            : I18n::tr("File staging will be skipped. Uncommitted changes on protected paths will be lost after restart."),
                                   shutdown ? I18n::tr("Direct shutdown") : I18n::tr("Direct restart"), DialogAction::Destructive});
}
class PowerWarningNote final : public QWidget {
 public:
  explicit PowerWarningNote(const QString& text, QWidget* parent)
      : QWidget(parent), m_iconHost(new QWidget(this)), m_text(new TextLabel(text, TextStyle::Caption, this)) {
    setObjectName(QStringLiteral("powerWarningNote"));
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(9);
    m_iconHost->setFixedWidth(17);
    m_icon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/status_warning.svg")), 17, design::Color::Muted, m_iconHost);
    m_icon->setObjectName(QStringLiteral("powerWarningIcon"));
    m_text->setObjectName(QStringLiteral("powerWarningText"));
    m_text->setColorRole(design::Color::Muted);
    m_text->setWordWrap(true);
    m_text->setLineHeight(1.7);
    m_text->setAlignment(Qt::AlignLeading | Qt::AlignTop);
    row->addWidget(m_iconHost);
    row->addWidget(m_text, 1);
    m_text->installEventFilter(this);
  }
  TextLabel* textLabel() const { return m_text; }
  IconView* iconView() const { return m_icon; }

 protected:
  bool event(QEvent* event) override {
    const bool handled = QWidget::event(event);
    if (event->type() == QEvent::LayoutRequest) alignIcon();
    return handled;
  }
  bool eventFilter(QObject* object, QEvent* event) override {
    if (object == m_text && (event->type() == QEvent::Resize || event->type() == QEvent::Show || event->type() == QEvent::FontChange)) alignIcon();
    return QWidget::eventFilter(object, event);
  }

 private:
  void alignIcon() {
    const auto line = m_text->firstLineBoxBounds();
    m_icon->move(0, qRound(m_text->y() - m_iconHost->y() + line.center().y() - m_icon->height() / 2.0));
  }
  QWidget* m_iconHost;
  IconView* m_icon;
  TextLabel* m_text;
};
}  // namespace

app::PowerActionDialogOutcome runPowerActionDialog(QWidget* parent, app::PowerActionDialogRequest request) {
  const bool shutdown = request.action == app::PowerAction::Shutdown;
  const bool direct = request.mode == app::PowerActionDialogMode::Direct;
  presentation::PowerDialogViewModel viewModel(std::move(request));
  const bool preparing = viewModel.preparationRequired();
  const auto safeText = shutdown ? I18n::tr("Safe shutdown") : I18n::tr("Safe restart");
  const auto directText = shutdown ? I18n::tr("Direct shutdown") : I18n::tr("Direct restart");
  if (direct) {
    viewModel.start();
    if (confirmDirectPower(parent, shutdown)) viewModel.confirmDirect();
    return viewModel.finish(false);
  }
  const auto icon = ThemeManager::instance().icon(shutdown ? QStringLiteral(":/icons/shutdown.svg") : QStringLiteral(":/icons/restart.svg"));
  PanelDialog dialog(parent);
  dialog.setObjectName(QStringLiteral("powerConfirmDialog"));
  dialog.setWindowTitle(direct ? directText : safeText);
  dialog.setHeaderStyle(DialogWindow::HeaderStyle::Power);
  dialog.setHeaderIcon(icon);
  dialog.setDescription(direct     ? I18n::tr("Files in File staging will not be saved.")
                        : shutdown ? I18n::tr("Save staged files to disk, then shut down.")
                                   : I18n::tr("Save staged files to disk, then restart."));
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(588, 500), QSize(480, 0));
  frame->scrollArea()->setObjectName(QStringLiteral("powerConfirmationScroll"));
  auto* layout = frame->body();
  layout->setContentsMargins(32, 0, 32, 24);
  layout->setSpacing(18);
  auto* card = new Surface(design::Color::Surface, &dialog);
  card->setObjectName(QStringLiteral("powerSafetyCard"));
  card->setMinimumHeight(132);
  card->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  auto* summary = new QVBoxLayout(card);
  summary->setContentsMargins(19, 17, 19, 17);
  summary->setSpacing(0);
  auto* heading = new QHBoxLayout();
  heading->setSpacing(9);
  auto* summaryIcon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/file_staging.svg")), 18, design::Color::Muted, card);
  auto* summaryTitle = new TextLabel(I18n::tr("File staging"), TextStyle::Label, card);
  auto titleFont = summaryTitle->font();
  titleFont.setWeight(QFont::Medium);
  summaryTitle->setFont(titleFont);
  heading->addWidget(summaryIcon);
  heading->addWidget(summaryTitle, 1);
  summary->addLayout(heading);
  summary->addSpacing(12);
  auto* totalsHost = new QWidget(card);
  totalsHost->setFixedHeight(42);
  auto* totals = new QHBoxLayout(totalsHost);
  totals->setContentsMargins(0, 0, 0, 0);
  totals->setSpacing(8);
  auto* spinner = new BusyIndicator(card);
  spinner->setFixedSize(20, 20);
  totals->addWidget(spinner, 0, Qt::AlignVCenter);
  auto* total = new TextLabel({}, TextStyle::Title, card);
  total->setObjectName(QStringLiteral("powerFileCount"));
  auto* units = new TextLabel(I18n::tr("staged files"), TextStyle::Caption, card);
  units->setColorRole(design::Color::Muted);
  totals->addWidget(total, 0, Qt::AlignVCenter);
  totals->addWidget(units, 0, Qt::AlignBaseline);
  totals->addStretch();
  auto* percentage = new TextLabel({}, TextStyle::Caption, card);
  percentage->setColorRole(design::Color::Muted);
  totals->addWidget(percentage);
  summary->addWidget(totalsHost);
  summary->addSpacing(3);
  auto* detail = new TextLabel({}, TextStyle::Caption, card);
  detail->setObjectName(QStringLiteral("powerStagingDetail"));
  detail->setColorRole(design::Color::Muted);
  detail->setWordWrap(true);
  summary->addWidget(detail);
  auto* progressArea = new QWidget(card);
  auto* progressLayout = new QVBoxLayout(progressArea);
  progressLayout->setContentsMargins(0, 18, 0, 0);
  progressLayout->setSpacing(12);
  auto* progress = new QProgressBar(progressArea);
  progress->setObjectName(QStringLiteral("powerStagingProgress"));
  progress->setTextVisible(false);
  progress->setFixedHeight(4);
  progressLayout->addWidget(progress);
  auto* currentPath = new QLineEdit(progressArea);
  currentPath->setObjectName(QStringLiteral("powerStagingCurrentPath"));
  currentPath->setLayoutDirection(Qt::LeftToRight);
  currentPath->setReadOnly(true);
  currentPath->setFrame(false);
  currentPath->setProperty("dialogPath", true);
  auto pathFont = currentPath->font();
  pathFont.setFamilies(DesignSystem::monospaceFamilies());
  pathFont.setPixelSize(11);
  currentPath->setFont(pathFont);
  progressLayout->addWidget(currentPath);
  summary->addWidget(progressArea);
  auto* failureDisclosure = new Disclosure(I18n::tr("View failure details"), card);
  failureDisclosure->setObjectName(QStringLiteral("powerFailureDisclosure"));
  auto* failureHost = new QWidget(failureDisclosure);
  auto* failureLayout = new QVBoxLayout(failureHost);
  failureLayout->setContentsMargins(0, 12, 0, 0);
  auto* failureSurface = new Surface(design::Color::Surface, failureHost);
  failureSurface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 5);
  auto* failureTextLayout = new QVBoxLayout(failureSurface);
  failureTextLayout->setContentsMargins(16, 10, 16, 10);
  failureTextLayout->setSpacing(10);
  auto* failureSummary = new TextLabel({}, TextStyle::Caption, failureSurface);
  failureSummary->setColorRole(design::Color::Muted);
  failureSummary->setWordWrap(true);
  failureSummary->setLineHeight(1.65);
  failureTextLayout->addWidget(failureSummary);
  auto* failureDetails = new ContentHintBox(failureSurface);
  failureDetails->setObjectName(QStringLiteral("powerStagingFailureDetails"));
  failureDetails->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  failureDetails->setStyleSheet(QStringLiteral("QTextBrowser { background: transparent; border: 0; padding: 0; }"));
  auto failureFont = failureDetails->font();
  failureFont.setFamilies(DesignSystem::monospaceFamilies());
  failureFont.setPixelSize(12);
  failureDetails->setFont(failureFont);
  failureDetails->setLineHeight(20);
  // Keep the complete wrapped document in the body; DialogLayout owns vertical overflow.
  failureTextLayout->addWidget(failureDetails);
  auto* remainingFailures = new TextLabel({}, TextStyle::Caption, failureSurface);
  remainingFailures->setColorRole(design::Color::Muted);
  remainingFailures->setWordWrap(true);
  remainingFailures->setLineHeight(1.65);
  failureTextLayout->addWidget(remainingFailures);
  failureLayout->addWidget(failureSurface);
  failureDisclosure->setContent(failureHost);
  summary->addWidget(failureDisclosure);
  layout->addWidget(card);
  auto* note = new PowerWarningNote(
      shutdown ? I18n::tr("Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.")
               : I18n::tr("Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart."),
      &dialog);
  auto* noteText = note->textLabel();
  auto* noteIcon = note->iconView();
  layout->addWidget(note);
  auto* failureNotice = new Notice(&dialog);
  failureNotice->setTone(Notice::Tone::Warning);
  layout->addWidget(failureNotice);
  failureNotice->hide();
  layout->addStretch();

  auto* buttons = new QDialogButtonBox(&dialog);
  auto* actionButton = new QPushButton(direct ? directText : safeText, &dialog);
  actionButton->setIcon(icon);
  actionButton->setObjectName(direct ? (shutdown ? QStringLiteral("directShutdownBtn") : QStringLiteral("directRestartBtn"))
                                     : (shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn")));
  actionButton->setProperty("designPaddingX", 16);
  setDialogAction(actionButton, direct ? DialogAction::DestructiveSecondary : DialogAction::Primary);
  buttons->addButton(actionButton, QDialogButtonBox::AcceptRole);
  auto* cancelButton = buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  auto* leading = new QDialogButtonBox(&dialog);
  QPushButton* directButton = nullptr;
  if (!direct) {
    directButton = leading->addButton(directText + QChar(0x2026), QDialogButtonBox::ActionRole);
    directButton->setObjectName(shutdown ? QStringLiteral("directShutdownBtn") : QStringLiteral("directRestartBtn"));
    directButton->setProperty("designRole", "dangerQuiet");
    directButton->setProperty("designPaddingX", 5);
    auto font = directButton->font();
    font.setPixelSize(12);
    directButton->setFont(font);
    directButton->setAutoDefault(false);
    directButton->setToolTip(shutdown ? I18n::tr("Direct shutdown skips File staging commits.") : I18n::tr("Direct restart skips File staging commits."));
    QObject::connect(directButton, &QPushButton::clicked, &dialog, [&] {
      viewModel.beginDirectConfirmation();
      if (confirmDirectPower(&dialog, shutdown))
        viewModel.confirmDirect();
      else
        viewModel.cancelDirectConfirmation();
    });
  }
  auto* leadingHost = new QWidget(&dialog);
  auto* leadingLayout = new QHBoxLayout(leadingHost);
  leadingLayout->setContentsMargins(0, 0, 0, 0);
  leadingLayout->setSpacing(8);
  leadingLayout->addWidget(leading);
  auto* footerInfo = new QWidget(leadingHost);
  auto* footerInfoLayout = new QHBoxLayout(footerInfo);
  footerInfoLayout->setContentsMargins(0, 0, 0, 0);
  footerInfoLayout->setSpacing(8);
  auto* footerIcon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")), 14, design::Color::Muted, footerInfo);
  auto* footerText = new TextLabel({}, TextStyle::Caption, footerInfo);
  footerText->setColorRole(design::Color::Muted);
  footerInfoLayout->addWidget(footerIcon);
  footerInfoLayout->addWidget(footerText);
  leadingLayout->addWidget(footerInfo);
  footerInfo->hide();
  frame->setActions(buttons, leadingHost);
  actionButton->setAutoDefault(false);
  actionButton->setEnabled(!preparing);
  cancelButton->setDefault(true);
  cancelButton->setFocus(Qt::OtherFocusReason);
  QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
  QObject::connect(actionButton, &QPushButton::clicked, &viewModel, &presentation::PowerDialogViewModel::confirm);
  QObject::connect(&viewModel, &presentation::PowerDialogViewModel::accepted, &dialog, &QDialog::accept);

  const auto setTotal = [&](const QString& value, const int size) {
    total->setText(value);
    auto font = total->font();
    font.setPixelSize(size);
    font.setWeight(size <= 16 ? QFont::Normal : QFont::DemiBold);
    font.setLetterSpacing(QFont::AbsoluteSpacing, size == 30 ? -.6 : 0);
    total->setFont(font);
  };
  const auto showPrepared = [&] {
    total->setColorRole(design::Color::Text);
    spinner->hide();
    progress->hide();
    progressArea->hide();
    percentage->hide();
    currentPath->hide();
    failureDisclosure->hide();
    const bool hasFiles = viewModel.hasStaging() && viewModel.filesToCommit() > 0;
    summaryTitle->setText(hasFiles ? I18n::tr("Ready to save") : I18n::tr("File staging"));
    summaryIcon->setIcon(ThemeManager::instance().icon(hasFiles ? QStringLiteral(":/icons/file_staging.svg") : QStringLiteral(":/icons/status_success.svg")));
    setTotal(hasFiles ? countText(viewModel.filesToCommit()) : I18n::tr("No files to commit"), hasFiles ? 30 : 20);
    units->setVisible(hasFiles);
    detail->setText(hasFiles   ? I18n::tr("Large staging lists may take longer to save.")
                    : shutdown ? I18n::tr("UWF safe shutdown will run directly.")
                               : I18n::tr("UWF safe restart will run directly."));
    actionButton->setEnabled(true);
  };
  QObject::connect(&viewModel, &presentation::PowerDialogViewModel::preparationReady, &dialog, showPrepared);
  QObject::connect(&viewModel, &presentation::PowerDialogViewModel::progressChanged, &dialog, [&](const app::PowerStagingProgress& status) {
    progress->setMaximum(std::max(1, progressValue(status.totalFiles)));
    progress->setValue(progressValue(status.processedFiles));
    setTotal(countText(status.processedFiles), 30);
    units->setText(I18n::tr("/ %1 files").arg(countText(status.totalFiles)));
    percentage->setText(
        QString::number(status.totalFiles
                            ? std::clamp(static_cast<int>(100.0 * static_cast<double>(status.processedFiles) / static_cast<double>(status.totalFiles)), 0, 100)
                            : 0) +
        u'%');
    if (!status.currentPath.isEmpty()) {
      currentPath->setText(status.currentPath);
      currentPath->setToolTip(status.currentPath);
      currentPath->setCursorPosition(0);
      currentPath->show();
    }
  });
  QObject::connect(&viewModel, &presentation::PowerDialogViewModel::stagingBegan, &dialog, [&] {
    leading->hide();
    footerIcon->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")));
    footerText->setText(I18n::tr("Committing"));
    footerInfo->show();
    dialog.setWindowTitle(I18n::tr("Saving staged files"));
    dialog.setDescription(shutdown ? I18n::tr("Shutdown will continue after saving. Keep the application open.")
                                   : I18n::tr("Restart will continue after saving. Keep the application open."));
    summaryTitle->setText(I18n::tr("Saving to disk"));
    setTotal(QStringLiteral("0"), 30);
    units->setText(I18n::tr("/ %1 files").arg(countText(viewModel.filesToCommit())));
    units->show();
    detail->hide();
    progress->setRange(0, std::max(1, progressValue(viewModel.filesToCommit())));
    progress->setValue(0);
    progress->show();
    progressArea->show();
    percentage->show();
    actionButton->setText(I18n::tr("Saving…"));
    actionButton->setIcon({});
    actionButton->setEnabled(false);
    if (directButton) {
      directButton->setEnabled(false);
      directButton->hide();
    }
    dialog.setRejectionEnabled(false);
    noteText->setText(I18n::tr("File saving has started and cannot be canceled."));
    noteIcon->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")));
  });
  QObject::connect(&viewModel, &presentation::PowerDialogViewModel::failureObserved, &dialog, [&](const app::PowerStagingProgress& status) {
    const bool canContinue = viewModel.canContinueAfterStagingFailure();
    layout->setSpacing(12);
    summary->setContentsMargins(19, 12, 19, 12);
    leading->hide();
    footerIcon->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/status_warning.svg")));
    footerText->setText(I18n::tr("Confirm before continuing"));
    footerInfo->setVisible(canContinue);
    dialog.setWindowTitle(canContinue ? I18n::tr("Some files could not be saved") : I18n::tr("Automatic saving incomplete"));
    dialog.setDescription(canContinue ? (shutdown ? I18n::tr("Review the failure details before continuing shutdown.")
                                                  : I18n::tr("Review the failure details before restarting."))
                                      : I18n::tr("File staging stopped. Resolve the error before trying again."));
    summaryTitle->setText(I18n::tr("File saving incomplete"));
    dialog.setHeaderIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/status_warning.svg")));
    summaryIcon->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/status_warning.svg")));
    spinner->hide();
    setTotal(I18n::tr("Automatic saving incomplete"), 20);
    totalsHost->hide();
    total->show();
    units->hide();
    detail->hide();
    percentage->hide();
    progress->hide();
    progressArea->hide();
    currentPath->hide();
    renderStagingFailureDetails(*failureDetails, *failureSummary, *remainingFailures, status);
    failureDisclosure->show();
    failureDisclosure->setOpen(!canContinue);
    noteText->setText(shutdown ? I18n::tr("Continuing shutdown will discard changes in files that could not be saved.")
                               : I18n::tr("Continuing restart will discard changes in files that could not be saved."));
    failureNotice->setText(noteText->text());
    failureNotice->setVisible(canContinue);
    note->hide();
    actionButton->setText(shutdown ? I18n::tr("Shut down anyway") : I18n::tr("Restart anyway"));
    setDialogAction(actionButton, DialogAction::DestructiveSecondary);
    actionButton->setEnabled(canContinue);
    actionButton->setVisible(canContinue);
    if (directButton) {
      directButton->setEnabled(false);
      directButton->hide();
    }
    dialog.setRejectionEnabled(true);
    cancelButton->setText(canContinue ? I18n::tr("Cancel") : I18n::tr("Close"));
    cancelButton->setDefault(true);
    cancelButton->setFocus(Qt::OtherFocusReason);
  });
  progress->hide();
  progressArea->hide();
  currentPath->hide();
  failureDisclosure->hide();
  percentage->hide();
  if (preparing) {
    total->setColorRole(design::Color::Muted);
    setTotal(I18n::tr("Counting staged files…"), 15);
    units->hide();
    detail->setText(I18n::tr("Scanning staged paths"));
  } else {
    showPrepared();
  }
  if (direct) {
    summaryTitle->setText(I18n::tr("File staging will be skipped"));
    setTotal(shutdown ? I18n::tr("Direct shutdown") : I18n::tr("Direct restart"), 20);
    detail->setText(I18n::tr("Files in File staging will not be saved."));
  }
  viewModel.start();
  return viewModel.finish(dialog.exec() == QDialog::Accepted);
}
}  // namespace uwf::ui
