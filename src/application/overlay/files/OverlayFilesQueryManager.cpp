// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/overlay/files/OverlayFilesQueryManager.h"

#include <QFutureWatcher>
#include <QPointer>
#include <QThread>
#include <algorithm>
#include <atomic>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "application/overlay/files/OverlayFilesFailure.h"
#include "domain/uwf/contracts/Types.h"

namespace uwf::app {
struct OverlayFilesQueryManager::Observer {
  QPointer<QObject> receiver;
  Completion completion;
  std::weak_ptr<std::atomic_bool> delivery;
  std::atomic_bool active{true};
};
struct OverlayFilesQueryManager::Job {
  QString volume;
  std::vector<std::weak_ptr<Observer>> observers;
  // Losing the completion channel does not establish provider quiescence.
  // Keep that failure as the same-volume admission barrier.
  std::optional<Result> unresolved;
};

OverlayFilesQueryManager::Subscription::Subscription(std::shared_ptr<Observer> observer) : m_observer(std::move(observer)) {}
OverlayFilesQueryManager::Subscription::Subscription(Subscription&& other) noexcept = default;
OverlayFilesQueryManager::Subscription& OverlayFilesQueryManager::Subscription::operator=(Subscription&& other) noexcept {
  if (this != &other) {
    cancel();
    m_observer = std::move(other.m_observer);
  }
  return *this;
}
OverlayFilesQueryManager::Subscription::~Subscription() { cancel(); }
void OverlayFilesQueryManager::Subscription::cancel() noexcept {
  if (m_observer) m_observer->active = false;
  m_observer.reset();
}

OverlayFilesQueryManager::OverlayFilesQueryManager(OverlayFileLoader loader, QObject* parent) : QObject(parent), m_loader(std::move(loader)) {
  if (!m_loader) throw std::invalid_argument("An overlay enumeration loader is required");
}
OverlayFilesQueryManager::~OverlayFilesQueryManager() { shutdown(); }
void OverlayFilesQueryManager::shutdown() noexcept {
  m_closed = true;
  *m_delivery = false;
  m_shutdown.request_stop();
  for (const auto& [volume, job] : m_jobs) {
    (void)volume;
    for (const auto& weak : job->observers)
      if (const auto observer = weak.lock()) observer->active = false;
  }
  m_jobs.clear();
}

OverlayFilesQueryManager::Subscription OverlayFilesQueryManager::subscribe(const QString& volumeGuid, QObject* receiver, Completion completion) {
  if (QThread::currentThread() != thread() || !receiver || receiver->thread() != thread())
    throw std::invalid_argument("Overlay enumeration subscriptions require a receiver on the manager thread");
  if (m_closed) throw std::logic_error("The overlay enumeration manager has shut down");
  if (!completion) throw std::invalid_argument("An overlay enumeration completion callback is required");
  const auto identity = compat::parseVolumeId(volumeGuid.toStdString());
  if (!identity) throw std::invalid_argument("Overlay enumeration requires a complete volume GUID");
  const auto volume = QString::fromStdString(*identity);
  auto observer = std::make_shared<Observer>();
  observer->receiver = receiver;
  observer->completion = std::move(completion);
  observer->delivery = m_delivery;
  if (const auto existing = m_jobs.find(volume); existing != m_jobs.end()) {
    const auto& job = existing->second;
    std::erase_if(job->observers, [](const auto& value) {
      const auto prior = value.lock();
      return !prior || !prior->active || !prior->receiver;
    });
    job->observers.push_back(observer);
    if (job->unresolved) deliver(observer, *job->unresolved);
    return Subscription(std::move(observer));
  }
  auto job = std::make_shared<Job>();
  job->volume = volume;
  job->observers.push_back(observer);
  m_jobs.emplace(volume, job);
  QFutureWatcher<Result>* watcher = nullptr;
  try {
    watcher = new QFutureWatcher<Result>(this);
    connect(watcher, &QFutureWatcher<Result>::finished, this, [this, watcher, job] {
      watcher->deleteLater();
      finish(job, watcher->future().takeResult());
    });
    watcher->setFuture(launchBackground(
        [loader = m_loader, volume](const std::stop_token stop) -> Entries { return std::make_shared<const QVector<OverlayFileEntry>>(loader(volume, stop)); },
        m_shutdown.get_token()));
  } catch (...) {
    delete watcher;
    m_jobs.erase(volume);
    throw;
  }
  return Subscription(std::move(observer));
}

void OverlayFilesQueryManager::deliver(const std::shared_ptr<Observer>& observer, Result result) {
  if (!observer->active || !observer->receiver) return;
  QMetaObject::invokeMethod(
      observer->receiver,
      [weak = std::weak_ptr(observer), result = std::move(result)]() mutable {
        const auto current = weak.lock();
        const auto delivery = current ? current->delivery.lock() : nullptr;
        if (current && delivery && *delivery && current->active && current->receiver) {
          // One terminal result per subscription. A callback may detach itself
          // or subscribe again without invalidating its currently executing copy.
          current->active = false;
          auto completion = std::move(current->completion);
          completion(std::move(result));
        }
      },
      Qt::QueuedConnection);
}

void OverlayFilesQueryManager::finish(const std::shared_ptr<Job>& job, Result result) {
  if (m_closed) return;
  bool unknown = false;
  if (const auto* failure = std::get_if<std::exception_ptr>(&result)) {
    try {
      std::rethrow_exception(*failure);
    } catch (const OverlayFilesQueryUncertain&) {
      unknown = true;
    } catch (...) {
    }
  }
  if (unknown)
    job->unresolved = result;
  else
    m_jobs.erase(job->volume);
  for (const auto& weak : job->observers)
    if (const auto observer = weak.lock()) deliver(observer, result);
  if (!unknown) job->observers.clear();
}
}  // namespace uwf::app
