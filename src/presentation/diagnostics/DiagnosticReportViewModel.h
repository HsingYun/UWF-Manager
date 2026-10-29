// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QList>
#include <QString>

#include "app/api/DiagnosticReportTypes.h"
namespace uwf::presentation {
class DiagnosticReportViewModel final {
 public:
  struct Field {
    QString label;
    QString value;
  };
  struct Section {
    QString title;
    QString icon;
    QList<Field> fields;
  };
  explicit DiagnosticReportViewModel(QString report);
  DiagnosticReportViewModel(const app::DiagnosticReportData& report, quint32 totalRamMb);
  [[nodiscard]] const QString& report() const { return m_report; }
  [[nodiscard]] const QList<Section>& sections() const { return m_sections; }
  [[nodiscard]] bool partial() const { return m_partial; }

 private:
  QString m_report;
  QList<Section> m_sections;
  bool m_partial = false;
};
}  // namespace uwf::presentation
