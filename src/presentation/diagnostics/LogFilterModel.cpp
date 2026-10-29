// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "LogFilterModel.h"

#include "presentation/common/PresentationText.h"

namespace uwf::presentation {
namespace {
LogLevelFilter levelOf(const QString& level) {
  if (level == QLatin1String("E") || level == QLatin1String("F")) return LogLevelFilter::Error;
  if (level == QLatin1String("W")) return LogLevelFilter::Warning;
  if (level == QLatin1String("I")) return LogLevelFilter::Information;
  if (level == QLatin1String("D")) return LogLevelFilter::Debug;
  return LogLevelFilter::All;
}
QString levelText(const QString& level) {
  switch (levelOf(level)) {
    case LogLevelFilter::Error:
      return PresentationText::tr("Error");
    case LogLevelFilter::Warning:
      return PresentationText::tr("Warning");
    case LogLevelFilter::Information:
      return PresentationText::tr("Information");
    case LogLevelFilter::Debug:
      return PresentationText::tr("Debug");
    case LogLevelFilter::All:
      return level;
  }
  return level;
}
}  // namespace
LogFilterModel::LogFilterModel(QObject* parent) : QSortFilterProxyModel(parent) {}
void LogFilterModel::setQuery(const QString& query) {
  if (m_query == query) return;
  beginFilterChange();
  m_query = query;
  endFilterChange(Direction::Rows);
}
void LogFilterModel::setLevel(const LogLevelFilter level) {
  if (m_level == level) return;
  beginFilterChange();
  m_level = level;
  endFilterChange(Direction::Rows);
}
bool LogFilterModel::filterAcceptsRow(const int row, const QModelIndex& parent) const {
  const auto* source = sourceModel();
  if (!source) return false;
  if (m_level != LogLevelFilter::All && levelOf(source->index(row, 1, parent).data().toString()) != m_level) return false;
  if (m_query.isEmpty()) return true;
  for (int column = 0; column < source->columnCount(parent); ++column)
    if (source->index(row, column, parent).data().toString().contains(m_query, Qt::CaseInsensitive)) return true;
  return false;
}
QVariant LogFilterModel::data(const QModelIndex& index, const int role) const {
  const auto value = QSortFilterProxyModel::data(index, role);
  if (role == Qt::DisplayRole && index.column() == 1) return levelText(value.toString());
  if (role == Qt::DisplayRole && index.column() == 3) return value.toString().section(u'\n', 0, 0);
  return value;
}
QVariant LogFilterModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 2) return PresentationText::tr("Module");
  return QSortFilterProxyModel::headerData(section, orientation, role);
}
}  // namespace uwf::presentation
