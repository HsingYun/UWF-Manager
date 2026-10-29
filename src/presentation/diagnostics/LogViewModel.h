// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractTableModel>
#include <array>
#include <memory>
#include <vector>

#include "LogFilterModel.h"
#include "app/api/LogSource.h"
#include "presentation/common/AsyncTask.h"
#include "presentation/common/ContentLoadState.h"
#include "presentation/common/LoadingGate.h"
namespace uwf::presentation {
class LogViewModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  using LogRow = std::array<QString, 4>;
  explicit LogViewModel(std::shared_ptr<app::LogSource> source);
  ~LogViewModel() override;
  [[nodiscard]] const std::vector<LogRow>& entries() const { return m_entries; }
  [[nodiscard]] QString allRowsToText() const;
  [[nodiscard]] bool loading() const { return m_loading.isActive(); }
  [[nodiscard]] ContentLoadState loadState() const { return m_state; }
  [[nodiscard]] QString errorText() const { return m_error; }
  [[nodiscard]] QAbstractItemModel* filteredModel() const { return m_filtered.get(); }
  void setQuery(const QString& query) { m_filtered->setQuery(query); }
  void setLevelFilter(LogLevelFilter level) { m_filtered->setLevel(level); }
  [[nodiscard]] QString query() const { return m_filtered->query(); }
  [[nodiscard]] LogLevelFilter levelFilter() const { return m_filtered->level(); }
  void reload();
  void cancelLoading();
  void clear();
 signals:
  void loadingStarted();
  void loadFailed(const QString& message);
  void entriesChanged();
  void loadingFinished(int total);
  void loadingCanceled();

 private:
  void onLoadFinished(LoadingGate::Ticket generation, AsyncResult<std::vector<LogRow>> result);
  std::shared_ptr<app::LogSource> m_source;
  std::vector<LogRow> m_entries;
  std::unique_ptr<LogFilterModel> m_filtered;
  ContentLoadState m_state = ContentLoadState::Idle;
  QString m_error;
  LoadingGate m_loading{this};
  AsyncTask m_loader{this};
};
}  // namespace uwf::presentation
