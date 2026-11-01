// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <stop_token>

#include "application/execution/BackgroundWork.h"
#include "application/overlay/contracts/OverlayFileLoader.h"

namespace uwf::app {
// Owns enumeration independently of windows. All public methods run on the
// object's thread; callbacks are queued to the subscribing receiver's thread.
class OverlayFilesQueryManager final : public QObject {
  struct Observer;
  struct Job;

 public:
  using Entries = std::shared_ptr<const QVector<OverlayFileEntry>>;
  using Result = BackgroundResult<Entries>;
  using Completion = std::function<void(Result)>;

  class Subscription final {
   public:
    Subscription() = default;
    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;
    Subscription(Subscription&& other) noexcept;
    Subscription& operator=(Subscription&& other) noexcept;
    ~Subscription();
    void cancel() noexcept;

   private:
    friend class OverlayFilesQueryManager;
    explicit Subscription(std::shared_ptr<Observer> observer);
    std::shared_ptr<Observer> m_observer;
  };

  explicit OverlayFilesQueryManager(OverlayFileLoader loader, QObject* parent = nullptr);
  ~OverlayFilesQueryManager() override;
  [[nodiscard]] Subscription subscribe(const QString& volumeGuid, QObject* receiver, Completion completion);
  // Close admission before requesting client shutdown. This does not claim to
  // interrupt work already executing in the Windows provider.
  void shutdown() noexcept;

 private:
  static void deliver(const std::shared_ptr<Observer>& observer, Result result);
  void finish(const std::shared_ptr<Job>& job, Result result);
  OverlayFileLoader m_loader;
  std::map<QString, std::shared_ptr<Job>> m_jobs;
  std::stop_source m_shutdown;
  std::shared_ptr<std::atomic_bool> m_delivery = std::make_shared<std::atomic_bool>(true);
  bool m_closed = false;
};
}  // namespace uwf::app
