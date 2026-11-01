// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"

#include <limits>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "application/localization/ApplicationText.h"
#include "application/staging/policy/AutomaticCommitPolicy.h"
#include "application/uwf/adapters/UwfCall.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "domain/uwf/contracts/UwfStateFacts.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"
#include "domain/uwf/policies/IndividualCommitPolicy.h"

namespace uwf::app {
namespace {
template <class T>
const T& requireObserved(const compat::Fact<T>& fact, const char* message) {
  if (!fact.value) throw OperationError(std::make_error_code(std::errc::state_not_recoverable), message);
  return *fact.value;
}
void requireIndividualCommitMode(const compat::UwfStateFacts& state) {
  const auto horm = requireObserved(state.runtime.horm, "The global HORM state is unavailable");
  std::optional<std::uint32_t> flags;
  if (!horm) {
    if (state.current.overlayFlags.knownMask != std::numeric_limits<std::uint32_t>::max())
      throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global Current overlay flags are unavailable");
    flags = state.current.overlayFlags.valueBits;
  }
  const auto mode = compat::individualCommitState(state);
  if (!horm && flags && (*flags & core::kLoadedReadOnlyMediaFlag) != 0 && !mode.romDisposition && mode.fullCommit != true)
    throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global ROM disposition is unavailable");
  switch (core::individualCommitRestriction(true, true, mode)) {
    case core::IndividualCommitRestriction::HormActive:
      throw OperationError(std::make_error_code(std::errc::permission_denied),
                           ApplicationText::tr("File and registry commits are unavailable while HORM is active.").toStdString());
    case core::IndividualCommitRestriction::ReadOnlyMediaActive:
      throw OperationError(std::make_error_code(std::errc::permission_denied),
                           ApplicationText::tr("File and registry commits are unavailable while Read-Only Media mode is active.").toStdString());
    case core::IndividualCommitRestriction::PersistentViewUnavailable:
      throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The shared persistent file view is not ready");
    case core::IndividualCommitRestriction::None:
      return;
  }
}

bool volumeProtected(const compat::UwfStateFacts& state, const std::string& volume) {
  const auto found = state.current.volumes.find(compat::normalizeVolumeId(volume));
  if (found == state.current.volumes.end()) {
    if (state.current.volumesObserved) return false;
    throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global Current volume inventory is unavailable");
  }
  if (!requireObserved(found->second.present, "The global Current volume membership is unavailable")) return false;
  return requireObserved(found->second.protectedVolume, "The global Current volume protection is unavailable");
}

class VolumeTarget final : public UwfVolumeTarget {
 public:
  VolumeTarget(std::shared_ptr<compat::Uwf> system, const compat::Volume& volume, const compat::Session session)
      : m_system(std::move(system)), m_id(compat::normalizeVolumeId(volume.id)), m_session(session), m_volume(toVolumeRecord(volume)) {}
  const core::VolumeRecord& volume() const override { return m_volume; }
  std::vector<std::string> exclusions() const override {
    const auto state = m_system->stateSnapshot();
    const auto& session = m_session == compat::Session::Current ? state->current : state->next;
    const auto found = session.volumes.find(m_id);
    if (found == session.volumes.end())
      throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global file exclusion target is unavailable");
    return requireObserved(found->second.fileExclusions, "The global file exclusions are unavailable");
  }
  WriteEffect commitFile(const std::string& path) const override {
    return callUwfWrite(*m_system, compat::CommitFile{m_id, toUwfFilePath(path, m_volume.driveLetter)});
  }
  WriteEffect commitFileDeletion(const std::string& path) const override {
    return callUwfWrite(*m_system, compat::CommitFileDeletion{m_id, toUwfFilePath(path, m_volume.driveLetter)});
  }

 private:
  std::shared_ptr<compat::Uwf> m_system;
  std::string m_id;
  compat::Session m_session;
  core::VolumeRecord m_volume;
};

class RegistryMaintenanceTarget final : public UwfRegistryMaintenanceTarget {
 public:
  explicit RegistryMaintenanceTarget(std::shared_ptr<compat::Uwf> system) : m_system(std::move(system)) {}
  std::vector<std::string> exclusions() const override {
    const auto state = m_system->stateSnapshot();
    return requireObserved(state->current.registryExclusions, "The global Current registry exclusions are unavailable");
  }
  std::uint32_t currentControlSet() const override {
    const auto state = m_system->stateSnapshot();
    return requireObserved(state->runtime.currentControlSet, "The global current registry control set is unavailable");
  }
  WriteEffect commit(const std::string& key, const std::string& valueName) const override {
    return callUwfWrite(*m_system, compat::CommitRegistry{key, valueName});
  }
  WriteEffect commitDeletion(const std::string& key, const std::optional<std::string>& valueName) const override {
    return callUwfWrite(*m_system, compat::CommitRegistryDeletion{key, valueName});
  }

 private:
  std::shared_ptr<compat::Uwf> m_system;
};

class RegistryExclusionConfiguration final : public UwfRegistryExclusionConfiguration {
 public:
  explicit RegistryExclusionConfiguration(std::shared_ptr<compat::Uwf> system) : m_system(std::move(system)) {}
  std::vector<std::string> exclusions() const override {
    const auto state = m_system->stateSnapshot();
    return requireObserved(state->next.registryExclusions, "The global Next registry exclusions are unavailable");
  }
  void addExclusion(const std::string& key) const override { (void)callUwf(*m_system, compat::EditRegistryExclusion{key, compat::ExclusionAction::Add}); }

 private:
  std::shared_ptr<compat::Uwf> m_system;
};

class MaintenanceRepository final : public UwfMaintenanceRepository {
 public:
  MaintenanceRepository(std::shared_ptr<compat::Uwf> system) : m_system(std::move(system)) {
    if (!m_system) throw std::invalid_argument("UWF compatibility service is required");
  }
  CommitRequirement fileCommitRequirement(const std::string& volumeId) const override {
    const auto state = m_system->stateSnapshot();
    if (!requireObserved(state->current.filterEnabled, "The global Current filter state is unavailable") || !volumeProtected(*state, volumeId))
      return CommitRequirement::AlreadyPersistent;
    requireIndividualCommitMode(*state);
    return CommitRequirement::RequiresUwfCommit;
  }
  CommitRequirement registryCommitRequirement() const override {
    const auto state = m_system->stateSnapshot();
    if (!requireObserved(state->current.filterEnabled, "The global Current filter state is unavailable")) return CommitRequirement::AlreadyPersistent;
    const auto& systemVolume = requireObserved(state->systemVolumeId, "The global system volume identity is unavailable");
    if (!volumeProtected(*state, systemVolume)) return CommitRequirement::AlreadyPersistent;
    requireIndividualCommitMode(*state);
    return CommitRequirement::RequiresUwfCommit;
  }
  AutomaticCommitDisposition automaticFileCommitDisposition(const std::string& volumeId) const override {
    const auto state = m_system->stateSnapshot();
    const auto id = compat::normalizeVolumeId(volumeId);
    const auto protectedIn = [&](const compat::SessionStateFacts& session) -> std::optional<bool> {
      const auto found = session.volumes.find(id);
      if (found == session.volumes.end()) return session.volumesObserved ? std::optional{false} : std::nullopt;
      if (found->second.present.value != true) return found->second.present.value ? std::optional{false} : std::nullopt;
      return found->second.protectedVolume.value;
    };
    const auto mode = compat::individualCommitState(*state);
    const AutomaticCommitContext context{UwfOverlayOperations::snapshot(*state), mode.hormEnabled, mode.romDisposition, mode.fullCommit};
    return uwf::app::automaticFileCommitDisposition(context, protectedIn(state->current), protectedIn(state->next));
  }
  void refreshAutomaticCommitState() const override {
    const auto response = m_system->refreshState();
    if (response.failure) throwUwfOperationError(compat::Error(*response.failure, response.attempts));
  }
  CommitRequirement registryWriteRequirement() const override {
    const auto state = m_system->stateSnapshot();
    if (state->refreshFailure)
      throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global UWF state refresh failed; refresh it before a registry write");
    switch (core::registryWriteDisposition(compat::configurationModeState(*state))) {
      case core::RegistryWriteDisposition::Direct:
        return CommitRequirement::AlreadyPersistent;
      case core::RegistryWriteDisposition::RequiresCommit:
        return CommitRequirement::RequiresUwfCommit;
      case core::RegistryWriteDisposition::HormActive:
        throw OperationError(std::make_error_code(std::errc::permission_denied),
                             ApplicationText::tr("Registry writes are unavailable while HORM is active.").toStdString());
      case core::RegistryWriteDisposition::ReadOnlyMediaActive:
        throw OperationError(std::make_error_code(std::errc::permission_denied),
                             ApplicationText::tr("Registry writes cannot be persisted while Read-Only Media mode is active.").toStdString());
      case core::RegistryWriteDisposition::Unknown:
        throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global registry write persistence state is unavailable");
    }
    throw std::logic_error("Unknown registry write disposition");
  }

  std::vector<std::shared_ptr<UwfVolumeTarget>> prepareVolumeTargets() const override {
    const auto state = m_system->stateSnapshot();
    if (!state->current.volumesObserved)
      throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global Current volume inventory is unavailable");
    std::vector<std::shared_ptr<UwfVolumeTarget>> result;
    for (const auto& [id, observed] : state->current.volumes) {
      if (!requireObserved(observed.present, "The global Current volume membership is unavailable")) continue;
      const auto& drive = observed.physicalDriveLetter.isKnown() ? observed.physicalDriveLetter : observed.driveLetter;
      const compat::Volume volume{id,
                                  requireObserved(drive, "The global volume drive letter is unavailable"),
                                  requireObserved(observed.protectedVolume, "The global Current volume protection is unavailable"),
                                  requireObserved(observed.bindByDriveLetter, "The global volume binding is unavailable"),
                                  {}};
      result.push_back(std::make_shared<VolumeTarget>(m_system, volume, compat::Session::Current));
    }
    return result;
  }
  std::shared_ptr<UwfRegistryMaintenanceTarget> prepareRegistryMaintenance() const override { return std::make_shared<RegistryMaintenanceTarget>(m_system); }
  std::shared_ptr<UwfRegistryExclusionConfiguration> prepareRegistryExclusions() const override {
    return std::make_shared<RegistryExclusionConfiguration>(m_system);
  }
  std::function<void()> preparePowerAction(const PowerAction action) const override {
    const auto target = action == PowerAction::Shutdown ? compat::PowerAction::Shutdown : compat::PowerAction::Restart;
    const auto capability = m_system->probe(compat::Power{target});
    if (capability.support == compat::Support::Unsupported || capability.support == compat::Support::Unavailable)
      throwUwfOperationError(
          compat::Error({capability.support == compat::Support::Unsupported ? compat::ErrorKind::Unsupported : compat::ErrorKind::Unavailable,
                         compat::Effect::None,
                         capability.reason,
                         {}}));
    return [system = m_system, target] { (void)callUwf(*system, compat::Power{target}); };
  }

 private:
  std::shared_ptr<compat::Uwf> m_system;
};
}  // namespace

std::shared_ptr<UwfMaintenanceRepository> uwfMaintenanceRepository(std::shared_ptr<compat::Uwf> system) {
  return std::make_shared<MaintenanceRepository>(std::move(system));
}
}  // namespace uwf::app
