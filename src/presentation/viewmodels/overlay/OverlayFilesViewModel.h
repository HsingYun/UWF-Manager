// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractTableModel>
#include <optional>

#include "application/files/contracts/TextFileWriter.h"
#include "application/overlay/contracts/OverlayFilesTypes.h"
#include "application/overlay/files/OverlayFilesFailure.h"
#include "application/overlay/files/OverlayFilesQueryManager.h"
#include "application/paths/PathPolicy.h"
#include "presentation/viewmodels/common/AsyncTask.h"
#include "presentation/viewmodels/common/ContentLoadState.h"
#include "presentation/viewmodels/common/LoadingGate.h"
namespace uwf::presentation {
class OverlayFilesViewModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  OverlayFilesViewModel(const QString& driveLetter, const QString& volumeGuid, std::shared_ptr<app::OverlayFilesQueryManager> queries,
                        app::TextFileWriter textFileWriter = {});
  ~OverlayFilesViewModel() override;
  [[nodiscard]] const QString& driveLetter() const { return m_driveLetter; }
  [[nodiscard]] const QVector<app::OverlayFileEntry>& entries() const { return m_entries; }
  [[nodiscard]] bool loaded() const { return m_loaded; }
  [[nodiscard]] bool loading() const { return m_loading.isActive(); }
  [[nodiscard]] ContentLoadState loadState() const { return m_state; }
  [[nodiscard]] QString errorText() const;
  [[nodiscard]] QString errorHint() const;
  [[nodiscard]] QString errorDetail() const;
  [[nodiscard]] bool reloadAllowed() const { return !m_failure || m_failure->kind != app::OverlayFilesFailure::Kind::CompletionUnknown; }
  [[nodiscard]] QString exportContent() const;
  [[nodiscard]] bool canExport() const { return m_state == ContentLoadState::Ready && !m_entries.isEmpty() && m_textFileWriter; }
  [[nodiscard]] QString exportFile(const QString& path) const;
  void setCommitAvailability(bool allowed, QString reason = {});
  [[nodiscard]] bool canCommit(const QString& path) const;
  [[nodiscard]] QString commitRestriction(const QString& path) const;
  void requestCommit(const QString& path);
  void startLoading();
  void cancelLoading();
  void retranslate();
 signals:
  void loadingStarted();
  void loadingStopped();
  void loadFailed(const QString& summary);
  void entriesLoaded();
  void commitAvailabilityChanged();
  void commitFileRequested(const QString& absolutePath);

 private:
  using LoadResult = AsyncResult<QVector<app::OverlayFileEntry>>;
  [[nodiscard]] std::optional<app::paths::VolumeFilePath> commitTarget(const QString& path) const;
  void onQueryFinished(LoadingGate::Ticket generation, app::OverlayFilesQueryManager::Result result);
  void onLoadFinished(LoadingGate::Ticket generation, LoadResult result);
  void publishLoad(LoadingGate::Ticket generation, LoadResult result);
  struct PendingLoad {
    LoadingGate::Ticket generation;
    LoadResult result;
  };
  std::optional<PendingLoad> m_pendingLoad;
  bool m_publishing = false;
  QString m_driveLetter;
  QString m_volumeGuid;
  std::shared_ptr<app::OverlayFilesQueryManager> m_queries;
  app::OverlayFilesQueryManager::Subscription m_subscription;
  app::TextFileWriter m_textFileWriter;
  QVector<app::OverlayFileEntry> m_entries;
  bool m_commitsAllowed = false;
  QString m_commitRestriction;
  bool m_loaded = false;
  ContentLoadState m_state = ContentLoadState::Idle;
  std::optional<app::OverlayFilesFailure> m_failure;
  LoadingGate m_loading{this};
  AsyncTask m_worker{this};
};
}  // namespace uwf::presentation
