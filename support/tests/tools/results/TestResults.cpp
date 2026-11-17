// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TestResults.h"

#include <QSet>
#include <QXmlStreamReader>
#include <stdexcept>

namespace uwf::test::tools {

TestResults readTestResults(QIODevice& input) {
  QXmlStreamReader xml(&input);
  if (!xml.readNextStartElement() || xml.name() != QLatin1String("TestCase")) throw std::runtime_error("Expected QtTest XML output.");
  const QString suite = xml.attributes().value(QLatin1String("name")).toString();
  if (suite.isEmpty()) throw std::runtime_error("Missing QtTest class name.");
  TestResults result;
  QSet<QString> lifecycle;
  while (xml.readNextStartElement()) {
    if (xml.name() != QLatin1String("TestFunction")) {
      xml.skipCurrentElement();
      continue;
    }
    const auto function = xml.attributes().value(QLatin1String("name")).toString();
    if (function.isEmpty()) throw std::runtime_error("Missing QtTest function name.");
    const bool infrastructure = function == QLatin1String("initTestCase") || function == QLatin1String("cleanupTestCase");
    bool recordedOutcome = false;
    while (xml.readNextStartElement()) {
      const QString outcome = xml.attributes().value(QLatin1String("type")).toString();
      const bool incident = xml.name() == QLatin1String("Incident") || (xml.name() == QLatin1String("Message") && outcome == QLatin1String("skip"));
      if (!incident) {
        xml.skipCurrentElement();
        continue;
      }
      recordedOutcome = true;
      QString tag;
      while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("DataTag"))
          tag = xml.readElementText();
        else
          xml.skipCurrentElement();
      }
      const QString id = suite + QStringLiteral("::") + function + u'(' + tag + u')';
      if (infrastructure) {
        ++result.lifecycleRecords;
        lifecycle.insert(function);
      } else if (result.rows.contains(id)) {
        result.problems.append(QStringLiteral("duplicate result: ") + id);
      } else {
        result.rows.insert(id, outcome);
      }
      if (outcome != QLatin1String("pass")) result.problems.append(id + QStringLiteral(": ") + outcome);
    }
    if (!recordedOutcome) result.problems.append(suite + QStringLiteral("::") + function + QStringLiteral(": no outcome recorded"));
  }
  while (!xml.atEnd()) xml.readNext();
  if (xml.hasError()) throw std::runtime_error(xml.errorString().toStdString());
  if (result.rows.isEmpty()) result.problems.append(QStringLiteral("no test rows executed"));
  if (!lifecycle.contains(QStringLiteral("initTestCase")) || !lifecycle.contains(QStringLiteral("cleanupTestCase")))
    result.problems.append(QStringLiteral("incomplete QtTest lifecycle"));
  return result;
}

QJsonObject reconcileTestResults(const TestResults& actual, const QJsonArray& expected) {
  auto problems = actual.problems;
  QSet<QString> identifiers;
  QJsonArray missing;
  QJsonArray removed;
  for (const auto& value : expected) {
    if (!value.isObject()) throw std::runtime_error("Expected case entries must be objects.");
    const auto entry = value.toObject();
    const QString id = entry.value(QStringLiteral("id")).toString();
    if (id.isEmpty() || identifiers.contains(id)) throw std::runtime_error("Empty or duplicate expected case ID.");
    identifiers.insert(id);
    if (entry.contains(QStringLiteral("target")) && !entry.value(QStringLiteral("target")).isString() && !entry.value(QStringLiteral("target")).isNull())
      throw std::runtime_error("Target case ID must be a string or an explicit null removal.");
    if (entry.contains(QStringLiteral("target")) && entry.value(QStringLiteral("target")).isNull()) {
      if (entry.value(QStringLiteral("reason")).toString().trimmed().isEmpty()) throw std::runtime_error("Removing a case requires its disposition reason.");
      removed.append(entry);
      continue;
    }
    const QString target = entry.value(QStringLiteral("target")).toString(id);
    if (target.isEmpty()) throw std::runtime_error("Empty target case ID.");
    if (!actual.rows.contains(target) || actual.rows.value(target) != QLatin1String("pass"))
      missing.append(QJsonObject{{QStringLiteral("id"), id}, {QStringLiteral("target"), target}});
  }
  if (actual.rows.isEmpty()) problems.append(QStringLiteral("no test results supplied"));
  return {{QStringLiteral("passed"), problems.isEmpty() && missing.isEmpty()},
          {QStringLiteral("executedRows"), actual.rows.size()},
          {QStringLiteral("expectedRows"), expected.size()},
          {QStringLiteral("lifecycleRecords"), actual.lifecycleRecords},
          {QStringLiteral("missingOrFailed"), missing},
          {QStringLiteral("removed"), removed},
          {QStringLiteral("problems"), QJsonArray::fromStringList(problems)}};
}

}  // namespace uwf::test::tools
