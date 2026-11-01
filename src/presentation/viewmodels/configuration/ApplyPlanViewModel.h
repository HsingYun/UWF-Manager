// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <memory>

#include "application/configuration/contracts/ConfigurationActions.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "application/files/contracts/TextFileWriter.h"
#include "application/staging/contracts/FileStagingTypes.h"
#include "presentation/viewmodels/common/AsyncTask.h"

namespace uwf::presentation {

class ApplyPlanViewModel final : public QAbstractListModel {
  Q_OBJECT
 public:
  using ScriptScope = app::ConfigurationScriptScope;
  enum Role { KindRole = Qt::UserRole + 1, DescriptionRole, BeforeRole, AfterRole, PathRole, LastRole };
  enum RowKind { Heading, Entry, Commands };
  struct InteractionAvailability {
    bool apply = true;
    bool safeRestart = true;
    bool directRestart = true;
    QString safeRestartRestriction;
    QString directRestartRestriction;
    bool operator==(const InteractionAvailability&) const = default;
  };
  struct Change {
    QString title;
    QString description;
    QString before;
    QString after;
    bool path = false;
  };
  struct ChangeGroup {
    QString title;
    QString icon;
    QList<Change> items;
  };

  ApplyPlanViewModel(core::PendingChanges changes, core::UwfSnapshot snapshot, std::shared_ptr<app::ConfigurationActions> actions,
                     std::optional<app::FileStagingChanges> stagingChanges = {}, app::TextFileWriter textFileWriter = {});
  ApplyPlanViewModel(app::ConfigurationPlan plan, std::shared_ptr<app::ConfigurationActions> actions, app::TextFileWriter textFileWriter = {});
  int rowCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  [[nodiscard]] const core::PendingChanges& changes() const { return m_plan.changes; }
  [[nodiscard]] app::ConfigurationChanges configurationChanges() const { return {m_plan.changes, m_plan.overlayPolicy, m_plan.maintenance}; }
  [[nodiscard]] const core::UwfSnapshot& snapshot() const { return m_plan.snapshot; }
  [[nodiscard]] std::size_t changeCommandCount() const { return m_changeCommandCount; }
  [[nodiscard]] const std::vector<app::UwfmgrCommand>& exportCommands(ScriptScope scope = ScriptScope::PendingChanges) const { return script(scope).commands; }
  [[nodiscard]] const app::ConfigurationApplyResult& result() const { return m_result; }
  [[nodiscard]] bool canApply() const { return m_canApply && m_interaction.apply && !m_applying; }
  [[nodiscard]] bool applyActionVisible() const { return m_canApply || m_applying || m_result.steps.empty(); }
  [[nodiscard]] bool applying() const { return m_applying; }
  void setInteractionAvailability(InteractionAvailability availability);
  [[nodiscard]] bool hasStagingChanges() const { return m_plan.hasStagingChanges(); }
  [[nodiscard]] bool hasMaintenanceChanges() const { return !m_plan.maintenance.empty(); }
  [[nodiscard]] const app::OverlayMaintenanceChanges& maintenanceChanges() const { return m_plan.maintenance; }
  [[nodiscard]] bool onlyDisablesHorm() const {
    return m_plan.changes.empty() && m_plan.overlayPolicy.empty() && !hasStagingChanges() && m_plan.maintenance.hormEnabled == false &&
           !m_plan.maintenance.storageVolume && !m_plan.maintenance.fullCommit && !m_plan.maintenance.allVolumesProtected;
  }
  [[nodiscard]] std::size_t changeCount() const { return m_plan.count(); }
  [[nodiscard]] const QList<ChangeGroup>& groups() const { return m_groups; }
  [[nodiscard]] QString changeCommandsText() const;
  [[nodiscard]] QString targetCommandsText() const;
  [[nodiscard]] QString exportRestrictionText(ScriptScope scope = ScriptScope::PendingChanges) const;
  [[nodiscard]] QString resultText() const;
  [[nodiscard]] bool resultFailed() const;
  [[nodiscard]] bool canRestart() const;
  [[nodiscard]] bool canSafeRestart() const;
  [[nodiscard]] bool canDirectRestart() const;
  [[nodiscard]] QString safeRestartRestriction() const;
  [[nodiscard]] QString directRestartRestriction() const;
  [[nodiscard]] QString resultHeading() const;
  [[nodiscard]] QString activationText() const;
  [[nodiscard]] QString resultDescription() const;
  [[nodiscard]] QString warning() const { return m_warning; }
  void apply();
  [[nodiscard]] bool canExport(ScriptScope scope = ScriptScope::PendingChanges) const {
    return m_validPlan && m_textFileWriter && !hasStagingChanges() && script(scope).exportSupported && !script(scope).commands.empty();
  }
  [[nodiscard]] QString exportFile(const QString& path, ScriptScope scope = ScriptScope::PendingChanges) const;

 signals:
  void resultChanged();
  void reconciliationRequired();
  void applyingChanged();
  void availabilityChanged();

 private:
  [[nodiscard]] const app::ConfigurationScript& script(ScriptScope scope) const {
    return scope == ScriptScope::PendingChanges ? m_changeScript : m_targetScript;
  }
  app::ConfigurationPlan m_plan;
  std::shared_ptr<app::ConfigurationActions> m_actions;
  std::size_t m_changeCommandCount = 0;
  app::ConfigurationScript m_changeScript;
  app::ConfigurationScript m_targetScript;
  app::ConfigurationApplyResult m_result;
  bool m_canApply = false;
  bool m_validPlan = true;
  InteractionAvailability m_interaction;
  bool m_applying = false;
  app::TextFileWriter m_textFileWriter;
  QList<ChangeGroup> m_groups;
  struct Row {
    int group;
    int entry;
  };
  QList<Row> m_rows;
  QString m_warning;
  AsyncTask m_apply;
  void publishResult(app::ConfigurationApplyResult result);
};

}  // namespace uwf::presentation
