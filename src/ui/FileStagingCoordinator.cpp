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
#include "FileStagingCoordinator.h"

#include <QDialog>
#include <QElapsedTimer>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <chrono>
#include <exception>
#include <limits>
#include <optional>
#include <utility>

#include "../util/Log.h"
#include "../uwf/FileStagingTask.h"
#include "FileStagingPresentation.h"
#include "I18n.h"
#include "ThemeManager.h"
#include "UiTiming.h"

namespace uwf::ui {

namespace {

constexpr int kPollIntervalMs = 16;

int progressValue(const std::size_t value) { return static_cast<int>(std::min(value, static_cast<std::size_t>(std::numeric_limits<int>::max()))); }

void notifyCompletion(const FileStagingCoordinator::Completion& completion, const FileStagingBatchResult& result) noexcept {
  try {
    completion(result);
  } catch (const std::exception& error) {
    UWF_LOG_E("staging") << "file staging completion observer failed: error=" << error.what();
  } catch (...) {
    UWF_LOG_E("staging") << "file staging completion observer failed: error=non-standard-exception";
  }
}

void notifyExternalCompletion(FileStagingCoordinator::ExternalCompletion& completion, const FileStagingBatchResult& result,
                              FileStagingCoordinator::ExternalBatch batch) noexcept {
  try {
    completion(result, std::move(batch));
  } catch (const std::exception& error) {
    UWF_LOG_E("staging") << "file staging external completion observer failed: error=" << error.what();
  } catch (...) {
    UWF_LOG_E("staging") << "file staging external completion observer failed: error=non-standard-exception";
  }
}

void notifyProgress(const std::function<void(std::size_t, std::size_t)>& progress, const std::size_t processed, const std::size_t total) noexcept {
  try {
    progress(processed, total);
  } catch (const std::exception& error) {
    UWF_LOG_E("staging") << "file staging progress observer failed: error=" << error.what();
  } catch (...) {
    UWF_LOG_E("staging") << "file staging progress observer failed: error=non-standard-exception";
  }
}

}  // namespace

class FileStagingCoordinator::Operation final : public QDialog {
 public:
  using Completion = std::function<void(const FileStagingBatchResult&)>;
  using Progress = std::function<void(std::size_t, std::size_t)>;

  Operation(WmiOperations& session, app::FileStagingStore& store, const UwfCapability capability, QWidget* parent, Completion completion, Progress progress)
      : QDialog(parent), m_task(session, store, capability), m_completion(std::move(completion)), m_progressObserver(std::move(progress)) {
    setObjectName(QStringLiteral("fileStagingProgressDialog"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setMinimumWidth(430);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(12);
    m_heading = new QLabel(I18n::tr("Committing staged files"), this);
    QFont font = m_heading->font();
    font.setWeight(QFont::Medium);
    font.setPointSizeF(font.pointSizeF() + 1.0);
    m_heading->setFont(font);
    layout->addWidget(m_heading);

    m_detail = new QLabel(I18n::tr("Calculating files for automatic commit…"), this);
    m_detail->setWordWrap(true);
    m_detail->setStyleSheet(QStringLiteral("color: %1;").arg(ThemeManager::instance().color(Sem::FgMuted).name()));
    layout->addWidget(m_detail);

    m_progress = new QProgressBar(this);
    m_progress->setObjectName(QStringLiteral("fileStagingCommandProgress"));
    m_progress->setRange(0, 0);
    layout->addWidget(m_progress);

    m_path = new QLineEdit(this);
    m_path->setLayoutDirection(Qt::LeftToRight);
    m_path->setObjectName(QStringLiteral("fileStagingCommandPath"));
    m_path->setReadOnly(true);
    m_path->setFrame(false);
    m_path->setStyleSheet(
        QStringLiteral("QLineEdit { color: %1; background: transparent; border: 0; padding: 0; }").arg(ThemeManager::instance().color(Sem::FgMuted).name()));
    m_path->hide();
    layout->addWidget(m_path);

    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, [this] { advance(); });
  }

  void start() {
    show();
    raise();
    activateWindow();
    m_visibleElapsed.start();
    m_timer.start(0);
  }

  void reject() override {
    // 自动提交是系统关机屏障的一部分；Esc、Alt+F4 和窗口系统关闭请求都不能
    // 把一个只处理到一半的批次伪装成完成。
  }

 private:
  void advance() {
    try {
      if (!m_task.preparationFinished()) {
        if (!m_task.pollPreparation()) {
          const std::size_t discovered = m_task.discoveredDuringScan();
          if (discovered != m_lastReportedProgress) {
            m_lastReportedProgress = discovered;
            notifyProgress(m_progressObserver, discovered, 0);
          }
          m_timer.start(kPollIntervalMs);
          return;
        }
        m_progress->setRange(0, std::max(1, progressValue(m_task.totalFiles())));
        m_progress->setValue(0);
      }

      if (!m_task.finished()) {
        const auto progress = m_task.advance();
        m_progress->setMaximum(std::max(1, progressValue(progress.totalFiles)));
        m_progress->setValue(progressValue(progress.processedFiles));
        m_detail->setText(
            I18n::tr("Processed %1 of %2 file(s).").arg(static_cast<qulonglong>(progress.processedFiles)).arg(static_cast<qulonglong>(progress.totalFiles)));
        m_path->setText(progress.currentPath);
        m_path->setCursorPosition(0);
        m_path->setToolTip(progress.currentPath);
        m_path->setVisible(!progress.currentPath.isEmpty());
        notifyProgress(m_progressObserver, progress.processedFiles, progress.totalFiles);
        m_timer.start(0);
        return;
      }

      FileStagingBatchResult batch{toApplicationCommandResult(m_task.result()), m_task.sourceEntries()};
      m_heading->setText(batch.command.completed() ? I18n::tr("File staging completed") : I18n::tr("File staging failed"));
      switch (batch.command.outcome) {
        case app::ApplicationCommandOutcome::Succeeded:
          m_detail->setText(I18n::tr("All staged targets were processed."));
          break;
        case app::ApplicationCommandOutcome::CompletedWithFailures:
          m_detail->setText(I18n::tr("All staged targets were processed; one or more operations failed."));
          break;
        case app::ApplicationCommandOutcome::Rejected:
        case app::ApplicationCommandOutcome::Failed:
          m_detail->setText(batch.command.detail.isEmpty() ? I18n::tr("Automatic file staging stopped because of an unknown error.") : batch.command.detail);
          break;
      }
      finishAfterMinimumDuration(std::move(batch));
    } catch (const std::exception& error) {
      finishAfterMinimumDuration({app::ApplicationCommandResult::infrastructureFailure(QString::fromUtf8(error.what())), m_task.sourceEntries()});
    } catch (...) {
      finishAfterMinimumDuration({app::ApplicationCommandResult::infrastructureFailure(I18n::tr("Automatic file staging stopped because of an unknown error.")),
                                  m_task.sourceEntries()});
    }
  }

  void finishAfterMinimumDuration(FileStagingBatchResult result) {
    if (m_finalResult) return;
    m_finalResult = std::move(result);
    const auto elapsed = std::chrono::milliseconds{m_visibleElapsed.elapsed()};
    const auto remaining = timing::kMinimumProgressDisplayDuration - elapsed;
    if (remaining > std::chrono::milliseconds::zero()) {
      m_timer.stop();
      QTimer::singleShot(remaining, this, [this] { complete(); });
      return;
    }
    complete();
  }

  void complete() {
    if (!m_finalResult) return;
    hide();
    auto completion = std::move(m_completion);
    auto result = std::move(*m_finalResult);
    m_finalResult.reset();
    completion(result);
  }

  FileStagingTask m_task;
  Completion m_completion;
  Progress m_progressObserver;
  QLabel* m_heading = nullptr;
  QLabel* m_detail = nullptr;
  QProgressBar* m_progress = nullptr;
  QLineEdit* m_path = nullptr;
  QTimer m_timer;
  QElapsedTimer m_visibleElapsed;
  std::optional<FileStagingBatchResult> m_finalResult;
  std::size_t m_lastReportedProgress = 0;
};

FileStagingCoordinator::FileStagingCoordinator(WmiOperations& session, app::FileStagingStore& store, const UwfCapability capability, QWidget* parentWindow,
                                               QObject* parent)
    : QObject(parent), m_session(session), m_store(store), m_capability(capability), m_parentWindow(parentWindow) {}

FileStagingCoordinator::~FileStagingCoordinator() = default;

const std::array<FileStagingCoordinator::OwnershipTransition,
                 static_cast<std::size_t>(FileStagingCoordinator::Ownership::Count) * static_cast<std::size_t>(FileStagingCoordinator::OwnershipEvent::Count)>&
FileStagingCoordinator::ownershipTransitions() {
  static constexpr auto table = [] {
    std::array<OwnershipTransition, static_cast<std::size_t>(Ownership::Count) * static_cast<std::size_t>(OwnershipEvent::Count)> result{};
    const auto set = [&result](const Ownership from, const OwnershipEvent event, const Ownership to) {
      result[static_cast<std::size_t>(from) * static_cast<std::size_t>(OwnershipEvent::Count) + static_cast<std::size_t>(event)] = {to, true};
    };
    set(Ownership::Idle, OwnershipEvent::CommandStarted, Ownership::Command);
    set(Ownership::Idle, OwnershipEvent::ExternalReserved, Ownership::External);
    set(Ownership::Command, OwnershipEvent::CommandCompleted, Ownership::Idle);
    set(Ownership::Command, OwnershipEvent::CommandHandedOff, Ownership::External);
    set(Ownership::External, OwnershipEvent::ExternalCompleted, Ownership::Idle);
    set(Ownership::External, OwnershipEvent::ExternalReleased, Ownership::Idle);
    return result;
  }();
  return table;
}

void FileStagingCoordinator::postOwnershipEvent(const OwnershipEvent event) {
  const std::size_t index = static_cast<std::size_t>(m_ownership) * static_cast<std::size_t>(OwnershipEvent::Count) + static_cast<std::size_t>(event);
  const auto& transition = ownershipTransitions().at(index);
  if (!transition.valid) throw std::logic_error("invalid file staging ownership transition");
  m_ownership = transition.next;
}

FileStagingCoordinator::ExternalBatch::~ExternalBatch() { release(); }

FileStagingCoordinator::ExternalBatch::ExternalBatch(ExternalBatch&& other) noexcept : m_owner(std::exchange(other.m_owner, {})) {}

FileStagingCoordinator::ExternalBatch& FileStagingCoordinator::ExternalBatch::operator=(ExternalBatch&& other) noexcept {
  if (this == &other) return *this;
  release();
  m_owner = std::exchange(other.m_owner, {});
  return *this;
}

void FileStagingCoordinator::ExternalBatch::complete(const FileStagingBatchResult& result) {
  if (!m_owner) return;
  auto owner = std::exchange(m_owner, {});
  owner->completeExternalBatch(result);
}

void FileStagingCoordinator::ExternalBatch::release() {
  if (!m_owner) return;
  auto owner = std::exchange(m_owner, {});
  owner->releaseExternalBatch();
}

void FileStagingCoordinator::requestCommit(Completion completion) {
  if (!completion) throw std::invalid_argument("file staging completion callback is required");
  m_completions.push_back(std::move(completion));
  if (m_ownership != Ownership::Idle) return;
  startCommandBatch();
}

std::optional<FileStagingCoordinator::ExternalBatch> FileStagingCoordinator::reserveExternalBatch(ExternalCompletion completedActiveBatch) {
  if (!completedActiveBatch) throw std::invalid_argument("file staging external completion callback is required");
  if (m_ownership == Ownership::External) {
    throw std::logic_error("a file staging external batch already owns execution");
  }
  if (m_ownership == Ownership::Command) {
    if (m_externalCompletion) throw std::logic_error("a file staging external batch is already waiting for ownership");
    m_externalCompletion = std::move(completedActiveBatch);
    return std::nullopt;
  }
  postOwnershipEvent(OwnershipEvent::ExternalReserved);
  return ExternalBatch(this);
}

bool FileStagingCoordinator::active() const { return m_ownership != Ownership::Idle; }

void FileStagingCoordinator::startCommandBatch() {
  if (m_ownership != Ownership::Idle || m_completions.empty()) return;
  const auto fail = [this](QString detail) {
    m_operation.reset();
    const FileStagingBatchResult result{app::ApplicationCommandResult::infrastructureFailure(std::move(detail)), std::nullopt};
    auto completions = std::exchange(m_completions, {});
    for (const auto& completion : completions) notifyCompletion(completion, result);
  };
  try {
    auto operation = std::make_unique<Operation>(
        m_session, m_store, m_capability, m_parentWindow, [this](const FileStagingBatchResult& result) { finish(result); },
        [this](const std::size_t processed, const std::size_t total) { emit progressChanged(processed, total); });
    m_operation = std::move(operation);
    m_operation->start();
    postOwnershipEvent(OwnershipEvent::CommandStarted);
  } catch (const std::exception& error) {
    fail(I18n::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what())));
  } catch (...) {
    fail(I18n::tr("Automatic file staging stopped because of an unknown error."));
  }
}

void FileStagingCoordinator::completeExternalBatch(const FileStagingBatchResult& result) {
  if (m_ownership != Ownership::External) return;
  postOwnershipEvent(OwnershipEvent::ExternalCompleted);
  auto completions = std::exchange(m_completions, {});
  for (const auto& completion : completions) notifyCompletion(completion, result);
}

void FileStagingCoordinator::releaseExternalBatch() {
  if (m_ownership != Ownership::External) return;
  postOwnershipEvent(OwnershipEvent::ExternalReleased);
  startCommandBatch();
}

void FileStagingCoordinator::finish(const FileStagingBatchResult& result) {
  UWF_LOG_I("staging") << "command batch completed: outcome=" << static_cast<int>(result.command.outcome) << " discovered=" << result.command.discoveredFiles
                       << " committed=" << result.command.committedFiles << " skippedFiles=" << result.command.skippedFiles
                       << " skippedEntries=" << result.command.skippedEntries << " failures=" << result.command.failedFiles;
  auto completions = std::exchange(m_completions, {});
  // finish() 由 Operation 自己的完成回调进入，不能在其成员函数尚未返回时
  // 直接 delete this；把所有权交还 Qt 事件循环，在当前调用栈退出后销毁。
  Operation* completedOperation = m_operation.release();
  completedOperation->deleteLater();
  auto externalCompletion = std::exchange(m_externalCompletion, std::nullopt);
  postOwnershipEvent(externalCompletion ? OwnershipEvent::CommandHandedOff : OwnershipEvent::CommandCompleted);
  for (const auto& completion : completions) notifyCompletion(completion, result);
  if (externalCompletion) notifyExternalCompletion(*externalCompletion, result, ExternalBatch(this));
}

}  // namespace uwf::ui
