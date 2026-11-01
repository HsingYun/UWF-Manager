// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/enhanced/EnhancedModeDialog.h"

#include <QColor>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QStringList>
#include <QStyle>
#include <QVBoxLayout>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/feedback/StatusBadge.h"
#include "presentation/controls/layout/AlignedGraphic.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/layout/VerticalScrollArea.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"

namespace uwf::ui {

namespace {

void renderIssueLabel(Notice& label, const QString& text, const Sem tone) {
  label.setText(text);
  label.setVisible(!text.isEmpty());
  label.setTone(tone == Sem::Danger ? Notice::Tone::Error : tone == Sem::Warn ? Notice::Tone::Warning : Notice::Tone::Neutral);
}

ContentHintBox* selectableValue(const QString& text, QWidget* parent, const qreal lineHeight = 19.2) {
  auto* value = new ContentHintBox(parent);
  auto font = value->font();
  font.setPixelSize(12);
  value->setFont(font);
  value->setLineHeight(lineHeight);
  value->setTextInteractionFlags(Qt::TextSelectableByMouse);
  value->setFocusPolicy(Qt::ClickFocus);
  value->setPlainText(text);
  return value;
}

}  // namespace

EnhancedModeDialog::EnhancedModeDialog(std::shared_ptr<presentation::EnhancedModeViewModel> viewModel, QWidget* parent)
    : PanelDialog(parent), m_viewModel(std::move(viewModel)) {
  setObjectName(QStringLiteral("enhancedModeDialog"));
  setWindowTitle(I18n::tr("Enhanced mode"));
  auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(740, 690), QSize(540, 0));
  auto* rootLayout = frame->body();
  frame->scrollArea()->setObjectName(QStringLiteral("enhancedModeScroll"));
  rootLayout->setSpacing(16);
  auto* hero = new QHBoxLayout();
  hero->setContentsMargins(0, 2, 0, 7);
  hero->setSpacing(15);
  auto* shieldSurface = new Surface(design::Color::Selected, this);
  shieldSurface->setFixedSize(48, 48);
  shieldSurface->setBorder({}, 8);
  auto* shieldLayout = new QVBoxLayout(shieldSurface);
  shieldLayout->setContentsMargins(10, 10, 10, 10);
  auto* heroIcon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/enhanced.svg")), 28, design::Color::Accent, shieldSurface);
  heroIcon->setObjectName(QStringLiteral("enhancedModeHeroIcon"));
  shieldLayout->addWidget(heroIcon);
  auto* introduction = new QWidget(this);
  auto* introductionLayout = new QVBoxLayout(introduction);
  introductionLayout->setContentsMargins(0, 0, 0, 0);
  introductionLayout->setSpacing(4);
  auto* title = new TextLabel(I18n::tr("Coordinate with Windows shutdown"), TextStyle::Section, introduction);
  title->setObjectName(QStringLiteral("enhancedModeHeroTitle"));
  auto type = title->font();
  type.setPixelSize(16);
  title->setFont(type);
  title->setWordWrap(true);
  introductionLayout->addWidget(title);
  auto* tagline = new TextLabel(I18n::tr("Automatically commit eligible staged files during Windows shutdown and restart."), TextStyle::Caption, introduction);
  tagline->setObjectName(QStringLiteral("enhancedModeHeroDescription"));
  tagline->setWordWrap(true);
  tagline->setColorRole(design::Color::Muted);
  introductionLayout->addWidget(tagline);
  hero->addWidget(new AlignedGraphic(shieldSurface, introduction, TextAnchor::VisibleBlock, this));
  hero->addWidget(introduction, 1);
  m_statusLabel = new StatusBadge(this);
  m_statusLabel->setObjectName(QStringLiteral("enhancedModeStatus"));
  hero->addWidget(m_statusLabel, 0, Qt::AlignVCenter);
  rootLayout->addLayout(hero);

  auto* settings = new Surface(design::Color::Surface, this);
  settings->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  settings->setClipContent(true);
  auto* rows = new QVBoxLayout(settings);
  rows->setContentsMargins(1, 1, 1, 1);
  rows->setSpacing(0);
  const auto addSetting = [settings, rows](const QString& name, const QString& description, const QString& icon, const QString& objectName,
                                           const Qt::Edges borders = Qt::BottomEdge) {
    auto* row = new Surface(design::Color::Surface, settings);
    row->setObjectName(objectName + QStringLiteral("Row"));
    row->setBorder(borders);
    row->setMinimumHeight(73);
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(18, 15, 18, 15);
    layout->setSpacing(14);
    auto* graphic = new IconView(ThemeManager::instance().icon(icon), 20, design::Color::Muted, row);
    graphic->setObjectName(objectName + QStringLiteral("Icon"));
    auto* text = new QVBoxLayout();
    text->setSpacing(3);
    auto* label = new TextLabel(name, TextStyle::Label, row);
    label->setObjectName(objectName + QStringLiteral("Title"));
    auto font = label->font();
    font.setWeight(QFont::Medium);
    label->setFont(font);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignLeading | Qt::AlignTop);
    text->addWidget(label);
    auto* hint = new TextLabel(description, TextStyle::Caption, row);
    hint->setObjectName(objectName + QStringLiteral("Description"));
    hint->setColorRole(design::Color::Muted);
    hint->setWordWrap(true);
    text->addWidget(hint);
    layout->addWidget(new AlignedGraphic(graphic, label, row));
    layout->addLayout(text, 1);
    auto* value = new TextLabel({}, TextStyle::Caption, row);
    value->setObjectName(objectName + QStringLiteral("State"));
    value->setColorRole(design::Color::Muted);
    layout->addWidget(new AlignedGraphic(value, label, row));
    rows->addWidget(row);
    return std::pair{value, hint};
  };
  m_serviceValue = addSetting(I18n::tr("Background service"), I18n::tr("Starts automatically with Windows"), QStringLiteral(":/icons/service.svg"),
                              QStringLiteral("enhancedModeService"))
                       .first;
  const auto staging =
      addSetting(I18n::tr("Automatic staged file commits"), {}, QStringLiteral(":/icons/file_staging.svg"), QStringLiteral("enhancedModeStaging"), {});
  m_stagingValue = staging.first;
  m_stagingValue->setObjectName(QStringLiteral("enhancedModeStagingState"));
  m_stagingDescription = staging.second;
  m_stagingDescription->setObjectName(QStringLiteral("enhancedModeStagingDescription"));
  rootLayout->addWidget(settings);
  auto* fallback =
      new TextLabel(I18n::tr("Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files."), TextStyle::Caption, this);
  fallback->setColorRole(design::Color::Muted);
  fallback->setObjectName(QStringLiteral("enhancedModeFallback"));
  fallback->setWordWrap(true);
  fallback->setLineHeight(1.7);
  m_fallbackHint = new QWidget(this);
  m_fallbackHint->setObjectName(QStringLiteral("enhancedModeFallbackRow"));
  auto* fallbackRow = new QHBoxLayout(m_fallbackHint);
  fallbackRow->setContentsMargins(0, 0, 0, 0);
  fallbackRow->setSpacing(9);
  auto* fallbackIcon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")), 17, design::Color::Muted, this);
  fallbackIcon->setObjectName(QStringLiteral("enhancedModeFallbackIcon"));
  fallbackRow->addWidget(new AlignedGraphic(fallbackIcon, fallback, this));
  fallbackRow->addWidget(fallback, 1);
  rootLayout->addWidget(m_fallbackHint);
  auto* safety = new Disclosure(I18n::tr("Operation and security"), this);
  safety->setObjectName(QStringLiteral("enhancedModeSafety"));
  auto* safetyContent = new QWidget(safety);
  auto* safetyLayout = new QVBoxLayout(safetyContent);
  safetyLayout->setContentsMargins(23, 12, 0, 2);
  safetyLayout->setSpacing(10);
  const auto safetyParagraph = [safetyContent](const QString& text) {
    auto* paragraph = new TextLabel(text, TextStyle::Caption, safetyContent);
    paragraph->setColorRole(design::Color::Muted);
    paragraph->setWordWrap(true);
    paragraph->setLineHeight(1.65);
    return paragraph;
  };
  safetyLayout->addWidget(safetyParagraph(
      I18n::tr("Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.")));
  auto* locationLayout = new QVBoxLayout();
  locationLayout->setSpacing(6);
  locationLayout->addWidget(safetyParagraph(I18n::tr("Keep the application in a fixed folder that standard users cannot modify. Suggested location:")));
  auto* locationSurface = new Surface(design::Color::SurfaceSoft, safetyContent);
  locationSurface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 4);
  auto* pathLayout = new QVBoxLayout(locationSurface);
  pathLayout->setContentsMargins(10, 8, 10, 8);
  const auto suggestedLocation = QDir::toNativeSeparators(
      QDir(qEnvironmentVariable("ProgramFiles", QStringLiteral("%ProgramFiles%"))).filePath(QStringLiteral("UWF Manager/Service/UWF.exe")));
  auto* location = selectableValue(suggestedLocation, locationSurface, 18);
  location->setObjectName(QStringLiteral("enhancedModeSuggestedLocation"));
  auto locationFont = location->font();
  locationFont.setFamilies(DesignSystem::monospaceFamilies());
  location->setFont(locationFont);
  pathLayout->addWidget(location);
  locationLayout->addWidget(locationSurface);
  safetyLayout->addLayout(locationLayout);
  safetyLayout->addWidget(safetyParagraph(
      I18n::tr("If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can "
               "make shutdown or restart take longer.")));
  safety->setContent(safetyContent);
  rootLayout->addWidget(safety);
  auto* details = new Disclosure(I18n::tr("Service details"), this);
  details->setObjectName(QStringLiteral("enhancedServiceDetails"));
  auto* serviceContent = new QWidget(details);
  auto* serviceLayout = new QVBoxLayout(serviceContent);
  serviceLayout->setContentsMargins(0, 12, 0, 0);
  auto* facts = new Surface(design::Color::Surface, serviceContent);
  facts->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 5);
  m_serviceFacts = facts;
  auto* fields = new QGridLayout(facts);
  fields->setContentsMargins(17, 15, 17, 15);
  fields->setSpacing(10);
  const QStringList captions{I18n::tr("Service name"), I18n::tr("Account"), I18n::tr("Startup type"), I18n::tr("Application path"), I18n::tr("UI agent")};
  for (int index = 0; index < 5; ++index) {
    auto* label = new TextLabel(captions[index], TextStyle::Caption, facts);
    label->setColorRole(design::Color::Muted);
    label->setFixedWidth(92);
    label->setWordWrap(true);
    auto* value = selectableValue({}, facts);
    value->setObjectName(QStringLiteral("enhancedModeServiceValue%1").arg(index));
    if (index == 0 || index == 1 || index == 3) {
      auto font = value->font();
      font.setFamilies(DesignSystem::monospaceFamilies());
      value->setFont(font);
    }
    m_serviceValues[static_cast<std::size_t>(index)] = value;
    fields->addWidget(label, index, 0, Qt::AlignTop);
    fields->addWidget(value, index, 1, Qt::AlignTop);
  }
  fields->setColumnStretch(1, 1);
  serviceLayout->addWidget(facts);
  m_serviceDetails = new ContentHintBox(serviceContent);
  auto detailsFont = m_serviceDetails->font();
  detailsFont.setPixelSize(12);
  m_serviceDetails->setFont(detailsFont);
  m_serviceDetails->setLineHeight(22.8);
  serviceLayout->addWidget(m_serviceDetails);
  m_serviceDetails->hide();
  details->setContent(serviceContent);
  rootLayout->addWidget(details);
  m_issueLabel = new Notice(this);
  m_issueLabel->setObjectName(QStringLiteral("enhancedModeIssue"));
  rootLayout->insertWidget(1, m_issueLabel);

  m_detailLabel = new Notice(this);
  m_detailLabel->setObjectName(QStringLiteral("enhancedModeDetail"));
  rootLayout->addWidget(m_detailLabel);
  rootLayout->addStretch();
  for (auto* text : {m_issueLabel, m_detailLabel}) {
    text->setFocusPolicy(Qt::ClickFocus);
  }
  auto* actions = new QDialogButtonBox(this);
  auto* auxiliary = new QDialogButtonBox(this);
  m_removeButton = new QPushButton(I18n::tr("Delete service") + QChar(0x2026), this);
  m_removeButton->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/trash.svg")));
  m_removeButton->setProperty("designRole", "dangerQuiet");
  m_removeButton->setObjectName(QStringLiteral("enhancedModeRemoveButton"));
  m_removeButton->setToolTip(I18n::tr("Stop and delete the enhanced mode service, including any remaining service registry data."));
  m_removeButton->setAutoDefault(false);
  setDialogAction(m_removeButton, DialogAction::DestructiveSecondary);
  auxiliary->addButton(m_removeButton, QDialogButtonBox::DestructiveRole);

  auto* close = new QPushButton(I18n::tr("Close"), this);
  close->setObjectName(QStringLiteral("enhancedModeCloseButton"));
  close->setDefault(true);
  actions->addButton(close, QDialogButtonBox::RejectRole);

  m_changeButton = new QPushButton(this);
  m_changeButton->setObjectName(QStringLiteral("enhancedModeChangeButton"));
  m_changeButton->setAutoDefault(false);
  actions->addButton(m_changeButton, QDialogButtonBox::AcceptRole);
  connect(close, &QPushButton::clicked, this, &QDialog::reject);
  connect(m_changeButton, &QPushButton::clicked, m_viewModel.get(), &presentation::EnhancedModeViewModel::applyRequestedState);
  connect(m_removeButton, &QPushButton::clicked, m_viewModel.get(), &presentation::EnhancedModeViewModel::removeService);
  connect(m_viewModel.get(), &presentation::EnhancedModeViewModel::statusChanged, this, &EnhancedModeDialog::render);
  connect(m_viewModel.get(), &presentation::EnhancedModeViewModel::statusUnavailable, this, &EnhancedModeDialog::render);
  connect(m_viewModel.get(), &presentation::EnhancedModeViewModel::feedbackChanged, this, &EnhancedModeDialog::render);
  connect(m_viewModel.get(), &presentation::EnhancedModeViewModel::operationStarted, this, &EnhancedModeDialog::render);
  frame->setActions(actions, auxiliary);

  m_viewModel->refreshStatus();
}

void EnhancedModeDialog::render() {
  const auto view = m_viewModel->statusText();
  using Tone = presentation::EnhancedModeViewModel::FeedbackTone;
  auto* badge = static_cast<StatusBadge*>(m_statusLabel);
  badge->setText(view.label);
  const bool enabled = view.statusTone == Tone::Success;
  badge->setTone(enabled                            ? design::Color::Success
                 : view.statusTone == Tone::Warning ? design::Color::Warning
                 : view.statusTone == Tone::Error   ? design::Color::Danger
                                                    : design::Color::Muted,
                 enabled                            ? design::Color::SuccessBg
                 : view.statusTone == Tone::Warning ? design::Color::WarningBg
                 : view.statusTone == Tone::Error   ? design::Color::DangerBg
                                                    : design::Color::SurfaceSoft);
  badge->setIcon(view.icon.isEmpty() ? QIcon{} : ThemeManager::instance().icon(view.icon));
  m_serviceValue->setText(view.service);
  static_cast<TextLabel*>(m_serviceValue)->setColorRole(enabled ? design::Color::Success : design::Color::Muted);
  m_stagingValue->setText(view.staging);
  m_stagingDescription->setText(view.stagingDescription);
  m_fallbackHint->setVisible(view.showFallbackCommitHint);
  m_serviceFacts->setVisible(view.showFacts);
  m_serviceDetails->setVisible(!view.showFacts && !view.details.isEmpty());
  m_serviceDetails->setPlainText(view.details);
  const std::array values{QStringLiteral("UWFManagerEnhanced"), view.account, view.startup, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()),
                          view.agent};
  for (std::size_t index = 0; index < values.size(); ++index)
    if (m_serviceValues[index]->toPlainText() != values[index]) m_serviceValues[index]->setPlainText(values[index]);
  m_changeButton->setText(view.action);
  m_changeButton->setIcon(view.actionIcon.isEmpty() ? QIcon{} : ThemeManager::instance().icon(view.actionIcon));
  setDialogAction(m_changeButton, view.primaryAction ? DialogAction::Primary : DialogAction::Secondary);
  m_changeButton->setEnabled(view.canApply);
  m_removeButton->setEnabled(view.canRemove);
  renderIssueLabel(*m_issueLabel, view.issues, view.issueTone == Tone::Error ? Sem::Danger : view.issueTone == Tone::Warning ? Sem::Warn : Sem::FgMuted);
  m_issueLabel->setHeading(view.issueHeading);
  renderOperationFeedback();
}

void EnhancedModeDialog::renderOperationFeedback() {
  if (!m_viewModel->operationFeedback()) {
    m_detailLabel->setText({});
    m_detailLabel->hide();
    return;
  }

  Sem detailSem = Sem::FgMuted;
  switch (m_viewModel->operationFeedback()->tone) {
    case presentation::EnhancedModeViewModel::FeedbackTone::Neutral:
      break;
    case presentation::EnhancedModeViewModel::FeedbackTone::Success:
      detailSem = Sem::AddOk;
      break;
    case presentation::EnhancedModeViewModel::FeedbackTone::Warning:
      detailSem = Sem::Warn;
      break;
    case presentation::EnhancedModeViewModel::FeedbackTone::Error:
      detailSem = Sem::Danger;
      break;
  }
  m_detailLabel->setText(m_viewModel->operationFeedback()->detail);
  m_detailLabel->show();
  m_detailLabel->setTone(detailSem == Sem::Danger  ? Notice::Tone::Error
                         : detailSem == Sem::Warn  ? Notice::Tone::Warning
                         : detailSem == Sem::AddOk ? Notice::Tone::Success
                                                   : Notice::Tone::Neutral);
}

}  // namespace uwf::ui

#include "moc_EnhancedModeDialog.cpp"
