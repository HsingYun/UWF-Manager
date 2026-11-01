// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <type_traits>
#include <utility>

#include "domain/uwf/contracts/Requests.h"
#include "domain/uwf/contracts/UwfStateFacts.h"

namespace uwf::compat {
struct Context {
  std::stop_token stop;
  const UwfStateFacts* facts = nullptr;
};
struct Attempt {
  std::string backend;
  Capability capability;
  std::optional<Failure> failure;
};
struct Response {
  Value value;
  std::optional<Failure> failure;
  Effect effect = Effect::None;
  std::vector<Attempt> attempts;
  std::vector<std::string> warnings = {};
  [[nodiscard]] bool succeeded() const { return !failure; }
  template <class T = std::monostate>
  static Response success(T value = {}, Effect effect = Effect::None) {
    return {Value{std::move(value)}, {}, effect, {}, {}};
  }
  static Response fail(ErrorKind kind, std::string message, Effect effect = Effect::None, std::vector<Diagnostic> diagnostics = {},
                       ReadCompletion readCompletion = ReadCompletion::NotDispatched) {
    return {{}, Failure{kind, effect, std::move(message), std::move(diagnostics), readCompletion}, effect, {}, {}};
  }
};
// An uncertain effect is never a successful completion receipt. Normalize at
// every compatibility execution boundary, including injected implementations.
[[nodiscard]] inline Response normalizeResponse(const Request& request, Response response) {
  if (!isWrite(request) && (response.effect != Effect::None || (response.failure && response.failure->effect != Effect::None))) {
    response.failure = readOnlyEffectViolation(response.failure.value_or(Failure{}), response.effect, "A UWF observation reported a mutation effect");
    response.effect = response.failure->effect;
  }
  if (response.succeeded()) {
    if (auto failure = normalizeValue(request, response.value)) {
      failure->effect = isWrite(request) ? Effect::Uncertain : Effect::None;
      if (std::holds_alternative<GetOverlayFiles>(request)) failure->readCompletion = ReadCompletion::ProviderCompleted;
      response.failure = std::move(failure);
    }
  }
  if (response.succeeded() && response.effect == Effect::Uncertain) {
    response.failure = Failure{ErrorKind::Failed, Effect::Uncertain, "The UWF operation outcome could not be confirmed", {}};
  }
  if (response.failure) {
    const auto effect = combineEffects(response.effect, response.failure->effect);
    response.effect = response.failure->effect = effect;
  }
  return response;
}
class Error final : public std::runtime_error {
 public:
  explicit Error(Failure failure, std::vector<Attempt> attempts = {})
      : std::runtime_error(failure.message), m_failure(std::move(failure)), m_attempts(std::move(attempts)) {}
  [[nodiscard]] const Failure& failure() const { return m_failure; }
  [[nodiscard]] const std::vector<Attempt>& attempts() const { return m_attempts; }

 private:
  Failure m_failure;
  std::vector<Attempt> m_attempts;
};

using UwfStateSnapshot = std::shared_ptr<const UwfStateFacts>;

// Synchronous port governed by Requests.h. Scheduling and UI policy belong to
// callers; platform execution contexts remain internal. The UWF abstraction owns
// shared state, prechecks and cross-capability composition.
class Uwf {
 public:
  virtual ~Uwf() = default;
  [[nodiscard]] virtual Capability probe(const Request& request) const = 0;
  [[nodiscard]] virtual Response execute(const Request& request, const Context& context = {}) const = 0;
  // Readers retain one immutable in-memory publication. Implementations always
  // return a non-null handle; ordinary reads do not copy inventories.
  [[nodiscard]] virtual UwfStateSnapshot stateSnapshot() const {
    static const auto empty = std::make_shared<const UwfStateFacts>();
    return empty;
  }
  // Refresh/edit callers explicitly request an owned copy of the same source.
  [[nodiscard]] UwfStateFacts state() const { return *stateSnapshot(); }
  virtual void updateState(UwfStateFacts) const {}
  // A refresh owns its reads and publication as one operation, serialized with
  // writes. Reentry from that operation executes in the same session.
  virtual void synchronize(const std::function<void()>& work) const { work(); }
  [[nodiscard]] virtual Response refreshState(const Context& = {}) const { return Response::success(); }
};
template <class Work>
auto synchronizedUwf(const Uwf& system, Work work) -> std::invoke_result_t<Work> {
  using WorkResult = std::invoke_result_t<Work>;
  if constexpr (std::is_void_v<WorkResult>) {
    system.synchronize(work);
  } else {
    std::optional<WorkResult> result;
    system.synchronize([&] { result.emplace(work()); });
    return std::move(*result);
  }
}
class Backend : public Uwf {
 public:
  // Stable logical identity shared by routing and backend-origin diagnostics;
  // module filenames and implementation paths are not backend identities.
  [[nodiscard]] virtual std::string name() const = 0;
};
template <class R>
[[nodiscard]] typename R::Result call(const Uwf& system, const R& request, const Context& context = {}) {
  const Request operation{request};
  auto response = normalizeResponse(operation, system.execute(operation, context));
  if (response.failure) throw Error(std::move(*response.failure), std::move(response.attempts));
  return std::move(std::get<typename R::Result>(response.value));
}
}  // namespace uwf::compat
