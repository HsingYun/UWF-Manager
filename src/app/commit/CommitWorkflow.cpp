// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitWorkflow.h"

#include <QDir>
#include <QHash>
#include <QSet>
#include <stdexcept>
#include <utility>

#include "CommitPreparation.h"
#include "CommitTarget.h"
#include "app/common/ApplicationText.h"
#include "app/paths/PathExclusionIndex.h"
#include "app/paths/PathPolicy.h"
#include "app/uwf/UwfRegistryFilter.h"
#include "app/uwf/UwfVolume.h"
#include "shared/RegistryPath.h"

namespace uwf::app {
namespace {
class ExclusionAwareSource final : public CommitTraversalSource {
 public:
  ExclusionAwareSource(const CommitTraversalSource& source, const std::vector<std::string>& exclusions, bool registry)
      : m_source(source), m_exclusions(exclusions), m_registry(registry) {}
  std::optional<CommitNode> inspect(const CommitNode& node) const override {
    const auto hit = m_exclusions.covering(node.path);
    if (hit.isEmpty()) return m_source.inspect(node);
    auto skipped = node;
    skipped.skippedReason = (m_registry ? ApplicationText::tr("This key is in the registry exclusion list.\nExclusion: %1")
                                        : ApplicationText::tr("This path is in the file exclusion list.\nExclusion: %1"))
                                .arg(hit);
    return skipped;
  }
  QString read(const CommitNode& node, std::stop_token stop, const CommitNodeVisitor& discover) const override { return m_source.read(node, stop, discover); }
  bool exists(const CommitNode& node) const override { return m_source.exists(node); }
  std::string volumeForPath(const QString& path) const override { return m_source.volumeForPath(path); }
  std::vector<std::string> mountedLetters(const std::string& volume) const override { return m_source.mountedLetters(volume); }

 private:
  const CommitTraversalSource& m_source;
  paths::PathExclusionIndex m_exclusions;
  bool m_registry;
};
class PreparedCommitBackend final : public CommitBackend {
 public:
  PreparedCommitBackend(std::function<wmi::WmiOperations&()> session, core::UwfSnapshot snapshot, std::shared_ptr<const CommitTraversalSource> source)
      : m_session(std::move(session)), m_snapshot(std::move(snapshot)), m_source(std::move(source)) {}
  CommitPlan prepare(const CommitRequest& request, CommitPlan previous, const std::stop_token stop, const CommitPreparationProgress& progress) override {
    if (stop.stop_requested()) {
      previous.canceled = true;
      return previous;
    }
    auto& session = m_session();
    const bool registry = request.target.kind == CommitTargetKind::RegistryKey || request.target.kind == CommitTargetKind::RegistryValue;
    if (registry) {
      const auto hit = paths::findCoveringExclusion(m_snapshot.current.registryExclusions, request.target.path.toStdString());
      if (!hit.empty())
        throw std::runtime_error(
            ApplicationText::tr("This key is in the registry exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)).toStdString());
      const auto rows = provider::UwfRegistryFilter(session).readAll();
      const auto* row = provider::findBySession(rows, provider::Session::Current);
      if (!row) throw std::runtime_error(ApplicationText::tr("No current-session registry filter record found.").toStdString());
      m_registry = *row;
    } else {
      const auto driveLetter = m_source->volumeForPath(request.target.path);
      if (driveLetter.empty()) throw std::runtime_error(ApplicationText::tr("The path has no drive letter; cannot identify the target volume.").toStdString());
      const auto rows = provider::UwfVolume(session, [this](const std::string& volume) { return m_source->mountedLetters(volume); }).readAll();
      const auto* row =
          provider::findBySession(rows, provider::Session::Current, [&](const provider::VolumeRow& volume) { return volume.driveLetter == driveLetter; });
      if (!row)
        throw std::runtime_error(ApplicationText::tr("No current-session record found for volume %1.").arg(QString::fromStdString(driveLetter)).toStdString());
      m_volume = *row;
      const auto excluded = m_snapshot.current.fileExclusions.find(row->volumeName);
      if (excluded != m_snapshot.current.fileExclusions.end()) {
        const auto hit = paths::findCoveringExclusion(excluded->second, request.target.path.toStdString());
        if (!hit.empty())
          throw std::runtime_error(
              ApplicationText::tr("This path is in the file exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)).toStdString());
      }
    }
    const std::vector<std::string> noExclusions;
    const auto files = m_volume ? m_snapshot.current.fileExclusions.find(m_volume->volumeName) : m_snapshot.current.fileExclusions.end();
    const auto& exclusions = registry ? m_snapshot.current.registryExclusions : files != m_snapshot.current.fileExclusions.end() ? files->second : noExclusions;
    ExclusionAwareSource source(*m_source, exclusions, registry);
    auto plan = prepareCommit(request, source, std::move(previous), stop, progress);
    m_knownValues.clear();
    for (const auto& record : std::as_const(plan.records))
      if (record.status == TraversalStatus::Succeeded && record.node.kind == CommitTargetKind::RegistryValue)
        m_knownValues[record.node.path.toCaseFolded()].insert(record.node.valueName.toCaseFolded());
    return plan;
  }
  CommitTargetResult execute(const CommitNode& node, const CommitOperation operation) override {
    auto commit = [&](const CommitNode& target) {
      auto& session = m_session();
      if (target.kind == CommitTargetKind::File || target.kind == CommitTargetKind::Folder) {
        const auto observed = m_source->inspect(target);
        if (!observed || observed->kind != target.kind || !observed->skippedReason.isEmpty())
          throw std::runtime_error(
              ApplicationText::tr("The selected path does not exist or is not the expected file type:\n%1").arg(target.path).toStdString());
        provider::UwfVolume volume(session, [this](const std::string& name) { return m_source->mountedLetters(name); });
        if (operation == CommitOperation::Persist)
          volume.commitFile(*m_volume, target.path.toStdString());
        else {
          // Do not recursively remove content that appeared after preparation.
          verifyDeletionContents(target);
          volume.commitFileDeletion(*m_volume, target.path.toStdString());
        }
      } else {
        provider::UwfRegistryFilter registry(session);
        if (operation == CommitOperation::Persist)
          registry.commitRegistry(*m_registry, target.path.toStdString(), target.valueName.toStdString());
        else {
          verifyDeletionContents(target);
          registry.commitRegistryDeletion(*m_registry, target.path.toStdString(), target.valueName.toStdString());
        }
      }
    };
    if (operation == CommitOperation::Persist) {
      decltype(nullptr) noProbe = nullptr;
      return executeCommitTarget(node, node.display(), commit, noProbe);
    }
    auto exists = [this](const CommitNode& target) { return m_source->exists(target); };
    return executeCommitTarget(node, node.display(), commit, exists);
  }

 private:
  void verifyDeletionContents(const CommitNode& target) const {
    if (target.kind != CommitTargetKind::Folder && target.kind != CommitTargetKind::RegistryKey) return;
    QString violation;
    const auto error = m_source->read(target, {}, [&](const CommitNode& child) {
      if (target.kind == CommitTargetKind::Folder)
        violation = ApplicationText::tr("The folder is not empty; unprocessed content has been retained.");
      else if (child.kind == CommitTargetKind::RegistryKey)
        violation = ApplicationText::tr("The registry key still has subkeys; unprocessed content has been retained.");
      else if (!m_knownValues.value(target.path.toCaseFolded()).contains(child.valueName.toCaseFolded()))
        violation = ApplicationText::tr("The registry key has new values; unprocessed content has been retained.");
    });
    if (!error.isEmpty()) throw std::runtime_error(error.toStdString());
    if (!violation.isEmpty()) throw std::runtime_error(violation.toStdString());
  }
  std::function<wmi::WmiOperations&()> m_session;
  core::UwfSnapshot m_snapshot;
  std::shared_ptr<const CommitTraversalSource> m_source;
  std::optional<provider::VolumeRow> m_volume;
  std::optional<provider::RegistryFilterRow> m_registry;
  QHash<QString, QSet<QString>> m_knownValues;
};
}  // namespace
CommitWorkflow::CommitWorkflow(wmi::WmiOperations& session, const core::UwfSnapshot& snapshot, CommitInteraction interaction,
                               std::shared_ptr<const CommitTraversalSource> source)
    : m_session(session.threadAccessor()), m_snapshot(snapshot), m_interaction(std::move(interaction)), m_source(std::move(source)) {
  if (!m_source || !m_interaction.run) throw std::invalid_argument("commit workflow requires a source and an interaction");
}
void CommitWorkflow::run(const CommitOperation operation, const CommitTargetKind kind, const QString& path, const QString& valueName) {
  if (path.isEmpty()) return;
  const bool registry = kind == CommitTargetKind::RegistryKey || kind == CommitTargetKind::RegistryValue;
  const QString normalized = registry ? QString::fromStdString(regkey::normalize(path.toStdString())) : QDir::toNativeSeparators(path);
  if (normalized.isEmpty()) return;
  m_interaction.run({operation, {kind, normalized, valueName, {}}}, std::make_shared<PreparedCommitBackend>(m_session, m_snapshot, m_source));
}
void CommitWorkflow::commitFilePath(const QString& path) { run(CommitOperation::Persist, CommitTargetKind::File, path); }
void CommitWorkflow::commitFileDeletionPath(const QString& path) { run(CommitOperation::DeleteAndPersist, CommitTargetKind::File, path); }
void CommitWorkflow::commitRegistryKey(const QString& key, const QString& valueName) {
  run(CommitOperation::Persist, valueName.isEmpty() ? CommitTargetKind::RegistryKey : CommitTargetKind::RegistryValue, key, valueName);
}
void CommitWorkflow::commitRegistryDeletionKey(const QString& key, const QString& valueName) {
  run(CommitOperation::DeleteAndPersist, valueName.isEmpty() ? CommitTargetKind::RegistryKey : CommitTargetKind::RegistryValue, key, valueName);
}
}  // namespace uwf::app
