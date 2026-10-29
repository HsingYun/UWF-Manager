// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QByteArray>
#include <QString>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>

namespace uwf::app {

enum class ApplicationCommandKind : std::uint16_t {
  Activate = 1,
  EnsureRunning = 2,
  CommitStage = 3,
};

enum class ApplicationCommandOutcome : std::uint16_t {
  Succeeded = 1,
  CompletedWithFailures = 2,
  Rejected = 3,
  Failed = 4,
};

struct ApplicationCommandResult {
  ApplicationCommandOutcome outcome = ApplicationCommandOutcome::Succeeded;
  std::size_t discoveredFiles = 0;
  std::size_t committedFiles = 0;
  std::size_t skippedFiles = 0;
  std::size_t skippedEntries = 0;
  std::size_t failedFiles = 0;
  QString detail;

  [[nodiscard]] static ApplicationCommandResult infrastructureFailure(QString detail) {
    ApplicationCommandResult result;
    result.outcome = ApplicationCommandOutcome::Failed;
    result.detail = std::move(detail);
    return result;
  }

  [[nodiscard]] bool completed() const {
    return outcome == ApplicationCommandOutcome::Succeeded || outcome == ApplicationCommandOutcome::CompletedWithFailures;
  }
};

struct ApplicationCommandRequest {
  ApplicationCommandKind kind = ApplicationCommandKind::Activate;
  std::uint64_t requestId = 0;

  bool operator==(const ApplicationCommandRequest&) const = default;
};

// UI 单实例管道使用有界、带版本和请求 ID 的协议。
// 解码器区分“不完整帧”和“无效帧”：前者等待后续字节，后者直接拒绝连接。
class ApplicationCommandProtocolError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

[[nodiscard]] QByteArray encodeCommandRequest(const ApplicationCommandRequest& request);
[[nodiscard]] QByteArray encodeCommandResult(std::uint64_t requestId, const ApplicationCommandResult& result);

[[nodiscard]] std::optional<ApplicationCommandRequest> decodeCommandRequest(const QByteArray& bytes);
[[nodiscard]] std::optional<std::pair<std::uint64_t, ApplicationCommandResult>> decodeCommandResult(const QByteArray& bytes);
[[nodiscard]] std::size_t applicationCommandFrameSize(const QByteArray& bytes);

}  // namespace uwf::app
