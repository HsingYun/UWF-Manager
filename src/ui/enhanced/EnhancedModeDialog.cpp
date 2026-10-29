// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "EnhancedModeDialog.h"

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

#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/DesignSystem.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/Notice.h"
#include "ui/widgets/StatusBadge.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TextLabel.h"
#include "ui/widgets/VerticalScrollArea.h"

namespace uwf::ui {

namespace {

void renderIssueLabel(Notice& label, const QString& text, const Sem tone) {
  label.setText(text);
  label.setVisible(!text.isEmpty());
  label.setTone(tone == Sem::Danger ? Notice::Tone::Error : tone == Sem::Warn ? Notice::Tone::Warning : Notice::Tone::Neutral);
}

ContentHintBox* selectableValue(const QString& text, QWidget* parent, const qreal lineHeight = 19.2) {
  auto* value = new ContentHintBox(parent);
  value->setProperty("designPlainDocument", true);
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
  shieldLayout->addWidget(new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/enhanced.svg")), 28, design::Color::Accent, shieldSurface));
  hero->addWidget(shieldSurface);
  auto* introduction = new QVBoxLayout();
  introduction->setSpacing(4);
  auto* title = new TextLabel(I18n::tr("Coordinate with Windows shutdown"), TextStyle::Section, this);
  auto type = title->font();
  type.setPixelSize(16);
  title->setFont(type);
  title->setWordWrap(true);
  introduction->addWidget(title);
  auto* tagline = new TextLabel(I18n::tr("Automatically save eligible staged files during normal shutdown and restart."), TextStyle::Caption, this);
  tagline->setWordWrap(true);
  tagline->setColorRole(design::Color::Muted);
  introduction->addWidget(tagline);
  hero->addLayout(introduction, 1);
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
  const auto addSetting = [settings, rows](const QString& name, const QString& description, const QString& icon) {
    auto* row = new Surface(design::Color::Surface, settings);
    row->setBorder(Qt::BottomEdge);
    row->setMinimumHeight(73);
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(18, 15, 18, 15);
    layout->setSpacing(14);
    layout->addWidget(new IconView(ThemeManager::instance().icon(icon), 20, design::Color::Muted, row));
    auto* text = new QVBoxLayout();
    text->setSpacing(3);
    auto* label = new TextLabel(name, TextStyle::Label, row);
    auto font = label->font();
    font.setWeight(QFont::Medium);
    label->setFont(font);
    label->setWordWrap(true);
    text->addWidget(label);
    auto* hint = new TextLabel(description, TextStyle::Caption, row);
    hint->setColorRole(design::Color::Muted);
    hint->setWordWrap(true);
    text->addWidget(hint);
    layout->addLayout(text, 1);
    auto* value = new TextLabel({}, TextStyle::Caption, row);
    value->setColorRole(design::Color::Muted);
    layout->addWidget(value);
    rows->addWidget(row);
    return value;
  };
  m_serviceValue = addSetting(I18n::tr("Background service"), I18n::tr("Starts automatically with Windows"), QStringLiteral(":/icons/service.svg"));
  m_stagingValue = addSetting(I18n::tr("Automatic file staging"), I18n::tr("Commits staged files before normal shutdown and restart"),
                              QStringLiteral(":/icons/file_staging.svg"));
  static_cast<Surface*>(m_stagingValue->parentWidget())->setBorder({});
  rootLayout->addWidget(settings);
  auto* fallback =
      new TextLabel(I18n::tr("Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files."), TextStyle::Caption, this);
  fallback->setColorRole(design::Color::Muted);
  fallback->setWordWrap(true);
  fallback->setLineHeight(1.7);
  auto* fallbackRow = new QHBoxLayout();
  fallbackRow->setSpacing(9);
  fallbackRow->addWidget(new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")), 17, design::Color::Muted, this), 0, Qt::AlignTop);
  fallbackRow->addWidget(fallback, 1);
  rootLayout->addLayout(fallbackRow);
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
  m_serviceDetails->setStyleSheet(QStringLiteral("QTextBrowser { background: transparent; border: 0; padding: 0; }"));
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
  connect(m_changeButton, &QPushButton::clicked, this, &EnhancedModeDialog::applyRequestedState);
  connect(m_removeButton, &QPushButton::clicked, this, &EnhancedModeDialog::removeService);
  connect(m_viewModel.get(), &presentation::EnhancedModeViewModel::statusChanged, this, &EnhancedModeDialog::render);
  connect(m_viewModel.get(), &presentation::EnhancedModeViewModel::statusUnavailable, this, &EnhancedModeDialog::renderStatusUnavailable);
  connect(m_viewModel.get(), &presentation::EnhancedModeViewModel::feedbackChanged, this, &EnhancedModeDialog::renderOperationFeedback);
  connect(m_viewModel.get(), &presentation::EnhancedModeViewModel::operationStarted, this, [this] {
    m_changeButton->setEnabled(false);
    m_removeButton->setEnabled(false);
  });
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
  m_serviceFacts->show();
  m_serviceDetails->hide();
  const std::array values{QStringLiteral("UWFManagerEnhanced"), view.account, view.startup,
                          QDir::toNativeSeparators(QCoreApplication::applicationFilePath()), view.agent};
  for (std::size_t index = 0; index < values.size(); ++index)
    if (m_serviceValues[index]->toPlainText() != values[index]) m_serviceValues[index]->setPlainText(values[index]);
  m_changeButton->setText(view.action);
  m_changeButton->setIcon(enabled ? QIcon{} : ThemeManager::instance().icon(QStringLiteral(":/icons/enhanced.svg")));
  setDialogAction(m_changeButton, view.primaryAction ? DialogAction::Primary : DialogAction::Secondary);
  m_changeButton->setEnabled(!m_viewModel->busy());
  m_removeButton->setEnabled(!m_viewModel->busy() && view.canRemove);
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

void EnhancedModeDialog::renderStatusUnavailable(const QString& detail) {
  m_changeButton->setEnabled(true);
  m_changeButton->setText(I18n::tr("Retry"));
  m_changeButton->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
  m_removeButton->setEnabled(false);
  m_serviceValue->setText(I18n::tr("Unavailable"));
  m_stagingValue->setText(I18n::tr("Unavailable"));
  m_statusLabel->setText(I18n::tr("Status unavailable"));
  static_cast<StatusBadge*>(m_statusLabel)->setTone(design::Color::Danger, design::Color::DangerBg);
  static_cast<StatusBadge*>(m_statusLabel)->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/status_error.svg")));
  m_issueLabel->setHeading(I18n::tr("Unable to read service status"));
  setDialogAction(m_changeButton, DialogAction::Primary);
  renderIssueLabel(*m_issueLabel, I18n::tr("The enhanced mode service status could not be read. Check service permissions and retry."), Sem::Danger);
  m_serviceFacts->hide();
  m_serviceDetails->setPlainText(detail);
  m_serviceDetails->show();
}

void EnhancedModeDialog::applyRequestedState() {
  if (!m_viewModel->status())
    m_viewModel->refreshStatus();
  else
    m_viewModel->applyRequestedState();
}

void EnhancedModeDialog::removeService() { m_viewModel->removeService(); }

}  // namespace uwf::ui

#include "moc_EnhancedModeDialog.cpp"
