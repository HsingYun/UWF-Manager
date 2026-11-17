// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractTableModel>
#include <QStringView>
#include <optional>

#include "application/registry/contracts/RegistryValueInfo.h"

namespace uwf::presentation {
class RegistryValueModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  enum Role { NameRole = Qt::UserRole };
  enum class Selection { None, NamedValues, AllValues };
  explicit RegistryValueModel(Selection selection, QObject* parent = nullptr);
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  [[nodiscard]] bool canSelectValue(QStringView name) const;
  void clear();
  // Only observed values enter this model. A missing default value receives a
  // separate display-only row, never a fabricated RegistryValueInfo.
  void setValues(const std::vector<uwf::app::RegistryValueInfo>& values);

 private:
  enum class RowKind { ObservedValue, DefaultPlaceholder };
  struct Row {
    QString name;
    QString displayName;
    QString type;
    QString preview;
    RowKind kind;
  };
  [[nodiscard]] bool selectable(const Row& row) const;
  void publishRows(QList<Row> rows);
  Selection m_selection;
  QList<Row> m_rows;
  std::optional<QList<Row>> m_pendingRows;
  bool m_publishing = false;
};
}  // namespace uwf::presentation
