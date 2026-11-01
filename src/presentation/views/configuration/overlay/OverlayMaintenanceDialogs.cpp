// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/overlay/OverlayMaintenanceDialogs.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPointer>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <memory>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/BusyIndicator.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/layout/AlignedGraphic.h"
#include "presentation/controls/layout/FlowLayout.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/views/configuration/overlay/HormUsageDialog.h"

namespace uwf::ui {
namespace {
using Color = design::Color;
using Model = presentation::OverlayMaintenanceViewModel;
// Marked literals remain visible to lupdate even inside conditional labels.
QString localized(const char* source) { return I18n::tr(source); }
TextLabel* label(QWidget* owner, int pixels = 13, Color color = Color::Text, QFont::Weight weight = QFont::Normal) {
  auto* result = new TextLabel({}, TextStyle::Body, owner);
  auto font = result->font();
  font.setPixelSize(pixels);
  font.setWeight(weight);
  result->setFont(font);
  result->setColorRole(color);
  result->setWordWrap(true);
  result->setLineHeight(1.85);
  return result;
}
TextLabel* translated(QWidget* owner, const char* source, int pixels = 13, Color color = Color::Text, QFont::Weight weight = QFont::Normal) {
  auto* result = label(owner, pixels, color, weight);
  I18n::text(result, [source] { return localized(source); });
  return result;
}
void hero(QVBoxLayout* body, QWidget* owner, const char* technical, const char* description, const QString& resource) {
  const bool horm = resource == QStringLiteral(":/icons/restart.svg");
  const auto header = createDialogHero(owner, ThemeManager::instance().icon(resource));
  header.icon->setObjectName(QStringLiteral("maintenanceHeaderIcon"));
  header.heading->parentWidget()->setObjectName(QStringLiteral("maintenanceHeroCopy"));
  header.heading->setObjectName(QStringLiteral("maintenanceHeroHeading"));
  header.technical->setObjectName(QStringLiteral("maintenanceHeroTechnical"));
  I18n::bind(header.widget, [header, technical, description] { updateDialogHeroText(header, localized(description), QString::fromLatin1(technical)); });
  body->addWidget(header.widget);
  body->addSpacing(horm ? 21 : 18);
}
struct RuledText {
  QWidget* host;
  QVBoxLayout* content;
};
RuledText ruledText(QWidget* owner, const int textInset) {
  auto* host = new QWidget(owner);
  auto* row = new QHBoxLayout(host);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(textInset);
  auto* rule = new Surface(Color::Line, host);
  rule->setObjectName(QStringLiteral("maintenanceEffectRule"));
  rule->setFixedWidth(2);
  row->addWidget(rule);
  auto* copy = new QWidget(host);
  auto* content = new QVBoxLayout(copy);
  content->setContentsMargins(0, 0, 0, 0);
  row->addWidget(copy, 1);
  return {host, content};
}
void effect(QVBoxLayout* body, QWidget* owner, const char* title, const char* text) {
  const auto item = ruledText(owner, 14);
  auto* layout = item.content;
  layout->setSpacing(3);
  layout->addWidget(translated(item.host, title, 13, Color::Text, QFont::Medium));
  layout->addWidget(translated(item.host, text, 12, Color::Muted));
  body->addWidget(item.host);
}
QWidget* summaryRow(QWidget* owner, const char* source, TextLabel*& value) {
  auto* row = new QWidget(owner);
  auto* layout = new QHBoxLayout(row);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(12);
  auto* caption = translated(row, source, 12, Color::Muted);
  caption->setLineHeight(1.6);
  layout->addWidget(caption, 1);
  value = label(row, 12, Color::Text, QFont::Medium);
  value->setLineHeight(1.6);
  value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  layout->addWidget(value, 1);
  return row;
}
}  // namespace

OverlayMaintenanceDialog::OverlayMaintenanceDialog(std::shared_ptr<Model> model, QWidget* parent)
    : PanelDialog(parent), m_model(Model::editDraft(std::move(model))) {
  setHeaderStyle(HeaderStyle::Message);
  setHeaderMetrics({QMargins(28, 26, 28, 22), 21, 1.5});
  setWindowModality(Qt::WindowModal);
  m_frame = new DialogLayout(this, DialogContent::Scrollable, QSize(548, 0), QSize(320, 220));
  setMaximumHeight(std::max(220, std::min(720, screen()->availableGeometry().height() - 48)));
  m_frame->setAutoSizeToContent(true);
  m_frame->setActionMetrics(28, 16, 72);
  m_frame->body()->setContentsMargins(28, 0, 28, 24);
  m_frame->body()->setSpacing(0);
  m_feedback = new Notice(this);
  m_feedback->setObjectName(QStringLiteral("maintenanceFeedback"));
  m_feedback->hide();
  m_activity = new QWidget(this);
  m_activity->setObjectName(QStringLiteral("maintenanceActivity"));
  auto* activity = new QHBoxLayout(m_activity);
  activity->setContentsMargins(0, 0, 0, 0);
  activity->setSpacing(9);
  auto* spinner = new BusyIndicator(m_activity);
  spinner->setFixedSize(18, 18);
  m_activityText = label(m_activity, 12, Color::Muted);
  activity->addWidget(new AlignedGraphic(spinner, m_activityText, m_activity));
  activity->addWidget(m_activityText, 1);
  m_activity->hide();
  auto* actions = new QWidget(this);
  auto* row = new QHBoxLayout(actions);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(9);
  m_cancel = new QPushButton(actions);
  m_cancel->setObjectName(QStringLiteral("maintenanceCancel"));
  m_cancel->setFixedHeight(36);
  setDialogAction(m_cancel, DialogAction::Secondary);
  bindRejectionButton(m_cancel);
  m_submit = new QPushButton(actions);
  m_submit->setObjectName(QStringLiteral("maintenanceSubmit"));
  m_submit->setFixedHeight(36);
  setDialogAction(m_submit, DialogAction::Primary);
  m_submit->setAutoDefault(false);
  m_cancel->setAutoDefault(true);
  m_cancel->setDefault(true);
  row->addWidget(m_cancel);
  row->addWidget(m_submit);
  auto* leading = new QWidget(this);
  m_leadingActions = new QHBoxLayout(leading);
  m_leadingActions->setContentsMargins(0, 0, 0, 0);
  m_leadingActions->setSpacing(8);
  m_frame->setActions(actions, leading);
  connect(m_cancel, &QPushButton::clicked, this, &QDialog::reject);
}
void OverlayMaintenanceDialog::finishBody() {
  if (m_frame->body()->indexOf(m_activity) < 0) {
    m_frame->body()->addSpacing(18);
    m_frame->body()->addWidget(m_activity);
  }
  if (m_frame->body()->indexOf(m_feedback) < 0) m_frame->body()->addWidget(m_feedback);
  m_cancel->setFocus(Qt::OtherFocusReason);
  m_model->refresh();
}
void OverlayMaintenanceDialog::renderDialogState(const Model::DialogState& state) {
  setWindowTitle(state.title);
  setRejectionEnabled(!state.busy);
  m_activity->setVisible(state.busy);
  m_activityText->setText(localized(QT_TRANSLATE_NOOP("uwf", "Reading state…")));
  switch (state.feedbackTone) {
    case Model::DialogFeedbackTone::Neutral:
      m_feedback->setTone(Notice::Tone::Neutral);
      break;
    case Model::DialogFeedbackTone::Warning:
      m_feedback->setTone(Notice::Tone::Warning);
      break;
    case Model::DialogFeedbackTone::Success:
      m_feedback->setTone(Notice::Tone::Success);
      break;
    case Model::DialogFeedbackTone::Error:
      m_feedback->setTone(Notice::Tone::Error);
      break;
  }
  m_feedback->setHeading(state.feedbackHeading);
  m_feedback->setText(state.feedback);
  m_feedback->setVisible(!state.busy && !state.feedback.isEmpty());
  m_cancel->setText(state.cancelLabel);
  m_submit->setVisible(state.submitVisible);
  m_submit->setEnabled(state.submitEnabled);
  m_submit->setText(state.submitLabel);
  m_submit->setToolTip(state.submitHint);
}
void OverlayMaintenanceDialog::resizeEvent(QResizeEvent* event) {
  PanelDialog::resizeEvent(event);
  const int padding = width() < 500 ? 20 : m_hormChrome ? 30 : 28;
  setHeaderMetrics({QMargins(padding, 26, padding, 22), 21, 1.5});
  m_frame->body()->setContentsMargins(padding, 0, padding, 24);
  m_frame->setActionMetrics(padding, 16, 72);
}

FullCommitDialog::FullCommitDialog(std::shared_ptr<Model> model, QWidget* parent) : OverlayMaintenanceDialog(std::move(model), parent) {
  setObjectName(QStringLiteral("fullCommitDialog"));
  m_submit->setObjectName(QStringLiteral("fullCommitStart"));
  m_cancel->setObjectName(QStringLiteral("fullCommitCancel"));
  auto* body = m_frame->body();
  hero(body, this, "Full Commit", QT_TRANSLATE_NOOP("uwf", "Save changes on all protected volumes"), QStringLiteral(":/icons/disk.svg"));
  auto* lead = translated(this, QT_TRANSLATE_NOOP("uwf",
                                                  "Attempts to save all additions, changes and deletions made on every protected volume since this startup. "
                                                  "This includes files and system settings; you cannot select only some changes."));
  lead->setObjectName(QStringLiteral("fullCommitDescription"));
  body->addWidget(lead);
  body->addSpacing(22);
  effect(body, this, QT_TRANSLATE_NOOP("uwf", "Successfully saved changes will remain"),
         QT_TRANSLATE_NOOP("uwf", "Successfully saved changes can no longer be undone by restarting."));
  body->addSpacing(16);
  effect(body, this, QT_TRANSLATE_NOOP("uwf", "Later changes will also be saved until you restart"),
         QT_TRANSLATE_NOOP("uwf", "Once started, new changes are written directly to disk until you restart the computer."));
  body->addSpacing(16);
  effect(body, this, QT_TRANSLATE_NOOP("uwf", "Cannot be undone once started"),
         QT_TRANSLATE_NOOP("uwf", "Make sure this is the state you want to keep before continuing."));
  finishBody();
  connect(m_model.get(), &Model::stateChanged, this, &FullCommitDialog::render);
  connect(m_submit, &QPushButton::clicked, this, [this] {
    const auto model = m_model;
    const QPointer self(this);
    if (model->submitFullCommitDraft() && self) self->accept();
  });
  I18n::bind(this, [this] { render(); });
}
void FullCommitDialog::render() { renderDialogState(m_model->fullCommitDialogState()); }

SwapDialog::SwapDialog(std::shared_ptr<Model> model, QWidget* parent) : OverlayMaintenanceDialog(std::move(model), parent) {
  setObjectName(QStringLiteral("swapDialog"));
  m_submit->setObjectName(QStringLiteral("swapCreate"));
  m_cancel->setObjectName(QStringLiteral("swapCancel"));
  auto* body = m_frame->body();
  hero(body, this, "create swapfile", QT_TRANSLATE_NOOP("uwf", "Choose where to store the disk overlay"), QStringLiteral(":/icons/disk.svg"));
  body->addWidget(translated(this, QT_TRANSLATE_NOOP("uwf", "Choose the disk used to store temporary changes. Takes effect at the next startup.")));
  body->addSpacing(22);
  auto* session = new Surface(Color::Window, this);
  session->setFillEnabled(false);
  session->setBorder(Qt::BottomEdge);
  auto* sessionFlow = new FlowLayout(session);
  sessionFlow->setContentsMargins(0, 0, 0, 14);
  sessionFlow->setSpacing(12);
  const auto sessionPair = [session, sessionFlow](const char* source, TextLabel*& value) {
    auto* pair = new QWidget(session);
    auto* row = new QHBoxLayout(pair);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(10);
    row->addWidget(translated(pair, source, 12, Color::Muted));
    value = label(pair, 12, Color::Text, QFont::Medium);
    row->addWidget(value);
    sessionFlow->addWidget(pair);
  };
  sessionPair(QT_TRANSLATE_NOOP("uwf", "Current UWF"), m_filter);
  sessionPair(QT_TRANSLATE_NOOP("uwf", "Current overlay"), m_currentType);
  m_currentType->setObjectName(QStringLiteral("swapCurrentOverlayType"));
  body->addWidget(session);
  body->addSpacing(18);
  body->addWidget(m_activity);
  body->addWidget(m_feedback);
  m_statusGap = new QWidget(this);
  m_statusGap->setObjectName(QStringLiteral("swapFeedbackSpacing"));
  m_statusGap->setFixedHeight(16);
  body->addWidget(m_statusGap);
  auto* volumeLabel = translated(this, QT_TRANSLATE_NOOP("uwf", "Storage volume:"), 12, Color::Text, QFont::Medium);
  volumeLabel->setObjectName(QStringLiteral("swapVolumeLabel"));
  body->addWidget(volumeLabel);
  body->addSpacing(7);
  m_volumes = new QComboBox(this);
  m_volumes->setObjectName(QStringLiteral("swapVolume"));
  m_volumes->setMinimumWidth(0);
  m_volumes->setMinimumContentsLength(10);
  m_volumes->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  m_volumes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_volumes->setMinimumHeight(35);
  volumeLabel->setBuddy(m_volumes);
  body->addWidget(m_volumes);
  auto* summary = new Surface(Color::Window, this);
  summary->setFillEnabled(false);
  summary->setBorder(Qt::BottomEdge);
  auto* summaryBody = new QVBoxLayout(summary);
  summaryBody->setContentsMargins(0, 17, 0, 17);
  summaryBody->setSpacing(10);
  body->addSpacing(2);
  summaryBody->addWidget(summaryRow(summary, QT_TRANSLATE_NOOP("uwf", "Selected storage volume"), m_target));
  m_target->setObjectName(QStringLiteral("swapTargetSummary"));
  summaryBody->addWidget(summaryRow(summary, QT_TRANSLATE_NOOP("uwf", "Overlay type"), m_nextType));
  m_nextType->setObjectName(QStringLiteral("swapTargetOverlayType"));
  summaryBody->addWidget(summaryRow(summary, QT_TRANSLATE_NOOP("uwf", "Overlay capacity (keep current setting)"), m_capacity));
  m_capacity->setObjectName(QStringLiteral("swapCapacitySummary"));
  auto* previousHostRow = summaryRow(summary, QT_TRANSLATE_NOOP("uwf", "Currently configured storage volume"), m_previousHost);
  m_previousHost->setObjectName(QStringLiteral("swapPreviousHost"));
  summaryBody->addWidget(previousHostRow);
  body->addWidget(summary);
  body->addSpacing(16);
  body->addWidget(translated(
      this,
      QT_TRANSLATE_NOOP(
          "uwf", "Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk."),
      12, Color::Muted));
  body->addSpacing(7);
  body->addWidget(translated(
      this, QT_TRANSLATE_NOOP("uwf", "This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting."), 12,
      Color::Muted));
  finishBody();
  connect(m_model.get(), &Model::stateChanged, this, &SwapDialog::render);
  connect(m_volumes, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { m_model->selectStorageVolume(m_volumes->currentData().toString()); });
  connect(m_submit, &QPushButton::clicked, this, [this] {
    const auto model = m_model;
    const QPointer self(this);
    if (model->submitStorageDraft() && self) self->accept();
  });
  I18n::bind(this, [this] { render(); });
}
void SwapDialog::render() {
  const auto state = m_model->storageDialogState();
  {
    const QSignalBlocker blocker(m_volumes);
    m_volumes->clear();
    for (const auto& volume : state.volumes) {
      m_volumes->addItem(volume.text, volume.id);
      if (auto* items = qobject_cast<QStandardItemModel*>(m_volumes->model())) items->item(m_volumes->count() - 1)->setEnabled(volume.eligible);
    }
    m_volumes->setCurrentIndex(state.selectedIndex);
  }
  renderDialogState(state.dialog);
  m_filter->setText(state.filter);
  m_currentType->setText(state.currentType);
  m_target->setText(state.selectedVolume);
  m_nextType->setText(state.nextType);
  m_capacity->setText(state.capacity);
  m_previousHost->setText(state.previousHost);
  m_previousHost->setToolTip(state.previousHostHint);
  m_statusGap->setVisible(!m_activity->isHidden() || !m_feedback->isHidden());
  m_volumes->setEnabled(state.selectionEnabled);
}

HormDialog::HormDialog(std::shared_ptr<Model> model, QWidget* parent) : OverlayMaintenanceDialog(std::move(model), parent) {
  setObjectName(QStringLiteral("hormDialog"));
  m_hormChrome = true;
  resize(520, height());
  setHeaderMetrics({QMargins(30, 27, 30, 22), 21, 1.5});
  m_frame->body()->setContentsMargins(30, 0, 30, 24);
  m_frame->setActionMetrics(28, 16, 70);
  m_submit->setObjectName(QStringLiteral("hormConfirm"));
  m_cancel->setObjectName(QStringLiteral("hormCancel"));
  auto* body = m_frame->body();
  hero(body, this, "HORM", QT_TRANSLATE_NOOP("uwf", "Resume the same hibernation state repeatedly"), QStringLiteral(":/icons/restart.svg"));
  const auto effects = ruledText(this, 13);
  m_effectHost = effects.host;
  auto* effectBody = effects.content;
  effectBody->setSpacing(5);
  m_effect = label(effects.host, 12, Color::Muted);
  effectBody->addWidget(m_effect);
  body->addWidget(effects.host);
  m_checklist = new QWidget(this);
  m_checklist->setObjectName(QStringLiteral("hormRequirements"));
  auto* checklist = new QVBoxLayout(m_checklist);
  checklist->setContentsMargins(0, 0, 0, 0);
  checklist->setSpacing(11);
  auto* divider = new Surface(Color::Line, m_checklist);
  divider->setFixedHeight(1);
  checklist->addWidget(divider);
  checklist->addSpacing(3);
  constexpr std::array requirements{QT_TRANSLATE_NOOP("uwf", "UWF protection is active in the current session"),
                                    QT_TRANSLATE_NOOP("uwf", "The current overlay uses RAM Read-Only Media mode"),
                                    QT_TRANSLATE_NOOP("uwf", "System hibernation is available"),
                                    QT_TRANSLATE_NOOP("uwf", "Mounted fixed-disk protection check")};
  for (const char* requirement : requirements) {
    auto* row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(8);
    auto* icon = new IconView({}, 15, Color::Muted, m_checklist);
    const auto rowId = QString::number(m_requirementLabels.size());
    icon->setObjectName(QStringLiteral("hormRequirementIcon") + rowId);
    auto* name = translated(m_checklist, requirement, 12);
    name->setObjectName(QStringLiteral("hormRequirementLabel") + rowId);
    name->setAlignment(Qt::AlignLeading | Qt::AlignTop);
    auto* state = label(m_checklist, 11, Color::Muted);
    state->setObjectName(QStringLiteral("hormRequirementState") + rowId);
    state->setWordWrap(false);
    state->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    row->addWidget(new AlignedGraphic(icon, name, m_checklist));
    row->addWidget(name, 1);
    row->addWidget(new AlignedGraphic(state, name, m_checklist));
    m_requirementLabels.append(name);
    m_requirementStates.append(state);
    m_requirementIcons.append(icon);
    checklist->addLayout(row);
  }
  auto* blockers = new Surface(Color::Surface, m_checklist);
  blockers->setObjectName(QStringLiteral("hormProtectionDetails"));
  blockers->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  m_blockers = blockers;
  m_blockerRows = new QGridLayout(blockers);
  m_blockerRows->setContentsMargins(12, 10, 12, 10);
  m_blockerRows->setHorizontalSpacing(8);
  m_blockerRows->setVerticalSpacing(8);
  m_blockerRows->setColumnStretch(0, 1);
  checklist->addWidget(m_blockers);
  auto* protectionActions = new QHBoxLayout;
  protectionActions->setContentsMargins(0, 0, 0, 0);
  protectionActions->setSpacing(12);
  m_protectionPending = label(m_checklist, 11, Color::Muted);
  m_protectionPending->setObjectName(QStringLiteral("hormProtectionPending"));
  protectionActions->addWidget(m_protectionPending, 1);
  m_protectAll = new QPushButton(m_checklist);
  m_protectAll->setObjectName(QStringLiteral("hormProtectAll"));
  setDialogAction(m_protectAll, DialogAction::Secondary);
  m_protectAll->setFixedHeight(34);
  m_protectAll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto protectionFont = m_protectAll->font();
  protectionFont.setPixelSize(12);
  m_protectAll->setFont(protectionFont);
  m_protectAll->setAutoDefault(false);
  protectionActions->addWidget(m_protectAll, 0, Qt::AlignRight | Qt::AlignVCenter);
  checklist->addLayout(protectionActions);
  body->addWidget(m_checklist);
  auto* usage = new QPushButton(this);
  usage->setObjectName(QStringLiteral("hormUsage"));
  usage->setProperty("designRole", "link");
  usage->setProperty("designTone", "muted");
  usage->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")));
  usage->setIconSize(QSize(15, 15));
  auto usageFont = usage->font();
  usageFont.setPixelSize(12);
  usage->setFont(usageFont);
  usage->setAutoDefault(false);
  I18n::text(usage, [] { return localized(QT_TRANSLATE_NOOP("uwf", "Usage guide")); });
  m_leadingActions->addWidget(usage);
  connect(usage, &QPushButton::clicked, this, [this, usage] {
    HormUsageDialog dialog(this);
    dialog.exec();
    usage->setFocus(Qt::OtherFocusReason);
  });
  connect(m_protectAll, &QPushButton::clicked, this, [this] { m_model->stageAllVolumesProtection(true); });
  finishBody();
  connect(m_model.get(), &Model::stateChanged, this, &HormDialog::render);
  connect(m_submit, &QPushButton::clicked, this, [this] {
    const auto model = m_model;
    const QPointer self(this);
    if (model->submitHormDraft() && self) self->accept();
  });
  I18n::bind(this, [this] { render(); });
}
void HormDialog::render() {
  const auto state = m_model->hormDialogState();
  renderDialogState(state.dialog);
  m_effectHost->setVisible(state.disabling);
  m_effect->setText(localized(QT_TRANSLATE_NOOP("uwf",
                                                "Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on "
                                                "protected disks. Restart before checking the available actions.")));
  m_checklist->setVisible(!state.disabling);
  for (qsizetype index = 0; index < m_requirementStates.size(); ++index) {
    const auto& requirement = state.requirements[static_cast<std::size_t>(index)];
    const auto ready = requirement.met;
    m_requirementStates[index]->setText(requirement.text);
    m_requirementStates[index]->setToolTip(requirement.hint);
    m_requirementStates[index]->setColorRole(Color::Muted);
    m_requirementIcons[index]->setColorRole(!ready ? Color::Muted : *ready ? Color::Success : Color::Warning);
    m_requirementIcons[index]->setIcon(ThemeManager::instance().icon(!ready   ? QStringLiteral(":/icons/info.svg")
                                                                     : *ready ? QStringLiteral(":/icons/check.svg")
                                                                              : QStringLiteral(":/icons/status_warning.svg")));
  }
  m_requirementLabels.back()->setToolTip(
      localized(QT_TRANSLATE_NOOP("uwf", "Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.")));
  while (const auto cell = std::unique_ptr<QLayoutItem>(m_blockerRows->takeAt(0))) {
    const std::unique_ptr<QWidget> widget(cell->widget());
  }
  m_blockerRows->addWidget(translated(m_blockers, QT_TRANSLATE_NOOP("uwf", "Partition"), 11, Color::Muted), 0, 0);
  m_blockerRows->addWidget(translated(m_blockers, QT_TRANSLATE_NOOP("uwf", "Status"), 11, Color::Muted), 0, 1, Qt::AlignRight);
  int partitionRow = 1;
  for (const auto& volume : state.blockedVolumes) {
    const auto& name = volume.displayName;
    auto* identity = new QWidget(m_blockers);
    identity->setFixedHeight(28);
    auto* identityRow = new QHBoxLayout(identity);
    identityRow->setContentsMargins(0, 0, 0, 0);
    identityRow->setSpacing(8);
    auto* icon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/disk.svg")), 15, Color::Muted, identity);
    icon->setObjectName(QStringLiteral("hormPartitionIcon"));
    auto* title = label(identity, 12);
    title->setObjectName(QStringLiteral("hormPartitionName"));
    title->setText(name);
    title->setWordWrap(false);
    title->setElideMode(Qt::ElideRight);
    title->setMaximumWidth(140);
    title->setToolTip(name);
    title->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
    title->setVerticalReference(TextVerticalReference::GlyphBounds);
    identityRow->addWidget(new AlignedGraphic(icon, title, identity));
    identityRow->addWidget(title);
    auto* guid = label(identity, 11, Color::Muted);
    guid->setObjectName(QStringLiteral("hormPartitionGuid"));
    guid->setWordWrap(false);
    guid->setElideMode(Qt::ElideMiddle);
    guid->setText(volume.volumeId);
    guid->setToolTip(volume.volumeId);
    guid->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
    guid->setVerticalReference(TextVerticalReference::GlyphBounds);
    guid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    identityRow->addWidget(guid, 1);
    m_blockerRows->addWidget(identity, partitionRow, 0);
    auto* status = translated(m_blockers, QT_TRANSLATE_NOOP("uwf", "Unprotected"), 11, Color::Warning);
    status->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    status->setVerticalReference(TextVerticalReference::GlyphBounds);
    m_blockerRows->addWidget(status, partitionRow++, 1);
  }
  m_blockers->setVisible(state.blockersVisible);
  m_protectAll->setVisible(state.protectionVisible);
  m_protectionPending->setVisible(state.protectionPendingVisible);
  m_protectionPending->setText(state.protectionText);
  m_protectAll->setText(localized(QT_TRANSLATE_NOOP("uwf", "Protect all")));
  m_protectAll->setEnabled(state.protectEnabled);
  m_protectAll->setToolTip(state.protectionHint);
}
}  // namespace uwf::ui
#include "moc_OverlayMaintenanceDialogs.cpp"
