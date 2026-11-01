// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/review/ApplyPlanDialog.h"

#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>

#include "application/configuration/planning/ConfigurationScript.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/BusyIndicator.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/feedback/ContentState.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/feedback/StatusBadge.h"
#include "presentation/controls/inputs/CodeEditor.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/layout/VerticalScrollArea.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/tables/RecyclingListView.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/dialogs/InformationDialog.h"
#include "presentation/views/configuration/review/ApplyPlanList.h"

namespace uwf::ui {
namespace {
// Confirmation and execution share one modal surface. The worker starts at once;
// only presentation completion is deferred to avoid a flashing progress state.
class ApplyExecutionConfirmation final : public InformationDialog {
 public:
  ApplyExecutionConfirmation(std::shared_ptr<presentation::ApplyPlanViewModel> model, QString confirmation, QWidget* parent)
      : InformationDialog(parent), m_model(std::move(model)) {
    setObjectName(QStringLiteral("applyExecutionConfirmation"));
    setWindowTitle(I18n::tr("Confirm apply"));
    setHeaderStyle(HeaderStyle::Message);
    auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(design::messageWidth, 0));
    m_pages = new QStackedWidget(this);
    auto* message = new ContentHintBox(m_pages);
    message->setObjectName(QStringLiteral("applyConfirmationMessage"));
    message->setPlainText(confirmation);
    message->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    auto font = message->font();
    font.setPixelSize(13);
    message->setFont(font);
    message->setLineHeight(13 * 1.9);
    m_pages->addWidget(message);

    auto* busy = new Surface(design::Color::SurfaceSoft, m_pages);
    busy->setObjectName(QStringLiteral("applyExecutionProgress"));
    busy->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
    auto* row = new QHBoxLayout(busy);
    row->setContentsMargins(20, 20, 20, 20);
    row->setSpacing(16);
    m_spinner = new BusyIndicator(busy);
    m_spinner->setObjectName(QStringLiteral("applyExecutionSpinner"));
    m_spinner->setFixedSize(40, 40);
#if QT_CONFIG(accessibility)
    m_spinner->setAccessibleName(I18n::tr("Applying changes"));
#endif
    row->addWidget(m_spinner, 0, Qt::AlignTop);
    auto* copy = new QVBoxLayout;
    copy->setContentsMargins(0, 0, 0, 0);
    copy->setSpacing(6);
    auto* heading = new TextLabel(I18n::tr("Applying changes"), TextStyle::Label, busy);
    heading->setObjectName(QStringLiteral("applyExecutionHeading"));
    copy->addWidget(heading);
    auto* detail = new TextLabel(I18n::tr("Waiting for Windows to finish. This may take a while."), TextStyle::Caption, busy);
    detail->setColorRole(design::Color::Muted);
    detail->setWordWrap(true);
    copy->addWidget(detail);
    m_elapsedLabel = new TextLabel({}, TextStyle::Eyebrow, busy);
    m_elapsedLabel->setObjectName(QStringLiteral("applyExecutionElapsed"));
    m_elapsedLabel->setColorRole(design::Color::Faint);
    copy->addWidget(m_elapsedLabel);
    row->addLayout(copy, 1);
    m_pages->addWidget(busy);
    frame->body()->addWidget(m_pages);

    auto* buttons = new QDialogButtonBox(this);
    m_applyButton = buttons->addButton(I18n::tr("Apply changes"), QDialogButtonBox::AcceptRole);
    m_applyButton->setObjectName(QStringLiteral("confirmApplyButton"));
    setDialogAction(m_applyButton, DialogAction::Primary);
    m_applyButton->setAutoDefault(false);
    connect(m_applyButton, &QPushButton::clicked, this, &ApplyExecutionConfirmation::accept);
    m_cancelButton = buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    m_cancelButton->setDefault(true);
    m_cancelButton->setFocus();
    frame->setActions(buttons);

    m_elapsedTimer.setInterval(100);
    connect(&m_elapsedTimer, &QTimer::timeout, this, [this] { updateElapsed(); });
    m_finishTimer.setSingleShot(true);
    m_finishTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_finishTimer, &QTimer::timeout, this, [this] { finishPresentation(); });
    connect(m_model.get(), &presentation::ApplyPlanViewModel::availabilityChanged, this, [this] {
      if (!m_started) m_applyButton->setEnabled(m_model->canApply());
    });
    connect(m_model.get(), &presentation::ApplyPlanViewModel::resultChanged, this, [this] {
      if (!m_started || m_operationFinished) return;
      m_operationFinished = true;
      finishPresentation();
    });
  }

  void accept() override {
    if (m_started || !m_model->canApply()) return;
    m_started = true;
    m_elapsed.start();
    setRejectionEnabled(false);
    m_cancelButton->hide();
    m_applyButton->setEnabled(false);
    m_applyButton->setText(I18n::tr("Applying changes"));
    m_pages->setCurrentIndex(1);
    updateElapsed();
    m_elapsedTimer.start();
    const QPointer self(this);
    const auto model = m_model;
    model->apply();
    // A synchronous admission callback may destroy the owner or invalidate the
    // plan. Neither case may leave a progress window without an operation.
    if (self && !m_operationFinished && !model->applying()) {
      setRejectionEnabled(true);
      reject();
    }
  }

 private:
  void updateElapsed() { m_elapsedLabel->setText(I18n::tr("Elapsed: %1 s").arg(m_elapsed.elapsed() / 1000)); }
  void finishPresentation() {
    if (!m_operationFinished) return;
    constexpr qint64 minimumBusyMs = 1000;
    const auto remaining = minimumBusyMs - m_elapsed.elapsed();
    if (remaining > 0) {
      m_finishTimer.start(static_cast<int>(remaining));
      return;
    }
    m_elapsedTimer.stop();
    QDialog::accept();
  }

  std::shared_ptr<presentation::ApplyPlanViewModel> m_model;
  QStackedWidget* m_pages = nullptr;
  BusyIndicator* m_spinner = nullptr;
  TextLabel* m_elapsedLabel = nullptr;
  QPushButton* m_applyButton = nullptr;
  QPushButton* m_cancelButton = nullptr;
  QElapsedTimer m_elapsed;
  QTimer m_elapsedTimer;
  QTimer m_finishTimer;
  bool m_started = false;
  bool m_operationFinished = false;
};

CodeEditor* commandDocument(const QString& text, QWidget* parent) {
  auto* editor = new CodeEditor(parent);
  editor->setReadOnly(true);
  editor->setLineNumbersVisible(false);
  editor->setLineHeight(23.4);
  editor->setContentMargins(QMargins(18, 14, 18, 14));
  auto type = editor->font();
  type.setPixelSize(12);
  editor->setFont(type);
  editor->setPlainText(text);
  return editor;
}
}  // namespace

ApplyPlanDialog::ApplyPlanDialog(std::shared_ptr<presentation::ApplyPlanViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent)
    : PanelDialog(parent), m_fileDialogs(fileDialogs), m_viewModel(std::move(model)) {
  setWindowTitle(I18n::tr("Review and apply changes"));
  setDescription(I18n::tr("Review these changes before writing them to the system."));
  auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(design::panelWidth, design::panelHeight), QSize(720, 500));
  frame->setDividerColor(design::Color::LineStrong);
  auto* tabs = new TabView(this);
  tabs->setObjectName(QStringLiteral("reviewTabs"));
  auto* pending = new QWidget(tabs);
  auto* layout = new QVBoxLayout(pending);
  layout->setContentsMargins(0, 22, 0, 0);
  layout->setSpacing(16);
  auto* summary = new QWidget(pending);
  auto* summaryLayout = new QHBoxLayout(summary);
  summaryLayout->setContentsMargins(0, 0, 0, 0);
  auto* count = new TextLabel(I18n::tr("%1 changes").arg(m_viewModel->changeCount()), TextStyle::Label, summary);
  auto countFont = count->font();
  countFont.setPixelSize(15);
  countFont.setWeight(QFont::DemiBold);
  count->setFont(countFont);
  summaryLayout->addWidget(count, 1);
  auto* badge = new StatusBadge(summary);
  badge->setObjectName(QStringLiteral("applyPlanSummaryBadge"));
  badge->setText(m_viewModel->activationText());
  badge->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")));
  summaryLayout->addWidget(badge);
  layout->addWidget(summary);
  if (!m_viewModel->warning().isEmpty()) {
    auto* warning = new Notice(pending);
    warning->setTone(Notice::Tone::Warning);
    warning->setText(m_viewModel->warning());
    layout->addWidget(warning);
  }
  auto* feedback = new Notice(pending);
  feedback->setObjectName(QStringLiteral("applyResultNotice"));
  feedback->hide();
  layout->addWidget(feedback);
  auto* changes = new QWidget(pending);
  auto* groups = new QVBoxLayout(changes);
  groups->setContentsMargins(0, 0, 0, 0);
  groups->setSpacing(16);
  auto* commands = new Disclosure(I18n::tr("View commands for these changes"), changes);
  commands->setObjectName(QStringLiteral("changeCommandsDisclosure"));
  commands->setCount(static_cast<int>(m_viewModel->changeCommandCount()));
  auto* commandText = commandDocument({}, commands);
  commandText->setObjectName(QStringLiteral("changeCommandPreview"));
  commandText->setMinimumHeight(150);
  auto* commandContent = new QWidget(commands);
  auto* commandLayout = new QVBoxLayout(commandContent);
  commandLayout->setContentsMargins(0, 12, 0, 0);
  commandLayout->setSpacing(12);
  auto* commandSummary = new TextLabel(I18n::tr("%1 changes · %2 commands").arg(m_viewModel->changeCount()).arg(m_viewModel->changeCommandCount()),
                                       TextStyle::Eyebrow, commandContent);
  commandSummary->setObjectName(QStringLiteral("changeCommandSummary"));
  commandSummary->setColorRole(design::Color::Muted);
  commandLayout->addWidget(commandSummary);
  commandLayout->addWidget(commandText);
  commands->setContent(commandContent);
  connect(commands, &Disclosure::openChanged, commandText, [this, commandText, loaded = false](const bool open) mutable {
    if (!open || loaded) return;
    loaded = true;
    commandText->setPlainText(m_viewModel->changeCommandsText());
  });
  groups->addWidget(commands);
  auto* results = commandDocument({}, changes);
  results->setObjectName(QStringLiteral("applyResultDetails"));
  results->setMinimumHeight(160);
  results->hide();
  groups->addWidget(results);
  groups->addStretch();
  auto* scroll = createApplyPlanList(*m_viewModel, changes, pending);
  connect(commands, &Disclosure::openChanged, scroll, [scroll] { scroll->refreshRowSize(scroll->model()->index(scroll->model()->rowCount() - 1, 0)); });
  layout->addWidget(scroll, 1);
  if (m_viewModel->groups().isEmpty()) {
    summary->hide();
    scroll->hide();
    auto* empty = new ContentState(pending, ContentState::ScrollMode::HostManaged);
    empty->setText(I18n::tr("No pending changes"),
                   I18n::tr("The configuration matches the next-session target. View or export the supported settings in Command script."));
    empty->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/status_success.svg")));
    layout->addWidget(empty, 1);
  }
  tabs->addTab(pending, I18n::tr("Pending changes"));
  tabs->setTabCount(0, static_cast<int>(m_viewModel->changeCount()));
  auto* full = new QWidget(tabs);
  auto* fullLayout = new QVBoxLayout(full);
  fullLayout->setContentsMargins(0, 22, 0, 0);
  fullLayout->setSpacing(16);
  auto* target = new QHBoxLayout();
  target->setContentsMargins(0, 0, 0, 0);
  target->addWidget(new TextLabel(I18n::tr("Next-session configuration"), TextStyle::Caption, full), 1);
  auto* commandCount =
      new TextLabel(I18n::tr("%1 commands").arg(m_viewModel->exportCommands(presentation::ApplyPlanViewModel::ScriptScope::NextConfiguration).size()),
                    TextStyle::Caption, full);
  commandCount->setColorRole(design::Color::Faint);
  target->addWidget(commandCount);
  fullLayout->addLayout(target);
  auto* documentSurface = new Surface(design::Color::Surface, full);
  documentSurface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 5);
  documentSurface->setClipContent(true);
  auto* documentLayout = new QVBoxLayout(documentSurface);
  documentLayout->setContentsMargins(1, 1, 1, 1);
  auto* fullDocument = commandDocument({}, documentSurface);
  fullDocument->setObjectName(QStringLiteral("configurationCommandPreview"));
  documentLayout->addWidget(fullDocument);
  fullLayout->addWidget(documentSurface, 1);
  tabs->addTab(full, I18n::tr("Command script"));
  connect(tabs, &QTabWidget::currentChanged, fullDocument, [this, fullDocument, loaded = false](const int page) mutable {
    if (page != 1 || loaded) return;
    loaded = true;
    fullDocument->setPlainText(m_viewModel->targetCommandsText());
  });
  frame->body()->addWidget(tabs, 1);

  auto* buttons = new QDialogButtonBox(this);
  auto* commitBtn = buttons->addButton(I18n::tr("Apply changes"), QDialogButtonBox::AcceptRole);
  commitBtn->setObjectName(QStringLiteral("primaryBtn"));
  commitBtn->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/apply.svg")));
  setDialogAction(commitBtn, DialogAction::Primary);
  auto* restartBtn = buttons->addButton(I18n::tr("Safe restart"), QDialogButtonBox::ActionRole);
  restartBtn->setObjectName(QStringLiteral("restartBtn"));
  restartBtn->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/restart.svg")));
  setDialogAction(restartBtn, DialogAction::Primary);
  restartBtn->hide();
  auto* closeBtn = buttons->addButton(I18n::tr("Continue editing"), QDialogButtonBox::RejectRole);
  auto* leading = new QDialogButtonBox(this);
  auto* exportBtn = leading->addButton(I18n::tr("Export commands"), QDialogButtonBox::ActionRole);
  exportBtn->setObjectName(QStringLiteral("applyPlanExportButton"));
  const auto selectedScope = [tabs] {
    return tabs->currentIndex() == 0 ? presentation::ApplyPlanViewModel::ScriptScope::PendingChanges
                                     : presentation::ApplyPlanViewModel::ScriptScope::NextConfiguration;
  };
  const auto renderExport = [this, exportBtn, selectedScope] {
    exportBtn->setEnabled(m_viewModel->canExport(selectedScope()));
    exportBtn->setToolTip(m_viewModel->exportRestrictionText(selectedScope()));
  };
  connect(tabs, &QTabWidget::currentChanged, this, renderExport);
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::availabilityChanged, this, renderExport);
  renderExport();
  exportBtn->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/export.svg")));
  auto* directRestartBtn = leading->addButton(I18n::tr("Direct restart…"), QDialogButtonBox::ActionRole);
  directRestartBtn->setObjectName(QStringLiteral("directRestartBtn"));
  directRestartBtn->setProperty("designRole", "dangerQuiet");
  directRestartBtn->setToolTip(I18n::tr("Direct restart skips staged file commits."));
  directRestartBtn->hide();

  connect(exportBtn, &QPushButton::clicked, this, [this, selectedScope] {
    const auto scope = selectedScope();
    const QString suggested =
        QStringLiteral("uwfmgr-%1-%2.cmd")
            .arg(scope == presentation::ApplyPlanViewModel::ScriptScope::PendingChanges ? QStringLiteral("changes") : QStringLiteral("next-configuration"),
                 QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    const QString path =
        m_fileDialogs.saveFile(this, {I18n::tr("Export commands to file"), QDir::home().filePath(suggested), I18n::tr("Command files (*.cmd);;All files (*)")});
    if (path.isEmpty()) return;
    const auto error = m_viewModel->exportFile(path, scope);
    if (!error.isEmpty()) {
      dialogs::warning(this, I18n::tr("Export failed"), error);
      return;
    }
    dialogs::information(this, I18n::tr("Export finished"),
                         I18n::tr("Exported %1 commands to:\n%2").arg(m_viewModel->exportCommands(scope).size()).arg(QDir::toNativeSeparators(path)));
  });
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
  setRejectionEnabled(!m_viewModel->applying());
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::applyingChanged, this,
          [this] { setRejectionEnabled(!m_viewModel->applying() && !m_applyConfirmation); });
  const auto renderAvailability = [this, commitBtn, restartBtn, directRestartBtn] {
    commitBtn->setEnabled(m_viewModel->canApply());
    commitBtn->setVisible(m_viewModel->applyActionVisible());
    restartBtn->setVisible(m_viewModel->canRestart());
    restartBtn->setEnabled(m_viewModel->canSafeRestart());
    restartBtn->setToolTip(m_viewModel->safeRestartRestriction());
    directRestartBtn->setVisible(m_viewModel->canRestart());
    directRestartBtn->setEnabled(m_viewModel->canDirectRestart());
    directRestartBtn->setToolTip(m_viewModel->directRestartRestriction());
  };
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::availabilityChanged, this, renderAvailability);
  renderAvailability();
  connect(restartBtn, &QPushButton::clicked, this, &ApplyPlanDialog::safeRestartRequested);
  connect(directRestartBtn, &QPushButton::clicked, this, &ApplyPlanDialog::directRestartRequested);
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::reconciliationRequired, this, &ApplyPlanDialog::reconciliationRequired);
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::resultChanged, this,
          [this, tabs, summary, feedback, results, scroll, commitBtn, closeBtn, renderAvailability] {
            summary->hide();
            feedback->setHeading(m_viewModel->resultHeading());
            feedback->setTone(m_viewModel->resultFailed() ? Notice::Tone::Warning : Notice::Tone::Success);
            feedback->setText(m_viewModel->resultDescription());
            feedback->show();
            results->setPlainText(m_viewModel->resultText());
            results->show();
            scroll->refreshRowSize(scroll->model()->index(scroll->model()->rowCount() - 1, 0));
            tabs->setTabText(0, I18n::tr("Application result"));
            commitBtn->setText(I18n::tr("Retry"));
            closeBtn->setText(I18n::tr("Close"));
            renderAvailability();
          });
  connect(commitBtn, &QPushButton::clicked, this, [this] {
    if (m_applyConfirmation || !m_viewModel->canApply()) return;
    auto confirmation = I18n::tr("Write these configuration changes to the system?") + QStringLiteral("\n\n") + m_viewModel->activationText();
    if (m_viewModel->maintenanceChanges().fullCommit) confirmation += QStringLiteral("\n\n") + m_viewModel->warning();
    auto* execution = new ApplyExecutionConfirmation(m_viewModel, std::move(confirmation), this);
    execution->setAttribute(Qt::WA_DeleteOnClose);
    m_applyConfirmation = execution;
    setRejectionEnabled(false);
    connect(execution, &QDialog::finished, this, [this] {
      m_applyConfirmation.clear();
      setRejectionEnabled(!m_viewModel->applying());
    });
    execution->open();
  });
  frame->setActions(buttons, leading);
}
}  // namespace uwf::ui
#include "moc_ApplyPlanDialog.cpp"
