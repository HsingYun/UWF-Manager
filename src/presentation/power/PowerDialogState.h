// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <array>
#include <stdexcept>

#include "app/api/PowerInteraction.h"
namespace uwf::presentation {
class PowerDialogState final {
 public:
  enum class State {
    Preparing,
    Confirmation,
    Staging,
    FailureDecision,
    Confirmed,
    Canceled,
    Count,
  };

  enum class Event {
    PreparationReady,
    BeginStaging,
    StagingCompleted,
    FailureObserved,
    Confirm,
    Cancel,
    Count,
  };

  explicit PowerDialogState(const bool preparationRequired) : m_state(preparationRequired ? State::Preparing : State::Confirmation) {}

  [[nodiscard]] State state() const { return m_state; }
  [[nodiscard]] bool terminal() const { return m_state == State::Confirmed || m_state == State::Canceled; }

  void post(const Event event) {
    const Transition transition = transitions().at(index(m_state, event));
    if (!transition.valid) throw std::logic_error("invalid power dialog state transition");
    m_state = transition.next;
    if (event == Event::FailureObserved) m_failureObserved = true;
  }

  [[nodiscard]] app::PowerActionDialogOutcome outcome() const {
    if (!terminal()) throw std::logic_error("power dialog outcome requested before a terminal state");
    if (m_state == State::Confirmed) {
      return m_failureObserved ? app::PowerActionDialogOutcome::ContinuedAfterStagingFailure : app::PowerActionDialogOutcome::Confirmed;
    }
    return m_failureObserved ? app::PowerActionDialogOutcome::CanceledAfterStagingFailure : app::PowerActionDialogOutcome::Canceled;
  }

 private:
  struct Transition {
    State next = State::Canceled;
    bool valid = false;
  };

  static constexpr std::size_t stateCount = static_cast<std::size_t>(State::Count);
  static constexpr std::size_t eventCount = static_cast<std::size_t>(Event::Count);

  static constexpr std::size_t index(const State state, const Event event) {
    return static_cast<std::size_t>(state) * eventCount + static_cast<std::size_t>(event);
  }

  static const std::array<Transition, stateCount * eventCount>& transitions() {
    static constexpr auto table = [] {
      std::array<Transition, stateCount * eventCount> result{};
      const auto set = [&result](const State from, const Event event, const State to) { result[index(from, event)] = {to, true}; };

      set(State::Preparing, Event::PreparationReady, State::Confirmation);
      set(State::Preparing, Event::FailureObserved, State::FailureDecision);
      set(State::Preparing, Event::Cancel, State::Canceled);

      set(State::Confirmation, Event::BeginStaging, State::Staging);
      set(State::Confirmation, Event::FailureObserved, State::FailureDecision);
      set(State::Confirmation, Event::Confirm, State::Confirmed);
      set(State::Confirmation, Event::Cancel, State::Canceled);

      set(State::Staging, Event::StagingCompleted, State::Confirmed);
      set(State::Staging, Event::FailureObserved, State::FailureDecision);

      set(State::FailureDecision, Event::Confirm, State::Confirmed);
      set(State::FailureDecision, Event::Cancel, State::Canceled);
      return result;
    }();
    return table;
  }

  State m_state;
  bool m_failureObserved = false;
};

}  // namespace uwf::presentation
