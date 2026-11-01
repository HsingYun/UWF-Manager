// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/CompositeAllVolumeProtection.h"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "application/uwf/adapters/UwfCall.h"
#include "domain/uwf/policies/AllVolumeProtectionProjection.h"
#include "domain/uwf/solvers/AllVolumeProtectionSolver.h"

namespace uwf::app {
namespace {
using namespace compat;
void canceled(const Context& context) {
  if (context.stop.stop_requested()) throw Error({ErrorKind::Canceled, Effect::None, "The volume protection workflow was canceled", {}});
}
Failure exceptionFailure(const std::exception& error, const Effect unknown = Effect::None) {
  if (const auto* value = dynamic_cast<const Error*>(&error)) return value->failure();
  return {ErrorKind::Failed, unknown, error.what(), {}};
}
[[noreturn]] void fail(Failure failure) { throw Error(std::move(failure)); }
void appendFailure(std::optional<Failure>& target, const Failure& failure) {
  if (!target)
    target = failure;
  else {
    target->diagnostics.insert(target->diagnostics.end(), failure.diagnostics.begin(), failure.diagnostics.end());
    target->diagnostics.push_back({{}, "AllVolumeProtection", 0, failure.message});
  }
}
class CompositeAllVolumeProtection final : public compat::AllVolumeProtection {
 public:
  CompositeAllVolumeProtection(std::shared_ptr<Uwf> system, AllVolumeProtectionEnvironment environment)
      : m_system(std::move(system)), m_environment(std::move(environment)) {
    if (!m_system) throw std::invalid_argument("The volume protection workflow requires a UWF interface");
  }
  AllVolumeProtectionObservation read(const GetAllProtected&, const Context& context) const override {
    return compat::synchronizedUwf(*m_system, [&] { return observe(true, context); });
  }
  AllVolumeProtectionObservation read(const GetAllUnprotected&, const Context& context) const override {
    return compat::synchronizedUwf(*m_system, [&] { return observe(false, context); });
  }
  AllVolumeProtectionObservation snapshot(const GetAllProtected&) const override { return project(true); }
  AllVolumeProtectionObservation snapshot(const GetAllUnprotected&) const override { return project(false); }
  compat::AllVolumeProtectionResult execute(const ProtectAll&, const Context& context) const override { return apply(true, context); }
  compat::AllVolumeProtectionResult execute(const UnprotectAll&, const Context& context) const override { return apply(false, context); }

 private:
  AllVolumeProtectionObservation observe(const bool protect, const Context& context) const {
    AllVolumeProtectionObservation result;
    std::set<std::string> absent;
    std::optional<std::vector<Volume>> registered;
    std::optional<bool> administrator;
    if (context.stop.stop_requested()) {
      result.failure = Failure{ErrorKind::Canceled, Effect::None, "The volume protection observation was canceled", {}};
      return result;
    }
    if (m_environment.isAdministrator) {
      try {
        administrator = m_environment.isAdministrator(context);
      } catch (const std::exception&) {
      }
    }
    try {
      canceled(context);
      std::vector<std::string> scope;
      if (protect) {
        if (!m_environment.protectScope) fail({ErrorKind::Unsupported, Effect::None, "The complete physical volume protection scope is unavailable", {}});
        const auto filter = compat::call(*m_system, GetBoolean{BooleanProperty::FilterEnabled, Session::Current}, context);
        scope = m_environment.protectScope(filter, context);
      } else {
        // Complete registered Next inventory is the scope, independent of
        // physical discovery. Disconnected/configured volumes remain targets.
        registered = compat::call(*m_system, GetVolumes{Session::Next}, context);
        for (const auto& volume : *registered) scope.push_back(volume.id);
      }
      std::set<std::string> seen;
      for (const auto& volume : scope) {
        const auto identity = parseVolumeId(volume);
        if (!identity) fail({ErrorKind::Protocol, Effect::None, "The complete volume protection scope contains an invalid identity", {}});
        if (seen.insert(*identity).second) result.volumes.push_back({*identity, {}, {}});
      }
      result.scopeConfirmed = true;
      if (protect && result.volumes.empty()) fail({ErrorKind::Unavailable, Effect::None, "The complete volume protection scope is empty", {}});
      for (auto& volume : result.volumes) {
        canceled(context);
        try {
          volume.nextProtected = compat::call(*m_system, GetVolumeProperty{volume.volume, VolumeProperty::Protected, Session::Next}, context);
        } catch (const Error& error) {
          // Known absence is already unprotected. A transport/schema error is
          // never absence and prevents this observation's aggregate becoming true.
          if (error.failure().kind == ErrorKind::NotFound && error.failure().effect == Effect::None) {
            volume.nextProtected = false;
            absent.insert(volume.volume);
          } else {
            volume.failure = error.failure();
            appendFailure(result.failure, *volume.failure);
            if (error.failure().kind == ErrorKind::Canceled) throw;
          }
        } catch (const std::exception& error) {
          volume.failure = exceptionFailure(error);
          appendFailure(result.failure, *volume.failure);
        }
      }
      result.value = protect ? solveGetAllProtected(GetAllProtected{}, result) : solveGetAllUnprotected(GetAllUnprotected{}, result);
    } catch (const std::exception& error) {
      appendFailure(result.failure, exceptionFailure(error));
      result.value.reset();
    }
    auto state = m_system->state();
    if (administrator) state.administrator = Fact<bool>::known(*administrator);
    if (protect) {
      // A failed discovery cannot leave an older physical scope usable.
      state.runtime.protectScope = {};
      if (result.scopeConfirmed && !result.volumes.empty()) {
        std::vector<std::string> scope;
        for (const auto& volume : result.volumes) scope.push_back(volume.volume);
        state.runtime.protectScope = Fact<std::vector<std::string>>::known(std::move(scope));
      }
    }
    if (registered) {
      state.next.volumesObserved = true;
      for (auto& [id, volume] : state.next.volumes) {
        (void)id;
        volume.present = Fact<bool>::known(false);
      }
      for (const auto& volume : *registered) {
        auto& observed = state.next.volumes[normalizeVolumeId(volume.id)];
        observed.present = Fact<bool>::known(true);
        observed.driveLetter = Fact<std::string>::known(volume.driveLetter);
        observed.bindByDriveLetter = Fact<bool>::known(volume.bindByDriveLetter);
        observed.protectedVolume = Fact<bool>::known(volume.protectedVolume);
      }
    }
    if (result.scopeConfirmed) {
      for (const auto& volume : result.volumes) {
        auto& observed = state.next.volumes[volume.volume];
        if (absent.contains(volume.volume)) {
          observed.present = Fact<bool>::known(false);
          observed.protectedVolume = Fact<bool>::known(false);
        } else if (volume.nextProtected) {
          observed.present = Fact<bool>::known(true);
          observed.protectedVolume = Fact<bool>::known(*volume.nextProtected);
        } else {
          observed.protectedVolume = {};
          // Only a successful inventory independently proves membership. An old
          // absence would otherwise turn this failed getter into an unprotected target.
          if (!registered) observed.present = {};
        }
      }
    }
    m_system->updateState(std::move(state));
    return result;
  }
  AllVolumeProtectionObservation project(const bool protect) const { return projectAllVolumeProtection(m_system->state(), protect); }
  AllVolumeProtectionFacts admission(const Context& context) const {
    canceled(context);
    const auto state = m_system->state();
    if (state.refreshFailure)
      fail({ErrorKind::Unavailable, Effect::None, "The global UWF state refresh failed; refresh it before another write: " + state.refreshFailure->message,
            state.refreshFailure->diagnostics});
    return {state.administrator.value, state.runtime.horm.value, {}, compat::configurationModeState(state)};
  }
  compat::AllVolumeProtectionResult apply(const bool protect, const Context& context) const {
    compat::AllVolumeProtectionResult result;
    AllVolumeProtectionObservation before;
    try {
      auto facts = admission(context);
      // Validate session admission before processing even already-matching volumes.
      const auto session = protect ? solveProtectAll(ProtectAll{}, facts) : solveUnprotectAll(UnprotectAll{}, facts);
      if (session.failure) fail(*session.failure);
      before = project(protect);
      facts.observation = before;
      const auto evaluation = protect ? solveProtectAll(ProtectAll{}, facts) : solveUnprotectAll(UnprotectAll{}, facts);
      if (evaluation.failure) fail(*evaluation.failure);
      if (evaluation.resolution != AllVolumeProtectionResolution::Ready)
        fail(before.failure.value_or(Failure{ErrorKind::Unavailable, Effect::None, "The complete volume protection scope could not be read", {}}));
      for (const auto& volume : before.volumes) {
        canceled(context);
        compat::AllVolumeProtectionItemResult item;
        item.volume = volume.volume;
        item.before = volume.nextProtected;
        if (volume.failure || !volume.nextProtected) {
          item.failure = volume.failure.value_or(Failure{ErrorKind::Unavailable, Effect::None, "The next-startup volume protection could not be read", {}});
        } else if (*volume.nextProtected == protect) {
          item.confirmedProtection = item.before;
          item.completionConfirmed = true;
        } else {
          // Session admission failure stops the batch; each setter also validates its own operation.
          auto fresh = admission(context);
          fresh.observation = before;
          const auto allowed = protect ? solveProtectAll(ProtectAll{}, fresh) : solveUnprotectAll(UnprotectAll{}, fresh);
          if (allowed.failure) fail(*allowed.failure);
          const SetVolume request{volume.volume, VolumeProperty::Protected, protect, {}, false};
          try {
            auto receipt = normalizeResponse(Request{request}, m_system->execute(request, context));
            item.effect = receipt.effect;
            item.failure = std::move(receipt.failure);
            item.attempts = std::move(receipt.attempts);
          } catch (const std::exception& error) {
            item.failure = exceptionFailure(error, Effect::Uncertain);
            item.effect = item.failure->effect;
          } catch (...) {
            item.failure = Failure{ErrorKind::Failed, Effect::Uncertain, "The volume protection write failed with an unknown error", {}};
            item.effect = Effect::Uncertain;
          }
          if (!item.failure) {
            item.confirmedProtection = protect;
            item.completionConfirmed = true;
          }
          if (item.failure) item.failure->effect = combineEffects(item.failure->effect, item.effect);
        }
        result.effect = combineEffects(result.effect, item.effect);
        if (item.failure) appendFailure(result.failure, *item.failure);
        const bool stop = item.failure && item.failure->kind == ErrorKind::Canceled;
        result.items.push_back(std::move(item));
        if (stop) break;
      }
    } catch (const std::exception& error) {
      appendFailure(result.failure, exceptionFailure(error));
    }
    result.observation = project(protect);
    // The aggregate is display data. Successful entry-point receipts determine
    // completion even when a projected observation is unavailable.
    result.completionConfirmed = !result.failure && result.items.size() == before.volumes.size() &&
                                 std::ranges::all_of(result.items, [](const auto& item) { return item.completionConfirmed; });
    if (result.failure) result.failure->effect = result.effect = combineEffects(result.failure->effect, result.effect);
    return result;
  }
  std::shared_ptr<Uwf> m_system;
  AllVolumeProtectionEnvironment m_environment;
};

AllVolumeProtectionSnapshot observeFacade(const compat::AllVolumeProtection& operations, const std::stop_token stop, const bool refresh = true) {
  const auto protectedState = refresh ? operations.read(GetAllProtected{}, {stop}) : operations.snapshot(GetAllProtected{});
  const auto unprotectedState = refresh ? operations.read(GetAllUnprotected{}, {stop}) : operations.snapshot(GetAllUnprotected{});
  AllVolumeProtectionSnapshot result;
  result.protectSupported = !protectedState.failure || protectedState.failure->kind != ErrorKind::Unsupported;
  result.unprotectSupported = !unprotectedState.failure || unprotectedState.failure->kind != ErrorKind::Unsupported;
  result.protectScopeConfirmed = protectedState.scopeConfirmed;
  result.unprotectScopeConfirmed = unprotectedState.scopeConfirmed;
  result.nextAllProtected = protectedState.value;
  result.nextAllUnprotected = unprotectedState.value;
  if (protectedState.failure) result.protectError = QString::fromStdString(protectedState.failure->message);
  if (unprotectedState.failure) result.unprotectError = QString::fromStdString(unprotectedState.failure->message);
  for (const auto& volume : protectedState.volumes) result.protectTargetVolumes.push_back(QString::fromStdString(volume.volume));
  for (const auto& volume : unprotectedState.volumes) result.unprotectTargetVolumes.push_back(QString::fromStdString(volume.volume));
  return result;
}
OperationFailure appFailure(const compat::Failure& failure, const std::vector<compat::Attempt>& attempts = {}) {
  try {
    throwUwfOperationError(compat::Error(failure, attempts));
  } catch (const std::exception& error) {
    return operationFailure(error);
  }
}
class AllVolumeProtectionFacade final : public app::AllVolumeProtectionActions {
 public:
  explicit AllVolumeProtectionFacade(std::shared_ptr<compat::AllVolumeProtection> operations) : m_operations(std::move(operations)) {
    if (!m_operations) throw std::invalid_argument("The complete volume protection interface is missing");
  }
  AllVolumeProtectionSnapshot observe(const std::stop_token stop) override { return observeFacade(*m_operations, stop); }
  std::optional<AllVolumeProtectionSnapshot> snapshot() const override { return observeFacade(*m_operations, {}, false); }
  app::AllVolumeProtectionResult setProtected(const bool enabled, const std::stop_token stop) override {
    const auto receipt = enabled ? m_operations->execute(ProtectAll{}, {stop}) : m_operations->execute(UnprotectAll{}, {stop});
    app::AllVolumeProtectionResult result;
    result.snapshot = observeFacade(*m_operations, stop, false);
    result.effect = uwfWriteEffect(receipt.effect);
    result.completionConfirmed = receipt.completionConfirmed;
    if (receipt.failure) {
      result.error = QString::fromStdString(uwfDiagnostics(compat::Error(*receipt.failure)));
      result.failure = appFailure(*receipt.failure);
    }
    for (const auto& item : receipt.items) {
      app::AllVolumeProtectionItemResult row;
      row.volumeId = QString::fromStdString(item.volume);
      row.before = item.before;
      row.confirmedProtection = item.confirmedProtection;
      row.effect = uwfWriteEffect(item.effect);
      row.completionConfirmed = item.completionConfirmed;
      if (item.failure) {
        row.error = QString::fromStdString(uwfDiagnostics(compat::Error(*item.failure, item.attempts)));
        row.failure = appFailure(*item.failure, item.attempts);
      }
      result.items.push_back(std::move(row));
    }
    return result;
  }

 private:
  std::shared_ptr<compat::AllVolumeProtection> m_operations;
};
}  // namespace
std::shared_ptr<compat::AllVolumeProtection> composeAllVolumeProtection(std::shared_ptr<compat::Uwf> system,
                                                                        compat::AllVolumeProtectionEnvironment environment) {
  return std::make_shared<CompositeAllVolumeProtection>(std::move(system), std::move(environment));
}
std::shared_ptr<AllVolumeProtectionActions> allVolumeProtectionActions(std::shared_ptr<compat::AllVolumeProtection> operations) {
  return std::make_shared<AllVolumeProtectionFacade>(std::move(operations));
}
}  // namespace uwf::app
