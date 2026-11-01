// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractListModel>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "application/staging/contracts/AutomaticCommitDisposition.h"
#include "application/staging/contracts/FileStagingEditing.h"
#include "application/uwf/contracts/RuntimeWriteRestriction.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/AsyncTask.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"

namespace uwf::presentation {

class FileStagingViewModel final : public QAbstractListModel {
  Q_OBJECT
 public:
  struct AutomaticCommitVolumes {
    // Keys are normalized volume GUIDs from the shared execution state.
    std::map<std::string, app::AutomaticCommitDisposition> dispositions;
    app::AutomaticCommitDisposition unregistered = app::AutomaticCommitDisposition::Unavailable;
    bool operator==(const AutomaticCommitVolumes&) const = default;
  };
  struct Availability {
    bool readOnly = false;
    bool enhancedModeAvailable = false;
    app::RuntimeWriteRestriction editingRestriction = app::RuntimeWriteRestriction::None;
    app::AutomaticCommitSummary automaticCommit{.unavailable = true};
    std::optional<AutomaticCommitVolumes> automaticCommitVolumes{};
    bool additionsAllowed = true;
    bool operator==(const Availability&) const = default;
  };
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  explicit FileStagingViewModel(std::shared_ptr<app::FileStagingEditing> editing);
  ~FileStagingViewModel() override;
  [[nodiscard]] const QString& driveLetter() const { return m_editing->driveLetter(); }
  [[nodiscard]] const QList<app::FileStagingEntry>& entries() const { return m_entries; }
  [[nodiscard]] bool storageAvailable() const { return m_editing->storageAvailable(); }
  [[nodiscard]] const QString& storageError() const { return m_editing->storageError(); }
  [[nodiscard]] const Availability& availability() const { return m_availability; }
  [[nodiscard]] bool readOnly() const { return m_availability.readOnly; }
  [[nodiscard]] bool enhancedModeAvailable() const { return m_availability.enhancedModeAvailable; }
  [[nodiscard]] bool busy() const { return m_editing->busy(); }
  [[nodiscard]] bool writable() const { return !busy() && storageAvailable() && !readOnly() && editingRestriction() == app::RuntimeWriteRestriction::None; }
  [[nodiscard]] bool canAddEntries() const { return writable() && m_availability.additionsAllowed; }
  [[nodiscard]] app::RuntimeWriteRestriction editingRestriction() const { return m_availability.editingRestriction; }
  [[nodiscard]] QString editingRestrictionText() const;
  [[nodiscard]] AutomaticCommitPresentation automaticCommitPresentation() const;
  void load();
  void retranslate();
  [[nodiscard]] QString summaryText() const;
  void setAvailability(Availability availability);
  void addEntries(const QList<app::FileStagingEntry>& entries);
  void removePaths(const QStringList& paths);
  void restorePaths(const QStringList& paths);

 signals:
  void entriesChanged();
  void storageUnavailable();
  void storageAvailabilityChanged();
  void modificationAvailabilityChanged();
  void enhancedModeAvailabilityChanged();
  void automaticCommitStateChanged();
  void warningRaised(const QString& title, const QString& body);

 private:
  void reportRejection(const app::FileStagingRejection& rejection);
  void rebuildEntries();
  void runPreparation(const std::function<app::FileStagingPreparation()>& create);
  [[nodiscard]] app::AutomaticCommitDisposition automaticCommitDisposition(const app::FileStagingEntry& entry) const;
  [[nodiscard]] app::AutomaticCommitSummary automaticCommitSummary() const;
  [[nodiscard]] AutomaticCommitPresentation automaticCommitPresentation(const app::FileStagingEntry& entry) const;

  std::shared_ptr<app::FileStagingEditing> m_editing;
  QList<app::FileStagingEntry> m_entries;
  struct Row {
    app::FileStagingEntry entry;
    ItemStatus status;
  };
  QList<Row> m_rows;
  bool m_rebuildingEntries = false;
  bool m_rebuildPending = false;
  Availability m_availability;
  AsyncTask m_preparation;
  std::function<void()> m_cancelPreparation;
};

}  // namespace uwf::presentation
