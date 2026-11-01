// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QStringList>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include "application/registry/contracts/RegistryCommitTypes.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "support/app/FileTargetFixture.h"
#include "support/uwf/FakeUwfBackend.h"

namespace uwf::test::application {

// Record application repository calls while delegating normal behavior to the production repository and simulated UWF.
class UwfMaintenanceFixture final {
 public:
  static constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
  struct State {
    struct RegistryDeletion {
      std::string key;
      std::optional<std::string> valueName;
    };
    std::string volumeId = UwfMaintenanceFixture::volumeId;
    std::function<app::FileCommitTarget(const QString&)> fileTargetResolver;
    QString fileMountPrefix;
    QString drive = QStringLiteral("C:");
    QStringList exclusions;
    bool configuredVolume = true;
    bool protectedVolume = true;
    bool filterEnabled = true;
    // This fixture explicitly models its selected volume as the system volume.
    // Tests for independent C:/D: protection arrange a different identity here.
    std::optional<std::string> systemVolumeId = std::string(volumeId);
    int filterReads = 0;
    int volumeReads = 0;
    int exclusionReads = 0;
    std::vector<std::string> commits;
    std::exception_ptr filterFailure;
    std::exception_ptr volumeFailure;
    std::exception_ptr exclusionFailure;
    std::exception_ptr commitFailure;
    std::function<void()> beforeFilter;
    std::function<void()> beforeVolumes;
    std::function<void()> beforeExclusions;
    std::function<void()> beforeCommit;
    int registryTargets = 0;
    std::vector<app::RegistryCommitTarget> registryCommits;
    std::vector<RegistryDeletion> registryDeletions;
    std::exception_ptr registryTargetFailure;
    std::exception_ptr registryCommitFailure;
    std::function<void()> beforeRegistryTarget;
    std::function<void()> beforeRegistryCommit;
  };
  const std::shared_ptr<State> state = std::make_shared<State>();
  const std::shared_ptr<FakeUwfBackend> backend = std::make_shared<FakeUwfBackend>();

  // Resolve real temporary paths before arranging the simulated UWF row/data.
  // Never replace failed physical resolution with the fixed memory-fixture ID.
  [[nodiscard]] app::FileCommitTarget useFileSystemTarget(const app::CommitTraversalSource& source, const QString& path) const {
    const auto target = source.resolveFileTarget(path);
    const auto oldIdentity = state->volumeId;
    state->volumeId = target.volumeId;
    state->drive = QString::fromStdString(source.driveLetterForPath(path));
    auto spelling = path;
    spelling.replace(u'/', u'\\');
    if (!spelling.endsWith(target.relativePath, Qt::CaseInsensitive)) throw std::logic_error("The fixture path does not match its resolved volume path");
    state->fileMountPrefix = spelling.first(spelling.size() - target.relativePath.size());
    // The borrowed source must outlive this fixture.
    state->fileTargetResolver = [&source](const QString& selected) { return source.resolveFileTarget(selected); };
    if (state->systemVolumeId == oldIdentity) state->systemVolumeId = state->volumeId;
    return target;
  }

  // Arrange pending data explicitly. Repository calls below never create or
  // repair their targets, including when a target disappears after traversal.
  void stageFile(const QString& path, const std::string& contents = "fixture") const {
    const auto target =
        state->fileTargetResolver
            ? state->fileTargetResolver(path)
            : app::FileCommitTarget{state->volumeId, QString::fromStdString(app::toUwfFilePath(path.toStdString(), state->drive.toStdString()))};
    if (target.volumeId != state->volumeId) throw std::logic_error("The staged fixture path moved to another volume");
    backend->seedFile(target.volumeId, target.relativePath.toStdString(), contents, FakeUwfBackend::Storage::Overlay);
  }
  void stageFiles(const QStringList& paths, const std::string& contents = "fixture") const {
    for (const auto& path : paths) stageFile(path, contents);
  }
  void stageRegistryValue(const std::string& key, const std::string& name, const std::string& contents = "fixture") const {
    backend->seedRegistryValue(key, name, contents, FakeUwfBackend::Storage::Overlay);
  }
  void stageRegistryValues(const std::vector<app::RegistryCommitTarget>& targets) const {
    for (const auto& target : targets) stageRegistryValue(target.key, target.valueName);
  }

  [[nodiscard]] std::shared_ptr<app::UwfMaintenanceRepository> repository() const {
    auto seeded = backend->snapshot();
    seeded.runtime.systemVolumeId = state->systemVolumeId;
    seeded.runtime.fileMaintenancePrefixes[state->volumeId] = state->drive.toStdString();
    seeded.current.filterEnabled = state->filterEnabled;
    seeded.current.volumes.clear();
    if (state->configuredVolume) {
      FakeUwfBackend::VolumeConfiguration volume;
      volume.volume = {state->volumeId, state->drive.toStdString(), state->protectedVolume, false, false};
      for (const auto& exclusion : std::as_const(state->exclusions)) {
        if (state->fileTargetResolver && exclusion.size() >= 3 && exclusion[1] == u':') {
          // Strip only the known mount prefix; the remaining exclusion pattern must retain its literal spelling.
          if (!exclusion.startsWith(state->fileMountPrefix, Qt::CaseInsensitive))
            throw std::logic_error("The fixture exclusion belongs to another volume mount");
          const auto relative = exclusion.mid(state->fileMountPrefix.size());
          if (!relative.isEmpty() && !relative.startsWith(u'\\')) throw std::logic_error("The fixture exclusion crosses the mount-prefix boundary");
          volume.fileExclusions.push_back(relative.isEmpty() ? "\\" : relative.toStdString());
        } else
          volume.fileExclusions.push_back(app::toUwfFilePath(exclusion.toStdString(), state->drive.toStdString()));
      }
      seeded.current.volumes.emplace(state->volumeId, std::move(volume));
    }
    seeded.next = seeded.current;
    backend->seed(std::move(seeded));
    return std::make_shared<Repository>(app::uwfMaintenanceRepository(backend), state);
  }

 private:
  static void check(const std::function<void()>& before, const std::exception_ptr& failure) {
    if (before) before();
    if (failure) std::rethrow_exception(failure);
  }
  class Target final : public app::UwfVolumeTarget {
   public:
    Target(std::shared_ptr<app::UwfVolumeTarget> target, std::shared_ptr<State> state) : m_target(std::move(target)), m_state(std::move(state)) {}
    const core::VolumeRecord& volume() const override { return m_target->volume(); }
    std::vector<std::string> exclusions() const override {
      ++m_state->exclusionReads;
      check(m_state->beforeExclusions, m_state->exclusionFailure);
      return m_target->exclusions();
    }
    app::WriteEffect commitFile(const std::string& path) const override {
      m_state->commits.push_back(path);
      check(m_state->beforeCommit, m_state->commitFailure);
      return m_target->commitFile(path);
    }
    app::WriteEffect commitFileDeletion(const std::string& path) const override { return m_target->commitFileDeletion(path); }

   private:
    std::shared_ptr<app::UwfVolumeTarget> m_target;
    std::shared_ptr<State> m_state;
  };
  class RegistryTarget final : public app::UwfRegistryMaintenanceTarget {
   public:
    RegistryTarget(std::shared_ptr<app::UwfRegistryMaintenanceTarget> target, std::shared_ptr<State> state)
        : m_target(std::move(target)), m_state(std::move(state)) {}
    std::vector<std::string> exclusions() const override { return m_target->exclusions(); }
    std::uint32_t currentControlSet() const override { return m_target->currentControlSet(); }
    app::WriteEffect commit(const std::string& key, const std::string& valueName) const override {
      m_state->registryCommits.push_back({key, valueName});
      check(m_state->beforeRegistryCommit, m_state->registryCommitFailure);
      return m_target->commit(key, valueName);
    }
    app::WriteEffect commitDeletion(const std::string& key, const std::optional<std::string>& valueName) const override {
      m_state->registryDeletions.push_back({key, valueName});
      check(m_state->beforeRegistryCommit, m_state->registryCommitFailure);
      return m_target->commitDeletion(key, valueName);
    }

   private:
    std::shared_ptr<app::UwfRegistryMaintenanceTarget> m_target;
    std::shared_ptr<State> m_state;
  };
  class Repository final : public app::UwfMaintenanceRepository {
   public:
    Repository(std::shared_ptr<app::UwfMaintenanceRepository> repository, std::shared_ptr<State> state)
        : m_repository(std::move(repository)), m_state(std::move(state)) {}
    app::CommitRequirement fileCommitRequirement(const std::string& volumeId) const override {
      ++m_state->filterReads;
      check(m_state->beforeFilter, m_state->filterFailure);
      return m_repository->fileCommitRequirement(volumeId);
    }
    app::CommitRequirement registryCommitRequirement() const override {
      ++m_state->filterReads;
      check(m_state->beforeFilter, m_state->filterFailure);
      return m_repository->registryCommitRequirement();
    }
    app::AutomaticCommitDisposition automaticFileCommitDisposition(const std::string& volumeId) const override {
      return m_repository->automaticFileCommitDisposition(volumeId);
    }
    void refreshAutomaticCommitState() const override { m_repository->refreshAutomaticCommitState(); }
    app::CommitRequirement registryWriteRequirement() const override {
      ++m_state->filterReads;
      check(m_state->beforeFilter, m_state->filterFailure);
      return m_repository->registryWriteRequirement();
    }
    std::vector<std::shared_ptr<app::UwfVolumeTarget>> prepareVolumeTargets() const override {
      ++m_state->volumeReads;
      check(m_state->beforeVolumes, m_state->volumeFailure);
      std::vector<std::shared_ptr<app::UwfVolumeTarget>> result;
      for (auto& target : m_repository->prepareVolumeTargets()) result.push_back(std::make_shared<Target>(std::move(target), m_state));
      return result;
    }
    std::shared_ptr<app::UwfRegistryMaintenanceTarget> prepareRegistryMaintenance() const override {
      ++m_state->registryTargets;
      check(m_state->beforeRegistryTarget, m_state->registryTargetFailure);
      auto target = m_repository->prepareRegistryMaintenance();
      return target ? std::make_shared<RegistryTarget>(std::move(target), m_state) : nullptr;
    }
    std::shared_ptr<app::UwfRegistryExclusionConfiguration> prepareRegistryExclusions() const override { return m_repository->prepareRegistryExclusions(); }
    std::function<void()> preparePowerAction(const app::PowerAction action) const override { return m_repository->preparePowerAction(action); }

   private:
    std::shared_ptr<app::UwfMaintenanceRepository> m_repository;
    std::shared_ptr<State> m_state;
  };
};
}  // namespace uwf::test::application
