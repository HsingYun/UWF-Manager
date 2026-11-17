// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/importing/ImportDialog.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QStringList>
#include <QTabWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/clipboard/ClipboardText.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/inputs/CodeEditor.h"
#include "presentation/controls/layout/AlignedGraphic.h"
#include "presentation/controls/layout/FlowLayout.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/tables/ResponsiveTableView.h"
#include "presentation/controls/tables/TablePresentation.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/dialogs/InformationDialog.h"
#include "presentation/models/ItemRoles.h"

namespace uwf::ui {

namespace {

using RecommendedSection = presentation::ImportViewModel::RecommendedSection;

int chooseDefaultRuleSections(const QList<presentation::ImportViewModel::RecommendedGroup>& groups, QWidget* parent) {
  PanelDialog dlg(parent);
  dlg.setWindowTitle(I18n::tr("Choose recommended configuration"));
  auto* frame = new DialogLayout(&dlg, DialogContent::Scrollable, QSize(580, 0), QSize(400, 0));
  auto* layout = frame->body();

  auto* hint =
      new QLabel(I18n::tr("Select the recommended configuration groups to append. You can review or delete individual commands before importing."), &dlg);
  hint->setWordWrap(true);
  layout->addWidget(hint);

  auto* sourceHint = new QLabel(I18n::tr("From Microsoft official documentation."), &dlg);
  sourceHint->setWordWrap(true);
  sourceHint->setProperty("dialogTone", "muted");
  layout->addWidget(sourceHint);

  QList<std::pair<RecommendedSection, QCheckBox*>> checks;
  for (const auto& group : groups) {
    auto* check = new QCheckBox(group.title, &dlg);
    check->setChecked(true);
    layout->addWidget(check);
    checks.append({group.section, check});
  }

  auto* btns = new QDialogButtonBox(&dlg);
  auto* okBtn = btns->addButton(I18n::tr("OK"), QDialogButtonBox::AcceptRole);
  auto* cancelBtn = btns->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  setDialogAction(okBtn, DialogAction::Primary);
  frame->setActions(btns);

  const auto updateOk = [checks, okBtn] { okBtn->setEnabled(std::ranges::any_of(checks, [](const auto& item) { return item.second->isChecked(); })); };
  for (const auto& item : std::as_const(checks)) QObject::connect(item.second, &QCheckBox::toggled, &dlg, [updateOk](bool) { updateOk(); });
  QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
  updateOk();

  if (dlg.exec() != QDialog::Accepted) return 0;

  int sections = 0;
  for (const auto& [section, check] : std::as_const(checks))
    if (check->isChecked()) sections |= section;
  return sections;
}

void appendTextBlock(CodeEditor* target, const QString& block) {
  target->moveCursor(QTextCursor::End);
  if (!target->textCursor().atBlockStart()) target->insertPlainText(QStringLiteral("\n"));
  target->insertPlainText(block);
  target->moveCursor(QTextCursor::End);
}

void appendDefaultRules(QWidget* parent, CodeEditor* target, const presentation::ImportViewModel& model) {
  const int sections = chooseDefaultRuleSections(model.recommendedGroups(), parent);
  if (sections == 0) return;

  appendTextBlock(target, model.defaultRulesText(sections));
}

}  // namespace

ImportDialog::ImportDialog(std::shared_ptr<presentation::ImportViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent)
    : PanelDialog(parent), m_viewModel(std::move(model)), m_fileDialogs(fileDialogs) {
  setWindowTitle(I18n::tr("Import configuration"));
  setDescription(I18n::tr("Add UWF configuration from a script or text."));
  auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(design::panelWidth, design::panelHeight), QSize());
  auto* layout = frame->body();
  auto* pages = new TabView(this);
  pages->setMeasureCurrentPage(true);
  m_tabs = pages;
  m_tabs->setDocumentMode(true);
  m_tabs->setObjectName(QStringLiteral("importTabs"));
  auto* editorPage = new QWidget(m_tabs);
  auto* editorLayout = new QVBoxLayout(editorPage);
  editorLayout->setContentsMargins(0, 20, 0, 0);
  editorLayout->setSpacing(20);
  auto* toolbar = new QHBoxLayout();
  toolbar->setSpacing(8);
  auto* loadBtn = new QPushButton(ThemeManager::instance().icon(QStringLiteral(":/icons/folder.svg")), I18n::tr("Load from file…"), this);
  loadBtn->setEnabled(m_viewModel->canLoadFiles());
  auto* defaultRulesBtn = new QPushButton(ThemeManager::instance().icon(QStringLiteral(":/icons/spark.svg")), I18n::tr("Load recommended configuration"), this);
  defaultRulesBtn->setProperty("designRole", "quiet");
  for (auto* button : {loadBtn, defaultRulesBtn}) {
    button->setFixedHeight(30);
    auto type = button->font();
    type.setPixelSize(12);
    button->setFont(type);
    toolbar->addWidget(button);
  }
  toolbar->addStretch();
  auto* lineHint = new TextLabel(I18n::tr("One uwfmgr command per line"), TextStyle::Caption, this);
  lineHint->setProperty("designTone", "muted");
  toolbar->addWidget(lineHint);
  editorLayout->addLayout(toolbar);

  auto* editor = new Surface(design::Color::Surface, this);
  editor->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  editor->setClipContent(true);
  auto* editorStack = new QVBoxLayout(editor);
  editorStack->setContentsMargins(1, 1, 1, 1);
  editorStack->setSpacing(0);
  auto* editorHeading = new Surface(design::Color::Surface, editor);
  editorHeading->setBorder(Qt::BottomEdge);
  editorHeading->setFixedHeight(42);
  auto* editorHeadingRow = new QHBoxLayout(editorHeading);
  editorHeadingRow->setContentsMargins(16, 0, 16, 0);
  auto* editorName = new TextLabel(I18n::tr("uwfmgr commands"), TextStyle::Caption, editorHeading);
  editorName->setObjectName(QStringLiteral("importEditorTitle"));
  editorName->setProperty("designTone", "muted");
  editorHeadingRow->setSpacing(8);
  auto* editorIcon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/code.svg")), 16, design::Color::Muted, editorHeading);
  editorIcon->setObjectName(QStringLiteral("importEditorIcon"));
  editorHeadingRow->addWidget(editorIcon);
  editorHeadingRow->addWidget(editorName);
  editorHeadingRow->addStretch();
  editorHeadingRow->addWidget(new TextLabel(I18n::tr("Editable"), TextStyle::Caption, editorHeading));
  editorStack->addWidget(editorHeading);
  m_text = new CodeEditor(editor);
  m_text->setObjectName(QStringLiteral("importTextEdit"));
  m_text->setPlaceholderText(I18n::tr("Paste commands, or load them from a file…"));
  editorStack->addWidget(m_text, 1);
  auto* editorFooter = new Surface(design::Color::Surface, editor);
  editorFooter->setBorder(Qt::TopEdge);
  editorFooter->setFixedHeight(32);
  auto* meta = new QHBoxLayout(editorFooter);
  meta->setContentsMargins(16, 0, 16, 0);
  meta->addWidget(new TextLabel(QStringLiteral("filter · overlay · volume · file · registry"), TextStyle::Eyebrow, editorFooter));
  meta->addStretch();
  auto* lineCount = new TextLabel({}, TextStyle::Eyebrow, editorFooter);
  meta->addWidget(lineCount);
  editorStack->addWidget(editorFooter);
  editorLayout->addWidget(editor, 1);
  auto* importHint =
      new TextLabel(I18n::tr("Import updates the draft. Confirm the changes in Review and apply to write them to the system."), TextStyle::Caption, this);
  importHint->setProperty("designTone", "muted");
  importHint->setObjectName(QStringLiteral("importDraftHint"));
  importHint->setWordWrap(true);
  auto* hintRow = new QHBoxLayout();
  hintRow->setSpacing(9);
  auto* hintIcon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")), 17, design::Color::Muted, this);
  hintIcon->setObjectName(QStringLiteral("importDraftHintIcon"));
  hintRow->addWidget(new AlignedGraphic(hintIcon, importHint, this));
  hintRow->addWidget(importHint, 1);
  editorLayout->addLayout(hintRow);

  auto* reportPage = new QWidget(m_tabs);
  auto* reportLayout = new QVBoxLayout(reportPage);
  reportLayout->setContentsMargins(0, 20, 0, 0);
  reportLayout->setSpacing(20);
  m_summary = new QWidget(reportPage);
  auto* summaryFlow = new FlowLayout(m_summary);
  summaryFlow->setContentsMargins(0, 0, 0, 0);
  summaryFlow->setSpacing(16);
  const QStringList summaryIcons{QStringLiteral(":/icons/status_success.svg"), QStringLiteral(":/icons/copy.svg"), QStringLiteral(":/icons/info.svg"),
                                 QStringLiteral(":/icons/status_error.svg")};
  const std::array summaryColors{design::Color::Success, design::Color::Muted, design::Color::Warning, design::Color::Danger};
  for (std::size_t index = 0; index < m_summaryLabels.size(); ++index) {
    auto* group = new QWidget(m_summary);
    auto* row = new QHBoxLayout(group);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);
    auto* icon = new IconView(ThemeManager::instance().icon(summaryIcons[static_cast<qsizetype>(index)]), 15, summaryColors[index], group);
    icon->setObjectName(QStringLiteral("importSummaryIcon%1").arg(index));
    m_summaryLabels[index] = new TextLabel({}, TextStyle::Caption, group);
    m_summaryLabels[index]->setObjectName(QStringLiteral("importSummaryText%1").arg(index));
    m_summaryLabels[index]->setColorRole(summaryColors[index]);
    row->addWidget(new AlignedGraphic(icon, m_summaryLabels[index], group));
    row->addWidget(m_summaryLabels[index]);
    summaryFlow->addWidget(group);
  }
  reportLayout->addWidget(m_summary);
  auto* reportSurface = new Surface(design::Color::Surface, reportPage);
  reportSurface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  reportSurface->setClipContent(true);
  auto* reportStack = new QVBoxLayout(reportSurface);
  reportStack->setContentsMargins(1, 1, 1, 1);
  m_report = new ResponsiveTableView({{design::importLineWidth}, {design::importStatusWidth}, {180, .48}, {180}}, reportSurface);
  m_report->setObjectName(QStringLiteral("importResultsTable"));
  m_report->setModel(m_viewModel.get());
  m_report->verticalHeader()->setDefaultSectionSize(design::importRowHeight);
  reportStack->addWidget(m_report);
  reportLayout->addWidget(reportSurface, 1);
  auto* reportHint =
      new TextLabel(I18n::tr("The list shows command results for the configuration draft. Select a row to view the complete command and details."),
                    TextStyle::Caption, reportPage);
  reportHint->setWordWrap(true);
  reportHint->setProperty("designTone", "muted");
  reportLayout->addWidget(reportHint);
  connect(m_report, &QTableView::clicked, this, &ImportDialog::showResult);
  connect(m_report, &QTableView::activated, this, &ImportDialog::showResult);
  m_tabs->addTab(editorPage, I18n::tr("Edit commands"));
  m_tabs->addTab(reportPage, I18n::tr("Import results"));
  static_cast<TabView*>(m_tabs)->setTabCount(1, m_viewModel->rowCount());
  layout->addWidget(m_tabs, 1);

  auto* buttons = new QDialogButtonBox(this);
  m_importBtn = new QPushButton(ThemeManager::instance().icon(QStringLiteral(":/icons/import.svg")), I18n::tr("Import configuration"), this);
  m_importBtn->setObjectName(QStringLiteral("primaryBtn"));
  setDialogAction(m_importBtn, DialogAction::Primary);
  m_importBtn->setEnabled(false);
  m_closeBtn = new QPushButton(I18n::tr("Close"), this);
  buttons->addButton(m_importBtn, QDialogButtonBox::AcceptRole);
  buttons->addButton(m_closeBtn, QDialogButtonBox::RejectRole);
  auto* footerHint = new QWidget(this);
  footerHint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* footerRow = new QHBoxLayout(footerHint);
  footerRow->setContentsMargins(0, 0, 0, 0);
  footerRow->setSpacing(8);
  auto* footerIcon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")), 16, design::Color::Muted, footerHint);
  footerIcon->setObjectName(QStringLiteral("importFooterIcon"));
  footerRow->addWidget(footerIcon);
  auto* footerText = new TextLabel(I18n::tr("Not written to the system"), TextStyle::Caption, footerHint);
  footerText->setObjectName(QStringLiteral("importFooterText"));
  footerText->setColorRole(design::Color::Muted);
  footerRow->addWidget(footerText, 1);
  frame->setActions(buttons, footerHint);
  connect(m_viewModel.get(), &presentation::ImportViewModel::availabilityChanged, this, &ImportDialog::renderActions);
  connect(m_viewModel.get(), &presentation::ImportViewModel::warningRaised, this,
          [this](const QString& title, const QString& body) { dialogs::warning(this, title, body); });
  connect(m_viewModel.get(), &presentation::ImportViewModel::textBlockReady, this, [this](const QString& block) { appendTextBlock(m_text, block); });
  connect(m_viewModel.get(), &presentation::ImportViewModel::reportAppended, this, &ImportDialog::renderReport);
  connect(m_text, &QTextEdit::textChanged, this, [this, lineCount] {
    m_viewModel->setText(m_text->toPlainText());
    renderActions();
    lineCount->setText(I18n::tr("UTF-8 · %1 lines").arg(m_text->blockCount()));
  });
  m_text->setPlainText(m_viewModel->text());
  lineCount->setText(I18n::tr("UTF-8 · %1 lines").arg(m_text->blockCount()));
  connect(m_importBtn, &QPushButton::clicked, this, [this] {
    if (m_tabs->currentIndex() == 0)
      onImportClicked();
    else {
      m_reviewRequested = true;
      accept();
    }
  });
  connect(defaultRulesBtn, &QPushButton::clicked, this, [this] { appendDefaultRules(this, m_text, *m_viewModel); });
  connect(loadBtn, &QPushButton::clicked, this, [this] {
    auto paths = m_fileDialogs.openFiles(
        this, {I18n::tr("Choose files containing uwfmgr commands"), QDir::homePath(), I18n::tr("Text files (*.txt *.cmd);;All files (*)")});
    for (const auto& path : std::as_const(paths)) m_viewModel->loadFile(path);
  });
  connect(m_closeBtn, &QPushButton::clicked, this, [this] {
    if (m_tabs->currentIndex() == 1)
      m_tabs->setCurrentIndex(0);
    else
      reject();
  });
  connect(m_tabs, &QTabWidget::currentChanged, this, &ImportDialog::renderActions);
  renderActions();
}

void ImportDialog::renderActions() {
  const bool report = m_tabs->currentIndex() == 1;
  m_importBtn->setText(report                      ? I18n::tr("Preview")
                       : m_viewModel->preparing() ? I18n::tr("Reading state…")
                                                  : I18n::tr("Import configuration"));
  m_importBtn->setIcon(ThemeManager::instance().icon(report ? QStringLiteral(":/icons/apply.svg") : QStringLiteral(":/icons/import.svg")));
  m_importBtn->setEnabled(report ? m_viewModel->totals().success > 0 : m_viewModel->canImport());
  m_closeBtn->setText(report ? I18n::tr("Continue editing") : I18n::tr("Close"));
}

void ImportDialog::onImportClicked() { m_viewModel->importText(); }

void ImportDialog::showResult(const QModelIndex& index) {
  if (!index.isValid() || !index.flags().testFlag(Qt::ItemIsSelectable)) return;
  const auto command = index.siblingAtColumn(2).data(presentation::FullTextRole).toString();
  const auto detail = index.siblingAtColumn(3).data(presentation::FullTextRole).toString();
  InformationDialog dialog(this);
  dialog.setObjectName(QStringLiteral("importCommandResultDialog"));
  dialog.setWindowTitle(I18n::tr("Command processing result"));
  dialog.setHeaderStyle(DialogWindow::HeaderStyle::Message);
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(design::messageWidth, 0));
  frame->body()->setSpacing(16);
  auto* explanation = new TextLabel(detail, TextStyle::Description, &dialog);
  explanation->setWordWrap(true);
  explanation->setColorRole(design::Color::Muted);
  frame->body()->addWidget(explanation);
  auto* code = new Surface(design::Color::Surface, &dialog);
  code->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 5);
  auto* layout = new QVBoxLayout(code);
  layout->setContentsMargins(19, 15, 19, 15);
  auto* text = new ContentHintBox(code);
  text->setObjectName(QStringLiteral("importResultCommand"));
  auto font = text->font();
  font.setFamilies(DesignSystem::monospaceFamilies());
  font.setPixelSize(12);
  text->setFont(font);
  text->setLineHeight(22.8);
  text->setPlainText(command);
  layout->addWidget(text);
  frame->body()->addWidget(code);
  auto* buttons = new QDialogButtonBox(&dialog);
  auto* copy = buttons->addButton(I18n::tr("Copy command"), QDialogButtonBox::ActionRole);
  copy->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
  auto* close = buttons->addButton(I18n::tr("Close"), QDialogButtonBox::RejectRole);
  close->setDefault(true);
  close->setFocus();
  connect(copy, &QPushButton::clicked, &dialog, [&dialog, command] {
    const QPointer target(&dialog);
    if (writeClipboardText(command) == ClipboardWriteResult::Confirmed && target) target->accept();
  });
  connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);
  frame->setActions(buttons);
  dialog.exec();
}

void ImportDialog::renderReport(const QList<app::ImportResult>& rows) {
  static_cast<TabView*>(m_tabs)->setTabCount(1, m_viewModel->rowCount());
  m_summaryLabels[0]->setText(I18n::tr("Imported: %1").arg(m_viewModel->totals().success));
  m_summaryLabels[1]->setText(I18n::tr("Duplicates: %1").arg(m_viewModel->totals().duplicate));
  m_summaryLabels[2]->setText(I18n::tr("Unsupported: %1").arg(m_viewModel->totals().unsupported));
  m_summaryLabels[3]->setText(I18n::tr("Failed: %1").arg(m_viewModel->totals().failed));
  m_summaryLabels[3]->parentWidget()->setVisible(m_viewModel->totals().failed > 0);
  m_summary->show();

  if (m_viewModel->totals().batches > 1) {
    const int separator = m_viewModel->rowCount() - static_cast<int>(rows.size()) - 1;
    m_report->setSpan(separator, 0, 1, m_viewModel->columnCount());
  }

  m_tabs->setCurrentIndex(1);
  renderActions();
  m_report->scrollToBottom();
}

}  // namespace uwf::ui

#include "moc_ImportDialog.cpp"
