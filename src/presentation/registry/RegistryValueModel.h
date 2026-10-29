// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractTableModel>

#include "app/api/RegistryValueInfo.h"
namespace uwf::presentation {
class RegistryValueModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  enum Role { NameRole = Qt::UserRole };
  explicit RegistryValueModel(bool selectable, QObject* parent = nullptr);
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  void setValues(const std::vector<uwf::app::RegistryValueInfo>& values);

 private:
  struct Row {
    QString name;
    QString displayName;
    QString type;
    QString preview;
  };
  bool m_selectable;
  QList<Row> m_rows;
};
}  // namespace uwf::presentation
