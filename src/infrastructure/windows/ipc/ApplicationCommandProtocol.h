// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QByteArray>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>

#include "application/startup/ApplicationCommand.h"

namespace uwf::windows {
inline constexpr std::size_t kApplicationCommandHeaderSize = 24;
inline constexpr std::size_t kApplicationCommandMaximumFrameSize = kApplicationCommandHeaderSize + 2 * sizeof(std::uint16_t);
inline constexpr auto kApplicationCommandExchangeTimeout = std::chrono::seconds{2};

struct ApplicationCommandRequest {
  app::ApplicationCommandKind kind = app::ApplicationCommandKind::Activate;
  std::uint64_t requestId = 0;
  bool operator==(const ApplicationCommandRequest&) const = default;
};

// Incomplete frames produce no decoded value; malformed frames throw. The
// transport owns buffering, peer rejection and exchange deadlines.
class ApplicationCommandProtocolError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};
[[nodiscard]] QByteArray encodeCommandRequest(const ApplicationCommandRequest& request);
[[nodiscard]] QByteArray encodeCommandAcknowledgement(std::uint64_t requestId);
[[nodiscard]] std::optional<ApplicationCommandRequest> decodeCommandRequest(const QByteArray& bytes);
[[nodiscard]] std::optional<std::uint64_t> decodeCommandAcknowledgement(const QByteArray& bytes);
[[nodiscard]] std::size_t applicationCommandFrameSize(const QByteArray& bytes);
}  // namespace uwf::windows
