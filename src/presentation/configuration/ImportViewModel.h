// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <functional>

#include "app/configuration/ImportReport.h"
#include "app/configuration/UwfmgrCommands.h"
#include "app/ports/TextFileReader.h"

namespace uwf::presentation {

class ImportViewModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  enum RecommendedSection {
    Ceip = 1 << 0,
    Bits = 1 << 1,
    Network = 1 << 2,
    Dst = 1 << 3,
    Defender = 1 << 4,
    Scep = 1 << 5,
    AllRecommendedSections = Ceip | Bits | Network | Dst | Defender | Scep
  };
  explicit ImportViewModel(app::TextFileReader reader = {}, QString systemDrive = QStringLiteral("C:"))
      : m_textFileReader(std::move(reader)), m_systemDrive(std::move(systemDrive)) {}
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  using Applier = std::function<QList<app::ImportResult>(const QList<app::UwfmgrCommand>&)>;
  struct Totals {
    int success = 0;
    int duplicate = 0;
    int failed = 0;
    int unsupported = 0;
    int batches = 0;
  };
  void setApplier(Applier applier) { m_applier = std::move(applier); }
  void setText(QString text) { m_text = std::move(text); }
  [[nodiscard]] const QString& text() const { return m_text; }
  [[nodiscard]] bool canImport() const { return !m_text.trimmed().isEmpty(); }
  [[nodiscard]] const Totals& totals() const { return m_totals; }
  [[nodiscard]] const QList<QList<app::ImportResult>>& batches() const { return m_batches; }
  void importText();
  void appendReport(const QList<app::ImportResult>& rows);
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  void loadFile(const QString& path);
  [[nodiscard]] bool canLoadFiles() const { return static_cast<bool>(m_textFileReader); }
  [[nodiscard]] QString defaultRulesText(int sections) const;

 signals:
  void warningRaised(const QString& title, const QString& body);
  void textBlockReady(const QString& block);
  void reportAppended(const QList<uwf::app::ImportResult>& rows);

 private:
  struct Row {
    int batch;
    int entry;
  };
  QList<Row> m_rows;
  Applier m_applier;
  app::TextFileReader m_textFileReader;
  QString m_systemDrive;
  QString m_text;
  Totals m_totals;
  QList<QList<app::ImportResult>> m_batches;
};

}  // namespace uwf::presentation
