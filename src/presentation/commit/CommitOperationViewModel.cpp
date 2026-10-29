// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitOperationViewModel.h"

#include <QPointer>
#include <algorithm>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "CommitBatchViewModel.h"
#include "presentation/common/PresentationText.h"
#include "presentation/common/PresentationTiming.h"
#include "presentation/common/TextFileExport.h"

namespace uwf::presentation {
struct CommitOperationViewModel::LiveProgress {
  std::mutex mutex;
  app::CommitTraversalProgress progress;
  int processed = 0;
};
namespace {
struct PreparationOutcome {
  app::CommitPlan plan;
  QList<app::CommitNode> targets;
  app::CommitTraversalProgress progress;
  QString error;
};
struct ExecutionOutcome {
  QList<CommitReportRow> rows;
  int untouched = 0;
  CommitCounts counts;
  QString error;
};
QString currentError(std::exception_ptr failure = std::current_exception()) {
  try {
    std::rethrow_exception(failure);
  } catch (const std::exception& error) {
    return QString::fromUtf8(error.what());
  } catch (...) {
    return PresentationText::tr("The operation failed with an unknown error.");
  }
}
PreparationOutcome preparationFailure(const app::CommitRequest& request, app::CommitPlan previous, QString error) {
  PreparationOutcome result;
  result.plan = std::move(previous);
  result.plan.request = request;
  result.plan.canceled = false;
  result.error = std::move(error);
  auto found = std::ranges::find_if(result.plan.records, [&](const auto& record) { return record.node.identity() == request.target.identity(); });
  if (found == result.plan.records.end()) {
    result.plan.records.append({request.target, app::TraversalStatus::Failed, result.error, {result.error}, 1});
  } else {
    found->status = app::TraversalStatus::Failed;
    found->error = result.error;
    found->errorHistory.append(result.error);
    ++found->attempts;
  }
  result.progress = result.plan.progress();
  result.progress.path = result.error;
  return result;
}
QString recordsText(const app::CommitPlan& plan) {
  QStringList lines{
      QStringList{PresentationText::tr("Path"), PresentationText::tr("Result"), PresentationText::tr("Error"), PresentationText::tr("Attempts")}.join(u'\t')};
  for (const auto& record : plan.records) {
    auto escaped = [](QString value) {
      return value.replace(u'\t', QStringLiteral("\\t")).replace(u'\r', QStringLiteral("\\r")).replace(u'\n', QStringLiteral("\\n"));
    };
    const auto status = record.status == app::TraversalStatus::Succeeded ? PresentationText::tr("Succeeded")
                        : record.status == app::TraversalStatus::Failed  ? PresentationText::tr("Failed")
                                                                         : PresentationText::tr("Skipped");
    lines.append(
        QStringList{escaped(record.node.display()), status, escaped(record.errorHistory.join(QStringLiteral("; "))), QString::number(record.attempts)}.join(
            u'\t'));
  }
  return lines.join(QStringLiteral("\r\n"));
}
}  // namespace
CommitOperationViewModel::CommitOperationViewModel(app::CommitRequest request, std::shared_ptr<app::CommitBackend> backend, QObject* parent)
    : QObject(parent), m_request(std::move(request)), m_backend(std::move(backend)) {
  if (!m_backend) throw std::invalid_argument("commit presentation requires an owned backend");
  m_pulse.setInterval(timing::kProgressUpdateInterval);
  connect(&m_pulse, &QTimer::timeout, this, [this] {
    {
      std::lock_guard lock(m_live->mutex);
      if (m_progress == m_live->progress && m_processed == m_live->processed) return;
      m_progress = m_live->progress;
      m_processed = m_live->processed;
    }
    if (m_phase == Phase::Preparing && m_progress.kind) m_request.target.kind = *m_progress.kind;
    emit stateChanged();
  });
}
CommitOperationViewModel::~CommitOperationViewModel() = default;
void CommitOperationViewModel::start() {
  if (m_phase == Phase::Idle) prepare();
}
void CommitOperationViewModel::retry() {
  if (m_executionStarted || (m_phase != Phase::Partial && m_phase != Phase::Empty && m_phase != Phase::Failed)) return;
  prepare();
}
void CommitOperationViewModel::prepare() {
  m_task.cancel();
  m_phase = Phase::Preparing;
  m_error.clear();
  m_partialAccepted = false;
  m_live = std::make_shared<LiveProgress>();
  m_live->progress = m_progress;
  const auto ticket = m_loading.begin();
  QPointer<CommitOperationViewModel> self(this);
  m_task.start(
      [backend = m_backend, request = m_request, previous = m_plan, live = m_live](std::stop_token token) mutable {
        PreparationOutcome result;
        result.plan = previous;
        try {
          result.plan = backend->prepare(request, std::move(previous), token, [live](const app::CommitTraversalProgress& progress) {
            std::lock_guard lock(live->mutex);
            live->progress = progress;
          });
          result.targets = result.plan.targets();
          result.progress = result.plan.progress();
          result.progress.path = result.plan.request.target.display();
          for (const auto& record : std::as_const(result.plan.records))
            if (record.status == app::TraversalStatus::Failed) {
              result.progress.path = record.node.display();
              break;
            }
        } catch (...) {
          result = preparationFailure(request, std::move(result.plan), currentError());
        }
        return result;
      },
      [this, self, ticket](AsyncResult<PreparationOutcome> outcome) mutable {
        if (!m_loading.isCurrent(ticket)) return;
        auto result = std::holds_alternative<PreparationOutcome>(outcome)
                          ? std::get<PreparationOutcome>(std::move(outcome))
                          : preparationFailure(m_request, m_plan, currentError(std::get<std::exception_ptr>(outcome)));
        m_pulse.stop();
        m_progress = result.progress;
        if (!result.plan.records.isEmpty()) m_request.target.kind = result.plan.request.target.kind;
        emit stateChanged();
        if (!self || !m_loading.isCurrent(ticket)) return;
        const bool recursive =
            result.plan.request.target.kind == app::CommitTargetKind::Folder || result.plan.request.target.kind == app::CommitTargetKind::RegistryKey;
        const bool failed = !result.error.isEmpty();
        auto publish = [this, result = std::move(result)]() mutable {
          m_plan = std::move(result.plan);
          if (!m_plan.records.isEmpty()) m_request = m_plan.request;
          m_progress = std::move(result.progress);
          m_targets = std::move(result.targets);
          m_error = std::move(result.error);
          m_phase = !m_error.isEmpty() ? Phase::Failed : m_progress.failed ? Phase::Partial : m_targets.isEmpty() ? Phase::Empty : Phase::Ready;
          emit stateChanged();
        };
        if (!recursive && !failed) {
          m_loading.cancel();
          publish();
        } else
          m_loading.publish(ticket, std::move(publish));
      });
  m_pulse.start();
  emit stateChanged();
}
void CommitOperationViewModel::continuePartial() {
  if (m_phase != Phase::Partial || m_targets.isEmpty()) return;
  m_partialAccepted = true;
  m_phase = Phase::Ready;
  emit stateChanged();
}
void CommitOperationViewModel::proceed() {
  if (m_phase == Phase::Partial)
    continuePartial();
  else
    commit();
}
void CommitOperationViewModel::commit() {
  if (m_phase != Phase::Ready || m_targets.isEmpty()) return;
  m_phase = Phase::Committing;
  m_executionStarted = true;
  m_processed = 0;
  m_live = std::make_shared<LiveProgress>();
  m_live->progress = m_progress;
  m_task.start(
      [backend = m_backend, targets = m_targets, operation = m_request.operation, live = m_live](std::stop_token token) {
        ExecutionOutcome result;
        int attempted = 0;
        app::CommitBatchRequest batch{static_cast<int>(targets.size()), operation, [&targets](int index) { return targets[index].display(); },
                                      [backend, &targets, operation, &attempted](int index, const QString&) {
                                        ++attempted;
                                        return backend->execute(targets[index], operation);
                                      }};
        CommitBatchViewModel execution(batch);
        try {
          execution.run({[token](int) { return !token.stop_requested(); },
                         [live](int processed, const QString& path) {
                           std::lock_guard lock(live->mutex);
                           live->processed = processed;
                           live->progress.path = path;
                         },
                         [live](const app::CommitTargetResult&, const QString&) {
                           std::lock_guard lock(live->mutex);
                           ++live->processed;
                         }});
          result.untouched = execution.untouched();
        } catch (...) {
          result.error = currentError();
          result.untouched = static_cast<int>(targets.size()) - attempted;
        }
        result.rows = execution.rows();
        if (attempted > result.rows.size()) {
          app::CommitTargetResult unknown;
          unknown.failureDetail = result.error;
          result.rows.append(makeCommitReportRow(unknown, targets[attempted - 1].display(), operation));
        }
        result.counts = countCommitResults(result.rows);
        return result;
      },
      [this](AsyncResult<ExecutionOutcome> outcome) {
        ExecutionOutcome result;
        if (const auto* failure = std::get_if<std::exception_ptr>(&outcome)) {
          result.error = currentError(*failure);
          result.untouched = total();
        } else
          result = std::get<ExecutionOutcome>(std::move(outcome));
        m_pulse.stop();
        m_rows = std::move(result.rows);
        m_processed = static_cast<int>(m_rows.size());
        m_untouched = result.untouched;
        m_counts = result.counts;
        m_error = std::move(result.error);
        if (!m_error.isEmpty()) m_progress.path = m_error;
        m_phase = !m_error.isEmpty() ? Phase::Failed : m_untouched ? Phase::Canceled : Phase::Complete;
        emit stateChanged();
      });
  m_pulse.start();
  emit stateChanged();
}
bool CommitOperationViewModel::cancel() {
  if (m_phase == Phase::Complete || m_phase == Phase::Canceled || m_phase == Phase::Failed || m_phase == Phase::Empty) return true;
  if (m_phase == Phase::Committing || m_phase == Phase::Canceling) {
    m_phase = Phase::Canceling;
    m_task.requestStop();
    emit stateChanged();
    return false;
  }
  m_task.cancel();
  m_loading.cancel();
  m_pulse.stop();
  m_phase = Phase::Canceled;
  return true;
}
void CommitOperationViewModel::exportRecords(const QString& path, app::TextFileWriter writer) {
  if (path.isEmpty()) return;
  auto* task = new AsyncTask(this);
  task->start([path, plan = m_plan, writer = std::move(writer)](std::stop_token) { return exportTextFile(path, recordsText(plan), writer); },
              [this, task](AsyncResult<QString> result) {
                task->deleteLater();
                emit exportFinished(std::holds_alternative<QString>(result) ? std::get<QString>(std::move(result))
                                                                            : currentError(std::get<std::exception_ptr>(result)));
              });
}
CommitCompletion CommitOperationViewModel::completion() const {
  const auto cause = m_phase == Phase::Canceled                       ? CommitCompletion::Cause::Canceled
                     : m_phase == Phase::Failed && m_executionStarted ? CommitCompletion::Cause::Interrupted
                                                                      : CommitCompletion::Cause::Finished;
  return {cause, m_untouched, m_progress.failed};
}
CommitOperationText CommitOperationViewModel::text() const {
  CommitOperationText view;
  const bool deleting = m_request.operation == app::CommitOperation::DeleteAndPersist;
  const bool registry = m_request.target.kind == app::CommitTargetKind::RegistryKey || m_request.target.kind == app::CommitTargetKind::RegistryValue;
  view.destructive = deleting;
  view.close = PresentationText::tr("Cancel");
  view.note = deleting ? PresentationText::tr("Nothing has been deleted or committed.") : PresentationText::tr("No changes have been committed.");
  view.count = registry ? PresentationText::tr("%1 values · %2 keys").arg(m_progress.items).arg(m_progress.containers)
                        : PresentationText::tr("%1 files · %2 folders").arg(m_progress.items).arg(m_progress.containers);
  if (m_progress.failed) view.count += PresentationText::tr(" · %1 branches could not be read").arg(m_progress.failed);
  view.detail =
      deleting ? PresentationText::tr("Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.")
               : PresentationText::tr("Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.");
  view.exportEnabled = !m_plan.records.isEmpty() && m_phase != Phase::Preparing;
  switch (m_request.target.kind) {
    case app::CommitTargetKind::File:
      view.title = deleting ? PresentationText::tr("Delete and commit this file?") : PresentationText::tr("Commit file changes?");
      break;
    case app::CommitTargetKind::Folder:
      view.title = deleting ? PresentationText::tr("Delete and commit this folder?") : PresentationText::tr("Commit folder changes?");
      break;
    case app::CommitTargetKind::RegistryKey:
      view.title = deleting ? PresentationText::tr("Delete and commit this registry key?") : PresentationText::tr("Commit registry key changes?");
      break;
    case app::CommitTargetKind::RegistryValue:
      view.title = deleting ? PresentationText::tr("Delete and commit this registry value?") : PresentationText::tr("Commit registry value changes?");
      break;
  }
  view.primary = deleting ? PresentationText::tr("Delete and commit") : PresentationText::tr("Commit changes");
  view.icon = QStringLiteral(":/icons/check.svg");
  switch (m_phase) {
    case Phase::Idle:
    case Phase::Preparing:
      view.title = deleting ? PresentationText::tr("Preparing deletion and commit") : PresentationText::tr("Preparing commit");
      view.heading = registry                                                 ? PresentationText::tr("Traversing the registry")
                     : m_request.target.kind == app::CommitTargetKind::Folder ? PresentationText::tr("Traversing the folder")
                                                                              : view.title;
      view.description = PresentationText::tr("Determining the scope. Unread branches are recorded while traversal continues elsewhere.");
      view.busy = true;
      view.activePath = true;
      view.primaryVisible = true;
      break;
    case Phase::Partial:
      view.title = PresentationText::tr("Traversal completed with unread content");
      view.heading = PresentationText::tr("Successful and failed records are retained");
      view.description =
          PresentationText::tr("%1 branches could not be read. Continue with discovered content, or retry only the failed items.").arg(m_progress.failed);
      view.primary = PresentationText::tr("Continue…");
      view.primaryVisible = true;
      view.primaryEnabled = !m_targets.isEmpty();
      view.retry = PresentationText::tr("Retry failed items");
      view.retryVisible = true;
      view.icon = QStringLiteral(":/icons/status_warning.svg");
      view.warning = true;
      view.activePath = true;
      break;
    case Phase::Ready:
      view.heading = m_partialAccepted ? PresentationText::tr("Process only successfully traversed content") : PresentationText::tr("Preparation complete");
      view.description =
          m_partialAccepted
              ? deleting ? PresentationText::tr("Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.")
                         : PresentationText::tr("Commit only successfully traversed content. Unread branches remain unchanged.")
          : deleting ? PresentationText::tr("Permanently delete the selected scope and save the deletions to the protected volume.")
                     : PresentationText::tr("Save changes in the selected scope to the protected volume.");
      view.primaryVisible = true;
      view.primaryEnabled = !m_targets.isEmpty();
      view.success = true;
      break;
    case Phase::Empty:
      view.title = PresentationText::tr("No actionable content");
      view.heading = PresentationText::tr("Traversal complete");
      view.description = PresentationText::tr("No actionable targets were found. Nothing has been deleted or committed.");
      view.retry = PresentationText::tr("Traverse again");
      view.retryVisible = true;
      view.close = PresentationText::tr("Close");
      view.icon = QStringLiteral(":/icons/info.svg");
      view.detail = PresentationText::tr("Traverse again, or close this window.");
      break;
    case Phase::Failed:
      if (m_executionStarted) break;
      view.title = PresentationText::tr("Unable to prepare the operation");
      view.heading = PresentationText::tr("The operation could not be completed");
      view.description = PresentationText::tr("Traversal records are retained and can be exported.");
      view.retry = PresentationText::tr("Retry");
      view.retryVisible = true;
      view.close = PresentationText::tr("Close");
      view.icon = QStringLiteral(":/icons/status_error.svg");
      view.error = true;
      view.activePath = true;
      return view;
    case Phase::Committing:
    case Phase::Canceling:
      view.title = deleting ? PresentationText::tr("Deleting and committing") : PresentationText::tr("Committing changes");
      view.heading = m_phase == Phase::Canceling ? PresentationText::tr("Stopping the operation") : view.title;
      view.description = PresentationText::tr("Remaining tasks can be canceled after the current target finishes.");
      view.note = deleting ? PresentationText::tr("Completed deletions and commits cannot be undone.")
                           : PresentationText::tr("Changes already committed are retained.");
      view.count = PresentationText::tr("Processed %1 / %2 items").arg(m_processed).arg(total());
      view.close = PresentationText::tr("Cancel remaining tasks");
      view.cancelEnabled = m_phase != Phase::Canceling;
      view.icon = deleting ? QStringLiteral(":/icons/trash.svg") : QStringLiteral(":/icons/apply.svg");
      view.writing = true;
      view.activePath = true;
      break;
    case Phase::Complete:
    case Phase::Canceled:
      break;
  }
  if (m_phase == Phase::Complete || m_phase == Phase::Canceled || (m_phase == Phase::Failed && m_executionStarted)) {
    const auto outcome = describeCommitOutcome(m_counts, completion());
    view.title = view.heading = outcome.title;
    view.description = outcome.summary;
    view.icon = outcome.icon;
    view.error = outcome.tone == CommitOutcome::Tone::Error;
    view.warning = outcome.tone == CommitOutcome::Tone::Warning;
    view.success = outcome.tone == CommitOutcome::Tone::Success;
    view.note = PresentationText::tr("Changes already committed are retained.");
    view.detail = PresentationText::tr("Traversal records are retained and can be exported.");
    view.close = PresentationText::tr("Done");
    view.showReport = !m_rows.isEmpty();
    view.activePath = m_phase == Phase::Failed;
  }
  return view;
}
}  // namespace uwf::presentation
#include "moc_CommitOperationViewModel.cpp"
