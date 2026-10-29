// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RegistryTreeCommitter.h"

#include <exception>
#include <stdexcept>
#include <utility>

#include "app/ports/RegistryKey.h"
#include "shared/Log.h"

namespace uwf::app {

namespace {
provider::RegistryFilterRow readBatchFilter(const provider::UwfRegistryFilter& registry, const char* operation) {
  try {
    return registry.read(provider::Session::Current);
  } catch (const std::exception& error) {
    UWF_LOG_E("registry") << "registry commit batch read failed: operation=" << operation << " error=" << error.what();
    throw;
  } catch (...) {
    UWF_LOG_E("registry") << "registry commit batch read failed: operation=" << operation << " error=non-standard-exception";
    throw;
  }
}

void recordFailure(RegistryCommitResult& result, const RegistryCommitTarget& target, const char* operation, std::string detail) {
  result.failures.push_back({target, std::move(detail)});
  // Record each failure before callers summarize the result or reuse the batch.
  try {
    const auto& failure = result.failures.back();
    UWF_LOG_E("registry") << "registry commit item failed: operation=" << operation << " key=" << target.key << " value=" << target.valueName
                          << " error=" << failure.detail;
  } catch (...) {
  }
}
}  // namespace

RegistryTreeCommitter::RegistryTreeCommitter(wmi::WmiOperations& session) : m_registry(session) {}

RegistryCommitResult RegistryTreeCommitter::commit(const std::vector<RegistryCommitTarget>& targets) const {
  RegistryCommitResult result;
  result.attempted = targets.size();
  if (targets.empty()) return result;

  const auto row = readBatchFilter(m_registry, "commit");
  for (const auto& target : targets) {
    try {
      m_registry.commitRegistry(row, target.key, target.valueName);
      ++result.committed;
    } catch (const std::exception& error) {
      recordFailure(result, target, "commit", error.what());
    } catch (...) {
      recordFailure(result, target, "commit", "non-standard exception");
    }
  }
  return result;
}

RegistryCommitResult RegistryTreeCommitter::commitDeletion(const std::vector<RegistryCommitTarget>& targets) const {
  RegistryCommitResult result;
  result.attempted = targets.size();
  std::vector<const RegistryCommitTarget*> existingTargets;
  existingTargets.reserve(targets.size());

  for (const auto& target : targets) {
    try {
      const bool exists = target.valueName.empty() ? regkey::keyExists(target.key) : regkey::valueExists(target.key, target.valueName);
      if (!exists) {
        ++result.skipped;
        continue;
      }
      existingTargets.push_back(&target);
    } catch (const std::exception& error) {
      recordFailure(result, target, "inspect-deletion", error.what());
    } catch (...) {
      recordFailure(result, target, "inspect-deletion", "non-standard exception");
    }
  }

  if (existingTargets.empty()) return result;

  const auto row = readBatchFilter(m_registry, "commit-deletion");
  for (const auto* target : existingTargets) {
    try {
      m_registry.commitRegistryDeletion(row, target->key, target->valueName);
      ++result.committed;
    } catch (const std::exception& error) {
      recordFailure(result, *target, "commit-deletion", error.what());
    } catch (...) {
      recordFailure(result, *target, "commit-deletion", "non-standard exception");
    }
  }
  return result;
}

}  // namespace uwf::app
