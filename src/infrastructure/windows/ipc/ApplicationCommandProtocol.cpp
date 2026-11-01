// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/ipc/ApplicationCommandProtocol.h"

#include <QtEndian>
#include <algorithm>
#include <array>
#include <type_traits>

namespace uwf::windows {
namespace {
constexpr std::array<char, 4> magic{'U', 'W', 'F', 'C'};
constexpr std::uint16_t protocolVersion = 2;
constexpr std::size_t headerSize = kApplicationCommandHeaderSize;
constexpr std::size_t requestPayloadSize = kApplicationCommandMaximumFrameSize - kApplicationCommandHeaderSize;

enum class FrameKind : std::uint16_t { Request = 1, Acknowledgement = 2 };

template <class Integer>
void appendInteger(QByteArray& bytes, const Integer value) {
  static_assert(std::is_integral_v<Integer>);
  const auto encoded = qToLittleEndian(value);
  bytes.append(reinterpret_cast<const char*>(&encoded), static_cast<qsizetype>(sizeof(encoded)));
}

template <class Integer>
Integer readInteger(const QByteArray& bytes, const std::size_t offset) {
  static_assert(std::is_integral_v<Integer>);
  if (offset > static_cast<std::size_t>(bytes.size()) || sizeof(Integer) > static_cast<std::size_t>(bytes.size()) - offset)
    throw ApplicationCommandProtocolError("application command frame is truncated");
  return qFromLittleEndian<Integer>(reinterpret_cast<const uchar*>(bytes.constData() + static_cast<qsizetype>(offset)));
}

struct Header {
  FrameKind kind;
  std::uint64_t requestId;
  std::size_t size;
};

std::optional<Header> readHeader(const QByteArray& bytes) {
  if (static_cast<std::size_t>(bytes.size()) < headerSize) return std::nullopt;
  if (!std::equal(magic.begin(), magic.end(), bytes.constData())) throw ApplicationCommandProtocolError("application command frame has an invalid signature");
  if (readInteger<std::uint16_t>(bytes, 4) != protocolVersion) throw ApplicationCommandProtocolError("application command protocol version is unsupported");
  if (readInteger<std::uint32_t>(bytes, 20) != 0) throw ApplicationCommandProtocolError("application command frame reserved field is nonzero");
  const auto kind = static_cast<FrameKind>(readInteger<std::uint16_t>(bytes, 6));
  const auto payload = readInteger<std::uint32_t>(bytes, 16);
  if ((kind != FrameKind::Request && kind != FrameKind::Acknowledgement) || payload != (kind == FrameKind::Request ? requestPayloadSize : 0))
    throw ApplicationCommandProtocolError("application command frame kind or payload is invalid");
  const auto requestId = readInteger<std::uint64_t>(bytes, 8);
  if (!requestId) throw ApplicationCommandProtocolError("application command request ID cannot be zero");
  return Header{kind, requestId, headerSize + payload};
}

QByteArray makeFrame(const FrameKind kind, const std::uint64_t requestId, const QByteArray& payload) {
  if (!requestId) throw ApplicationCommandProtocolError("application command request ID cannot be zero");
  QByteArray bytes;
  bytes.reserve(static_cast<qsizetype>(headerSize) + payload.size());
  bytes.append(magic.data(), static_cast<qsizetype>(magic.size()));
  appendInteger(bytes, protocolVersion);
  appendInteger(bytes, static_cast<std::uint16_t>(kind));
  appendInteger(bytes, requestId);
  appendInteger(bytes, static_cast<std::uint32_t>(payload.size()));
  appendInteger(bytes, std::uint32_t{0});
  bytes.append(payload);
  return bytes;
}

std::optional<Header> completeFrame(const QByteArray& bytes, const FrameKind kind) {
  const auto header = readHeader(bytes);
  if (!header) return {};
  if (header->kind != kind) throw ApplicationCommandProtocolError("application command frame has the wrong direction");
  if (static_cast<std::size_t>(bytes.size()) < header->size) return {};
  if (static_cast<std::size_t>(bytes.size()) != header->size) throw ApplicationCommandProtocolError("application command connection contains trailing data");
  return header;
}
}  // namespace

QByteArray encodeCommandRequest(const ApplicationCommandRequest& request) {
  switch (request.kind) {
    case app::ApplicationCommandKind::Activate:
    case app::ApplicationCommandKind::EnsureRunning:
      break;
    default:
      throw ApplicationCommandProtocolError("application command kind is invalid");
  }
  QByteArray payload;
  appendInteger(payload, static_cast<std::uint16_t>(request.kind));
  appendInteger(payload, std::uint16_t{0});
  return makeFrame(FrameKind::Request, request.requestId, payload);
}

std::optional<ApplicationCommandRequest> decodeCommandRequest(const QByteArray& bytes) {
  const auto header = completeFrame(bytes, FrameKind::Request);
  if (!header) return {};
  if (readInteger<std::uint16_t>(bytes, headerSize + 2) != 0) throw ApplicationCommandProtocolError("application command request reserved field is nonzero");
  const auto kind = static_cast<app::ApplicationCommandKind>(readInteger<std::uint16_t>(bytes, headerSize));
  switch (kind) {
    case app::ApplicationCommandKind::Activate:
    case app::ApplicationCommandKind::EnsureRunning:
      return ApplicationCommandRequest{kind, header->requestId};
    default:
      throw ApplicationCommandProtocolError("application command kind is invalid");
  }
}

QByteArray encodeCommandAcknowledgement(const std::uint64_t requestId) { return makeFrame(FrameKind::Acknowledgement, requestId, {}); }

std::optional<std::uint64_t> decodeCommandAcknowledgement(const QByteArray& bytes) {
  const auto header = completeFrame(bytes, FrameKind::Acknowledgement);
  return header ? std::optional{header->requestId} : std::nullopt;
}

std::size_t applicationCommandFrameSize(const QByteArray& bytes) {
  const auto header = readHeader(bytes);
  return header ? header->size : 0;
}
}  // namespace uwf::windows
