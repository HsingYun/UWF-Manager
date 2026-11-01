// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/enhanced/EnhancedModeOperations.h"

#include <algorithm>
#include <exception>
#include <utility>

#include "application/localization/ApplicationText.h"
#include "application/registry/RegistryTreeCommitter.h"
#include "foundation/logging/Log.h"

namespace uwf::app {

namespace {
QString summarizeFailures(const RegistryCommitResult& result) {
  if (result.succeeded()) return {};
  QString detail = ApplicationText::tr("%1 registry operation(s) failed.").arg(result.failures.size());
  constexpr std::size_t kVisible = 4;
  for (std::size_t index = 0; index < std::min(kVisible, result.failures.size()); ++index) {
    const auto& failure = result.failures[index];
    detail += QStringLiteral("\n%1: %2").arg(QString::fromStdString(failure.target.key), QString::fromStdString(failure.detail));
  }
  return detail;
}

}  // namespace

EnhancedModeOperations::EnhancedModeOperations(std::shared_ptr<EnhancedModeServiceControl> control, std::shared_ptr<UwfMaintenanceRepository> repository,
                                               const UwfCapability capability)
    : m_serviceControl(std::move(control)), m_repository(repository), m_capability(capability) {
  if (!m_serviceControl || !m_repository) throw std::invalid_argument("enhanced mode requires service control and a repository");
}

EnhancedModeStatus EnhancedModeOperations::status() const { return m_serviceControl->query(); }

EnhancedModeChangeResult EnhancedModeOperations::enable(const QString& serviceDescription) {
  const bool requiresCommit = registryWriteRequiresCommit();
  m_serviceControl->installAndStart(serviceDescription);
  EnhancedModeChangeResult result;
  persistConfiguration(requiresCommit, result);
  result.status = status();
  return result;
}

EnhancedModeChangeResult EnhancedModeOperations::start() {
  m_serviceControl->start();
  return {status(), {}};
}

EnhancedModeChangeResult EnhancedModeOperations::disable() {
  const bool requiresCommit = registryWriteRequiresCommit();
  m_serviceControl->disable();
  EnhancedModeChangeResult result;
  persistConfiguration(requiresCommit, result);
  result.status = status();
  return result;
}

EnhancedModeChangeResult EnhancedModeOperations::remove() {
  auto persistencePlan = prepareRemovalPersistence();
  // Registry-plan reads can outlive admission; reject a newly active protected
  // ROM/HORM mode before stopping the service or deleting its host registry key.
  if (!registryWriteRequiresCommit()) persistencePlan.reset();
  auto removal = m_serviceControl->stopAndMarkForDeletion();
  EnhancedModeChangeResult result;
  bool registryRemovalReady = true;
  if (removal.state == EnhancedModeServiceControl::RemovalState::Absent) {
    try {
      // Delete remnants only after SCM confirms absence. A service marked for
      // deletion still owns its registry key until SCM completes removal.
      if (!registryWriteRequiresCommit()) persistencePlan.reset();
      m_serviceControl->deleteRegistryRemnants();
    } catch (const std::exception& error) {
      registryRemovalReady = false;
      UWF_LOG_E("enhanced") << "enhanced mode registry remnant cleanup failed: error=" << error.what();
      result.persistenceWarning = QString::fromUtf8(error.what());
      result.persistenceFailure = operationFailure(error);
    } catch (...) {
      registryRemovalReady = false;
      UWF_LOG_E("enhanced") << "enhanced mode registry remnant cleanup failed: error=non-standard-exception";
      result.persistenceWarning = ApplicationText::tr("The operation failed with an unknown error.");
      result.persistenceFailure = OperationFailure{OperationFailureKind::Failed, WriteEffect::Uncertain, {}};
    }
  }
  if (persistencePlan && registryRemovalReady) persistRemoval(*persistencePlan, result);
  removal.barrier.reset();
  result.status = status();
  return result;
}

bool EnhancedModeOperations::registryWriteRequiresCommit() const {
  if (m_capability != UwfCapability::Available) return false;
  try {
    return m_repository->registryWriteRequirement() == CommitRequirement::RequiresUwfCommit;
  } catch (const std::exception& error) {
    UWF_LOG_E("enhanced") << "enhanced mode persistence preparation failed: error=" << error.what();
    throw;
  } catch (...) {
    UWF_LOG_E("enhanced") << "enhanced mode persistence preparation failed: error=non-standard-exception";
    throw;
  }
}

void EnhancedModeOperations::persistConfiguration(const bool requiresCommit, EnhancedModeChangeResult& result) const {
  if (!requiresCommit) return;
  bool registryOwnsFailure = false;
  try {
    RegistryTreeCommitter registry(m_repository);
    const auto targets = m_serviceControl->planRegistryCommit();
    registryOwnsFailure = true;
    result.persistence = registry.commit(targets);
    registryOwnsFailure = false;
    result.persistenceWarning = summarizeFailures(*result.persistence);
  } catch (const std::exception& error) {
    if (!registryOwnsFailure) UWF_LOG_E("enhanced") << "enhanced mode configuration persistence failed: error=" << error.what();
    result.persistenceWarning = QString::fromUtf8(error.what());
    result.persistenceFailure = operationFailure(error);
  } catch (...) {
    if (!registryOwnsFailure) UWF_LOG_E("enhanced") << "enhanced mode configuration persistence failed: error=non-standard-exception";
    result.persistenceWarning = ApplicationText::tr("The operation failed with an unknown error.");
    result.persistenceFailure = OperationFailure{OperationFailureKind::Failed, WriteEffect::Uncertain, {}};
  }
}

std::optional<std::vector<RegistryCommitTarget>> EnhancedModeOperations::prepareRemovalPersistence() const {
  if (!registryWriteRequiresCommit()) return std::nullopt;
  try {
    return m_serviceControl->planRegistryDeletion();
  } catch (const std::exception& error) {
    UWF_LOG_E("enhanced") << "enhanced mode removal persistence preparation failed: error=" << error.what();
    throw;
  } catch (...) {
    UWF_LOG_E("enhanced") << "enhanced mode removal persistence preparation failed: error=non-standard-exception";
    throw;
  }
}

void EnhancedModeOperations::persistRemoval(const std::vector<RegistryCommitTarget>& deletionPlan, EnhancedModeChangeResult& result) const {
  if (deletionPlan.empty()) return;
  bool registryOwnsFailure = false;
  try {
    RegistryTreeCommitter registry(m_repository);
    registryOwnsFailure = true;
    result.persistence = registry.persistDeletions(deletionPlan);
    registryOwnsFailure = false;
    result.persistenceWarning = summarizeFailures(*result.persistence);
  } catch (const std::exception& error) {
    if (!registryOwnsFailure) UWF_LOG_E("enhanced") << "enhanced mode removal persistence failed: error=" << error.what();
    result.persistenceWarning = QString::fromUtf8(error.what());
    result.persistenceFailure = operationFailure(error);
  } catch (...) {
    if (!registryOwnsFailure) UWF_LOG_E("enhanced") << "enhanced mode removal persistence failed: error=non-standard-exception";
    result.persistenceWarning = ApplicationText::tr("The operation failed with an unknown error.");
    result.persistenceFailure = OperationFailure{OperationFailureKind::Failed, WriteEffect::Uncertain, {}};
  }
}

}  // namespace uwf::app
