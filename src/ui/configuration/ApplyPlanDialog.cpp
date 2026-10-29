// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ApplyPlanDialog.h"

#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "ApplyPlanList.h"
#include "app/configuration/UwfmgrCommands.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/DesignSystem.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/widgets/CodeEditor.h"
#include "ui/widgets/ContentState.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/Notice.h"
#include "ui/widgets/RecyclingListView.h"
#include "ui/widgets/StatusBadge.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TabView.h"
#include "ui/widgets/TextLabel.h"
#include "ui/widgets/VerticalScrollArea.h"

namespace uwf::ui {
namespace {
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
  badge->setText(m_viewModel->hasStagingChanges() ? I18n::tr("File staging is saved after Apply") : I18n::tr("Takes effect after restart"));
  badge->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/restart.svg")));
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
  commandText->setMinimumHeight(150);
  commands->setContent(commandText);
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
                   I18n::tr("The configuration matches the next-session target. View or export commands in Full configuration."));
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
  auto* commandCount = new TextLabel(I18n::tr("%1 commands").arg(m_viewModel->exportCommands().size()), TextStyle::Caption, full);
  commandCount->setColorRole(design::Color::Faint);
  target->addWidget(commandCount);
  fullLayout->addLayout(target);
  auto* documentSurface = new Surface(design::Color::Surface, full);
  documentSurface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 5);
  documentSurface->setClipContent(true);
  auto* documentLayout = new QVBoxLayout(documentSurface);
  documentLayout->setContentsMargins(1, 1, 1, 1);
  auto* fullDocument = commandDocument({}, documentSurface);
  documentLayout->addWidget(fullDocument);
  fullLayout->addWidget(documentSurface, 1);
  auto* fullHint = new TextLabel(
      I18n::tr("Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script."), TextStyle::Caption, full);
  fullHint->setColorRole(design::Color::Muted);
  fullHint->setWordWrap(true);
  fullLayout->addWidget(fullHint);
  tabs->addTab(full, I18n::tr("Full configuration"));
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
  commitBtn->setEnabled(m_viewModel->canApply());
  auto* restartBtn = buttons->addButton(I18n::tr("Safe restart"), QDialogButtonBox::ActionRole);
  restartBtn->setObjectName(QStringLiteral("restartBtn"));
  restartBtn->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/restart.svg")));
  setDialogAction(restartBtn, DialogAction::Primary);
  restartBtn->hide();
  auto* closeBtn = buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  auto* leading = new QDialogButtonBox(this);
  auto* exportBtn = leading->addButton(I18n::tr("Export commands"), QDialogButtonBox::ActionRole);
  exportBtn->setEnabled(m_viewModel->canExport());
  if (m_viewModel->hasStagingChanges())
    exportBtn->setToolTip(I18n::tr("File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands."));
  exportBtn->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/export.svg")));
  auto* directRestartBtn = leading->addButton(I18n::tr("Direct restart…"), QDialogButtonBox::ActionRole);
  directRestartBtn->setObjectName(QStringLiteral("directRestartBtn"));
  directRestartBtn->setProperty("designRole", "dangerQuiet");
  directRestartBtn->setToolTip(I18n::tr("Direct restart skips File staging commits."));
  directRestartBtn->hide();

  connect(exportBtn, &QPushButton::clicked, this, [this] {
    const QString suggested = QStringLiteral("uwfmgr-commands-%1.cmd").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    const QString path =
        m_fileDialogs.saveFile(this, {I18n::tr("Export commands to file"), QDir::home().filePath(suggested), I18n::tr("Command files (*.cmd);;All files (*)")});
    if (path.isEmpty()) return;
    const auto error = m_viewModel->exportFile(path);
    if (!error.isEmpty()) {
      dialogs::warning(this, I18n::tr("Export failed"), error);
      return;
    }
    dialogs::information(this, I18n::tr("Export finished"),
                         I18n::tr("Exported %1 commands to:\n%2").arg(m_viewModel->exportCommands().size()).arg(QDir::toNativeSeparators(path)));
  });
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
  setRejectionEnabled(!m_viewModel->applying());
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::applyingChanged, this, [this] { setRejectionEnabled(!m_viewModel->applying()); });
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::availabilityChanged, this,
          [this, commitBtn] { commitBtn->setEnabled(m_viewModel->canApply()); });
  connect(restartBtn, &QPushButton::clicked, this, &ApplyPlanDialog::safeRestartRequested);
  connect(directRestartBtn, &QPushButton::clicked, this, &ApplyPlanDialog::directRestartRequested);
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::reconciliationRequired, this, &ApplyPlanDialog::reconciliationRequired);
  connect(m_viewModel.get(), &presentation::ApplyPlanViewModel::resultChanged, this,
          [this, tabs, summary, feedback, results, scroll, commitBtn, restartBtn, directRestartBtn, closeBtn] {
            summary->hide();
            feedback->setHeading(m_viewModel->resultHeading());
            feedback->setTone(m_viewModel->resultFailed() ? Notice::Tone::Warning : Notice::Tone::Success);
            feedback->setText(m_viewModel->resultDescription());
            feedback->show();
            results->setPlainText(m_viewModel->resultText());
            results->show();
            scroll->refreshRowSize(scroll->model()->index(scroll->model()->rowCount() - 1, 0));
            tabs->setTabText(0, I18n::tr("Application result"));
            restartBtn->setVisible(m_viewModel->canRestart());
            directRestartBtn->setVisible(m_viewModel->canRestart());
            commitBtn->setEnabled(m_viewModel->canApply());
            commitBtn->setVisible(m_viewModel->canApply());
            commitBtn->setText(I18n::tr("Retry"));
            closeBtn->setText(I18n::tr("Close"));
          });
  connect(commitBtn, &QPushButton::clicked, this, [this, commitBtn] {
    if (!dialogs::confirm(this,
                          {I18n::tr("Confirm apply"), I18n::tr("Write these configuration changes to the system? Most changes take effect after restart."),
                           I18n::tr("Apply changes"), DialogAction::Primary}))
      return;
    commitBtn->setEnabled(false);
    m_viewModel->apply();
  });
  frame->setActions(buttons, leading);
}
}  // namespace uwf::ui
#include "moc_ApplyPlanDialog.cpp"
