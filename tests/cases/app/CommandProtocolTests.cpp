// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/enhanced/EnhancedModeProtocol.h"
#include "app/startup/ApplicationCommand.h"

namespace {

class CommandProtocolTests final : public QObject {
  Q_OBJECT

 private slots:
  void applicationCommandProtocolRejectsTruncationAndCorruption();
  void enhancedAgentControlProtocolCorrelatesAcknowledgements();
};

void CommandProtocolTests::applicationCommandProtocolRejectsTruncationAndCorruption() {
  using namespace uwf::app;
  const ApplicationCommandRequest request{ApplicationCommandKind::CommitStage, std::numeric_limits<std::uint64_t>::max()};
  const QByteArray requestBytes = encodeCommandRequest(request);
  QCOMPARE(decodeCommandRequest(requestBytes), std::optional<ApplicationCommandRequest>{request});
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(encodeCommandRequest({ApplicationCommandKind::Activate, 0})));

  for (qsizetype length = 0; length < requestBytes.size(); ++length) {
    QVERIFY(!decodeCommandRequest(requestBytes.left(length)).has_value());
  }
  QByteArray corrupt = requestBytes;
  corrupt[0] = 'X';
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(corrupt)));
  corrupt = requestBytes;
  corrupt[4] = '\2';
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(corrupt)));
  corrupt = requestBytes;
  corrupt[6] = '\0';
  corrupt[7] = '\0';
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(corrupt)));
  corrupt = requestBytes;
  corrupt[20] = '\1';
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(corrupt)));
  corrupt = requestBytes;
  std::fill(corrupt.begin() + 8, corrupt.begin() + 16, '\0');
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(corrupt)));
  corrupt = requestBytes;
  corrupt[26] = '\1';
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(corrupt)));
  corrupt = requestBytes;
  corrupt.append('x');
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(corrupt)));

  const ApplicationCommandResult result{ApplicationCommandOutcome::CompletedWithFailures, 10, 7, 2, 1, 3, QStringLiteral("路径失败")};
  const QByteArray resultBytes = encodeCommandResult(request.requestId, result);
  const auto decoded = decodeCommandResult(resultBytes);
  QVERIFY(decoded.has_value());
  QCOMPARE(decoded->first, request.requestId);
  QCOMPARE(decoded->second.outcome, result.outcome);
  QCOMPARE(decoded->second.discoveredFiles, result.discoveredFiles);
  QCOMPARE(decoded->second.committedFiles, result.committedFiles);
  QCOMPARE(decoded->second.skippedFiles, result.skippedFiles);
  QCOMPARE(decoded->second.skippedEntries, result.skippedEntries);
  QCOMPARE(decoded->second.failedFiles, result.failedFiles);
  QCOMPARE(decoded->second.detail, result.detail);
  QVERIFY(decoded->second.completed());

  const auto infrastructure = ApplicationCommandResult::infrastructureFailure(QStringLiteral("批次基础设施失败\n没有逐目标结果"));
  const auto infrastructureRoundtrip = decodeCommandResult(encodeCommandResult(request.requestId, infrastructure));
  QVERIFY(infrastructureRoundtrip.has_value());
  QCOMPARE(infrastructureRoundtrip->second.outcome, ApplicationCommandOutcome::Failed);
  QCOMPARE(infrastructureRoundtrip->second.failedFiles, std::size_t{0});
  QCOMPARE(infrastructureRoundtrip->second.detail, infrastructure.detail);
  QVERIFY(!infrastructureRoundtrip->second.completed());

  corrupt = resultBytes;
  corrupt[corrupt.size() - 1] = static_cast<char>(0xFF);
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandResult(corrupt)));
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError,
                           static_cast<void>(encodeCommandResult(request.requestId, {ApplicationCommandOutcome::Succeeded, 1, 1, 0, 0, 1, {}})));
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError,
                           static_cast<void>(encodeCommandResult(request.requestId, {ApplicationCommandOutcome::CompletedWithFailures, 1, 0, 1, 0, 0, {}})));
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError,
                           static_cast<void>(encodeCommandResult(request.requestId, {ApplicationCommandOutcome::Succeeded, 1, 1, 1, 0, 0, {}})));
  corrupt = resultBytes;
  corrupt[24] = static_cast<char>(ApplicationCommandOutcome::Succeeded);
  corrupt[25] = '\0';
  QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandResult(corrupt)));

  const QString oversizedDetail(70 * 1024, QChar(u'界'));
  const auto oversized =
      decodeCommandResult(encodeCommandResult(request.requestId, {ApplicationCommandOutcome::CompletedWithFailures, 1, 0, 0, 0, 1, oversizedDetail}));
  QVERIFY(oversized.has_value());
  QCOMPARE(oversized->second.outcome, ApplicationCommandOutcome::CompletedWithFailures);
  QCOMPARE(oversized->second.failedFiles, std::size_t{1});
  QVERIFY(oversized->second.detail.toUtf8().size() <= 64 * 1024 - 48);
  QVERIFY(oversized->second.detail.endsWith(QStringLiteral("\n…")));
}

void CommandProtocolTests::enhancedAgentControlProtocolCorrelatesAcknowledgements() {
  using namespace uwf::app;

  constexpr EnhancedAgentControlMessage handled{EnhancedAgentControl::PreshutdownCommitHandled, 0x0102030405060708ULL};
  constexpr auto handledFrame = encodeEnhancedAgentControl(handled);
  static_assert(handledFrame == EnhancedAgentControlFrame{1, 8, 7, 6, 5, 4, 3, 2, 1});
  static_assert(decodeEnhancedAgentControl(handledFrame) == std::optional{handled});

  constexpr EnhancedAgentControlMessage required{EnhancedAgentControl::PreshutdownCommitRequired, std::numeric_limits<std::uint64_t>::max()};
  constexpr auto requiredFrame = encodeEnhancedAgentControl(required);
  static_assert(decodeEnhancedAgentControl(requiredFrame) == std::optional{required});

  auto invalid = handledFrame;
  invalid[0] = 0;
  QVERIFY(!decodeEnhancedAgentControl(invalid).has_value());
  invalid = handledFrame;
  std::ranges::fill(invalid.begin() + 1, invalid.end(), std::uint8_t{0});
  QVERIFY(!decodeEnhancedAgentControl(invalid).has_value());
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(encodeEnhancedAgentControl({EnhancedAgentControl::PreshutdownCommitHandled, 0})));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument,
                           static_cast<void>(encodeEnhancedAgentControl({static_cast<EnhancedAgentControl>(std::numeric_limits<std::uint8_t>::max()), 1})));

  // 确认必须关联完整请求，而不能只比较动作。相同动作的延迟 ACK 仍属于
  // 旧请求，不得放行下一次安全电源操作。
  constexpr EnhancedAgentControlMessage previous{EnhancedAgentControl::PreshutdownCommitHandled, 41};
  constexpr EnhancedAgentControlMessage current{EnhancedAgentControl::PreshutdownCommitHandled, 42};
  QVERIFY(decodeEnhancedAgentControl(encodeEnhancedAgentControl(previous)) != std::optional{current});
  QCOMPARE(decodeEnhancedAgentControl(encodeEnhancedAgentControl(current)), std::optional{current});
}

}  // namespace

QTEST_APPLESS_MAIN(CommandProtocolTests)

#include "CommandProtocolTests.moc"
