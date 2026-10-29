// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitOperationDialog.h"

#include <QDialogButtonBox>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "CommitReportDialog.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/widgets/BusyIndicator.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {
CommitOperationDialog::CommitOperationDialog(presentation::CommitOperationViewModel& model, dialogs::FileDialogProvider& files, app::TextFileWriter writer,
                                             QWidget* parent)
    : InformationDialog(parent), m_model(model) {
  setObjectName(QStringLiteral("commitOperationDialog"));
  setHeaderStyle(HeaderStyle::Message);
  setWindowModality(Qt::WindowModal);
  auto* frame = new DialogLayout(this, DialogContent::Form, QSize(design::messageWidth, 0));
  auto* body = frame->body();
  body->setSpacing(10);
  auto* target = new Surface(design::Color::Surface, this);
  target->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  auto* targetLayout = new QVBoxLayout(target);
  targetLayout->setContentsMargins(17, 15, 17, 15);
  m_target = new TextLabel({}, TextStyle::Description, target);
  m_target->setObjectName(QStringLiteral("commitOperationTarget"));
  m_target->setElideMode(Qt::ElideRight);
  m_target->setLayoutDirection(Qt::LeftToRight);
  auto font = m_target->font();
  font.setFamilies(DesignSystem::monospaceFamilies());
  font.setPixelSize(13);
  m_target->setFont(font);
  m_target->setLineHeight(1.8);
  targetLayout->addWidget(m_target);
  body->addWidget(target);
  auto* status = new QHBoxLayout;
  status->setSpacing(9);
  m_glyph = new QStackedWidget(this);
  m_glyph->setObjectName(QStringLiteral("commitOperationGlyph"));
  m_glyph->setFixedSize(18, 18);
  m_busy = new BusyIndicator(m_glyph);
  m_icon = new IconView({}, 18, design::Color::Accent, m_glyph);
  m_glyph->addWidget(m_busy);
  m_glyph->addWidget(m_icon);
  status->addWidget(m_glyph, 0, Qt::AlignVCenter);
  m_heading = new TextLabel({}, TextStyle::Label, this);
  m_heading->setObjectName(QStringLiteral("commitOperationHeading"));
  auto headingFont = m_heading->font();
  headingFont.setPixelSize(13);
  headingFont.setWeight(QFont::DemiBold);
  m_heading->setFont(headingFont);
  m_heading->setWordWrap(true);
  m_heading->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
  m_heading->setVerticalReference(TextVerticalReference::GlyphBounds);
  status->addWidget(m_heading, 1);
  auto* state = new QWidget(this);
  auto* stateLayout = new QVBoxLayout(state);
  stateLayout->setContentsMargins(0, 0, 0, 0);
  stateLayout->setSpacing(6);
  stateLayout->addLayout(status);
  m_description = new TextLabel({}, TextStyle::Description, this);
  m_description->setObjectName(QStringLiteral("commitOperationDescription"));
  m_description->setLineHeight(1.9);
  m_description->setWordWrap(true);
  m_description->setColorRole(design::Color::Muted);
  m_description->setMinimumHeight(50);
  stateLayout->addWidget(m_description);
  body->addWidget(state);
  auto* countRow = new QHBoxLayout;
  countRow->setSpacing(12);
  m_count = new TextLabel({}, TextStyle::Caption, this);
  m_count->setObjectName(QStringLiteral("commitOperationCount"));
  m_count->setWordWrap(true);
  m_count->setColorRole(design::Color::Muted);
  m_count->setLineHeight(1.7);
  countRow->addWidget(m_count, 1);
  m_export = new QPushButton(this);
  m_export->setProperty("designRole", "link");
  m_export->setObjectName(QStringLiteral("commitTraversalExport"));
  m_export->setProperty("designPaddingX", 0);
  m_export->setFixedHeight(20);
  auto exportFont = m_export->font();
  exportFont.setPixelSize(12);
  m_export->setFont(exportFont);
  countRow->addWidget(m_export);
  body->addLayout(countRow);
  m_details = new QStackedWidget(this);
  auto* activity = new QWidget(m_details);
  auto* activityLayout = new QVBoxLayout(activity);
  activityLayout->setContentsMargins(0, 0, 0, 0);
  activityLayout->setSpacing(8);
  m_progress = new QProgressBar(activity);
  m_progress->setObjectName(QStringLiteral("commitOperationProgress"));
  m_progress->setTextVisible(false);
  m_progress->setFixedHeight(4);
  activityLayout->addWidget(m_progress);
  auto progressPolicy = m_progress->sizePolicy();
  progressPolicy.setRetainSizeWhenHidden(true);
  m_progress->setSizePolicy(progressPolicy);
  m_path = new TextLabel({}, TextStyle::Caption, activity);
  m_path->setObjectName(QStringLiteral("commitOperationPath"));
  m_path->setElideMode(Qt::ElideRight);
  m_path->setLayoutDirection(Qt::LeftToRight);
  m_path->setColorRole(design::Color::Faint);
  font = m_path->font();
  font.setFamilies(DesignSystem::monospaceFamilies());
  font.setPixelSize(11);
  m_path->setFont(font);
  activityLayout->addWidget(m_path);
  activityLayout->addStretch();
  m_detail = new TextLabel({}, TextStyle::Caption, m_details);
  m_detail->setWordWrap(true);
  m_detail->setColorRole(design::Color::Muted);
  m_detail->setLineHeight(1.9);
  m_details->addWidget(activity);
  m_details->addWidget(m_detail);
  m_details->setMinimumHeight(46);
  body->addWidget(m_details);
  auto* buttons = new QDialogButtonBox(this);
  m_primary = buttons->addButton(QString(), QDialogButtonBox::AcceptRole);
  m_primary->setObjectName(QStringLiteral("commitOperationPrimary"));
  m_primary->setAutoDefault(false);
  m_retry = buttons->addButton(QString(), QDialogButtonBox::ActionRole);
  m_retry->setObjectName(QStringLiteral("commitOperationRetry"));
  m_report = buttons->addButton(QString(), QDialogButtonBox::ActionRole);
  m_report->setObjectName(QStringLiteral("commitOperationReport"));
  m_close = buttons->addButton(QString(), QDialogButtonBox::RejectRole);
  m_close->setObjectName(QStringLiteral("commitOperationCancel"));
  m_close->setDefault(true);
  m_noteContainer = new QWidget(this);
  m_noteContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* noteLayout = new QHBoxLayout(m_noteContainer);
  noteLayout->setContentsMargins(0, 0, 0, 0);
  noteLayout->setSpacing(8);
  m_noteIcon = new IconView({}, 16, design::Color::Muted, m_noteContainer);
  noteLayout->addWidget(m_noteIcon);
  m_note = new TextLabel({}, TextStyle::Caption, m_noteContainer);
  m_note->setObjectName(QStringLiteral("commitOperationNote"));
  m_note->setColorRole(design::Color::Muted);
  noteLayout->addWidget(m_note, 1);
  frame->setActions(buttons, m_noteContainer);
  connect(m_primary, &QPushButton::clicked, &model, &presentation::CommitOperationViewModel::proceed);
  connect(m_retry, &QPushButton::clicked, &model, &presentation::CommitOperationViewModel::retry);
  connect(m_close, &QPushButton::clicked, this, &CommitOperationDialog::reject);
  connect(m_report, &QPushButton::clicked, this, [this] { showCommitReport(this, m_model.rows(), m_model.request().operation, m_model.completion()); });
  connect(m_export, &QPushButton::clicked, this, [this, &files, writer = std::move(writer)] {
    const auto path = files.saveFile(
        this, {I18n::tr("Export traversal records"), QStringLiteral("uwf-traversal-records.txt"), I18n::tr("Text files (*.txt);;All files (*)")});
    if (!path.isEmpty()) m_model.exportRecords(path, writer);
  });
  connect(&model, &presentation::CommitOperationViewModel::exportFinished, this, [this](const QString& error) {
    if (!error.isEmpty()) dialogs::warning(this, I18n::tr("Export failed"), error);
  });
  connect(&model, &presentation::CommitOperationViewModel::stateChanged, this, &CommitOperationDialog::render);
  I18n::bind(this, [this] { render(); });
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] { render(); });
  render();
}
void CommitOperationDialog::render() {
  const auto view = m_model.text();
  setWindowTitle(view.title);
  m_target->setText(m_model.request().target.display());
  m_target->setToolTip(m_target->text());
  m_heading->setText(view.heading);
  m_description->setText(view.description);
  m_description->setToolTip(view.description);
  m_count->setText(view.count);
  m_detail->setText(view.detail);
  m_path->setText(m_model.progress().path);
  m_path->setToolTip(m_path->text());
  m_icon->setIcon(ThemeManager::instance().icon(view.icon));
  m_icon->setColorRole(view.error     ? design::Color::Danger
                       : view.warning ? design::Color::Warning
                       : view.success ? design::Color::Success
                                      : design::Color::Accent);
  m_glyph->setCurrentWidget(view.busy ? static_cast<QWidget*>(m_busy) : m_icon);
  m_details->setCurrentIndex(view.activePath ? 0 : 1);
  m_progress->setRange(0, view.busy ? 0 : std::max(1, m_model.total()));
  m_progress->setValue(m_model.processed());
  m_progress->setVisible(view.busy || view.writing);
  m_note->setText(view.note);
  m_note->setToolTip(view.note);
  m_noteIcon->setIcon(ThemeManager::instance().icon(view.writing || view.showReport ? QStringLiteral(":/icons/info.svg") : QStringLiteral(":/icons/lock.svg")));
  m_noteContainer->setVisible(!view.retryVisible || !view.primaryVisible);
  m_primary->setText(view.primary);
  m_primary->setVisible(view.primaryVisible);
  m_primary->setEnabled(view.primaryEnabled);
  m_primary->setIcon(view.retryVisible
                         ? QIcon{}
                         : ThemeManager::instance().icon(view.destructive ? QStringLiteral(":/icons/trash.svg") : QStringLiteral(":/icons/apply.svg")));
  setDialogAction(m_primary, view.destructive ? DialogAction::Destructive : DialogAction::Primary);
  m_retry->setText(view.retry);
  m_retry->setVisible(view.retryVisible);
  m_close->setText(view.close);
  setRejectionEnabled(view.cancelEnabled);
  m_export->setText(I18n::tr("Export traversal records"));
  m_export->setEnabled(view.exportEnabled);
  m_report->setText(I18n::tr("View results"));
  m_report->setVisible(view.showReport);
  if (m_previous != m_model.phase()) {
    m_close->setFocus(Qt::OtherFocusReason);
    m_previous = m_model.phase();
  }
}
bool CommitOperationDialog::requestRejection() { return m_model.cancel(); }
}  // namespace uwf::ui
#include "moc_CommitOperationDialog.cpp"
