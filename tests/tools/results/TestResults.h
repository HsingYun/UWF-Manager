// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QStringList>

namespace uwf::test::tools {

struct TestResults {
  QMap<QString, QString> rows;
  qsizetype lifecycleRecords = 0;
  QStringList problems;
};

TestResults readTestResults(QIODevice& input);
QJsonObject reconcileTestResults(const TestResults& actual, const QJsonArray& expected);

}  // namespace uwf::test::tools
