// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <type_traits>
#include <utility>

namespace uwf::windows {

// Owns initialized COM MTA workers; startup propagates initialization failures.
// Destruction drains accepted work and joins workers, so the last owner must
// be released outside the pool. invoke does not pump caller GUI/COM events;
// work must not depend synchronously on that caller or return apartment-bound resources.
class MtaExecutor final {
 public:
  MtaExecutor();
  explicit MtaExecutor(std::size_t workerCount);
  ~MtaExecutor();
  MtaExecutor(const MtaExecutor&) = delete;
  MtaExecutor& operator=(const MtaExecutor&) = delete;

  template <typename Work>
  [[nodiscard]] auto submit(Work work) -> std::future<std::invoke_result_t<Work>> {
    using Value = std::invoke_result_t<Work>;
    auto task = std::make_shared<std::packaged_task<Value()>>([ownedWork = std::make_unique<Work>(std::move(work))]() mutable -> Value {
      // A future may outlive the pool; release callable captures on the MTA
      // independently of the packaged task's lifetime.
      auto current = std::move(ownedWork);
      return std::invoke(std::move(*current));
    });
    auto result = task->get_future();
    enqueue([task = std::move(task)] { (*task)(); });
    return result;
  }

  template <typename Work>
  auto invoke(Work work) -> std::invoke_result_t<Work> {
    // Inline reentry avoids waiting behind the current pool operation.
    if (isCurrentThread()) return std::invoke(std::move(work));
    return submit(std::move(work)).get();
  }

  // Release apartment-bound members before their owner's executor lease.
  // Cleanup must not throw. Scheduling failure terminates rather than releasing
  // those members on the caller's apartment; this operation may allocate.
  template <typename Work>
  void finalize(Work work) noexcept {
    try {
      invoke(std::move(work));
    } catch (...) {
      std::terminate();
    }
  }

  [[nodiscard]] bool isCurrentThread() const noexcept;

 private:
  void enqueue(std::function<void()> work);
  struct State;
  std::unique_ptr<State> m_state;
};

// Keep this shared pool's owning lease until synchronous calls finish.
[[nodiscard]] std::shared_ptr<MtaExecutor> systemMtaExecutor();

}  // namespace uwf::windows
