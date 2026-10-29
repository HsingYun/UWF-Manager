// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTextStream>
#include <stdexcept>

#include "TestResults.h"

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QCommandLineParser parser;
  parser.setApplicationDescription(QStringLiteral("Reconcile actual QtTest XML records with an optional migration inventory."));
  parser.addHelpOption();
  parser.addOption(
      {QStringLiteral("expected"), QStringLiteral("Optional JSON object with a cases array; never generated from this run's passes."), QStringLiteral("file")});
  parser.addOption({QStringLiteral("report"), QStringLiteral("JSON output path."), QStringLiteral("file")});
  parser.addPositionalArgument(QStringLiteral("results"), QStringLiteral("QtTest XML result files."), QStringLiteral("results..."));
  parser.process(app);
  try {
    using namespace uwf::test::tools;
    if (parser.positionalArguments().isEmpty()) throw std::runtime_error("Specify QtTest XML result files.");
    TestResults actual;
    for (const auto paths = parser.positionalArguments(); const auto& path : paths) {
      QFile file(path);
      if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error(file.errorString().toStdString());
      const auto results = readTestResults(file);
      actual.lifecycleRecords += results.lifecycleRecords;
      actual.problems.append(results.problems);
      for (auto row = results.rows.cbegin(); row != results.rows.cend(); ++row) {
        if (actual.rows.contains(row.key())) throw std::runtime_error((QStringLiteral("Duplicate result across input files: ") + row.key()).toStdString());
        actual.rows.insert(row.key(), row.value());
      }
    }
    QJsonArray expected;
    if (parser.isSet(QStringLiteral("expected"))) {
      QFile file(parser.value(QStringLiteral("expected")));
      if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error(file.errorString().toStdString());
      QJsonParseError error;
      const auto document = QJsonDocument::fromJson(file.readAll(), &error);
      if (error.error != QJsonParseError::NoError || !document.isObject() || !document.object().value(QStringLiteral("cases")).isArray())
        throw std::runtime_error("Expected JSON must contain a cases array.");
      expected = document.object().value(QStringLiteral("cases")).toArray();
      if (expected.isEmpty()) throw std::runtime_error("Expected cases cannot be empty.");
    }
    const auto report = reconcileTestResults(actual, expected);
    const auto json = QJsonDocument(report).toJson();
    if (parser.isSet(QStringLiteral("report"))) {
      QSaveFile file(parser.value(QStringLiteral("report")));
      if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit()) throw std::runtime_error("Cannot write result report.");
    }
    QTextStream(stdout) << json;
    return report.value(QStringLiteral("passed")).toBool() ? 0 : 1;
  } catch (const std::exception& error) {
    QTextStream(stderr) << error.what() << '\n';
    return 2;
  }
}
