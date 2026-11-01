// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QSortFilterProxyModel>

namespace uwf::presentation {
enum class LogLevelFilter { All, Error, Warning, Information, Debug };

class LogFilterModel final : public QSortFilterProxyModel {
 public:
  explicit LogFilterModel(QObject* parent);
  void setQuery(const QString& query);
  void setLevel(LogLevelFilter level);
  [[nodiscard]] QString query() const { return m_query; }
  [[nodiscard]] LogLevelFilter level() const { return m_level; }
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

 protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

 private:
  QString m_query;
  LogLevelFilter m_level = LogLevelFilter::All;
};
}  // namespace uwf::presentation
