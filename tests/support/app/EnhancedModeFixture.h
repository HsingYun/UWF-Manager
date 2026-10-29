// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <stdexcept>

#include "app/ports/EnhancedModeAgentConnection.h"
#include "app/ports/EnhancedModeServiceControl.h"
namespace uwf::test::application {
app::EnhancedModeStatus completeEnhancedModeStatus();

class MemoryEnhancedModeServiceControl final : public app::EnhancedModeServiceControl {
 public:
  class Barrier final : public DeletionBarrier {
   public:
    explicit Barrier(MemoryEnhancedModeServiceControl& owner) : m_owner(owner) { ++m_owner.activeDeletionBarriers; }
    ~Barrier() override { --m_owner.activeDeletionBarriers; }

   private:
    MemoryEnhancedModeServiceControl& m_owner;
  };

  app::EnhancedModeStatus current;
  QString queryFailure;
  int installs = 0;
  int starts = 0;
  int removals = 0;
  int activeDeletionBarriers = 0;
  mutable int queries = 0;
  mutable bool queriedDuringDeletion = false;
  std::vector<uwf::app::RegistryCommitTarget> installationPlan;
  std::vector<uwf::app::RegistryCommitTarget> deletionPlan;
  QString installedDescription;
  std::function<void()> beforeInstall;
  std::function<void()> beforeCommitPlan;
  std::function<void()> beforeDeletionPlan;
  std::function<void()> beforeDeleteRemnants;
  RemovalState removalState = RemovalState::MarkedForDeletion;

  [[nodiscard]] app::EnhancedModeStatus query() const override {
    ++queries;
    if (!queryFailure.isEmpty()) throw std::runtime_error(queryFailure.toStdString());
    if (activeDeletionBarriers != 0) queriedDuringDeletion = true;
    return current;
  }
  void installAndStart(const QString& description) override {
    if (beforeInstall) beforeInstall();
    ++installs;
    installedDescription = description;
    current = completeEnhancedModeStatus();
  }
  void start() override {
    ++starts;
    current.running = true;
    current.preshutdownAccepted = true;
    current.state = current.serviceContractSatisfied() ? app::EnhancedModeState::Enabled : app::EnhancedModeState::RepairRequired;
  }
  [[nodiscard]] std::vector<uwf::app::RegistryCommitTarget> planRegistryCommit() const override {
    if (beforeCommitPlan) beforeCommitPlan();
    return installationPlan;
  }
  [[nodiscard]] std::vector<uwf::app::RegistryCommitTarget> planRegistryDeletion() const override {
    if (beforeDeletionPlan) beforeDeletionPlan();
    return deletionPlan;
  }
  void deleteRegistryRemnants() override {
    if (beforeDeleteRemnants) beforeDeleteRemnants();
    current.serviceRegistryPresent = false;
  }
  [[nodiscard]] Removal stopAndMarkForDeletion() override {
    ++removals;
    current = {};
    return {removalState, removalState == RemovalState::MarkedForDeletion ? std::make_unique<Barrier>(*this) : nullptr};
  }
};

class MemoryEnhancedModeAgent final : public app::EnhancedModeAgentConnection {
 public:
  int starts = 0;
  int stops = 0;
  bool canStart = true;
  bool connectOnStart = false;
  bool acknowledge = true;
  std::vector<bool> preshutdownCommitStates;

  void start() override {
    if (m_running) return;
    ++starts;
    m_running = canStart;
    if (m_running && connectOnStart) emit connectionStateChanged(true);
  }

  void stop() override {
    if (!m_running) return;
    m_running = false;
    ++stops;
    emit connectionStateChanged(false);
  }

  [[nodiscard]] bool running() const override { return m_running; }
  [[nodiscard]] bool markPreshutdownCommitHandled() override {
    preshutdownCommitStates.push_back(true);
    return acknowledge;
  }
  [[nodiscard]] bool requirePreshutdownCommit() override {
    preshutdownCommitStates.push_back(false);
    return acknowledge;
  }

  void publishConnectionState(const bool connected) { emit connectionStateChanged(connected); }

 private:
  bool m_running = false;
};

}  // namespace uwf::test::application
