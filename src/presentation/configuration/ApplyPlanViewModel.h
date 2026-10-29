// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <memory>

#include "app/api/ConfigurationActions.h"
#include "app/api/FileStagingTypes.h"
#include "app/configuration/UwfmgrCommands.h"
#include "app/ports/TextFileWriter.h"
#include "presentation/common/AsyncTask.h"

namespace uwf::presentation {

class ApplyPlanViewModel final : public QAbstractListModel {
  Q_OBJECT
 public:
  enum Role { KindRole = Qt::UserRole + 1, DescriptionRole, BeforeRole, AfterRole, PathRole, LastRole };
  enum RowKind { Heading, Entry, Commands };
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
  int rowCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  [[nodiscard]] const core::PendingChanges& changes() const { return m_plan.changes; }
  [[nodiscard]] const core::UwfSnapshot& snapshot() const { return m_plan.snapshot; }
  [[nodiscard]] std::size_t changeCommandCount() const { return m_changeCommandCount; }
  [[nodiscard]] const std::vector<app::UwfmgrCommand>& exportCommands() const { return m_exportCommands; }
  [[nodiscard]] const app::ConfigurationApplyResult& result() const { return m_result; }
  [[nodiscard]] bool canApply() const { return m_canApply && m_executionAllowed && !m_applying; }
  [[nodiscard]] bool applying() const { return m_applying; }
  void setExecutionAllowed(bool allowed);
  [[nodiscard]] bool hasStagingChanges() const { return m_plan.staging.has_value(); }
  [[nodiscard]] std::size_t changeCount() const { return m_plan.count(); }
  [[nodiscard]] const QList<ChangeGroup>& groups() const { return m_groups; }
  [[nodiscard]] QString changeCommandsText() const;
  [[nodiscard]] QString targetCommandsText() const;
  [[nodiscard]] QString resultText() const;
  [[nodiscard]] bool resultFailed() const;
  [[nodiscard]] bool canRestart() const;
  [[nodiscard]] QString resultHeading() const;
  [[nodiscard]] QString resultDescription() const;
  [[nodiscard]] QString warning() const { return m_warning; }
  void apply();
  [[nodiscard]] bool canExport() const { return m_textFileWriter && !hasStagingChanges(); }
  [[nodiscard]] QString exportFile(const QString& path) const;

 signals:
  void resultChanged();
  void reconciliationRequired();
  void applyingChanged();
  void availabilityChanged();

 private:
  app::ConfigurationPlan m_plan;
  std::shared_ptr<app::ConfigurationActions> m_actions;
  std::size_t m_changeCommandCount = 0;
  std::vector<app::UwfmgrCommand> m_exportCommands;
  app::ConfigurationApplyResult m_result;
  bool m_canApply = false;
  bool m_executionAllowed = true;
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
