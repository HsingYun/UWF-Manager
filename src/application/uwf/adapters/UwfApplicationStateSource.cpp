// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/adapters/UwfApplicationStateSource.h"

#include <stdexcept>
#include <utility>

#include "application/uwf/adapters/UwfCall.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "foundation/concurrency/AtomicSnapshot.h"
namespace uwf::app {
namespace {
class SystemApplicationStateSource final : public ApplicationStateSource {
 public:
  SystemApplicationStateSource(std::shared_ptr<compat::Uwf> system, std::shared_ptr<const app::VolumeRepository> volumes, bool elevated)
      : m_system(std::move(system)), m_volumes(std::move(volumes)), m_elevated(elevated) {}
  ReadOperation readOperation(const app::UwfCapability capability) override { return prepareOperation(capability); }
  ReadOperation publishedOperation(const app::UwfCapability capability) override {
    return [capability, system = m_system, cache = m_cache, elevated = m_elevated](const std::stop_token stop) {
      if (stop.stop_requested()) throw compat::Error({compat::ErrorKind::Canceled, compat::Effect::None, "Application state refresh canceled", {}});
      const auto facts = system->state();
      if (capability == app::UwfCapability::Available && facts.refreshFailure) throw compat::Error(*facts.refreshFailure);
      const auto previous = cache->snapshot.load();
      if (!previous) throw compat::Error({compat::ErrorKind::Unavailable, compat::Effect::None, "Application state has not been refreshed", {}});
      auto result = *previous;
      const compat::OverlayUsage usage{result.snapshot.runtime.currentConsumptionMb, result.snapshot.runtime.availableSpaceMb};
      result.snapshot = app::publishedUwfSnapshot(facts, capability, elevated, usage);
      result.overlayPolicy = app::UwfOverlayOperations::snapshot(facts);
      result.hormEnabled = facts.runtime.horm.value;
      result.overlayUsageSampled = false;
      return result;
    };
  }

 private:
  struct PublishedState {
    foundation::AtomicSnapshot<ApplicationState> snapshot;
  };
  ReadOperation prepareOperation(const app::UwfCapability capability) const {
    return [capability, system = m_system, volumes = m_volumes, cache = m_cache, elevated = m_elevated](const std::stop_token stop) {
      auto result = compat::synchronizedUwf(*system, [&] {
        const auto canceled = [&] {
          if (stop.stop_requested()) throw compat::Error({compat::ErrorKind::Canceled, compat::Effect::None, "Application state refresh canceled", {}});
        };
        canceled();
        auto physical = volumes->read();
        canceled();
        auto facts = system->state();
        facts.administrator = compat::Fact<bool>::known(elevated);
        if (capability != app::UwfCapability::Available) app::publishUwfPhysicalState(facts, physical);
        system->updateState(std::move(facts));
        if (capability == app::UwfCapability::Available) {
          const auto refreshed = app::refreshUwfStateWithPhysicalInventory(*system, physical, {stop});
          if (refreshed.failure) throw compat::Error(*refreshed.failure, refreshed.attempts);
        }
        canceled();
        if (capability == app::UwfCapability::Available)
          if (const auto failure = system->state().refreshFailure) throw compat::Error(*failure);
        const auto usage = capability == app::UwfCapability::Available ? app::callUwf(*system, compat::GetOverlayUsage{}, {stop}) : compat::OverlayUsage{};
        canceled();
        const auto published = system->state();
        ApplicationState observed{std::move(physical), app::publishedUwfSnapshot(published, capability, elevated, usage),
                                  app::UwfOverlayOperations::snapshot(published)};
        observed.hormEnabled = published.runtime.horm.value;
        return observed;
      });
      cache->snapshot.store(std::make_shared<const ApplicationState>(result));
      return result;
    };
  }

  std::shared_ptr<compat::Uwf> m_system;
  std::shared_ptr<const app::VolumeRepository> m_volumes;
  bool m_elevated;
  std::shared_ptr<PublishedState> m_cache = std::make_shared<PublishedState>();
};

}  // namespace
std::unique_ptr<ApplicationStateSource> uwfApplicationStateSource(std::shared_ptr<compat::Uwf> system, std::shared_ptr<const VolumeRepository> volumes,
                                                                  bool elevated) {
  if (!system || !volumes) throw std::invalid_argument("Application state requires UWF and volume sources");
  return std::make_unique<SystemApplicationStateSource>(std::move(system), std::move(volumes), elevated);
}
}  // namespace uwf::app
