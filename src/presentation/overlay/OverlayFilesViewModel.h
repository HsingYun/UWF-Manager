// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractTableModel>

#include "app/api/OverlayFilesTypes.h"
#include "app/ports/TextFileWriter.h"
#include "presentation/common/AsyncTask.h"
#include "presentation/common/ContentLoadState.h"
#include "presentation/common/LoadingGate.h"
namespace uwf::presentation {
class OverlayFilesViewModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  OverlayFilesViewModel(const QString& driveLetter, app::OverlayFileLoader loader, app::TextFileWriter textFileWriter = {});
  ~OverlayFilesViewModel() override;
  [[nodiscard]] const QString& driveLetter() const { return m_driveLetter; }
  [[nodiscard]] const QVector<app::OverlayFileEntry>& entries() const { return m_entries; }
  [[nodiscard]] bool loaded() const { return m_loaded; }
  [[nodiscard]] bool loading() const { return m_loading.isActive(); }
  [[nodiscard]] ContentLoadState loadState() const { return m_state; }
  [[nodiscard]] QString errorText() const { return m_error; }
  [[nodiscard]] const QString& errorHint() const { return m_errorHint; }
  [[nodiscard]] const QString& errorDetail() const { return m_errorDetail; }
  [[nodiscard]] QString exportContent() const;
  [[nodiscard]] bool canExport() const { return m_state == ContentLoadState::Ready && !m_entries.isEmpty() && m_textFileWriter; }
  [[nodiscard]] QString exportFile(const QString& path) const;
  void startLoading();
  void cancelLoading();
  void retranslate();
 signals:
  void loadingStarted();
  void loadingStopped();
  void loadFailed(const QString& summary);
  void entriesLoaded();

 private:
  using LoadResult = AsyncResult<QVector<app::OverlayFileEntry>>;
  void onLoadFinished(LoadingGate::Ticket generation, LoadResult result);
  QString m_driveLetter;
  app::OverlayFileLoader m_loader;
  app::TextFileWriter m_textFileWriter;
  QVector<app::OverlayFileEntry> m_entries;
  bool m_loaded = false;
  ContentLoadState m_state = ContentLoadState::Idle;
  QString m_error;
  QString m_errorHint;
  QString m_errorDetail;
  LoadingGate m_loading{this};
  AsyncTask m_worker{this};
};
}  // namespace uwf::presentation
