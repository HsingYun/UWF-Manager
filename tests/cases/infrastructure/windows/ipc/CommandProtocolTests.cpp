// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include "infrastructure/windows/ipc/ApplicationCommandProtocol.h"
#include "infrastructure/windows/ipc/EnhancedModeProtocol.h"

namespace {
class CommandProtocolTests final : public QObject {
  Q_OBJECT
 private slots:
  void activationProtocolHasLiteralFramesAndCorrelatedAcknowledgements() {
    using namespace uwf::app;
    using namespace uwf::windows;
    constexpr auto requestId = std::uint64_t{0x0102030405060708};
    const auto expectedRequest = QByteArray::fromHex("55 57 46 43 02 00 01 00 08 07 06 05 04 03 02 01 04 00 00 00 00 00 00 00 01 00 00 00");
    const auto expectedAck = QByteArray::fromHex("55 57 46 43 02 00 02 00 08 07 06 05 04 03 02 01 00 00 00 00 00 00 00 00");
    QCOMPARE(encodeCommandRequest({ApplicationCommandKind::Activate, requestId}), expectedRequest);
    QCOMPARE(encodeCommandAcknowledgement(requestId), expectedAck);
    QCOMPARE(applicationCommandFrameSize(expectedRequest), std::size_t{28});
    QCOMPARE(applicationCommandFrameSize(expectedAck), std::size_t{24});
    for (const auto kind : {ApplicationCommandKind::Activate, ApplicationCommandKind::EnsureRunning})
      for (const auto id : {std::uint64_t{1}, requestId, std::numeric_limits<std::uint64_t>::max()}) {
        const ApplicationCommandRequest request{kind, id};
        const auto bytes = encodeCommandRequest(request);
        QCOMPARE(decodeCommandRequest(bytes), std::optional{request});
        QCOMPARE(decodeCommandAcknowledgement(encodeCommandAcknowledgement(id)), std::optional{id});
        for (qsizetype length = 0; length < bytes.size(); ++length) QVERIFY(!decodeCommandRequest(bytes.left(length)));
        const auto ack = encodeCommandAcknowledgement(id);
        for (qsizetype length = 0; length < ack.size(); ++length) QVERIFY(!decodeCommandAcknowledgement(ack.left(length)));
      }
    QVERIFY(decodeCommandAcknowledgement(encodeCommandAcknowledgement(requestId - 1)) != std::optional{requestId});
    QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(expectedAck)));
    QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandAcknowledgement(expectedRequest)));
    QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(encodeCommandRequest({ApplicationCommandKind::Activate, 0})));
    QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(encodeCommandAcknowledgement(0)));
    QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(encodeCommandRequest({static_cast<ApplicationCommandKind>(3), 1})));
  }

  void activationProtocolRejectsEveryMalformedHeaderAndRetiredCommand_data() {
    QTest::addColumn<int>("fault");
    QTest::addColumn<bool>("acknowledgement");
    for (const bool ack : {false, true})
      for (int fault = 0; fault < 8; ++fault)
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/fault-%2").arg(ack ? "ack" : "request").arg(fault))) << fault << ack;
  }

  void activationProtocolRejectsEveryMalformedHeaderAndRetiredCommand() {
    using namespace uwf::app;
    using namespace uwf::windows;
    QFETCH(int, fault);
    QFETCH(bool, acknowledgement);
    auto bytes = acknowledgement ? encodeCommandAcknowledgement(1) : encodeCommandRequest({ApplicationCommandKind::EnsureRunning, 1});
    switch (fault) {
      case 0:
        bytes[0] = 'X';
        break;
      case 1:
        bytes[4] = '\1';  // Retired v1 batch-command protocol.
        break;
      case 2:
        bytes[6] = '\3';
        break;
      case 3:
        std::fill(bytes.begin() + 8, bytes.begin() + 16, '\0');
        break;
      case 4:
        bytes[16] = '\5';
        break;
      case 5:
        bytes[20] = '\1';
        break;
      case 6:
        bytes.append('x');
        break;
      case 7:
        if (acknowledgement) {
          bytes[16] = '\4';  // ACK never carries a legacy batch result.
        } else {
          bytes[24] = '\3';  // Retired CommitStage wire selector.
        }
        break;
    }
    if (acknowledgement)
      QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandAcknowledgement(bytes)));
    else
      QVERIFY_THROWS_EXCEPTION(ApplicationCommandProtocolError, static_cast<void>(decodeCommandRequest(bytes)));
  }

  void enhancedAgentControlProtocolCorrelatesAcknowledgements() {
    using namespace uwf::windows;
    constexpr EnhancedAgentControlMessage handled{EnhancedAgentControl::PreshutdownCommitHandled, 0x0102030405060708ULL};
    constexpr auto handledFrame = encodeEnhancedAgentControl(handled);
    static_assert(handledFrame == EnhancedAgentControlFrame{1, 8, 7, 6, 5, 4, 3, 2, 1});
    static_assert(decodeEnhancedAgentControl(handledFrame) == std::optional{handled});
    constexpr EnhancedAgentControlMessage required{EnhancedAgentControl::PreshutdownCommitRequired, std::numeric_limits<std::uint64_t>::max()};
    constexpr auto requiredFrame = encodeEnhancedAgentControl(required);
    static_assert(decodeEnhancedAgentControl(requiredFrame) == std::optional{required});
    auto invalid = handledFrame;
    invalid[0] = 0;
    QVERIFY(!decodeEnhancedAgentControl(invalid));
    invalid = handledFrame;
    std::ranges::fill(invalid.begin() + 1, invalid.end(), std::uint8_t{0});
    QVERIFY(!decodeEnhancedAgentControl(invalid));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(encodeEnhancedAgentControl({EnhancedAgentControl::PreshutdownCommitHandled, 0})));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument,
                             static_cast<void>(encodeEnhancedAgentControl({static_cast<EnhancedAgentControl>(std::numeric_limits<std::uint8_t>::max()), 1})));
    constexpr EnhancedAgentControlMessage previous{EnhancedAgentControl::PreshutdownCommitHandled, 41};
    constexpr EnhancedAgentControlMessage current{EnhancedAgentControl::PreshutdownCommitHandled, 42};
    QVERIFY(decodeEnhancedAgentControl(encodeEnhancedAgentControl(previous)) != std::optional{current});
    QCOMPARE(decodeEnhancedAgentControl(encodeEnhancedAgentControl(current)), std::optional{current});
  }
};
}  // namespace

QTEST_APPLESS_MAIN(CommandProtocolTests)
#include "CommandProtocolTests.moc"
