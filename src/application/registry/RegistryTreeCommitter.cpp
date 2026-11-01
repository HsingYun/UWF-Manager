// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/registry/RegistryTreeCommitter.h"

#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

#include "application/localization/ApplicationText.h"
#include "application/paths/ExclusionCoverage.h"
#include "foundation/logging/Log.h"
#include "foundation/text/Utf8.h"

namespace uwf::app {

namespace {
enum class RegistryOperation { Commit, PersistDeletion };

const char* operationName(const RegistryOperation operation) { return operation == RegistryOperation::Commit ? "commit" : "commit-deletion"; }

struct RegistryBatch {
  std::shared_ptr<UwfRegistryMaintenanceTarget> target;
  std::vector<std::string> exclusions;
};
RegistryBatch prepareRegistryBatch(const std::shared_ptr<UwfMaintenanceRepository>& registry, const char* operation) {
  try {
    if (registry->registryCommitRequirement() == CommitRequirement::AlreadyPersistent) return {};
    auto target = registry->prepareRegistryMaintenance();
    if (!target) throw std::runtime_error("current UWF registry session is unavailable");
    auto exclusions = target->exclusions();
    return {std::move(target), std::move(exclusions)};
  } catch (const std::exception& error) {
    UWF_LOG_E("registry") << "registry commit batch read failed: operation=" << operation << " error=" << error.what();
    throw;
  } catch (...) {
    UWF_LOG_E("registry") << "registry commit batch read failed: operation=" << operation << " error=non-standard-exception";
    throw;
  }
}

void recordFailure(RegistryCommitResult& result, const RegistryCommitTarget& target, const char* operation, std::string detail,
                   OperationFailure operationFailure = {OperationFailureKind::Failed, WriteEffect::Uncertain, {}}) {
  result.failures.push_back({target, std::move(detail), operationFailure});
  // Record each failure before callers summarize the result or reuse the batch.
  try {
    const auto& failure = result.failures.back();
    UWF_LOG_E("registry") << "registry commit item failed: operation=" << operation << " key=" << target.key << " value=" << target.valueName
                          << " error=" << failure.detail << " target_kind=" << (target.isKey() ? "Key" : "Value");
  } catch (...) {
  }
}
// Validate the finite application plan as a whole before observing or writing
// any item. A later malformed selector cannot follow an earlier deletion.
bool validateTargets(RegistryCommitResult& result, const std::vector<RegistryCommitTarget>& targets, const RegistryOperation operation) {
  for (const auto& target : targets) {
    try {
      const auto validText = [](const std::string& text) { return text.find('\0') == std::string::npos && utf8ToUtf16(text).has_value(); };
      if (target.key.empty() || !validText(target.key) || !validText(target.valueName))
        throw std::invalid_argument(ApplicationText::tr("A parameter was rejected by the system (invalid path or argument).").toStdString());
      if (operation == RegistryOperation::PersistDeletion)
        (void)target.deletionValueName();
      else if (target.kind != RegistryTargetKind::Value)
        throw std::invalid_argument("Ordinary registry commit requires an exact value target");
    } catch (const std::invalid_argument& error) {
      recordFailure(result, target, operationName(operation), error.what(), {OperationFailureKind::InvalidArgument, WriteEffect::None, {}});
    }
  }
  return result.failures.empty();
}
RegistryCommitResult executeRegistryBatch(const std::shared_ptr<UwfMaintenanceRepository>& registry, const std::vector<RegistryCommitTarget>& targets,
                                          const RegistryOperation operation) {
  RegistryCommitResult result;
  result.attempted = targets.size();
  if (targets.empty() || !validateTargets(result, targets, operation)) return result;

  const auto* name = operationName(operation);
  const auto [row, exclusions] = prepareRegistryBatch(registry, name);
  if (!row) {
    result.committed = targets.size();
    return result;
  }
  std::optional<std::uint32_t> selectedControlSet;
  const auto currentControlSet = [&] {
    if (!selectedControlSet) selectedControlSet = row->currentControlSet();
    return *selectedControlSet;
  };
  // Keep the caller's order, including the postorder plan for deletions. An
  // absent overlay value cannot prove that the persistent hive is up to date.
  for (const auto& target : targets) {
    bool dispatched = false;
    try {
      const auto coverage = registryExclusionCoverage(exclusions, QString::fromStdString(target.key), currentControlSet);
      // A prepared exclusion is not proof of durability after live mode changes.
      (void)registry->registryCommitRequirement();
      if (!coverage.writesBypass()) {
        if (coverage.commitSkipped()) throw std::runtime_error(coverage.unreliableCommitReason().toStdString());
        WriteEffect effect;
        if (operation == RegistryOperation::PersistDeletion) {
          const auto valueName = target.deletionValueName();
          dispatched = true;
          effect = row->commitDeletion(target.key, valueName);
        } else {
          dispatched = true;
          effect = row->commit(target.key, target.valueName);
        }
        result.effect = combineWriteEffects(result.effect, effect);
      }
      ++result.committed;
    } catch (const std::exception& error) {
      recordFailure(result, target, name, error.what(), operationFailure(error, dispatched ? WriteEffect::Uncertain : WriteEffect::None));
    } catch (...) {
      recordFailure(result, target, name, "non-standard exception",
                    {OperationFailureKind::Failed, dispatched ? WriteEffect::Uncertain : WriteEffect::None, {}});
    }
  }
  return result;
}
}  // namespace

RegistryTreeCommitter::RegistryTreeCommitter(std::shared_ptr<UwfMaintenanceRepository> repository) : m_registry(std::move(repository)) {
  if (!m_registry) throw std::invalid_argument("registry commits require a repository");
}

RegistryCommitResult RegistryTreeCommitter::commit(const std::vector<RegistryCommitTarget>& targets) const {
  return executeRegistryBatch(m_registry, targets, RegistryOperation::Commit);
}

RegistryCommitResult RegistryTreeCommitter::persistDeletions(const std::vector<RegistryCommitTarget>& targets) const {
  return executeRegistryBatch(m_registry, targets, RegistryOperation::PersistDeletion);
}

}  // namespace uwf::app
