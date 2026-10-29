// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractTableModel>
#include <QSet>
#include <QStringList>
#include <memory>

#include "app/ports/FileBrowser.h"
#include "presentation/common/AsyncTask.h"
#include "presentation/common/ContentLoadState.h"
#include "presentation/common/LoadingGate.h"

namespace uwf::presentation {
class FilePickerViewModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  enum class Mode { OpenFile, OpenFiles, Directory, Mixed, SaveFile };
  enum class Acceptance { Invalid, Navigate, Ready, Replace };
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
  [[nodiscard]] Acceptance prepareAcceptance();
  [[nodiscard]] Mode mode() const { return m_mode; }
  [[nodiscard]] QString path() const { return m_path; }
  [[nodiscard]] QString filename() const { return m_filename; }
  [[nodiscard]] QString error() const { return m_error; }
  [[nodiscard]] QStringList filters() const { return m_filters; }
  [[nodiscard]] int filterIndex() const { return m_filter; }
  [[nodiscard]] const QStringList& selectedPaths() const { return m_result; }
  [[nodiscard]] const QVector<app::FileBrowserEntry>& selectedItems() const { return m_resultItems; }
  [[nodiscard]] const QStringList& selectedEntries() const { return m_selected; }
  [[nodiscard]] const QVector<app::FileBrowserPlace>& places() const { return m_places; }
  [[nodiscard]] QAbstractItemModel* placesModel() const { return m_placesModel.get(); }
  [[nodiscard]] ContentLoadState loadState() const { return m_state; }
  [[nodiscard]] bool progressVisible() const { return m_loading.progressVisible(); }
  [[nodiscard]] bool canGoBack() const { return m_historyIndex > 0; }
  [[nodiscard]] bool canGoForward() const { return m_historyIndex + 1 < m_history.size(); }
  [[nodiscard]] bool canGoUp() const;
  [[nodiscard]] bool canAccept() const;
  [[nodiscard]] bool multiple() const { return m_mode == Mode::OpenFiles || m_mode == Mode::Mixed; }
  [[nodiscard]] bool isDirectory(int row) const;
  [[nodiscard]] QString entryPath(int row) const;

 signals:
  void stateChanged();
  void selectionChanged();
  void validationFailed(const QString& message);

 private:
  void load();
  void refilter();
  bool fail(const QString& message);
  [[nodiscard]] QString resolve(const QString& path) const;
  Mode m_mode;
  std::shared_ptr<const app::FileBrowser> m_browser;
  LoadingGate m_loading{this};
  AsyncTask m_worker{this};
  ContentLoadState m_state = ContentLoadState::Empty;
  QVector<app::FileBrowserEntry> m_entries;
  QVector<app::FileBrowserEntry> m_visible;
  QVector<app::FileBrowserPlace> m_places;
  std::unique_ptr<QAbstractItemModel> m_placesModel;
  QString m_path;
  QString m_query;
  QString m_filename;
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
  QStringList m_result;
  QVector<app::FileBrowserEntry> m_resultItems;
};
}  // namespace uwf::presentation
