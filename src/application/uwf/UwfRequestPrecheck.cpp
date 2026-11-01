// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/UwfRequestPrecheck.h"

#include <optional>
#include <stdexcept>
#include <utility>

#include "application/uwf/UwfOperations.h"
#include "application/uwf/UwfStateUpdates.h"
#include "domain/uwf/solvers/UwfFailureSolver.h"

namespace uwf::app {
using namespace compat;
namespace {
class RecordingUwf final : public Uwf {
 public:
  RecordingUwf(const Uwf& downstream, UwfStateFacts& state) : m_downstream(downstream), m_state(state) {}
  Capability probe(const Request& request) const override { return m_downstream.probe(request); }
  Response execute(const Request& request, const Context& context) const override {
    auto response = normalizeResponse(request, m_downstream.execute(request, context));
    const auto update = recordUwfState(m_state, request, response);
    if (response.succeeded() && isWrite(request) && update == UwfStateUpdate::RefreshRequired) m_refreshRequired = true;
    return response;
  }
  bool refreshRequired() const { return m_refreshRequired; }

 private:
  const Uwf& m_downstream;
  UwfStateFacts& m_state;
  mutable bool m_refreshRequired = false;
};
}  // namespace

class PrecheckedUwf::Session final {
 public:
  explicit Session(const PrecheckedUwf& owner) : owner(owner), previous(s_session), lock(owner.m_mutex) {
    if (const auto* active = owner.activeSession()) {
      working = active->working;
    } else {
      before = owner.m_published.load();
      // Allocate before native work. Publication in the destructor only moves
      // this already-owned allocation into an immutable shared pointer.
      working = std::make_shared<UwfStateFacts>(*before);
    }
    s_session = this;
  }
  ~Session() {
    s_session = previous;
    if (before) {
      std::shared_ptr<const UwfStateFacts> published = std::move(working);
      owner.m_published.store(std::move(published));
    }
  }
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;

  const PrecheckedUwf& owner;
  Session* previous;
  std::unique_lock<std::recursive_mutex> lock;
  std::shared_ptr<UwfStateFacts> working;
  std::shared_ptr<const UwfStateFacts> before;
};

thread_local PrecheckedUwf::Session* PrecheckedUwf::s_session = nullptr;

PrecheckedUwf::Session* PrecheckedUwf::activeSession() const noexcept {
  for (auto* current = s_session; current; current = current->previous)
    if (&current->owner == this) return current;
  return nullptr;
}

PrecheckedUwf::PrecheckedUwf(std::shared_ptr<Uwf> downstream, UwfFactsSource source, const bool enabled, UwfStateRefresh refresh)
    : m_downstream(std::move(downstream)), m_source(std::move(source)), m_enabled(enabled), m_refresh(std::move(refresh)) {
  if (!m_downstream) throw std::invalid_argument("UWF operations require a backend router");
  m_published.store(m_downstream->stateSnapshot());
}
UwfStateSnapshot PrecheckedUwf::stateSnapshot() const {
  // A transaction's working state is private and can still be edited. A handle
  // obtained inside it must not change underneath its reader after later writes.
  if (const auto* active = activeSession()) return std::make_shared<const UwfStateFacts>(*active->working);
  return m_published.load();
}
void PrecheckedUwf::updateState(UwfStateFacts state) const {
  const Session session(*this);
  state.refreshFailure = session.working->refreshFailure;
  *session.working = std::move(state);
}
void PrecheckedUwf::synchronize(const std::function<void()>& work) const {
  const Session session(*this);
  work();
}
Response PrecheckedUwf::refreshState(const Context& context) const {
  if (context.stop.stop_requested()) return Response::fail(ErrorKind::Canceled, "UWF state refresh canceled");
  const Session session(*this);
  Response result;
  try {
    result = m_refresh ? m_refresh(*this, context) : refreshUwfState(*this, context);
  } catch (const Error& error) {
    result = Response::fail(error.failure().kind, error.what(), error.failure().effect, error.failure().diagnostics, error.failure().readCompletion);
    result.attempts = error.attempts();
  } catch (const std::exception& error) {
    result = Response::fail(ErrorKind::Failed, error.what());
  } catch (...) {
    result = Response::fail(ErrorKind::Failed, "The global UWF refresh raised an unknown exception");
  }
  session.working->refreshFailure = result.failure;
  return result;
}
Capability PrecheckedUwf::probe(const Request& request) const {
  const auto facts = stateSnapshot();
  return probeUwfOperation(*m_downstream, request, {{}, facts.get()});
}
Response PrecheckedUwf::execute(const Request& request, const Context& context) const {
  if (context.stop.stop_requested()) return Response::fail(ErrorKind::Canceled, "UWF request canceled before execution");
  const std::unique_lock lock(m_mutex);
  const auto published = m_published.load();
  const auto* active = activeSession();
  const auto& current = active ? *active->working : *published;
  if (isWrite(request) && current.refreshFailure)
    return Response::fail(ErrorKind::Unavailable, "The global UWF state refresh failed; refresh it before another write: " + current.refreshFailure->message,
                          Effect::None, current.refreshFailure->diagnostics);
  const bool dataCommit = std::holds_alternative<CommitFile>(request) || std::holds_alternative<CommitFileDeletion>(request) ||
                          std::holds_alternative<CommitRegistry>(request) || std::holds_alternative<CommitRegistryDeletion>(request);
  std::optional<UwfStateFacts> prepared;
  const UwfStateFacts* facts = context.facts ? context.facts : &current;
  if (m_source && !context.facts) {
    try {
      prepared = m_source(request, {context.stop});
      facts = &*prepared;
    } catch (const Error& error) {
      return Response::fail(error.failure().kind, error.what(), error.failure().effect, error.failure().diagnostics, error.failure().readCompletion);
    } catch (const std::exception& error) {
      return Response::fail(ErrorKind::Failed, error.what());
    }
  }
  if (m_enabled) {
    const auto evaluation = solveUwfRequest(request, *facts);
    if (const auto* rejection = std::get_if<Reject>(&evaluation.decision))
      return Response::fail(rejection->kind, rejection->reason, Effect::None, {{"UwfFailureSolver", ruleName(rejection->rule), 0, rejection->reason}});
  }
  // Data maintenance never changes configuration facts. Retain the immutable
  // input and serialization, without a state copy, publication, or extra getter.
  if (dataCommit) return normalizeResponse(request, executeUwfOperation(*m_downstream, request, {context.stop, facts}));
  if (active && !context.facts && !prepared) {
    // Nested composition must not iterate a list that RecordingUwf is editing.
    prepared = current;
    facts = &*prepared;
  }
  const Session session(*this);
  const RecordingUwf recording(*m_downstream, *session.working);
  auto response = normalizeResponse(request, executeUwfOperation(recording, request, {context.stop, facts}));
  if (!isWrite(request)) (void)recordUwfState(*session.working, request, response);
  const bool needsRefresh = recording.refreshRequired() || (isWrite(request) && response.failure && response.effect != Effect::None);
  if (needsRefresh && (response.succeeded() || response.effect != Effect::None)) {
    // Cancellation cannot undo a configuration change already acknowledged by
    // the provider. Complete its state publication before another operation.
    const auto refreshed = refreshState();
    response.warnings.insert(response.warnings.end(), refreshed.warnings.begin(), refreshed.warnings.end());
    if (refreshed.failure) {
      response.warnings.push_back("Refreshing the global UWF state failed: " + refreshed.failure->message);
      if (!response.failure) {
        auto failure = *refreshed.failure;
        failure.message = "The UWF write completed, but refreshing the global state failed: " + failure.message;
        failure.effect = response.effect;
        response.failure = std::move(failure);
      }
    }
  }
  return response;
}
}  // namespace uwf::app
