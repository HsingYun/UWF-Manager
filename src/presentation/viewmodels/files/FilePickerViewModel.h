// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractTableModel>
#include <QStringList>
#include <cstdint>
#include <memory>

#include "application/files/contracts/FileBrowser.h"
#include "presentation/viewmodels/common/AsyncTask.h"
#include "presentation/viewmodels/common/ContentLoadState.h"
#include "presentation/viewmodels/common/LoadingGate.h"

namespace uwf::presentation {
class FilePickerViewModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  enum class Mode { OpenFile, OpenFiles, Directory, Mixed, SaveFile };
  enum class Acceptance { Invalid, Navigate, Ready, Replace };
  Q_ENUM(Acceptance)
  explicit FilePickerViewModel(Mode mode, std::shared_ptr<const app::FileBrowser> browser, QObject* parent = nullptr);
  ~FilePickerViewModel() override;
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
  void start(const QString& path);
  void navigate(const QString& path);
  void goBack();
  void goForward();
  void goUp();
  void refresh();
  void retranslate();
  void cancel();
  void setQuery(const QString& query);
  void setFilters(const QString& filters);
  void setAllowedRoots(const QStringList& roots);
  [[nodiscard]] bool isPathAllowed(const QString& path) const;
  void setFilterIndex(int index);
  void setFilename(const QString& name);
  void selectRows(const QList<int>& rows);
  void createDirectory(const QString& name);
  void renameEntry(int row, const QString& name);
  void requestAcceptance();
  [[nodiscard]] Mode mode() const { return m_mode; }
  [[nodiscard]] QString query() const { return m_query; }
  [[nodiscard]] QString path() const { return m_path; }
  [[nodiscard]] QString filename() const;
  [[nodiscard]] QString error() const { return m_error; }
  [[nodiscard]] QStringList filters() const { return m_filters; }
  [[nodiscard]] int filterIndex() const { return m_filter; }
  [[nodiscard]] const QStringList& selectedPaths() const { return m_result; }
  [[nodiscard]] const QVector<app::FileBrowserEntry>& selectedItems() const { return m_resultItems; }
  [[nodiscard]] const QStringList& selectedEntries() const { return m_selected; }
  [[nodiscard]] const QVector<app::FileBrowserPlace>& places() const { return m_places; }
  [[nodiscard]] QAbstractItemModel* placesModel() const { return m_placesModel.get(); }
  [[nodiscard]] ContentLoadState loadState() const { return m_state; }
  [[nodiscard]] bool progressVisible() const { return m_loading.progressVisible() || m_filtering.progressVisible(); }
  [[nodiscard]] bool busy() const { return m_loading.isActive() || m_filtering.isActive() || m_filterPublishing || m_refilterPending || m_readPublishing; }
  [[nodiscard]] bool canEditQuery() const { return !m_loading.isActive(); }
  [[nodiscard]] bool canGoBack() const { return !busy() && m_historyIndex > 0; }
  [[nodiscard]] bool canGoForward() const { return !busy() && m_historyIndex + 1 < m_history.size(); }
  [[nodiscard]] bool canGoUp() const;
  [[nodiscard]] bool canAccept() const;
  [[nodiscard]] bool multiple() const { return m_mode == Mode::OpenFiles || m_mode == Mode::Mixed; }
  [[nodiscard]] bool isDirectory(int row) const;
  [[nodiscard]] QString entryPath(int row) const;

 signals:
  void stateChanged();
  void selectionChanged();
  void validationFailed(const QString& message);
  void acceptanceFinished(uwf::presentation::FilePickerViewModel::Acceptance acceptance);

 private:
  void load(bool initial = false);
  void updatePlaces();
  void writeEntry(const QString& target, const QString& source = {});
  void refilter();
  void finishFilterPublication();
  bool fail(const QString& message);
  [[nodiscard]] QString resolve(const QString& path) const;
  Mode m_mode;
  std::shared_ptr<const app::FileBrowser> m_browser;
  LoadingGate m_loading{this};
  AsyncTask m_worker{this};
  LoadingGate m_filtering{this};
  AsyncTask m_filterWorker{this};
  ContentLoadState m_state = ContentLoadState::Empty;
  ContentLoadState m_listingState = ContentLoadState::Empty;
  QVector<app::FileBrowserEntry> m_entries;
  QVector<app::FileBrowserEntry> m_visible;
  QVector<app::FileBrowserPlace> m_places;
  QVector<app::FileBrowserPlace> m_allPlaces;
  bool m_placesLoaded = false;
  bool m_filterPublishing = false;
  bool m_refilterPending = false;
  bool m_readPublishing = false;
  std::unique_ptr<QAbstractItemModel> m_placesModel;
  QString m_path;
  QString m_query;
  // A selected existing save target retains its full object path in m_selected;
  // editing this text clears that selection and starts a new-name request.
  QString m_enteredName;
  QString m_error;
  QString m_selectAfterLoad;
  QStringList m_filters;
  QStringList m_allowedRoots;
  QStringList m_patterns;
  int m_filter = 0;
  int m_sortColumn = 0;
  Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
  QStringList m_history;
  qsizetype m_historyIndex = -1;
  QStringList m_selected;
  QStringList m_selectedNames;
  std::uint64_t m_selectionGeneration = 0;
  QStringList m_result;
  QVector<app::FileBrowserEntry> m_resultItems;
};
}  // namespace uwf::presentation
