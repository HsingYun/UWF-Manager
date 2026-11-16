// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/persistence/DurableFileStagingStore.h"

#include <optional>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/localization/ApplicationText.h"
#include "application/operations/contracts/OperationError.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/staging/persistence/FileStagingRegistry.h"

namespace uwf::app {
namespace {

template <class Work>
void executeDurabilityWrite(Work&& work) {
  bool attempted = false;
  try {
    std::forward<Work>(work)(attempted);
  } catch (const OperationError& error) {
    if (attempted) throw OperationError(error, WriteEffect::Uncertain);
    throw;
  } catch (const std::exception& error) {
    throw OperationError(std::make_error_code(std::errc::io_error), error.what(), attempted ? WriteEffect::Uncertain : WriteEffect::None);
  } catch (...) {
    throw OperationError(std::make_error_code(std::errc::io_error), ApplicationText::tr("File staging could not be saved").toStdString(),
                         attempted ? WriteEffect::Uncertain : WriteEffect::None);
  }
}

}  // namespace

DurableFileStagingStore::DurableFileStagingStore(std::shared_ptr<FileStagingStorage> storage, std::shared_ptr<UwfMaintenanceRepository> repository,
                                                 UwfCapability capability)
    : m_storage(std::move(storage)), m_repository(std::move(repository)), m_capability(capability) {
  if (!m_storage || !m_repository) throw std::invalid_argument("durable staging requires storage and a repository");
}
QList<FileStagingEntry> DurableFileStagingStore::load() const { return m_storage->load(); }
void DurableFileStagingStore::replace(const QList<FileStagingEntry>& entries) {
  executeDurabilityWrite([&](bool& attempted) {
    const auto requirement = m_capability == UwfCapability::Available ? m_repository->registryWriteRequirement() : CommitRequirement::AlreadyPersistent;
    auto current = prepareCurrentCommit(requirement);
    // Revalidate after exclusion discovery before choosing the host mutation.
    if (m_capability == UwfCapability::Available && m_repository->registryWriteRequirement() == CommitRequirement::AlreadyPersistent) current.reset();
    const auto mutateStorage = [&](auto&& mutation) {
      attempted = true;
      std::forward<decltype(mutation)>(mutation)();
    };
    if (!entries.isEmpty()) {
      mutateStorage([&] { m_storage->write(entries); });
      if (current) current->commit(std::string(kFileStagingRegistryKey), std::string(kFileStagingEntriesValue));
      return;
    }
    if (!current) {
      mutateStorage([&] { m_storage->erase(); });
      return;
    }
    // Persist an empty payload before deleting metadata so a later failure cannot
    // revive the old list. Visible key absence alone does not prove the base hive is empty.
    mutateStorage([&] { m_storage->write({}); });
    current->commit(std::string(kFileStagingRegistryKey), std::string(kFileStagingEntriesValue));
    const auto deletions = m_storage->deletionPlan();
    for (const auto& target : deletions) (void)target.deletionValueName();
    for (const auto& target : deletions) current->commitDeletion(target.key, target.deletionValueName());
  });
}
void DurableFileStagingStore::reconcileDurability() {
  executeDurabilityWrite([&](bool& attempted) {
    const auto entries = load();
    if (entries.isEmpty() || m_capability != UwfCapability::Available) return;
    const auto current = prepareCurrentCommit(m_repository->registryCommitRequirement());
    if (current) {
      attempted = true;
      current->commit(std::string(kFileStagingRegistryKey), std::string(kFileStagingEntriesValue));
    }
    const auto next = m_repository->prepareRegistryExclusions();
    if (!next) throw std::runtime_error("next UWF registry session is unavailable");
    core::UwfSnapshot snapshot;
    snapshot.next.registryExclusions = next->exclusions();
    const auto plan = planConfiguration({}, std::move(snapshot), FileStagingChanges{{}, entries, true});
    for (const auto& key : plan.changes.addRegistryExclusions) {
      attempted = true;
      next->addExclusion(key);
    }
  });
}
std::shared_ptr<UwfRegistryMaintenanceTarget> DurableFileStagingStore::prepareCurrentCommit(const CommitRequirement requirement) const {
  if (requirement == CommitRequirement::AlreadyPersistent) return {};
  const auto current = m_repository->prepareRegistryMaintenance();
  if (!current) throw std::runtime_error("file staging durability requires a current UWF registry filter record");
  // Complete admission and exclusion reads before creating a potentially volatile list.
  const auto exclusions = current->exclusions();
  // Direct erase also removes the parent metadata key, so list-key coverage
  // alone cannot establish durability for every mutation in this store.
  const auto currentControlSet = [&] { return current->currentControlSet(); };
  if (registryExclusionCoverage(exclusions, QString::fromStdString(std::string(kFileStagingRegistryRoot)), currentControlSet).writesBypass()) return nullptr;
  const auto listCoverage = registryExclusionCoverage(exclusions, QString::fromStdString(std::string(kFileStagingRegistryKey)), currentControlSet);
  if (listCoverage.commitSkipped() && !listCoverage.writesBypass())
    throw OperationError(std::make_error_code(std::errc::permission_denied),
                         ApplicationText::tr("UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and "
                                             "restart before applying this change.")
                             .toStdString());
  return current;
}

}  // namespace uwf::app
