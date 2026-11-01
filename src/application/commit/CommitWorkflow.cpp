// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/commit/CommitWorkflow.h"

#include <QDir>
#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>

#include "application/commit/CommitPlanner.h"
#include "application/commit/CommitTarget.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/localization/ApplicationText.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/policies/RegistryCommitPolicy.h"

namespace uwf::app {
namespace {
class CommitAdmissionSource final : public CommitTraversalSource {
 public:
  CommitAdmissionSource(const CommitTraversalSource& source, const std::vector<std::string>& exclusions, bool registry, bool alreadyPersistent = false)
      : m_source(source), m_exclusions(exclusions), m_registry(registry), m_alreadyPersistent(alreadyPersistent) {}
  std::optional<CommitNode> inspect(const CommitNode& node) const override {
    const auto selected = m_registry || node.fileTarget ? node : bindFileCommitTarget(m_source, node);
    if (!m_registry) validateFileCommitTarget(*selected.fileTarget);
    if (m_alreadyPersistent) {
      auto observed = m_source.inspect(selected);
      if (observed && observed->skippedReason.isEmpty())
        observed->skippedReason = m_registry ? ApplicationText::tr("Registry changes are already persistent; no UWF commit is required.")
                                             : ApplicationText::tr("File changes are already persistent; no UWF commit is required.");
      return observed;
    }
    if (m_registry && !core::preservesRegistryCommitTarget(node.path.toStdString()))
      throw std::runtime_error(ApplicationText::tr("UWF cannot preserve the exact registry target for this key.").toStdString());
    const auto hit =
        m_registry ? registryCommitSkipExclusion(m_exclusions, node.path) : fileCommitSkipExclusion(m_exclusions, selected.fileTarget->relativePath);
    if (hit.isEmpty()) {
      auto observed = m_source.inspect(selected);
      if (observed && !m_registry) observed->fileTarget = selected.fileTarget;
      return observed;
    }
    auto skipped = selected;
    skipped.skippedReason = (m_registry ? ApplicationText::tr("This key is in the registry exclusion list.\nExclusion: %1")
                                        : ApplicationText::tr("This path is in the file exclusion list.\nExclusion: %1"))
                                .arg(hit);
    return skipped;
  }
  QString enumerateChildren(const CommitNode& node, std::stop_token stop, const CommitNodeVisitor& discover) const override {
    return m_source.enumerateChildren(node, stop, discover);
  }
  bool exists(const CommitNode& node) const override { return m_source.exists(node); }
  std::string driveLetterForPath(const QString& path) const override { return m_source.driveLetterForPath(path); }
  FileCommitTarget resolveFileTarget(const QString& path) const override { return m_source.resolveFileTarget(path); }

 private:
  const CommitTraversalSource& m_source;
  std::vector<std::string> m_exclusions;
  bool m_registry;
  bool m_alreadyPersistent;
};
class PreparedCommitBackend final : public CommitBackend {
 public:
  PreparedCommitBackend(std::shared_ptr<UwfMaintenanceRepository> repository, std::shared_ptr<const CommitTraversalSource> source)
      : m_repository(std::move(repository)), m_source(std::move(source)) {}
  CommitPlan prepare(const CommitRequest& request, CommitPlan previous, const std::stop_token stop, const CommitPreparationProgress& progress) override {
    if (stop.stop_requested()) {
      previous.canceled = true;
      return previous;
    }
    validateCommitNodeText(request.target);
    const bool registry = request.target.kind == CommitTargetKind::RegistryKey || request.target.kind == CommitTargetKind::RegistryValue;
    if (registry && request.operation == CommitOperation::DeleteAndPersist && request.target.kind == CommitTargetKind::RegistryValue &&
        request.target.valueName.isEmpty())
      throw OperationError(std::make_error_code(std::errc::operation_not_supported), "UWF cannot preserve an exact default-value deletion target");
    auto admittedRequest = request;
    // Re-traversal changes discovery intent, never the physical selection that
    // was already reviewed. Resolve the display path against its prior binding.
    if (!registry && previous.request.target.fileTarget) admittedRequest.target.fileTarget = previous.request.target.fileTarget;
    if (registry) {
      if (m_repository->registryCommitRequirement() == CommitRequirement::AlreadyPersistent) {
        if (request.operation == CommitOperation::DeleteAndPersist)
          throw OperationError(std::make_error_code(std::errc::device_or_resource_busy),
                               ApplicationText::tr("Registry changes are already persistent; UWF cannot perform this deletion.").toStdString());
        const CommitAdmissionSource source(*m_source, {}, true, true);
        return prepareCommit(request, source, std::move(previous), stop, progress);
      }
    }
    if (registry) {
      const auto row = m_repository->prepareRegistryMaintenance();
      if (!row) throw std::runtime_error(ApplicationText::tr("No current-session registry filter record found.").toStdString());
      m_registry = row;
      m_registryExclusions = row->exclusions();
      const auto skipped = registryCommitSkipExclusion(m_registryExclusions, request.target.path);
      if (!skipped.isEmpty())
        throw std::runtime_error(ApplicationText::tr("This key is in the registry exclusion list.\nExclusion: %1").arg(skipped).toStdString());
    } else {
      const auto selected = bindFileCommitTarget(*m_source, admittedRequest.target);
      admittedRequest.target = selected;
      const auto& volumeId = selected.fileTarget->volumeId;
      if (m_repository->fileCommitRequirement(volumeId) == CommitRequirement::AlreadyPersistent) {
        if (request.operation == CommitOperation::DeleteAndPersist)
          throw OperationError(std::make_error_code(std::errc::device_or_resource_busy),
                               ApplicationText::tr("File changes are already persistent; UWF cannot perform this deletion.").toStdString());
        const CommitAdmissionSource source(*m_source, {}, false, true);
        auto boundRequest = request;
        boundRequest.target = selected;
        return prepareCommit(boundRequest, source, std::move(previous), stop, progress);
      }
      const auto rows = m_repository->prepareVolumeTargets();
      const auto found = std::ranges::find_if(rows, [&](const auto& target) { return compat::normalizeVolumeId(target->volume().volumeName) == volumeId; });
      const auto row = found == rows.end() ? nullptr : *found;
      if (!row)
        throw std::runtime_error(ApplicationText::tr("No current-session record found for volume %1.").arg(QString::fromStdString(volumeId)).toStdString());
      m_volume = row;
      m_fileExclusions = row->exclusions();
      {
        const auto skipped = fileCommitSkipExclusion(m_fileExclusions, selected.fileTarget->relativePath);
        if (!skipped.isEmpty())
          throw std::runtime_error(ApplicationText::tr("This path is in the file exclusion list.\nExclusion: %1").arg(skipped).toStdString());
      }
    }
    const std::vector<std::string> noExclusions;
    const auto& exclusions = registry ? m_registryExclusions : m_volume ? m_fileExclusions : noExclusions;
    CommitAdmissionSource source(*m_source, exclusions, registry);
    auto plan = prepareCommit(admittedRequest, source, std::move(previous), stop, progress);
    m_knownValues.clear();
    for (const auto& record : std::as_const(plan.records))
      if (record.status == TraversalStatus::Succeeded && record.node.kind == CommitTargetKind::RegistryValue)
        m_knownValues[record.node.path].insert(record.node.valueName);
    return plan;
  }
  CommitTargetResult execute(const CommitNode& node, const CommitOperation operation) override {
    validateCommitNodeText(node);
    auto commit = [&](const CommitNode& target) {
      if (target.kind == CommitTargetKind::File || target.kind == CommitTargetKind::Folder) {
        const auto bound = bindFileCommitTarget(*m_source, target);
        if (!m_volume || compat::normalizeVolumeId(m_volume->volume().volumeName) != bound.fileTarget->volumeId)
          throw OperationError(std::make_error_code(std::errc::device_or_resource_busy), "The prepared UWF volume does not match the selected file");
        const auto observed = m_source->inspect(bound);
        if (!observed || observed->kind != target.kind || !observed->skippedReason.isEmpty())
          throw std::runtime_error(
              ApplicationText::tr("The selected path does not exist or is not the expected file type:\n%1").arg(target.path).toStdString());
        if (operation == CommitOperation::Persist)
          m_volume->commitFile(bound.fileTarget->relativePath.toStdString());
        else {
          // Do not recursively remove content that appeared after preparation.
          verifyDeletionContents(bound);
          m_volume->commitFileDeletion(bound.fileTarget->relativePath.toStdString());
        }
      } else {
        if (operation == CommitOperation::Persist)
          m_registry->commit(target.path.toStdString(), target.valueName.toStdString());
        else {
          verifyDeletionContents(target);
          m_registry->commitDeletion(target.path.toStdString(),
                                     target.kind == CommitTargetKind::RegistryKey ? std::nullopt : std::optional<std::string>{target.valueName.toStdString()});
        }
      }
    };
    if (operation == CommitOperation::Persist) {
      decltype(nullptr) noProbe = nullptr;
      return executeCommitTarget(node, node.display(), operation, commit, noProbe);
    }
    auto exists = [this](const CommitNode& target) { return m_source->exists(target); };
    return executeCommitTarget(node, node.display(), operation, commit, exists);
  }

 private:
  void verifyDeletionContents(const CommitNode& target) const {
    if (target.kind != CommitTargetKind::Folder && target.kind != CommitTargetKind::RegistryKey) return;
    QString violation;
    const auto enumerationError = m_source->enumerateChildren(target, {}, [&](const CommitNode& child) {
      if (target.kind == CommitTargetKind::Folder)
        violation = ApplicationText::tr("The folder is not empty; unprocessed content has been retained.");
      else if (child.kind == CommitTargetKind::RegistryKey)
        violation = ApplicationText::tr("The registry key still has subkeys; unprocessed content has been retained.");
      else {
        const auto known = m_knownValues.find(target.path);
        if (known == m_knownValues.end() || !known->second.contains(child.valueName))
          violation = ApplicationText::tr("The registry key has new values; unprocessed content has been retained.");
      }
    });
    if (!enumerationError.isEmpty()) throw std::runtime_error(enumerationError.toStdString());
    if (!violation.isEmpty()) throw std::runtime_error(violation.toStdString());
  }
  std::shared_ptr<UwfMaintenanceRepository> m_repository;
  std::vector<std::string> m_registryExclusions;
  std::shared_ptr<const CommitTraversalSource> m_source;
  std::shared_ptr<UwfVolumeTarget> m_volume;
  std::shared_ptr<UwfRegistryMaintenanceTarget> m_registry;
  std::vector<std::string> m_fileExclusions;
  std::map<QString, std::set<QString, ObjectNameLess>, ObjectNameLess> m_knownValues;
};
}  // namespace
CommitWorkflow::CommitWorkflow(std::shared_ptr<UwfMaintenanceRepository> repository, CommitInteraction interaction,
                               std::shared_ptr<const CommitTraversalSource> source)
    : m_repository(repository), m_interaction(std::move(interaction)), m_source(std::move(source)) {
  if (!m_repository || !m_source || !m_interaction.run) throw std::invalid_argument("commit workflow requires a repository, source and interaction");
}
void CommitWorkflow::run(const CommitOperation operation, const CommitTargetKind kind, const QString& path, const QString& valueName) {
  if (path.isEmpty()) return;
  const bool registry = kind == CommitTargetKind::RegistryKey || kind == CommitTargetKind::RegistryValue;
  const QString target = registry ? paths::expandRegistryHive(path) : QDir::toNativeSeparators(path);
  m_interaction.run({operation, {kind, target, valueName, {}}}, std::make_shared<PreparedCommitBackend>(m_repository, m_source));
}
void CommitWorkflow::commitFilePath(const QString& path) { run(CommitOperation::Persist, CommitTargetKind::File, path); }
void CommitWorkflow::commitFileDeletionPath(const QString& path) { run(CommitOperation::DeleteAndPersist, CommitTargetKind::File, path); }
void CommitWorkflow::commitRegistryKey(const QString& key, const std::optional<QString>& valueName) {
  run(CommitOperation::Persist, valueName ? CommitTargetKind::RegistryValue : CommitTargetKind::RegistryKey, key, valueName.value_or(QString{}));
}
void CommitWorkflow::commitRegistryDeletionKey(const QString& key, const std::optional<QString>& valueName) {
  run(CommitOperation::DeleteAndPersist, valueName ? CommitTargetKind::RegistryValue : CommitTargetKind::RegistryKey, key, valueName.value_or(QString{}));
}
}  // namespace uwf::app
