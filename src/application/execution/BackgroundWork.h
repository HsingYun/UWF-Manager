// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QFuture>
#include <QPromise>
#include <QThreadPool>
#include <exception>
#include <functional>
#include <memory>
#include <stop_token>
#include <type_traits>
#include <utility>
#include <variant>

namespace uwf::app {
template <typename Value>
using BackgroundResult = std::variant<Value, std::exception_ptr>;

// The caller supplies owned inputs and a cooperative stop token. Neither the
// future nor its destruction joins the worker; results contain no receiver.
template <typename Work>
auto launchBackground(Work work, const std::stop_token stop = {}) {
  using Value = std::invoke_result_t<Work, std::stop_token>;
  using Result = BackgroundResult<Value>;
  auto promise = std::make_shared<QPromise<Result>>();
  promise->start();
  auto future = promise->future();
  try {
    QThreadPool::globalInstance()->start([promise, stop, work = std::move(work)]() mutable {
      try {
        promise->addResult(Result{std::in_place_index<0>, std::invoke(work, stop)});
      } catch (...) {
        promise->addResult(Result{std::in_place_index<1>, std::current_exception()});
      }
      promise->finish();
    });
  } catch (...) {
    promise->addResult(Result{std::in_place_index<1>, std::current_exception()});
    promise->finish();
  }
  return future;
}

template <typename Value>
Value takeBackgroundResult(QFuture<BackgroundResult<Value>>& future) {
  auto result = future.takeResult();
  if (const auto* error = std::get_if<std::exception_ptr>(&result)) std::rethrow_exception(*error);
  return std::get<Value>(std::move(result));
}
}  // namespace uwf::app
