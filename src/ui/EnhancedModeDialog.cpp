/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "EnhancedModeDialog.h"

#include <QColor>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QStringList>
#include <QStyle>
#include <QVBoxLayout>
#include <array>
#include <exception>
#include <stdexcept>

#include "ContentHintBox.h"
#include "I18n.h"
#include "ThemeManager.h"
#include "VerticalScrollArea.h"

namespace uwf::ui {

namespace {

struct StatusPresentation {
  QString text;
  Sem color;
};

StatusPresentation statusPresentation(const service::EnhancedModeState state) {
  switch (state) {
    case service::EnhancedModeState::Disabled:
      return {I18n::tr("Status: Disabled"), Sem::FgMuted};
    case service::EnhancedModeState::Enabled:
      return {I18n::tr("Status: Enabled"), Sem::AddOk};
    case service::EnhancedModeState::Stopped:
      return {I18n::tr("Status: Service stopped"), Sem::Warn};
    case service::EnhancedModeState::RepairRequired:
      return {I18n::tr("Status: Repair required"), Sem::Danger};
  }
  throw std::logic_error("unknown enhanced mode state");
}

struct ActionPresentation {
  QString text;
  QString style;
};

ActionPresentation actionPresentation(const service::EnhancedModeAction action) {
  switch (action) {
    case service::EnhancedModeAction::Enable:
      return {I18n::tr("Enable enhanced mode"), QStringLiteral("enable")};
    case service::EnhancedModeAction::Disable:
      return {I18n::tr("Disable enhanced mode"), QStringLiteral("neutral")};
    case service::EnhancedModeAction::Start:
      return {I18n::tr("Start service"), QStringLiteral("enable")};
    case service::EnhancedModeAction::Repair:
      return {I18n::tr("Repair enhanced mode"), QStringLiteral("repair")};
  }
  throw std::logic_error("unknown enhanced mode action");
}

QStringList statusIssues(const service::EnhancedModeStatus& status) {
  QStringList issues;
  if (status.serviceExists) {
    struct ContractRule {
      bool service::EnhancedModeStatus::* satisfied;
      const char* message;
    };
    constexpr std::array rules{
        ContractRule{&service::EnhancedModeStatus::ownProcess, QT_TRANSLATE_NOOP("uwf", "The service is not configured as an independent process.")},
        ContractRule{&service::EnhancedModeStatus::automaticStart, QT_TRANSLATE_NOOP("uwf", "The service is not configured for automatic startup.")},
        ContractRule{&service::EnhancedModeStatus::localSystemAccount, QT_TRANSLATE_NOOP("uwf", "The service is not configured to run as LocalSystem.")},
        ContractRule{&service::EnhancedModeStatus::executableMatches,
                     QT_TRANSLATE_NOOP("uwf", "The service executable path or startup arguments do not match this executable.")},
        ContractRule{&service::EnhancedModeStatus::preshutdownTimeoutConfigured,
                     QT_TRANSLATE_NOOP("uwf", "The service preshutdown timeout is not configured correctly.")},
        ContractRule{&service::EnhancedModeStatus::requiredPrivilegesConfigured,
                     QT_TRANSLATE_NOOP("uwf", "The service does not declare the privileges required to start the UI agent.")},
    };
    for (const auto& rule : rules) {
      if (!(status.*(rule.satisfied))) issues.append(I18n::tr(rule.message));
    }
    if (!status.running) {
      if (status.state != service::EnhancedModeState::Stopped) issues.append(I18n::tr("The service is not running."));
    } else if (!status.preshutdownAccepted) {
      issues.append(I18n::tr("The service has not accepted Windows preshutdown notifications."));
    }
  } else if (status.serviceRegistryPresent) {
    issues.append(I18n::tr("The service is absent, but its service registry data remains."));
  }

  if (status.running) {
    switch (status.agentState) {
      case service::EnhancedModeAgentState::Unobserved:
        break;
      case service::EnhancedModeAgentState::Connecting:
        issues.append(I18n::tr("Waiting for the UI agent to complete authentication."));
        break;
      case service::EnhancedModeAgentState::Disconnected:
        issues.append(I18n::tr("The service is running, but no authenticated UI agent is connected."));
        break;
      case service::EnhancedModeAgentState::Connected:
        break;
    }
  }
  if (!status.detail.isEmpty()) issues.append(status.detail);
  return issues;
}

Sem issueColor(const service::EnhancedModeStatus& status) {
  switch (status.state) {
    case service::EnhancedModeState::RepairRequired:
      return Sem::Danger;
    case service::EnhancedModeState::Stopped:
      return Sem::Warn;
    case service::EnhancedModeState::Disabled:
      return status.detail.isEmpty() ? Sem::FgMuted : Sem::Danger;
    case service::EnhancedModeState::Enabled:
      break;
  }
  if (!status.detail.isEmpty() || status.agentState == service::EnhancedModeAgentState::Disconnected) return Sem::Danger;
  return status.agentState == service::EnhancedModeAgentState::Connecting ? Sem::Warn : Sem::FgMuted;
}

void refreshDynamicStyle(QWidget& widget) {
  widget.style()->unpolish(&widget);
  widget.style()->polish(&widget);
  widget.update();
}

QString translucentBackground(const QColor& color, const int alpha) {
  return QStringLiteral("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(alpha);
}

void renderStatusLabel(QLabel& label, const StatusPresentation& presentation) {
  const auto& theme = ThemeManager::instance();
  const QColor color = theme.color(presentation.color);
  label.setText(presentation.text);
  label.setStyleSheet(QStringLiteral("QLabel#enhancedModeStatus { color: %1; background: %2; border: 1px solid %1; border-radius: 11px; padding: 4px 10px; }")
                          .arg(color.name(), translucentBackground(color, theme.isLight() ? 15 : 28)));
}

void renderIssueLabel(ContentHintBox& label, const QString& text, const QColor& color) {
  label.setPlainText(text);
  label.setVisible(!text.isEmpty());
  const auto& theme = ThemeManager::instance();
  label.setStyleSheet(
      QStringLiteral("QTextBrowser#enhancedModeIssue { color: %1; background: %2; border: 1px solid %3; border-radius: 8px; padding: 10px 12px; }")
          .arg(theme.color(Sem::Fg).name(), translucentBackground(color, theme.isLight() ? 10 : 18), theme.color(Sem::Border).name()));
}

QWidget* createChangeRow(QWidget* parent, const int number, const QString& text) {
  auto* row = new QWidget(parent);
  auto* layout = new QHBoxLayout(row);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(11);

  const auto& theme = ThemeManager::instance();
  auto* marker = new QLabel(QString::number(number), row);
  marker->setAlignment(Qt::AlignCenter);
  marker->setFixedSize(24, 24);
  QFont markerFont = marker->font();
  markerFont.setWeight(QFont::DemiBold);
  marker->setFont(markerFont);
  const QColor accent = theme.color(Sem::Accent);
  marker->setStyleSheet(
      QStringLiteral("QLabel { color: %1; background: %2; border-radius: 8px; }").arg(accent.name(), translucentBackground(accent, theme.isLight() ? 18 : 32)));
  layout->addWidget(marker, 0, Qt::AlignTop);

  auto* description = new QLabel(text, row);
  description->setWordWrap(true);
  description->setTextInteractionFlags(Qt::TextSelectableByMouse);
  description->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  layout->addWidget(description, 1);
  return row;
}

QWidget* createNoticeRow(QWidget* parent, const QString& text, const Sem tone = Sem::Warn) {
  auto* row = new QWidget(parent);
  auto* layout = new QHBoxLayout(row);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(10);

  const auto& theme = ThemeManager::instance();
  const QColor markerColor = theme.color(tone);
  auto* marker = new QLabel(QStringLiteral("!"), row);
  marker->setAlignment(Qt::AlignCenter);
  marker->setFixedSize(20, 20);
  QFont markerFont = marker->font();
  markerFont.setWeight(QFont::DemiBold);
  marker->setFont(markerFont);
  marker->setStyleSheet(QStringLiteral("QLabel { color: %1; background: %2; border-radius: 10px; }")
                            .arg(markerColor.name(), translucentBackground(markerColor, theme.isLight() ? 18 : 30)));
  layout->addWidget(marker, 0, Qt::AlignTop);

  auto* description = new QLabel(text, row);
  description->setWordWrap(true);
  description->setTextInteractionFlags(Qt::TextSelectableByMouse);
  description->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  description->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
  layout->addWidget(description, 1);
  return row;
}

QFrame* createDivider(QWidget* parent) {
  auto* divider = new QFrame(parent);
  divider->setFrameShape(QFrame::NoFrame);
  divider->setFixedHeight(1);
  divider->setStyleSheet(QStringLiteral("background: %1; border: 0;").arg(ThemeManager::instance().color(Sem::Border).name()));
  return divider;
}

}  // namespace

EnhancedModeDialog::EnhancedModeDialog(service::EnhancedModeManager& manager, QWidget* parent) : QDialog(parent), m_manager(manager) {
  setObjectName(QStringLiteral("enhancedModeDialog"));
  setWindowTitle(I18n::tr("Enhanced mode"));
  // 对话框包含会随服务状态出现或消失的故障与操作反馈。建立稳定的初始
  // 几何和足够的纵向余量，让这些内容消耗预留空间，而不是反复触发顶层
  // 窗口按 sizeHint 缩放，造成状态切换时整页跳动。
  setMinimumSize(700, 480);
  resize(740, 660);

  auto* windowLayout = new QVBoxLayout(this);
  windowLayout->setContentsMargins(26, 24, 26, 18);
  windowLayout->setSpacing(16);
  auto* content = new QWidget(this);
  auto* rootLayout = new QVBoxLayout(content);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(16);
  auto* scroll = new VerticalScrollArea(this);
  scroll->setObjectName(QStringLiteral("enhancedModeScroll"));
  scroll->setWidget(content);
  windowLayout->addWidget(scroll, 1);

  auto* header = new QHBoxLayout();
  header->setContentsMargins(0, 0, 0, 0);
  header->setSpacing(14);

  auto* shield = new QLabel(this);
  shield->setObjectName(QStringLiteral("enhancedModeShield"));
  shield->setFixedSize(48, 48);
  shield->setAlignment(Qt::AlignCenter);
  shield->setPixmap(ThemeManager::iconWithColor(":/icons/enhanced.svg", QColor(Qt::white), Qt::AlignCenter).pixmap(24, 24));
  shield->setStyleSheet(
      QStringLiteral("QLabel#enhancedModeShield { background: %1; border-radius: 12px; }").arg(ThemeManager::instance().color(Sem::Accent).name()));
  header->addWidget(shield, 0, Qt::AlignTop);

  auto* tagline = new QLabel(I18n::tr("Coordinate automatic file staging with Windows shutdown and restart."), this);
  tagline->setWordWrap(true);
  QFont taglineFont = tagline->font();
  taglineFont.setWeight(QFont::Medium);
  taglineFont.setPointSizeF(taglineFont.pointSizeF() + 1.0);
  tagline->setFont(taglineFont);
  header->addWidget(tagline, 1, Qt::AlignVCenter);

  m_statusLabel = new QLabel(this);
  m_statusLabel->setObjectName(QStringLiteral("enhancedModeStatus"));
  QFont statusFont = m_statusLabel->font();
  statusFont.setWeight(QFont::Medium);
  m_statusLabel->setFont(statusFont);
  header->addWidget(m_statusLabel, 0, Qt::AlignTop);
  rootLayout->addLayout(header);

  auto* summary = new QLabel(
      I18n::tr("Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit "
               "changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF."),
      this);
  summary->setWordWrap(true);
  summary->setStyleSheet(QStringLiteral("color: %1;").arg(ThemeManager::instance().color(Sem::FgMuted).name()));
  rootLayout->addWidget(summary);

  auto* disclosure = new QFrame(this);
  disclosure->setObjectName(QStringLiteral("enhancedModeDisclosure"));
  disclosure->setStyleSheet(QStringLiteral("QFrame#enhancedModeDisclosure { background: %1; border: 1px solid %2; border-radius: 8px; }")
                                .arg(ThemeManager::instance().color(Sem::Surface).name(), ThemeManager::instance().color(Sem::Border).name()));
  auto* disclosureLayout = new QVBoxLayout(disclosure);
  disclosureLayout->setContentsMargins(16, 14, 16, 14);
  disclosureLayout->setSpacing(10);
  auto* changesHeading = new QLabel(I18n::tr("System changes"), disclosure);
  QFont changesFont = changesHeading->font();
  changesFont.setWeight(QFont::Medium);
  changesHeading->setFont(changesFont);
  disclosureLayout->addWidget(changesHeading);
  disclosureLayout->addWidget(
      createChangeRow(disclosure, 1, I18n::tr("Creates the automatic service UWFManagerEnhanced and binds it to the current executable path")));
  disclosureLayout->addWidget(createDivider(disclosure));
  disclosureLayout->addWidget(createChangeRow(
      disclosure, 2, I18n::tr("Writes and commits the service configuration under HKLM\\SYSTEM\\CurrentControlSet\\Services\\UWFManagerEnhanced")));
  disclosureLayout->addWidget(createDivider(disclosure));
  disclosureLayout->addWidget(createChangeRow(
      disclosure, 3, I18n::tr("Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup")));
  disclosureLayout->addWidget(createDivider(disclosure));
  disclosureLayout->addWidget(createChangeRow(disclosure, 4, I18n::tr("Commits eligible staged files during normal Windows shutdown and restart")));
  rootLayout->addWidget(disclosure);

  auto* notices = new QFrame(this);
  notices->setObjectName(QStringLiteral("enhancedModeNotices"));
  const auto& theme = ThemeManager::instance();
  const QColor warning = theme.color(Sem::Warn);
  notices->setStyleSheet(QStringLiteral("QFrame#enhancedModeNotices { background: %1; border: 1px solid %2; border-radius: 8px; }")
                             .arg(translucentBackground(warning, theme.isLight() ? 7 : 12), theme.color(Sem::Border).name()));
  auto* noticesLayout = new QVBoxLayout(notices);
  noticesLayout->setContentsMargins(16, 13, 16, 13);
  noticesLayout->setSpacing(9);
  auto* noticesHeading = new QLabel(I18n::tr("Operational and security considerations"), notices);
  QFont noticesFont = noticesHeading->font();
  noticesFont.setWeight(QFont::Medium);
  noticesHeading->setFont(noticesFont);
  noticesLayout->addWidget(noticesHeading);
  noticesLayout->addWidget(createNoticeRow(
      notices, I18n::tr("Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable")));
  noticesLayout->addWidget(createNoticeRow(
      notices,
      I18n::tr("A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes")));
  noticesLayout->addWidget(createNoticeRow(
      notices,
      I18n::tr("Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a "
               "malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location "
               "(recommended location: %ProgramFiles%\\UWF Manager\\Service\\UWF.exe)"),
      Sem::Danger));
  rootLayout->addWidget(notices);

  m_issueLabel = new ContentHintBox(this);
  m_issueLabel->setObjectName(QStringLiteral("enhancedModeIssue"));
  m_issueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  rootLayout->addWidget(m_issueLabel);

  m_detailLabel = new ContentHintBox(this);
  m_detailLabel->setObjectName(QStringLiteral("enhancedModeDetail"));
  m_detailLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  rootLayout->addWidget(m_detailLabel);
  for (auto* text : {m_issueLabel, m_detailLabel}) {
    text->setFocusPolicy(Qt::ClickFocus);
  }
  rootLayout->addStretch(1);

  auto* actions = new QHBoxLayout();
  actions->setContentsMargins(0, 2, 0, 0);
  actions->setSpacing(10);
  m_removeButton = new QPushButton(I18n::tr("Delete service"), this);
  m_removeButton->setObjectName(QStringLiteral("enhancedModeRemoveButton"));
  m_removeButton->setToolTip(I18n::tr("Stop and delete the enhanced mode service, including any remaining service registry data."));
  m_removeButton->setAutoDefault(false);
  actions->addWidget(m_removeButton);
  actions->addStretch(1);

  auto* close = new QPushButton(I18n::tr("Close"), this);
  close->setObjectName(QStringLiteral("enhancedModeCloseButton"));
  close->setDefault(true);
  actions->addWidget(close);

  m_changeButton = new QPushButton(this);
  m_changeButton->setObjectName(QStringLiteral("enhancedModeChangeButton"));
  m_changeButton->setAutoDefault(false);
  actions->addWidget(m_changeButton);
  connect(close, &QPushButton::clicked, this, &QDialog::reject);
  connect(m_changeButton, &QPushButton::clicked, this, &EnhancedModeDialog::applyRequestedState);
  connect(m_removeButton, &QPushButton::clicked, this, &EnhancedModeDialog::removeService);
  connect(&m_manager, &service::EnhancedModeManager::agentConnectionStateChanged, this, [this](const service::EnhancedModeAgentState state) {
    if (!m_status) {
      refreshStatus();
      return;
    }
    auto status = *m_status;
    status.agentState = state;
    render(status);
  });
  windowLayout->addLayout(actions);

  refreshStatus();
}

void EnhancedModeDialog::refreshStatus() {
  try {
    render(m_manager.status());
  } catch (const std::exception& error) {
    renderStatusUnavailable(QString::fromUtf8(error.what()));
  } catch (...) {
    renderStatusUnavailable(I18n::tr("The enhanced mode status read failed because of an unknown error."));
  }
}

void EnhancedModeDialog::render(const service::EnhancedModeStatus& status) {
  m_status = status;
  m_action = status.availableAction();
  const auto state = statusPresentation(status.state);
  const auto action = actionPresentation(m_action);
  const auto& theme = ThemeManager::instance();
  renderStatusLabel(*m_statusLabel, state);
  m_changeButton->setText(action.text);
  m_changeButton->setProperty("actionStyle", action.style);
  refreshDynamicStyle(*m_changeButton);
  m_changeButton->setEnabled(true);
  m_removeButton->setEnabled(status.hasRemnants());

  const QString issues = statusIssues(status).join(QLatin1Char('\n'));
  const QColor issue = theme.color(issueColor(status));
  renderIssueLabel(*m_issueLabel, issues, issue);
  renderOperationFeedback();
}

void EnhancedModeDialog::setOperationFeedback(QString detail, const FeedbackTone tone) {
  m_operationFeedback.emplace(OperationFeedback{std::move(detail), tone});
  renderOperationFeedback();
}

void EnhancedModeDialog::clearOperationFeedback() {
  m_operationFeedback.reset();
  renderOperationFeedback();
}

void EnhancedModeDialog::renderOperationFeedback() {
  if (!m_operationFeedback) {
    m_detailLabel->clear();
    m_detailLabel->hide();
    return;
  }

  Sem detailSem = Sem::FgMuted;
  switch (m_operationFeedback->tone) {
    case FeedbackTone::Neutral:
      break;
    case FeedbackTone::Success:
      detailSem = Sem::AddOk;
      break;
    case FeedbackTone::Warning:
      detailSem = Sem::Warn;
      break;
    case FeedbackTone::Error:
      detailSem = Sem::Danger;
      break;
  }
  const auto& theme = ThemeManager::instance();
  const QColor detailColor = theme.color(detailSem);
  m_detailLabel->setPlainText(m_operationFeedback->detail);
  m_detailLabel->show();
  m_detailLabel->setStyleSheet(QStringLiteral("QTextBrowser#enhancedModeDetail { color: %1; background: %2; border-radius: 8px; padding: 10px 12px; }")
                                   .arg(detailColor.name(), translucentBackground(detailColor, theme.isLight() ? 10 : 18)));
}

void EnhancedModeDialog::renderStatusUnavailable(const QString& detail) {
  m_status.reset();
  m_changeButton->setEnabled(false);
  m_removeButton->setEnabled(false);
  renderStatusLabel(*m_statusLabel, {I18n::tr("Enhanced mode status could not be read"), Sem::Danger});
  renderIssueLabel(*m_issueLabel, detail, ThemeManager::instance().color(Sem::Danger));
  clearOperationFeedback();
}

void EnhancedModeDialog::recoverAfterOperationFailure(const QString& detail) {
  try {
    setOperationFeedback(detail, FeedbackTone::Error);
    render(m_manager.status());
  } catch (const std::exception& error) {
    renderStatusUnavailable(detail + QLatin1Char('\n') + I18n::tr("The current service state could not be read:\n%1").arg(QString::fromUtf8(error.what())));
  } catch (...) {
    renderStatusUnavailable(detail + QLatin1Char('\n') + I18n::tr("The current service state could not be read because of an unknown error."));
  }
}

void EnhancedModeDialog::applyRequestedState() {
  m_changeButton->setEnabled(false);
  m_removeButton->setEnabled(false);
  clearOperationFeedback();
  try {
    service::EnhancedModeChangeResult result;
    switch (m_action) {
      case service::EnhancedModeAction::Disable:
        result = m_manager.disable();
        break;
      case service::EnhancedModeAction::Start:
        result = m_manager.start();
        break;
      case service::EnhancedModeAction::Enable:
      case service::EnhancedModeAction::Repair:
        result = m_manager.enable(I18n::enhancedModeServiceDescription());
        break;
    }
    const QString message = result.persistenceWarning.isEmpty()
                                ? I18n::tr("The enhanced mode configuration was applied.")
                                : I18n::tr("The service configuration was applied, but UWF persistence reported:\n%1").arg(result.persistenceWarning);
    setOperationFeedback(message, result.persistenceWarning.isEmpty() ? FeedbackTone::Success : FeedbackTone::Warning);
    render(result.status);
  } catch (const std::exception& error) {
    recoverAfterOperationFailure(I18n::tr("The enhanced mode configuration failed:\n%1").arg(QString::fromUtf8(error.what())));
  } catch (...) {
    recoverAfterOperationFailure(I18n::tr("The enhanced mode configuration failed because of an unknown error."));
  }
}

void EnhancedModeDialog::removeService() {
  m_changeButton->setEnabled(false);
  m_removeButton->setEnabled(false);
  clearOperationFeedback();
  try {
    const auto result = m_manager.disable();
    const QString message = result.persistenceWarning.isEmpty()
                                ? I18n::tr("The enhanced mode service and its remaining configuration were deleted.")
                                : I18n::tr("The service deletion was requested, but residual cleanup reported:\n%1").arg(result.persistenceWarning);
    setOperationFeedback(message, result.persistenceWarning.isEmpty() ? FeedbackTone::Success : FeedbackTone::Warning);
    render(result.status);
  } catch (const std::exception& error) {
    recoverAfterOperationFailure(I18n::tr("The enhanced mode service could not be deleted:\n%1").arg(QString::fromUtf8(error.what())));
  } catch (...) {
    recoverAfterOperationFailure(I18n::tr("The enhanced mode service could not be deleted because of an unknown error."));
  }
}

}  // namespace uwf::ui
