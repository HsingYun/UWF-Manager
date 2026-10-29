// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "app/ports/WmiOperations.h"
namespace uwf::test::application {
// A recording transport may be resolved from several worker threads. Adapters
// share owned state, so a queued operation never borrows a stack fixture.
struct RecordingWmiState {
  std::recursive_mutex mutex;
  std::deque<std::vector<uwf::app::wmi::WmiRow>> queryResults;
  std::deque<uwf::app::wmi::WmiRow> objectResults;
  std::deque<uwf::app::wmi::WmiMethodOutput> readResults;
  std::vector<QString> invocations;
  std::vector<QString> invocationPaths;
  std::vector<uwf::app::wmi::WmiRow> invocationInputs;
  std::exception_ptr invocationFailure;
  std::exception_ptr queryFailure;
  std::exception_ptr workerAccessFailure;
  std::exception_ptr readFailure;
  std::function<void()> invocationObserver;
  std::function<void()> queryObserver;
  std::function<void()> workerAccessObserver;
  QString connectionFailure;
};
class RecordingWmiOperations final : public uwf::app::wmi::WmiOperations {
 private:
  std::shared_ptr<RecordingWmiState> m_state;

 public:
  RecordingWmiOperations() : RecordingWmiOperations(std::make_shared<RecordingWmiState>()) {}
  explicit RecordingWmiOperations(std::shared_ptr<RecordingWmiState> state)
      : m_state(std::move(state)),
        queryResults(m_state->queryResults),
        objectResults(m_state->objectResults),
        readResults(m_state->readResults),
        invocations(m_state->invocations),
        invocationPaths(m_state->invocationPaths),
        invocationInputs(m_state->invocationInputs),
        invocationFailure(m_state->invocationFailure),
        queryFailure(m_state->queryFailure),
        workerAccessFailure(m_state->workerAccessFailure),
        readFailure(m_state->readFailure),
        invocationObserver(m_state->invocationObserver),
        queryObserver(m_state->queryObserver),
        workerAccessObserver(m_state->workerAccessObserver),
        connectionFailure(m_state->connectionFailure) {}
  RecordingWmiOperations(const RecordingWmiOperations&) = delete;
  RecordingWmiOperations& operator=(const RecordingWmiOperations&) = delete;
  std::deque<std::vector<uwf::app::wmi::WmiRow>>& queryResults;
  std::deque<uwf::app::wmi::WmiRow>& objectResults;
  std::deque<uwf::app::wmi::WmiMethodOutput>& readResults;
  std::vector<QString>& invocations;
  std::vector<QString>& invocationPaths;
  std::vector<uwf::app::wmi::WmiRow>& invocationInputs;
  std::exception_ptr& invocationFailure;
  std::exception_ptr& queryFailure;
  std::exception_ptr& workerAccessFailure;
  std::exception_ptr& readFailure;
  std::function<void()>& invocationObserver;
  std::function<void()>& queryObserver;
  std::function<void()>& workerAccessObserver;
  QString& connectionFailure;
  std::function<uwf::app::wmi::WmiOperations&()> threadAccessor() override {
    return [worker = std::make_shared<RecordingWmiOperations>(m_state)]() -> uwf::app::wmi::WmiOperations& {
      const std::scoped_lock lock(worker->m_state->mutex);
      if (worker->workerAccessObserver) worker->workerAccessObserver();
      if (worker->workerAccessFailure) std::rethrow_exception(worker->workerAccessFailure);
      return *worker;
    };
  }

  void ensureConnected() const override {
    const std::scoped_lock lock(m_state->mutex);
    if (!connectionFailure.isEmpty()) throw std::runtime_error(connectionFailure.toStdString());
  }
  std::vector<uwf::app::wmi::WmiRow> query(const std::string&) const override { return takeQuery(); }
  std::vector<uwf::app::wmi::WmiRow> queryInstances(const std::string&) const override { return takeQuery(); }
  uwf::app::wmi::WmiClassStatus classStatus(const std::string&) const override { return uwf::app::wmi::WmiClassStatus::Present; }
  uwf::app::wmi::WmiRow getObject(const std::string&) const override {
    const std::scoped_lock lock(m_state->mutex);
    if (objectResults.empty()) throw std::runtime_error("no object result queued");
    auto row = std::move(objectResults.front());
    objectResults.pop_front();
    return row;
  }
  void invokeMethod(const std::string& path, const std::string& method, const uwf::app::wmi::WmiRow& inputs) const override {
    const std::scoped_lock lock(m_state->mutex);
    if (invocationObserver) invocationObserver();
    invocationPaths.push_back(QString::fromStdString(path));
    invocations.push_back(QString::fromStdString(method));
    invocationInputs.push_back(inputs);
    if (invocationFailure) std::rethrow_exception(std::exchange(invocationFailure, std::exception_ptr{}));
  }
  uwf::app::wmi::WmiMethodOutput callMethodRead(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&) const override {
    const std::scoped_lock lock(m_state->mutex);
    if (readFailure) std::rethrow_exception(readFailure);
    if (readResults.empty()) throw std::runtime_error("unexpected callMethodRead");
    auto result = std::move(readResults.front());
    readResults.pop_front();
    return result;
  }
  uwf::app::wmi::WmiMethodOutput callMethodReadCancelable(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&,
                                                          std::stop_token) const override {
    throw std::runtime_error("unexpected callMethodReadCancelable");
  }
  void putInstance(const std::string& className, const uwf::app::wmi::WmiRow& values, uwf::app::wmi::WmiPutMode) const override {
    invokeMethod(className, "PutInstance", values);
  }

 private:
  std::vector<uwf::app::wmi::WmiRow> takeQuery() const {
    const std::scoped_lock lock(m_state->mutex);
    if (queryObserver) queryObserver();
    if (queryFailure) std::rethrow_exception(queryFailure);
    if (queryResults.empty()) throw std::runtime_error("no query result queued");
    auto result = std::move(queryResults.front());
    queryResults.pop_front();
    return result;
  }
};

}  // namespace uwf::test::application
