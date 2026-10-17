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
#include "PowerConfirmDialog.h"

#include <QColor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <limits>
#include <stdexcept>
#include <utility>

#include "I18n.h"
#include "ThemeManager.h"
#include "UiTiming.h"
#include "VerticalScrollArea.h"

namespace uwf::ui {

namespace {

constexpr int kPreparationPollIntervalMs = 16;

int progressValue(const std::size_t value) { return static_cast<int>(std::min(value, static_cast<std::size_t>(std::numeric_limits<int>::max()))); }

class PowerDialog final : public QDialog {
 public:
  using QDialog::QDialog;

  void setRejectionEnabled(const bool enabled) { m_rejectionEnabled = enabled; }

  void reject() override {
    if (m_rejectionEnabled) QDialog::reject();
  }

 private:
  bool m_rejectionEnabled = true;
};

class PowerDialogLifecycle final {
 public:
  enum class State {
    Preparing,
    Confirmation,
    Staging,
    FailureDecision,
    Confirmed,
    Canceled,
    Count,
  };

  enum class Event {
    PreparationReady,
    BeginStaging,
    StagingCompleted,
    FailureObserved,
    Confirm,
    Cancel,
    Count,
  };

  explicit PowerDialogLifecycle(const bool preparationRequired) : m_state(preparationRequired ? State::Preparing : State::Confirmation) {}

  [[nodiscard]] State state() const { return m_state; }
  [[nodiscard]] bool terminal() const { return m_state == State::Confirmed || m_state == State::Canceled; }

  void post(const Event event) {
    const Transition transition = transitions().at(index(m_state, event));
    if (!transition.valid) throw std::logic_error("invalid power dialog state transition");
    m_state = transition.next;
    if (event == Event::FailureObserved) m_failureObserved = true;
  }

  [[nodiscard]] PowerActionDialogOutcome outcome() const {
    if (!terminal()) throw std::logic_error("power dialog outcome requested before a terminal state");
    if (m_state == State::Confirmed) {
      return m_failureObserved ? PowerActionDialogOutcome::ContinuedAfterStagingFailure : PowerActionDialogOutcome::Confirmed;
    }
    return m_failureObserved ? PowerActionDialogOutcome::CanceledAfterStagingFailure : PowerActionDialogOutcome::Canceled;
  }

 private:
  struct Transition {
    State next = State::Canceled;
    bool valid = false;
  };

  static constexpr std::size_t stateCount = static_cast<std::size_t>(State::Count);
  static constexpr std::size_t eventCount = static_cast<std::size_t>(Event::Count);

  static constexpr std::size_t index(const State state, const Event event) {
    return static_cast<std::size_t>(state) * eventCount + static_cast<std::size_t>(event);
  }

  static const std::array<Transition, stateCount * eventCount>& transitions() {
    static constexpr auto table = [] {
      std::array<Transition, stateCount * eventCount> result{};
      const auto set = [&result](const State from, const Event event, const State to) { result[index(from, event)] = {to, true}; };

      set(State::Preparing, Event::PreparationReady, State::Confirmation);
      set(State::Preparing, Event::FailureObserved, State::FailureDecision);
      set(State::Preparing, Event::Cancel, State::Canceled);

      set(State::Confirmation, Event::BeginStaging, State::Staging);
      set(State::Confirmation, Event::FailureObserved, State::FailureDecision);
      set(State::Confirmation, Event::Confirm, State::Confirmed);
      set(State::Confirmation, Event::Cancel, State::Canceled);

      set(State::Staging, Event::StagingCompleted, State::Confirmed);
      set(State::Staging, Event::FailureObserved, State::FailureDecision);

      set(State::FailureDecision, Event::Confirm, State::Confirmed);
      set(State::FailureDecision, Event::Cancel, State::Canceled);
      return result;
    }();
    return table;
  }

  State m_state;
  bool m_failureObserved = false;
};

}  // namespace

PowerActionDialogOutcome runPowerActionDialog(QWidget* parent, PowerActionDialogRequest request) {
  if (request.pollStagingPreparation && request.completedStagingFailure) {
    throw std::invalid_argument("power dialog cannot prepare and reuse a completed staging result simultaneously");
  }
  if (request.mode == PowerActionDialogMode::Direct && (request.pollStagingPreparation || request.completedStagingFailure)) {
    throw std::invalid_argument("direct power dialog cannot prepare or reuse file staging");
  }
  const PowerAction action = request.action;
  const bool preparationRequired = static_cast<bool>(request.pollStagingPreparation);
  const bool shutdown = action == PowerAction::Shutdown;
  const bool direct = request.mode == PowerActionDialogMode::Direct;
  const QString directText = shutdown ? I18n::tr("Direct shutdown") : I18n::tr("Direct restart");
  const QString title = direct ? directText : (shutdown ? I18n::tr("Safe shutdown") : I18n::tr("Safe restart"));
  const QString heading = direct ? (shutdown ? I18n::tr("Confirm direct shutdown?") : I18n::tr("Confirm direct restart?"))
                                 : (shutdown ? I18n::tr("Confirm safe shutdown?") : I18n::tr("Confirm safe restart?"));
  const QString actionText = direct ? directText : (shutdown ? I18n::tr("Safe shutdown") : I18n::tr("Safe restart"));
  const QString continueText = shutdown ? I18n::tr("Continue shutdown") : I18n::tr("Continue restart");
  const QString summaryText =
      direct ? (shutdown ? I18n::tr("The system will shut down without committing files in File staging.")
                         : I18n::tr("The system will restart without committing files in File staging."))
             : (shutdown ? I18n::tr("The system will shut down safely through UWF.") : I18n::tr("The system will restart safely through UWF."));
  const QString iconPath = shutdown ? QStringLiteral(":/icons/shutdown.svg") : QStringLiteral(":/icons/restart.svg");

  PowerDialog dialog(parent);
  dialog.setObjectName(QStringLiteral("powerConfirmDialog"));
  dialog.setWindowTitle(title);
  dialog.setWindowIcon(ThemeManager::instance().icon(iconPath));
  dialog.setMinimumWidth(direct ? 500 : 650);

  auto* windowLayout = new QVBoxLayout(&dialog);
  windowLayout->setContentsMargins(24, 22, 24, 16);
  windowLayout->setSpacing(16);
  auto* content = new QWidget(&dialog);
  auto* layout = new QVBoxLayout(content);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(16);
  auto* scroll = new VerticalScrollArea(&dialog);
  scroll->setObjectName(QStringLiteral("powerConfirmationScroll"));
  scroll->setWidget(content);
  windowLayout->addWidget(scroll, 1);

  auto* header = new QHBoxLayout();
  header->setContentsMargins(0, 0, 0, 0);
  header->setSpacing(16);

  const auto& theme = ThemeManager::instance();
  const QColor actionColor = theme.color(shutdown ? Sem::Danger : Sem::Warn);
  auto* icon = new QLabel(&dialog);
  icon->setObjectName(QStringLiteral("powerActionIcon"));
  icon->setAlignment(Qt::AlignCenter);
  icon->setFixedSize(52, 52);
  icon->setPixmap(ThemeManager::iconWithColor(iconPath, QColor(Qt::white)).pixmap(26, 26));
  icon->setStyleSheet(QStringLiteral("QLabel#powerActionIcon { background: %1; border-radius: 26px; }").arg(actionColor.name()));
  header->addWidget(icon, 0, Qt::AlignTop);

  auto* titles = new QVBoxLayout();
  titles->setContentsMargins(0, 1, 0, 0);
  titles->setSpacing(5);
  auto* headingLabel = new QLabel(heading, &dialog);
  headingLabel->setObjectName(QStringLiteral("powerActionHeading"));
  QFont headingFont = headingLabel->font();
  headingFont.setPointSizeF(headingFont.pointSizeF() + 2.0);
  headingFont.setWeight(QFont::Medium);
  headingLabel->setFont(headingFont);
  titles->addWidget(headingLabel);

  auto* summary = new QLabel(summaryText, &dialog);
  summary->setObjectName(QStringLiteral("powerActionSummary"));
  summary->setWordWrap(true);
  summary->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
  titles->addWidget(summary);
  header->addLayout(titles, 1);
  layout->addLayout(header);

  auto* safetyCard = new QFrame(&dialog);
  safetyCard->setObjectName(QStringLiteral("powerSafetyCard"));
  safetyCard->setStyleSheet(QStringLiteral("QFrame#powerSafetyCard { background: %1; border: 1px solid %2; border-radius: 9px; }")
                                .arg(theme.color(Sem::Surface).name(), theme.color(Sem::Border).name()));
  auto* safetyLayout = new QVBoxLayout(safetyCard);
  safetyLayout->setContentsMargins(14, 12, 14, 12);
  safetyLayout->setSpacing(4);
  auto* safetyHeading = new QLabel(direct ? I18n::tr("File staging will be skipped") : I18n::tr("UWF protection"), safetyCard);
  QFont safetyFont = safetyHeading->font();
  safetyFont.setWeight(QFont::Medium);
  safetyHeading->setFont(safetyFont);
  safetyLayout->addWidget(safetyHeading);
  auto* safetyDetail = new QLabel(direct ? (shutdown ? I18n::tr("Files in File staging will not be committed before shutdown.")
                                                     : I18n::tr("Files in File staging will not be committed before restart."))
                                         : I18n::tr("This operation remains available even if the UWF overlay is full."),
                                  safetyCard);
  safetyDetail->setWordWrap(true);
  safetyDetail->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
  safetyLayout->addWidget(safetyDetail);
  auto* stagingDetail = new QLabel(I18n::tr("Calculating files for automatic commit…"), safetyCard);
  stagingDetail->setObjectName(QStringLiteral("powerStagingDetail"));
  stagingDetail->setWordWrap(true);
  stagingDetail->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
  stagingDetail->setVisible(preparationRequired);
  safetyLayout->addWidget(stagingDetail);

  auto* progress = new QProgressBar(safetyCard);
  progress->setObjectName(QStringLiteral("powerStagingProgress"));
  progress->setRange(0, 0);
  progress->setTextVisible(true);
  progress->setVisible(preparationRequired);
  safetyLayout->addWidget(progress);

  auto* currentPath = new QLineEdit(safetyCard);
  currentPath->setLayoutDirection(Qt::LeftToRight);
  currentPath->setObjectName(QStringLiteral("powerStagingCurrentPath"));
  currentPath->setReadOnly(true);
  currentPath->setFrame(false);
  currentPath->setStyleSheet(QStringLiteral("QLineEdit { color: %1; background: transparent; border: 0; padding: 0; }").arg(theme.color(Sem::FgMuted).name()));
  currentPath->hide();
  safetyLayout->addWidget(currentPath);

  auto* failureDetails = new QPlainTextEdit(safetyCard);
  failureDetails->setObjectName(QStringLiteral("powerStagingFailureDetails"));
  failureDetails->setReadOnly(true);
  failureDetails->setLineWrapMode(QPlainTextEdit::WidgetWidth);
  failureDetails->setMaximumHeight(190);
  failureDetails->hide();
  safetyLayout->addWidget(failureDetails);
  layout->addWidget(safetyCard);

  auto* warningCard = new QFrame(&dialog);
  warningCard->setObjectName(QStringLiteral("powerWarningCard"));
  QColor warningBackground = theme.color(Sem::Danger);
  warningBackground.setAlpha(theme.isLight() ? 16 : 28);
  warningCard->setStyleSheet(QStringLiteral("QFrame#powerWarningCard { background: rgba(%1, %2, %3, %4); border: 1px solid %5; border-radius: 9px; }")
                                 .arg(warningBackground.red())
                                 .arg(warningBackground.green())
                                 .arg(warningBackground.blue())
                                 .arg(warningBackground.alpha())
                                 .arg(theme.color(Sem::Danger).name()));
  auto* warningLayout = new QVBoxLayout(warningCard);
  warningLayout->setContentsMargins(14, 11, 14, 11);
  warningLayout->setSpacing(4);
  auto* warningHeading = new QLabel(I18n::tr("Uncommitted changes will be lost"), warningCard);
  warningHeading->setObjectName(QStringLiteral("powerWarningHeading"));
  QFont warningFont = warningHeading->font();
  warningFont.setWeight(QFont::Medium);
  warningHeading->setFont(warningFont);
  warningHeading->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::Danger).name()));
  warningLayout->addWidget(warningHeading);
  auto* warningDetail = new QLabel(I18n::tr("Before continuing, save your work and commit any changes that you want to keep permanently."), warningCard);
  warningDetail->setObjectName(QStringLiteral("powerWarningDetail"));
  warningDetail->setWordWrap(true);
  warningLayout->addWidget(warningDetail);
  layout->addWidget(warningCard);

  auto* buttons = new QDialogButtonBox(&dialog);
  auto* actionButton = buttons->addButton(actionText, QDialogButtonBox::AcceptRole);
  actionButton->setObjectName(direct ? (shutdown ? QStringLiteral("directShutdownBtn") : QStringLiteral("directRestartBtn"))
                                     : (shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn")));
  QPushButton* directActionButton = nullptr;
  if (!direct) {
    directActionButton = buttons->addButton(directText, QDialogButtonBox::DestructiveRole);
    directActionButton->setObjectName(shutdown ? QStringLiteral("directShutdownBtn") : QStringLiteral("directRestartBtn"));
    directActionButton->setAutoDefault(false);
  }
  auto* cancelButton = buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
  auto* buttonRow = new QHBoxLayout();
  auto* directHint =
      new QLabel(shutdown ? I18n::tr("Direct shutdown skips File staging commits.") : I18n::tr("Direct restart skips File staging commits."), &dialog);
  directHint->setObjectName(QStringLiteral("directPowerHint"));
  directHint->setWordWrap(true);
  directHint->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
  directHint->setVisible(!direct);
  buttonRow->addWidget(directHint, 1);
  buttonRow->addWidget(buttons);
  windowLayout->addLayout(buttonRow);
  actionButton->setAutoDefault(false);
  actionButton->setEnabled(!preparationRequired);
  cancelButton->setAutoDefault(true);
  cancelButton->setDefault(true);
  cancelButton->setFocus(Qt::OtherFocusReason);

  PowerDialogLifecycle lifecycle(preparationRequired);
  std::size_t filesToCommit = 0;
  std::function<PowerStagingProgress()> advanceStaging;
  QTimer preparationTimer(&dialog);
  preparationTimer.setSingleShot(true);
  preparationTimer.setTimerType(Qt::PreciseTimer);
  QElapsedTimer preparationElapsed;
  std::optional<PowerStagingPreparation> completedPreparation;
  QTimer stepTimer(&dialog);
  stepTimer.setSingleShot(true);
  bool directSelected = false;
  bool stagingStarted = false;

  const auto showFailure = [&](const PowerStagingProgress& status) {
    preparationTimer.stop();
    stepTimer.stop();
    lifecycle.post(PowerDialogLifecycle::Event::FailureObserved);
    headingLabel->setText(I18n::tr("Automatic file staging did not complete"));
    summary->setText(I18n::tr("Review the operation details before deciding whether to continue."));
    safetyHeading->setText(I18n::tr("Operation details"));
    safetyDetail->hide();
    stagingDetail->hide();
    progress->setMaximum(std::max(1, progressValue(status.totalFiles)));
    progress->setValue(progressValue(status.processedFiles));
    progress->setVisible(status.totalFiles > 0);
    currentPath->hide();
    failureDetails->setPlainText(status.failureDetails.isEmpty() ? I18n::tr("No additional error details were provided.") : status.failureDetails);
    failureDetails->show();
    warningHeading->setText(I18n::tr("Some changes may not be preserved"));
    warningDetail->setText(shutdown ? I18n::tr("If you continue, the system will shut down without the files that could not be committed.")
                                    : I18n::tr("If you continue, the system will restart without the files that could not be committed."));
    warningCard->show();
    actionButton->setText(continueText);
    actionButton->setEnabled(true);
    if (directActionButton) {
      directActionButton->setVisible(!stagingStarted);
      directActionButton->setEnabled(!stagingStarted);
      directHint->setVisible(!stagingStarted);
    }
    cancelButton->setEnabled(true);
    dialog.setRejectionEnabled(true);
    cancelButton->setDefault(true);
    cancelButton->setFocus(Qt::OtherFocusReason);
  };

  QObject::connect(&stepTimer, &QTimer::timeout, &dialog, [&] {
    PowerStagingProgress status;
    try {
      status = advanceStaging();
    } catch (const std::exception& error) {
      status = {PowerStagingState::Failed,
                static_cast<std::size_t>(std::max(0, progress->value())),
                filesToCommit,
                {},
                I18n::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what()))};
    } catch (...) {
      status = {PowerStagingState::Failed,
                static_cast<std::size_t>(std::max(0, progress->value())),
                filesToCommit,
                {},
                I18n::tr("Automatic file staging stopped because of an unknown error.")};
    }

    progress->setMaximum(std::max(1, progressValue(status.totalFiles)));
    progress->setValue(progressValue(status.processedFiles));
    stagingDetail->setText(
        I18n::tr("Processed %1 of %2 file(s).").arg(static_cast<qulonglong>(status.processedFiles)).arg(static_cast<qulonglong>(status.totalFiles)));
    if (!status.currentPath.isEmpty()) {
      currentPath->setText(status.currentPath);
      currentPath->setToolTip(status.currentPath);
      currentPath->setCursorPosition(0);
      currentPath->show();
    }

    switch (status.state) {
      case PowerStagingState::InProgress:
        stepTimer.start(0);
        break;
      case PowerStagingState::Completed:
        lifecycle.post(PowerDialogLifecycle::Event::StagingCompleted);
        dialog.accept();
        break;
      case PowerStagingState::Failed:
        showFailure(status);
        break;
    }
  });

  QObject::connect(&preparationTimer, &QTimer::timeout, &dialog, [&] {
    if (!completedPreparation) {
      try {
        auto prepared = request.pollStagingPreparation();
        if (prepared) completedPreparation.emplace(std::move(*prepared));
      } catch (const std::exception& error) {
        completedPreparation.emplace(PowerStagingFailure{I18n::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what()))});
      } catch (...) {
        completedPreparation.emplace(PowerStagingFailure{I18n::tr("Automatic file staging stopped because of an unknown error.")});
      }
    }

    // 首次轮询已经返回，事件循环接下来即可完成进度条绘制；从这里开始计算
    // 最短展示时间，不能把首次同步探测占用的时间算作用户已经看见的时间。
    if (!preparationElapsed.isValid()) preparationElapsed.start();

    if (!completedPreparation) {
      preparationTimer.start(kPreparationPollIntervalMs);
      return;
    }

    // 扫描结果可以立即返回，但计算态至少稳定展示 2 秒。这里只延迟状态提交，
    // 不等待线程、不阻塞事件循环；精确定时器若提前唤醒，再按剩余时间调度。
    const auto elapsed = std::chrono::milliseconds{preparationElapsed.elapsed()};
    const auto remaining = timing::kMinimumProgressDisplayDuration - elapsed;
    if (remaining > std::chrono::milliseconds::zero()) {
      preparationTimer.start(remaining);
      return;
    }

    request.pollStagingPreparation = {};
    PowerStagingPreparation prepared = std::move(*completedPreparation);
    completedPreparation.reset();
    if (const auto* failure = std::get_if<PowerStagingFailure>(&prepared)) {
      showFailure({PowerStagingState::Failed, 0, 0, {}, failure->details});
      return;
    }

    if (auto* commit = std::get_if<PowerStagingCommit>(&prepared)) {
      filesToCommit = commit->filesToCommit;
      advanceStaging = std::move(commit->advanceStaging);
    }

    lifecycle.post(PowerDialogLifecycle::Event::PreparationReady);
    progress->hide();
    if (advanceStaging && filesToCommit > 0) {
      stagingDetail->setText(
          I18n::tr("%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.")
              .arg(static_cast<qulonglong>(filesToCommit)));
      stagingDetail->show();
    } else {
      stagingDetail->hide();
    }
    actionButton->setEnabled(true);
  });

  QObject::connect(actionButton, &QPushButton::clicked, &dialog, [&] {
    switch (lifecycle.state()) {
      case PowerDialogLifecycle::State::FailureDecision:
        lifecycle.post(PowerDialogLifecycle::Event::Confirm);
        dialog.accept();
        return;
      case PowerDialogLifecycle::State::Confirmation:
        if (!advanceStaging) {
          lifecycle.post(PowerDialogLifecycle::Event::Confirm);
          dialog.accept();
          return;
        }
        break;
      case PowerDialogLifecycle::State::Preparing:
      case PowerDialogLifecycle::State::Staging:
      case PowerDialogLifecycle::State::Confirmed:
      case PowerDialogLifecycle::State::Canceled:
      case PowerDialogLifecycle::State::Count:
        return;
    }

    lifecycle.post(PowerDialogLifecycle::Event::BeginStaging);
    stagingStarted = true;
    directHint->hide();
    headingLabel->setText(I18n::tr("Committing staged files"));
    summary->setText(I18n::tr("The power action will continue after file staging completes."));
    stagingDetail->setText(I18n::tr("Processed %1 of %2 file(s).").arg(0).arg(static_cast<qulonglong>(filesToCommit)));
    stagingDetail->show();
    progress->setRange(0, std::max(1, progressValue(filesToCommit)));
    progress->setValue(0);
    progress->show();
    warningCard->hide();
    actionButton->setEnabled(false);
    if (directActionButton) directActionButton->setEnabled(false);
    cancelButton->setEnabled(false);
    dialog.setRejectionEnabled(false);
    stepTimer.start(0);
  });

  if (directActionButton) {
    QObject::connect(directActionButton, &QPushButton::clicked, &dialog, [&] {
      preparationTimer.stop();
      stepTimer.stop();
      directSelected = true;
      dialog.accept();
    });
  }

  if (request.completedStagingFailure) {
    showFailure({PowerStagingState::Failed, 0, 0, {}, std::move(request.completedStagingFailure->details)});
  } else if (preparationRequired) {
    preparationTimer.start(kPreparationPollIntervalMs);
  }
  const bool accepted = dialog.exec() == QDialog::Accepted;
  if (directSelected) return PowerActionDialogOutcome::DirectConfirmed;
  if (!lifecycle.terminal()) {
    if (accepted)
      lifecycle.post(PowerDialogLifecycle::Event::Confirm);
    else
      lifecycle.post(PowerDialogLifecycle::Event::Cancel);
  }
  const auto outcome = lifecycle.outcome();
  if (direct && outcome == PowerActionDialogOutcome::Confirmed) return PowerActionDialogOutcome::DirectConfirmed;
  return outcome;
}

}  // namespace uwf::ui
