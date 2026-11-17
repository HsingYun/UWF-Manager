// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTextStream>
#include <stdexcept>

#include "ImageCompare.h"

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QCommandLineParser parser;
  parser.setApplicationDescription(QStringLiteral("Compare two fresh captures without a stored image baseline."));
  parser.addHelpOption();
  parser.addOption({QStringLiteral("reference"), QStringLiteral("Fresh reference capture directory."), QStringLiteral("directory")});
  parser.addOption({QStringLiteral("actual"), QStringLiteral("Fresh actual capture directory."), QStringLiteral("directory")});
  parser.addOption(
      {QStringLiteral("expected-count"), QStringLiteral("Optional scene inventory count for this run."), QStringLiteral("count"), QStringLiteral("0")});
  parser.addOption({QStringLiteral("report"), QStringLiteral("JSON output path."), QStringLiteral("file")});
  parser.process(app);
  QTextStream errors(stderr);
  try {
    bool valid = false;
    const auto expected = parser.value(QStringLiteral("expected-count")).toLongLong(&valid);
    if (!valid || !parser.isSet(QStringLiteral("reference")) || !parser.isSet(QStringLiteral("actual")))
      throw std::runtime_error("Specify --reference and --actual, and a valid --expected-count.");
    const auto report = uwf::test::tools::compareImageDirectories(parser.value(QStringLiteral("reference")), parser.value(QStringLiteral("actual")), expected);
    const auto json = QJsonDocument(report).toJson();
    if (parser.isSet(QStringLiteral("report"))) {
      QSaveFile file(parser.value(QStringLiteral("report")));
      if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit()) throw std::runtime_error("Cannot write comparison report.");
    }
    QTextStream(stdout) << json;
    return report.value(QStringLiteral("passed")).toBool() ? 0 : 1;
  } catch (const std::exception& error) {
    errors << error.what() << '\n';
    return 2;
  }
}
