// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/routing/FallbackUwf.h"

#include <algorithm>
#include <atomic>
#include <format>
#include <iterator>
#include <utility>

#include "foundation/logging/Log.h"

namespace uwf::compat {
namespace {
std::uint64_t nextLogId() noexcept {
  static std::atomic<std::uint64_t> next{0};
  return next.fetch_add(1, std::memory_order_relaxed) + 1;
}

std::string_view supportName(const Support support) noexcept {
  switch (support) {
    case Support::Supported:
      return "Supported";
    case Support::Unsupported:
      return "Unsupported";
    case Support::Unavailable:
      return "Unavailable";
    case Support::Unknown:
      return "Unknown";
  }
  return "Invalid";
}
std::string_view effectName(const Effect effect) noexcept {
  switch (effect) {
    case Effect::None:
      return "None";
    case Effect::Confirmed:
      return "Confirmed";
    case Effect::Uncertain:
      return "Uncertain";
  }
  return "Invalid";
}
std::string_view failureName(const ErrorKind kind) noexcept {
  switch (kind) {
    case ErrorKind::Unsupported:
      return "Unsupported";
    case ErrorKind::Unavailable:
      return "Unavailable";
    case ErrorKind::AccessDenied:
      return "AccessDenied";
    case ErrorKind::NotFound:
      return "NotFound";
    case ErrorKind::InvalidArgument:
      return "InvalidArgument";
    case ErrorKind::Canceled:
      return "Canceled";
    case ErrorKind::ResourceExhausted:
      return "ResourceExhausted";
    case ErrorKind::Conflict:
      return "Conflict";
    case ErrorKind::Protocol:
      return "Protocol";
    case ErrorKind::Failed:
      return "Failed";
  }
  return "Invalid";
}

void logFailure(LogStream& log, const Failure& failure, const std::string_view messageField = "reason") {
  log << " kind=" << failureName(failure.kind) << " effect=" << effectName(failure.effect) << ' ' << messageField << '=';
  logQuoted(log, failure.message);
  for (const auto& diagnostic : failure.diagnostics) {
    log << " diagnostic_backend=";
    logQuoted(log, diagnostic.backend);
    log << " diagnostic_operation=";
    logQuoted(log, diagnostic.operation);
    log << " code=" << std::format("0x{:08X}", static_cast<std::uint32_t>(diagnostic.code)) << " detail=";
    logQuoted(log, diagnostic.detail);
  }
}

// Capture optional logging metadata without probing or affecting execution.
std::vector<std::string> logBackendNames(const std::vector<std::shared_ptr<Backend>>& backends) noexcept {
  std::vector<std::string> names;
  try {
    names.reserve(backends.size());
    for (const auto& backend : backends) {
      try {
        names.push_back(backend ? backend->name() : std::string{});
      } catch (...) {
        names.emplace_back();
      }
    }
  } catch (...) {
    names.clear();
  }
  return names;
}
void logBackend(LogStream& log, const std::vector<std::string>& names, const std::size_t index) noexcept {
  if (index < names.size() && !names[index].empty())
    logQuoted(log, names[index]);
  else
    log << '"' << "backend#" << index + 1 << '"';
}
void logPolicy(const std::uint64_t router, const std::vector<std::string>& names, const std::size_t count) noexcept {
  try {
    std::string order;
    for (std::size_t index = 0; index < count; ++index) {
      if (index) order += " -> ";
      order += index < names.size() && !names[index].empty() ? names[index] : "backend#" + std::to_string(index + 1);
    }
    LogStream log('I', "uwf-routing");
    log << "event=policy router=" << router << " policy=ordered-fallback backends=" << count << " order=";
    logQuoted(log, order);
  } catch (...) {
  }
}

// Logging failures are isolated from backend calls and routing decisions.
class RoutingLog final {
 public:
  RoutingLog(const std::uint64_t router, const std::vector<std::string>& names, const Request& request, const std::string_view phase,
             const std::uint64_t parent = 0) noexcept
      : m_router(router), m_names(names), m_request(request), m_phase(phase), m_id(nextLogId()), m_parent(parent) {
    event("started", [](LogStream&) {});
  }
  [[nodiscard]] std::uint64_t id() const noexcept { return m_id; }
  template <typename Details>
  void event(const std::string_view name, Details&& details, const char level = 'D') const noexcept {
#if !defined(UWF_DEBUG_LOGGING)
    if (level == 'D') return;
#endif
    LogStream log(level, "uwf-routing");
    try {
      log << "event=" << name << " router=" << m_router << " request=" << m_id << " parent=" << m_parent << " phase=" << m_phase
          << " operation=" << operationName(m_request) << " session=";
      std::visit(
          [&](const auto& request) {
            if constexpr (requires { request.session; })
              log << (request.session == Session::Current ? "Current" : "Next");
            else
              log << "implicit";
          },
          m_request);
      std::forward<Details>(details)(log);
    } catch (...) {
      log.oss.reset();
    }
  }
  void attempt(const std::size_t index, const std::string_view action, const std::string_view result, const Capability* capability = nullptr,
               const Failure* failure = nullptr, const Effect effect = Effect::None) const noexcept {
    event("attempt", [&](LogStream& log) {
      log << " attempt=" << index + 1 << " backend=";
      logBackend(log, m_names, index);
      log << " action=" << action << " result=" << result;
      if (capability) {
        log << " capability=" << supportName(capability->support) << " capability_reason=";
        logQuoted(log, capability->reason);
      }
      if (failure)
        logFailure(log, *failure);
      else
        log << " effect=" << effectName(effect);
    });
  }
  void executing(const std::size_t index, const Capability& capability) noexcept {
    m_executed = index;
    attempt(index, "execute", "started", &capability);
  }
  void fallback(const std::size_t index, const std::size_t count, const std::string_view reason, const std::string_view cause,
                const Failure* failure = nullptr) const noexcept {
    if (index + 1 >= count) return;
    event(
        "fallback",
        [&](LogStream& log) {
          log << " attempt=" << index + 1 << " from=";
          logBackend(log, m_names, index);
          log << " to=";
          logBackend(log, m_names, index + 1);
          log << " reason=";
          logQuoted(log, reason);
          if (failure)
            logFailure(log, *failure, "cause");
          else {
            log << " cause=";
            logQuoted(log, cause);
          }
        },
        m_phase == "probe" ? 'D' : 'I');
  }
  void completed(const Capability& capability, const std::optional<std::size_t> selected = {}, const std::string_view stop = {}) const noexcept {
    event("completed", [&](LogStream& log) {
      log << " result=" << supportName(capability.support) << " backend=";
      if (selected)
        logBackend(log, m_names, *selected);
      else
        logQuoted(log, "none");
      log << " stop=" << (!stop.empty() ? stop : selected ? "supported" : "exhausted") << " reason=";
      logQuoted(log, capability.reason);
    });
  }
  void completed(const Response& response, const std::string_view stop, const std::optional<std::size_t> selected = {}) const noexcept {
    event(
        "completed",
        [&](LogStream& log) {
          log << " result=" << (response.succeeded() ? "success" : "failed") << " backend=";
          if (selected)
            logBackend(log, m_names, *selected);
          else
            logQuoted(log, "none");
          log << " executed_backend=";
          if (m_executed)
            logBackend(log, m_names, *m_executed);
          else
            logQuoted(log, "none");
          log << " stop=" << stop;
          if (response.failure)
            logFailure(log, *response.failure);
          else
            log << " effect=" << effectName(response.effect);
        },
        'I');
  }

 private:
  std::uint64_t m_router;
  const std::vector<std::string>& m_names;
  const Request& m_request;
  std::string_view m_phase;
  std::uint64_t m_id;
  std::uint64_t m_parent;
  std::optional<std::size_t> m_executed;
};

Response exceptionFailure(const Effect unknownEffect, const ReadCompletion unknownCompletion = ReadCompletion::NotDispatched) {
  try {
    throw;
  } catch (const Error& error) {
    return {{}, error.failure(), error.failure().effect, error.attempts(), {}};
  } catch (const std::exception& error) {
    return Response::fail(ErrorKind::Failed, error.what(), unknownEffect, {}, unknownCompletion);
  } catch (...) {
    return Response::fail(ErrorKind::Failed, "Unknown UWF backend exception", unknownEffect, {}, unknownCompletion);
  }
}
Response stopped() { return Response::fail(ErrorKind::Canceled, "UWF operation canceled before dispatch"); }
Response finish(Response response, std::vector<Attempt> attempts) {
  response.attempts.insert(response.attempts.begin(), std::make_move_iterator(attempts.begin()), std::make_move_iterator(attempts.end()));
  if (response.failure) {
    auto ownDiagnostics = std::move(response.failure->diagnostics);
    response.failure->diagnostics.clear();
    const auto equal = [](const Diagnostic& left, const Diagnostic& right) {
      return left.backend == right.backend && left.operation == right.operation && left.code == right.code && left.detail == right.detail;
    };
    const auto append = [&](const Diagnostic& diagnostic) {
      if (std::ranges::none_of(response.failure->diagnostics, [&](const auto& existing) { return equal(existing, diagnostic); }))
        response.failure->diagnostics.push_back(diagnostic);
    };
    for (const auto& attempt : response.attempts) {
      if (attempt.failure) {
        for (const auto& diagnostic : attempt.failure->diagnostics)
          if (std::ranges::none_of(ownDiagnostics, [&](const auto& own) { return equal(own, diagnostic); })) append(diagnostic);
        if (attempt.failure->diagnostics.empty()) append({attempt.backend, {}, 0, attempt.failure->message});
      } else if (attempt.capability.support != Support::Supported && !attempt.capability.reason.empty()) {
        append({attempt.backend, {}, 0, attempt.capability.reason});
      }
    }
    // Keep terminal native evidence last without duplicating attempt diagnostics.
    for (const auto& diagnostic : ownDiagnostics) append(diagnostic);
  }
  return response;
}
}  // namespace

FallbackUwf::FallbackUwf(std::vector<std::shared_ptr<Backend>> backends) : m_backends(std::move(backends)), m_logId(nextLogId()) {
  if (m_backends.empty() || std::ranges::any_of(m_backends, [](const auto& backend) { return !backend; }))
    throw std::invalid_argument("UWF fallback requires a non-empty list of backend instances");
  m_logBackendNames = logBackendNames(m_backends);
  logPolicy(m_logId, m_logBackendNames, m_backends.size());
}
Capability FallbackUwf::probe(const Request& request) const {
  if (const auto failure = requestInputFailure(request)) return {Support::Unsupported, failure->message};
  const RoutingLog log(m_logId, m_logBackendNames, request, "probe");
  Support aggregate = Support::Unsupported;
  std::string reasons;
  for (std::size_t index = 0; index < m_backends.size(); ++index) {
    const auto& backend = m_backends[index];
    log.attempt(index, "probe", "started");
    try {
      auto capability = backend->probe(request);
      log.attempt(index, "probe", supportName(capability.support), &capability);
      if (capability.support == Support::Supported) {
        log.completed(capability, index);
        return capability;
      }
      if (capability.support == Support::Unknown)
        aggregate = Support::Unknown;
      else if (capability.support == Support::Unavailable && aggregate != Support::Unknown)
        aggregate = Support::Unavailable;
      if (!capability.reason.empty()) reasons += backend->name() + ": " + capability.reason + "\n";
      log.fallback(index, m_backends.size(), capability.support == Support::Unknown ? "probe-inconclusive" : "capability-rejected", capability.reason);
    } catch (const Error& error) {
      if (error.failure().effect != Effect::None) {
        const auto failure = readOnlyEffectViolation(error.failure(), Effect::None, "A UWF capability observation reported a mutation");
        log.attempt(index, "probe", "failed", nullptr, &failure, failure.effect);
        log.completed(Capability{Support::Unavailable, failure.message}, index, "observation-effect");
        throw Error(failure, error.attempts());
      }
      log.attempt(index, "probe", "failed", nullptr, &error.failure());
      if (aggregate != Support::Unknown) aggregate = Support::Unavailable;
      reasons += backend->name() + ": " + error.what() + "\n";
      log.fallback(index, m_backends.size(), "probe-exception", error.what(), &error.failure());
    } catch (const std::exception& error) {
      log.event("attempt", [&](LogStream& line) {
        line << " attempt=" << index + 1 << " backend=";
        logBackend(line, m_logBackendNames, index);
        line << " action=probe result=failed kind=Failed effect=None reason=";
        logQuoted(line, error.what());
      });
      if (aggregate != Support::Unknown) aggregate = Support::Unavailable;
      reasons += backend->name() + ": " + error.what() + "\n";
      log.fallback(index, m_backends.size(), "probe-exception", error.what());
    } catch (...) {
      log.event("attempt", [&](LogStream& line) {
        line << " attempt=" << index + 1 << " backend=";
        logBackend(line, m_logBackendNames, index);
        line << " action=probe result=failed kind=Failed effect=None reason=\"Unknown capability probe exception\"";
      });
      if (aggregate != Support::Unknown) aggregate = Support::Unavailable;
      reasons += backend->name() + ": capability probe failed\n";
      log.fallback(index, m_backends.size(), "probe-exception", "Unknown capability probe exception");
    }
  }
  Capability result{aggregate, std::move(reasons)};
  log.completed(result);
  return result;
}
Response FallbackUwf::execute(const Request& request, const Context& context) const { return executeRouted(request, context, 0); }
Response FallbackUwf::executeRouted(const Request& request, const Context& context, const std::uint64_t parentRequestId) const {
  if (const auto failure = requestInputFailure(request)) return Response::fail(failure->kind, failure->message);
  RoutingLog log(m_logId, m_logBackendNames, request, "execute", parentRequestId);
  std::vector<Attempt> attempts;
  const auto finishRequest = [&](Response response, const std::string_view stop, const std::optional<std::size_t> selected = {}) {
    auto result = finish(std::move(response), std::move(attempts));
    log.completed(result, stop, selected);
    return result;
  };
  auto last = Response::fail(ErrorKind::Unsupported, "No UWF backend provides " + operationName(request));
  for (std::size_t index = 0; index < m_backends.size(); ++index) {
    const auto& backend = m_backends[index];
    if (context.stop.stop_requested()) return finishRequest(stopped(), "canceled");
    log.attempt(index, "probe", "started");
    Capability capability;
    try {
      capability = backend->probe(request);
    } catch (...) {
      auto failure = exceptionFailure(Effect::None);
      if (failure.effect != Effect::None) {
        failure.failure = readOnlyEffectViolation(*failure.failure, failure.effect, "A UWF capability observation reported a mutation");
        failure.effect = failure.failure->effect;
      }
      log.attempt(index, "probe", "failed", nullptr, &*failure.failure, failure.effect);
      attempts.push_back({backend->name(), {Support::Unavailable, "Capability probe failed"}, failure.failure});
      attempts.insert(attempts.end(), std::make_move_iterator(failure.attempts.begin()), std::make_move_iterator(failure.attempts.end()));
      failure.attempts.clear();
      if (failure.effect != Effect::None) return finishRequest(std::move(failure), "probe-effect", index);
      last = std::move(failure);
      log.fallback(index, m_backends.size(), "probe-exception", last.failure->message, &*last.failure);
      continue;
    }
    log.attempt(index, "probe", supportName(capability.support), &capability);
    if (capability.support == Support::Unsupported || capability.support == Support::Unavailable) {
      attempts.push_back({backend->name(), capability, {}});
      if (capability.support == Support::Unavailable) last = Response::fail(ErrorKind::Unavailable, capability.reason);
      log.fallback(index, m_backends.size(), "capability-rejected", capability.reason);
      continue;
    }
    log.executing(index, capability);
    Response response;
    try {
      response = backend->execute(request, context);
    } catch (...) {
      response = exceptionFailure(isWrite(request) ? Effect::Uncertain : Effect::None,
                                  std::holds_alternative<GetOverlayFiles>(request) ? ReadCompletion::Unknown : ReadCompletion::NotDispatched);
    }
    response = normalizeResponse(request, std::move(response));
    log.attempt(index, "execute", response.succeeded() ? "success" : "failed", &capability, response.failure ? &*response.failure : nullptr, response.effect);
    attempts.push_back({backend->name(), capability, response.failure});
    attempts.insert(attempts.end(), std::make_move_iterator(response.attempts.begin()), std::make_move_iterator(response.attempts.end()));
    response.attempts.clear();
    if (response.succeeded()) return finishRequest(std::move(response), "success", index);
    const auto effect = response.effect == Effect::None ? response.failure->effect : response.effect;
    if (effect != Effect::None) {
      response.effect = response.failure->effect = effect;
      if (!isWrite(request)) return finishRequest(std::move(response), "observation-effect", index);
      return finishRequest(std::move(response), "write-effect", index);
    }
    if (response.failure->kind == ErrorKind::Canceled) return finishRequest(std::move(response), "canceled", index);
    if (!isWrite(request) && response.failure->readCompletion != ReadCompletion::NotDispatched)
      return finishRequest(std::move(response), "read-dispatched", index);
    last = std::move(response);
    log.fallback(index, m_backends.size(), "execution-failed", last.failure->message, &*last.failure);
  }
  return finishRequest(std::move(last), "exhausted");
}
}  // namespace uwf::compat
