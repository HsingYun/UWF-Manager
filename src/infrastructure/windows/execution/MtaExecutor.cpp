// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/execution/MtaExecutor.h"

#include <Windows.h>
#include <combaseapi.h>

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <exception>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <vector>

namespace uwf::windows {
namespace {
// Permit independent calls to overlap even when hardware concurrency is unknown.
constexpr std::size_t kMinimumMtaWorkerCount = 2;
std::size_t defaultWorkerCount() noexcept { return std::max(kMinimumMtaWorkerCount, static_cast<std::size_t>(std::thread::hardware_concurrency())); }

class MtaApartment final {
 public:
  MtaApartment() {
    const auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(result)) throw std::system_error(static_cast<int>(result), std::system_category(), "Initialize MTA executor COM apartment");
  }
  ~MtaApartment() { CoUninitialize(); }
  MtaApartment(const MtaApartment&) = delete;
  MtaApartment& operator=(const MtaApartment&) = delete;
};
}  // namespace

struct MtaExecutor::State final {
  explicit State(const std::size_t workerCount) {
    if (workerCount == 0) throw std::invalid_argument("An MTA executor requires at least one worker");
    std::vector<std::future<void>> initialization;
    initialization.reserve(workerCount);
    workers.reserve(workerCount);
    try {
      for (std::size_t index = 0; index < workerCount; ++index) {
        std::promise<void> ready;
        initialization.push_back(ready.get_future());
        workers.emplace_back([this, ready = std::move(ready)]() mutable {
          std::optional<MtaApartment> apartment;
          try {
            apartment.emplace();
          } catch (...) {
            ready.set_exception(std::current_exception());
            return;
          }
          const WorkerMembership membership(this);
          ready.set_value();
          drain();
          // Callable captures are released before the COM apartment.
        });
      }
      for (auto& ready : initialization) ready.get();
    } catch (...) {
      // Wake initialized workers on startup failure; jthread stop alone does
      // not wake this condition-variable queue.
      stopAndJoin();
      throw;
    }
  }

  ~State() { stopAndJoin(); }

  void stopAndJoin() noexcept {
    // Self-join violates the executor's owner-lifetime contract.
    if (current == this) std::terminate();
    {
      const std::lock_guard lock(mutex);
      accepting = false;
    }
    changed.notify_all();
    for (auto& worker : workers)
      if (worker.joinable()) worker.join();
  }

  void enqueue(std::function<void()> work) {
    {
      const std::lock_guard lock(mutex);
      if (!accepting) throw std::runtime_error("The MTA executor is stopping");
      pending.push_back(std::move(work));
    }
    changed.notify_one();
  }

  void drain() {
    for (;;) {
      std::function<void()> work;
      {
        std::unique_lock lock(mutex);
        changed.wait(lock, [this] { return !accepting || !pending.empty(); });
        if (pending.empty()) return;
        work = std::move(pending.front());
        pending.pop_front();
      }
      work();
    }
  }

  struct WorkerMembership final {
    explicit WorkerMembership(const State* state) : previous(current) { current = state; }
    ~WorkerMembership() { current = previous; }
    WorkerMembership(const WorkerMembership&) = delete;
    WorkerMembership& operator=(const WorkerMembership&) = delete;
    const State* previous;
  };
  inline static thread_local const State* current = nullptr;

  std::mutex mutex;
  std::condition_variable changed;
  std::deque<std::function<void()>> pending;
  bool accepting = true;
  // Declared last so destruction joins workers before releasing shared state.
  std::vector<std::jthread> workers;
};

MtaExecutor::MtaExecutor() : MtaExecutor(defaultWorkerCount()) {}
MtaExecutor::MtaExecutor(const std::size_t workerCount) : m_state(std::make_unique<State>(workerCount)) {}
MtaExecutor::~MtaExecutor() {
  // Keep membership observable while accepted callables and their captures are
  // drained. unique_ptr destruction can clear m_state before deleting State.
  m_state->stopAndJoin();
}
void MtaExecutor::enqueue(std::function<void()> work) { m_state->enqueue(std::move(work)); }
bool MtaExecutor::isCurrentThread() const noexcept { return State::current == m_state.get(); }

std::shared_ptr<MtaExecutor> systemMtaExecutor() {
  static const auto executor = std::make_shared<MtaExecutor>();
  return executor;
}
}  // namespace uwf::windows
