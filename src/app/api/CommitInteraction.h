// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QList>
#include <QString>
#include <functional>
#include <memory>

#include "CommitResult.h"
namespace uwf::app {
enum class CommitTargetKind { File, Folder, RegistryKey, RegistryValue };
struct CommitBatchRequest {
  int total = 0;
  CommitOperation operation = CommitOperation::Persist;
  std::function<QString(int)> displayOf;
  std::function<CommitTargetResult(int, const QString&)> execute;
};
struct CommitRequest;
class CommitBackend;
// Requests and backends are owned values across asynchronous task boundaries.
struct CommitInteraction {
  std::function<void(const CommitRequest&, std::shared_ptr<CommitBackend>)> run;
};
}  // namespace uwf::app
