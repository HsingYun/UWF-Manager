// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <atomic>
#include <functional>
#include <memory>
#include <stop_token>
#include <type_traits>
#include <utility>

#include "app/common/BackgroundWork.h"

namespace uwf::presentation {
template <typename Value>
using AsyncResult = app::BackgroundResult<Value>;

// Workers own only their inputs and promise. QObject lifetime and completion
// delivery stay on the receiver thread; cancellation never joins a worker.
class AsyncTask final : public QObject {
 public:
  explicit AsyncTask(QObject* parent = nullptr) : QObject(parent) {}
  ~AsyncTask() override { cancel(); }
  [[nodiscard]] bool isRunning() const noexcept { return m_run && m_run->deliver && m_run->running; }
  void cancel() noexcept {
    if (!m_run) return;
    m_run->deliver = false;
    m_run->stop.request_stop();
  }
  // A write may already be in flight. Keep its completion report even when
  // cancellation requests that subsequent targets remain untouched.
  void requestStop() noexcept {
    if (m_run) m_run->stop.request_stop();
  }

  template <typename Work, typename Completion>
  void start(Work work, Completion completion) {
    using Value = std::invoke_result_t<Work, std::stop_token>;
    using Result = AsyncResult<Value>;
    cancel();
    auto run = m_run = std::make_shared<Run>();
    const auto token = run->stop.get_token();
    auto* watcher = new QFutureWatcher<Result>(this);
    connect(watcher, &QFutureWatcher<Result>::finished, this, [watcher, run, completion = std::move(completion)]() mutable {
      watcher->deleteLater();
      run->running = false;
      if (run->deliver) std::invoke(completion, watcher->future().takeResult());
    });
    watcher->setFuture(app::launchBackground(std::move(work), token));
  }

 private:
  struct Run {
    std::stop_source stop;
    std::atomic_bool deliver{true};
    bool running = true;
  };
  std::shared_ptr<Run> m_run;
};
}  // namespace uwf::presentation
