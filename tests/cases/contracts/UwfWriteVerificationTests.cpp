// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>

#include "app/uwf/WriteVerification.h"
#include "app/wmi/WmiException.h"

namespace {

QString text(const std::string& value) { return QString::fromUtf8(value); }

class UwfWriteVerificationTests final : public QObject {
  Q_OBJECT

 private slots:
  void writeVerificationConfirmsDefinitiveAndUncertainOutcomes();
  void writeVerificationPreservesRejectionAndRereadFailure();
};

void UwfWriteVerificationTests::writeVerificationConfirmsDefinitiveAndUncertainOutcomes() {
  int invokes = 0;
  int observes = 0;
  uwf::app::provider::invokeAndConfirm(
      "set filter", [&] { ++invokes; },
      [&] {
        ++observes;
        return true;
      });
  QCOMPARE(invokes, 1);
  QCOMPARE(observes, 1);

  uwf::app::provider::invokeAndConfirm(
      "set filter",
      [&] {
        ++invokes;
        throw uwf::app::wmi::WmiInvocationUncertain("set filter", "connection lost after dispatch");
      },
      [&] {
        ++observes;
        return true;
      });
  QCOMPARE(invokes, 2);
  QCOMPARE(observes, 2);

  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiStateVerificationError, uwf::app::provider::invokeAndConfirm("set filter", [] {}, [] { return false; }));
  QVERIFY_THROWS_EXCEPTION(
      uwf::app::wmi::WmiStateVerificationError,
      uwf::app::provider::invokeAndConfirm("set filter", [] { throw uwf::app::wmi::WmiInvocationUncertain("set filter", "lost"); }, [] { return false; }));
}

void UwfWriteVerificationTests::writeVerificationPreservesRejectionAndRereadFailure() {
  bool observed = false;
  try {
    uwf::app::provider::invokeAndConfirm(
        "protect volume", [] { throw uwf::app::wmi::WmiProviderError(87, "protect volume", "invalid parameter"); },
        [&] {
          observed = true;
          return true;
        });
    QFAIL("provider rejection must propagate");
  } catch (const uwf::app::wmi::WmiProviderError& error) {
    QCOMPARE(error.returnValue(), uint32_t{87});
  }
  QVERIFY(!observed);

  bool nestedPreserved = false;
  try {
    uwf::app::provider::invokeAndConfirm("protect volume", [] {}, []() -> bool { throw std::runtime_error("reread failed"); });
    QFAIL("reread failure must be wrapped");
  } catch (const uwf::app::wmi::WmiStateVerificationError& error) {
    QCOMPARE(text(error.operation()), QStringLiteral("protect volume"));
    try {
      std::rethrow_if_nested(error);
    } catch (const std::runtime_error& nested) {
      nestedPreserved = std::string(nested.what()) == "reread failed";
    }
  }
  QVERIFY(nestedPreserved);
}

}  // namespace

QTEST_APPLESS_MAIN(UwfWriteVerificationTests)

#include "UwfWriteVerificationTests.moc"
