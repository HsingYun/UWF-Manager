// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>

namespace uwf::app {

inline constexpr wchar_t kEnhancedServiceName[] = L"UWFManagerEnhanced";
inline constexpr wchar_t kEnhancedServiceDisplayName[] = L"UWF Manager Enhanced Mode";
inline constexpr wchar_t kEnhancedPipeName[] = L"\\\\.\\pipe\\UWFManager.EnhancedService.v3";
inline constexpr char kEnhancedServiceRegistryKey[] = R"(HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)";
inline constexpr std::array<char, 8> kEnhancedAgentHandshake{'U', 'W', 'F', 'A', 'G', 'T', '0', '3'};

enum class EnhancedAgentControl : std::uint8_t {
  PreshutdownCommitHandled = 1,
  PreshutdownCommitRequired = 2,
};

struct EnhancedAgentControlMessage {
  EnhancedAgentControl control = EnhancedAgentControl::PreshutdownCommitRequired;
  std::uint64_t requestId = 0;

  [[nodiscard]] bool operator==(const EnhancedAgentControlMessage&) const = default;
};

inline constexpr std::size_t kEnhancedAgentControlFrameSize = 1 + sizeof(std::uint64_t);
using EnhancedAgentControlFrame = std::array<std::uint8_t, kEnhancedAgentControlFrameSize>;

[[nodiscard]] constexpr EnhancedAgentControlFrame encodeEnhancedAgentControl(const EnhancedAgentControlMessage message) {
  if (message.requestId == 0) throw std::invalid_argument("enhanced agent control request ID cannot be zero");
  if (message.control != EnhancedAgentControl::PreshutdownCommitHandled && message.control != EnhancedAgentControl::PreshutdownCommitRequired) {
    throw std::invalid_argument("enhanced agent control is invalid");
  }
  EnhancedAgentControlFrame frame{};
  frame[0] = static_cast<std::uint8_t>(message.control);
  for (std::size_t index = 0; index < sizeof(message.requestId); ++index) {
    frame[index + 1] = static_cast<std::uint8_t>((message.requestId >> (index * 8U)) & 0xffU);
  }
  return frame;
}

[[nodiscard]] constexpr std::optional<EnhancedAgentControlMessage> decodeEnhancedAgentControl(const EnhancedAgentControlFrame& frame) {
  const auto rawControl = frame[0];
  if (rawControl != static_cast<std::uint8_t>(EnhancedAgentControl::PreshutdownCommitHandled) &&
      rawControl != static_cast<std::uint8_t>(EnhancedAgentControl::PreshutdownCommitRequired)) {
    return std::nullopt;
  }

  std::uint64_t requestId = 0;
  for (std::size_t index = 0; index < sizeof(requestId); ++index) {
    requestId |= static_cast<std::uint64_t>(frame[index + 1]) << (index * 8U);
  }
  if (requestId == 0) return std::nullopt;
  return EnhancedAgentControlMessage{static_cast<EnhancedAgentControl>(rawControl), requestId};
}

}  // namespace uwf::app
