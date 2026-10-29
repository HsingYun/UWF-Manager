// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QHash>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "app/commit/CommitPreparation.h"
namespace uwf::test::commit {
template <typename Waiting>
void waitForRelease(Waiting waiting) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{10};
  while (waiting()) {
    if (std::chrono::steady_clock::now() >= deadline) throw std::runtime_error("controlled worker was not released before the test deadline");
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
  }
}
class Tree final : public app::CommitTraversalSource {
 public:
  app::CommitNode root, branch, leaf;
  QHash<QString, QList<app::CommitNode>> children;
  std::atomic_bool fail{false}, block{false}, release{false}, ignoreStop{false}, failAfterChildren{false};
  std::function<void()> afterChild;
  mutable std::atomic_int reads{0}, failedReads{0};
  explicit Tree(bool registry) {
    root = {registry ? app::CommitTargetKind::RegistryKey : app::CommitTargetKind::Folder,
            registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Test") : QStringLiteral("C:\\Test"),
            {},
            {}};
    branch = {root.kind, root.path + QStringLiteral("\\Branch"), {}, {}};
    leaf = {registry ? app::CommitTargetKind::RegistryValue : app::CommitTargetKind::File,
            registry ? root.path : root.path + QStringLiteral("\\one.txt"),
            registry ? QStringLiteral("One") : QString(),
            {}};
    auto nested = leaf;
    nested.path = registry ? branch.path : branch.path + QStringLiteral("\\two.txt");
    children.insert(root.identity(), {branch, leaf});
    children.insert(branch.identity(), {nested});
  }
  std::optional<app::CommitNode> inspect(const app::CommitNode& node) const override {
    return node.path == root.path && node.kind == app::CommitTargetKind::File ? root : node;
  }
  bool exists(const app::CommitNode&) const override { return true; }
  std::string volumeForPath(const QString&) const override { return "C:"; }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {"C:"}; }
  QString read(const app::CommitNode& node, std::stop_token stop, const app::CommitNodeVisitor& discover) const override {
    ++reads;
    waitForRelease([&] { return block && !release && (ignoreStop || !stop.stop_requested()); });
    if (node.identity() == branch.identity() && fail) {
      ++failedReads;
      return QStringLiteral("access denied (0x80070005)");
    }
    const auto childNodes = children.value(node.identity());
    for (const auto& child : childNodes) {
      if (stop.stop_requested()) break;
      discover(child);
      if (afterChild) afterChild();
    }
    if (node.identity() == root.identity() && failAfterChildren) return QStringLiteral("enumeration interrupted");
    return {};
  }
};
class Backend final : public app::CommitBackend {
 public:
  std::shared_ptr<Tree> tree;
  std::atomic_int writes{0};
  std::atomic_bool blockWrite{false}, releaseWrite{false};
  std::atomic_bool failPrepare{false}, failWrites{false}, throwWrite{false};
  QList<app::CommitTargetResult> results;
  std::thread::id preparationThread, executionThread;
  explicit Backend(bool registry) : tree(std::make_shared<Tree>(registry)) {}
  app::CommitPlan prepare(const app::CommitRequest& request, app::CommitPlan previous, std::stop_token stop,
                          const app::CommitPreparationProgress& progress) override {
    preparationThread = std::this_thread::get_id();
    if (failPrepare) throw std::runtime_error("preparation unavailable");
    return app::prepareCommit(request, *tree, std::move(previous), stop, progress);
  }
  app::CommitTargetResult execute(const app::CommitNode&, app::CommitOperation) override {
    executionThread = std::this_thread::get_id();
    const int attempt = ++writes;
    waitForRelease([&] { return blockWrite && !releaseWrite; });
    if (throwWrite) throw std::runtime_error("write outcome unknown");
    if (!results.isEmpty()) return results.at(attempt - 1);
    app::CommitTargetResult result;
    result.succeeded = !failWrites;
    if (failWrites) result.failureDetail = QStringLiteral("provider rejected");
    return result;
  }
  app::CommitRequest request(app::CommitOperation operation = app::CommitOperation::Persist) const { return {operation, tree->root}; }
};
}  // namespace uwf::test::commit
