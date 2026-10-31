// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <qt_windows.h>
#include <wbemcli.h>

#include <QTest>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "app/uwf/UwfFilter.h"
#include "app/uwf/UwfOverlay.h"
#include "app/uwf/UwfOverlayConfig.h"
#include "app/uwf/UwfProviderTypes.h"
#include "app/uwf/UwfRegistryFilter.h"
#include "app/uwf/UwfStateReader.h"
#include "app/uwf/UwfVolume.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "shared/StringUtil.h"

namespace {

using namespace uwf;

struct MethodCall {
  std::string objectPath;
  std::string method;
  uwf::app::wmi::WmiRow inputs;
};

class MemoryWmiOperations final : public uwf::app::wmi::WmiOperations {
 public:
  mutable int ensureConnectedCalls = 0;
  mutable std::vector<std::string> projectionQueries;
  mutable std::vector<std::string> instanceQueries;
  mutable std::vector<std::string> objectReads;
  mutable std::deque<std::vector<uwf::app::wmi::WmiRow>> queryResults;
  mutable std::deque<uwf::app::wmi::WmiRow> objectResults;
  mutable std::deque<uwf::app::wmi::WmiMethodOutput> readResults;
  mutable std::vector<MethodCall> invocations;
  mutable std::vector<MethodCall> reads;
  mutable std::vector<std::pair<std::string, uwf::app::wmi::WmiPutMode>> puts;
  mutable std::vector<uwf::app::wmi::WmiRow> putProperties;
  mutable std::exception_ptr putFailure;
  uwf::app::wmi::WmiClassStatus reportedClassStatus = uwf::app::wmi::WmiClassStatus::Present;
  mutable std::exception_ptr classStatusFailure;

  void ensureConnected() const override { ++ensureConnectedCalls; }

  [[nodiscard]] std::vector<uwf::app::wmi::WmiRow> query(const std::string& wql) const override {
    projectionQueries.push_back(wql);
    return takeQuery();
  }
  [[nodiscard]] std::vector<uwf::app::wmi::WmiRow> queryInstances(const std::string& wql) const override {
    instanceQueries.push_back(wql);
    return takeQuery();
  }
  [[nodiscard]] uwf::app::wmi::WmiClassStatus classStatus(const std::string&) const override {
    if (classStatusFailure) std::rethrow_exception(classStatusFailure);
    return reportedClassStatus;
  }

  [[nodiscard]] uwf::app::wmi::WmiRow getObject(const std::string& objectPath) const override {
    objectReads.push_back(objectPath);
    if (objectResults.empty()) throw std::runtime_error("no object result queued");
    uwf::app::wmi::WmiRow row = std::move(objectResults.front());
    objectResults.pop_front();
    return row;
  }

  void invokeMethod(const std::string& objectPath, const std::string& methodName, const uwf::app::wmi::WmiRow& inputs) const override {
    invocations.push_back({objectPath, methodName, inputs});
  }

  [[nodiscard]] uwf::app::wmi::WmiMethodOutput callMethodRead(const std::string& objectPath, const std::string& methodName,
                                                              const uwf::app::wmi::WmiRow& inputs) const override {
    reads.push_back({objectPath, methodName, inputs});
    return takeReadResult();
  }

  [[nodiscard]] uwf::app::wmi::WmiMethodOutput callMethodReadCancelable(const std::string& objectPath, const std::string& methodName,
                                                                        const uwf::app::wmi::WmiRow& inputs, std::stop_token) const override {
    reads.push_back({objectPath, methodName, inputs});
    return takeReadResult();
  }

  void putInstance(const std::string& className, const uwf::app::wmi::WmiRow& props, const uwf::app::wmi::WmiPutMode mode) const override {
    puts.emplace_back(className, mode);
    putProperties.push_back(props);
    if (putFailure) std::rethrow_exception(putFailure);
  }

 private:
  [[nodiscard]] std::vector<uwf::app::wmi::WmiRow> takeQuery() const {
    if (queryResults.empty()) throw std::runtime_error("no query result queued");
    auto rows = std::move(queryResults.front());
    queryResults.pop_front();
    return rows;
  }

  [[nodiscard]] uwf::app::wmi::WmiMethodOutput takeReadResult() const {
    if (readResults.empty()) throw std::runtime_error("no method result queued");
    auto output = std::move(readResults.front());
    readResults.pop_front();
    return output;
  }
};

uwf::app::wmi::WmiRow filterRow(const bool currentEnabled, const bool nextEnabled, const std::string& path = "filter-path") {
  return {{"__PATH", uwf::app::wmi::WmiValue::fromString(path)},
          {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(currentEnabled)},
          {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(nextEnabled)}};
}

uwf::app::wmi::WmiRow overlayRow(const uint32_t consumption = 10, const uint32_t available = 90, const uint32_t critical = 80, const uint32_t warning = 70,
                                 const std::string& path = "overlay-path") {
  return {{"__PATH", uwf::app::wmi::WmiValue::fromString(path)},
          {"OverlayConsumption", uwf::app::wmi::WmiValue::fromUInt(consumption)},
          {"AvailableSpace", uwf::app::wmi::WmiValue::fromUInt(available)},
          {"CriticalOverlayThreshold", uwf::app::wmi::WmiValue::fromUInt(critical)},
          {"WarningOverlayThreshold", uwf::app::wmi::WmiValue::fromUInt(warning)}};
}

uwf::app::wmi::WmiRow overlayConfigRow(const bool current, const uint32_t type, const uint32_t maximum, const std::string& path) {
  return {{"__PATH", uwf::app::wmi::WmiValue::fromString(path)},
          {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(current)},
          {"Type", uwf::app::wmi::WmiValue::fromUInt(type)},
          {"MaximumSize", uwf::app::wmi::WmiValue::fromUInt(maximum)}};
}

uwf::app::wmi::WmiRow registryFilterRow(const bool current, const bool domain, const bool tscal, const std::string& path) {
  return {{"__PATH", uwf::app::wmi::WmiValue::fromString(path)},
          {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(current)},
          {"PersistDomainSecretKey", uwf::app::wmi::WmiValue::fromBool(domain)},
          {"PersistTSCAL", uwf::app::wmi::WmiValue::fromBool(tscal)}};
}

uwf::app::wmi::WmiRow volumeRow(const bool current, const std::string& drive, const std::string& volume, const bool bind, const bool pending,
                                const bool protectedState, const std::string& path) {
  return {{"__PATH", uwf::app::wmi::WmiValue::fromString(path)},           {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(current)},
          {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive)},     {"VolumeName", uwf::app::wmi::WmiValue::fromString(volume)},
          {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(bind)},  {"CommitPending", uwf::app::wmi::WmiValue::fromBool(pending)},
          {"Protected", uwf::app::wmi::WmiValue::fromBool(protectedState)}};
}

class WmiApiBehaviorTests final : public QObject {
  Q_OBJECT

 private slots:
  void wmiValueConversionsRejectLossAndNonFiniteInputs();
  void wmiValueConversionMatrixCoversKindsAndBoundaries();
  void rowDecodingDistinguishesMissingNullWrongTypeAndEmpty();
  void embeddedMofDecodingHandlesEscapesAndMissingFields();
  void embeddedMofPropertiesHaveStructuralBoundaries();
  void embeddedMofUInt64LiteralsPreserveBoundaries();
  void embeddedMofRejectsMalformedAndAmbiguousProperties_data();
  void embeddedMofRejectsMalformedAndAmbiguousProperties();
  void embeddedDirectFieldsCannotBeMaskedByMof();
  void overlayMofSizeCannotComeFromAFileName_data();
  void overlayMofSizeCannotComeFromAFileName();
  void wmiErrorsPreserveKnownAndUnknownCodes();
  void apiSessionLookupHonorsSessionAndPredicate();
  void filterRequiresExactlyOneRowAndConfirmsWrites();
  void filterPowerMethodsRemainOneShotAndRequireIdentity();
  void overlayDecodesFilesAndRoutesCancelableReads();
  void overlayThresholdWritesUseTypedInputsAndAuthoritativeState();
  void overlayConfigurationRejectsInvalidAndDuplicateSessions();
  void overlayConfigurationWritesPreserveSessionIdentity();
  void registryFilterBuildsTypedInputsAndDecodesArrays();
  void registryFilterMutationsConfirmMembershipAndRouteDeletion();
  void volumeDecodingSkipsUnmanagedRowsAndRejectsDuplicates();
  void volumeMountResolutionSelectsRealInstancesRegardlessOfOrder();
  void volumeMountResolutionKeepsUnconfiguredMountedDisks();
  void volumeMountResolutionPreservesLooseNextBindings();
  void volumeMountResolutionRefreshesMountsForEachRead();
  void volumeMountResolutionRejectsAmbiguityAndLookupFailures();
  void volumeMountResolutionRoutesWritesAndRegistration();
  void volumeFileOperationsNormalizePathsAndRejectCrossVolumeTargets();
  void volumeStateAndExclusionMutationsConfirmEveryPublicOperation();
  void volumeRegistrationHandlesExistingCreatedAndUncertainOutcomes();
  void capabilityProbeDistinguishesAbsenceFromInfrastructureFailure();
  void snapshotAssemblyPreservesSessionsAndSkipsUnprotectedCurrentExclusions();
  void diskInventoryClassifiesEverySupportBoundary();
};

void WmiApiBehaviorTests::wmiValueConversionsRejectLossAndNonFiniteInputs() {
  QCOMPARE(uwf::app::wmi::WmiValue::fromBool(true).toBool(), true);
  QCOMPARE(uwf::app::wmi::WmiValue::fromInt(-1).toInt(), -1);
  QCOMPARE(uwf::app::wmi::WmiValue::fromUInt(std::numeric_limits<uint32_t>::max()).toUInt(), std::numeric_limits<uint32_t>::max());
  QCOMPARE(uwf::app::wmi::WmiValue::fromString("18446744073709551615").toULongLong(), std::numeric_limits<uint64_t>::max());
  QCOMPARE(uwf::app::wmi::WmiValue::fromString("false").toBool(), false);
  QCOMPARE(uwf::app::wmi::WmiValue::fromDouble(1.5).toDouble(), 1.5);

  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue{}.toString());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue::fromInt(-1).toUInt());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError,
                           (void)uwf::app::wmi::WmiValue::fromUInt(static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1).toUInt());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue::fromString("12x").toInt());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue::fromString("nan").toDouble());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError,
                           (void)uwf::app::wmi::WmiValue::fromDouble(std::numeric_limits<double>::infinity()).toString());
}

void WmiApiBehaviorTests::wmiValueConversionMatrixCoversKindsAndBoundaries() {
  QCOMPARE(uwf::app::wmi::WmiValue::fromBool(false).toInt64(), int64_t{0});
  QCOMPARE(uwf::app::wmi::WmiValue::fromBool(true).toULongLong(), uint64_t{1});
  QCOMPARE(uwf::app::wmi::WmiValue::fromBool(true).toDouble(), 1.0);
  QCOMPARE(uwf::app::wmi::WmiValue::fromBool(false).toString(), std::string("false"));

  QCOMPARE(uwf::app::wmi::WmiValue::fromInt(std::numeric_limits<int32_t>::min()).toInt(), std::numeric_limits<int32_t>::min());
  QCOMPARE(uwf::app::wmi::WmiValue::fromInt(std::numeric_limits<int32_t>::max()).toInt(), std::numeric_limits<int32_t>::max());
  QCOMPARE(uwf::app::wmi::WmiValue::fromInt(std::numeric_limits<int64_t>::min()).toInt64(), std::numeric_limits<int64_t>::min());
  QCOMPARE(uwf::app::wmi::WmiValue::fromInt(std::numeric_limits<int64_t>::max()).toInt64(), std::numeric_limits<int64_t>::max());
  QCOMPARE(uwf::app::wmi::WmiValue::fromUInt(std::numeric_limits<uint64_t>::max()).toULongLong(), std::numeric_limits<uint64_t>::max());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue::fromInt(std::numeric_limits<int64_t>::min()).toULongLong());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError,
                           (void)uwf::app::wmi::WmiValue::fromUInt(static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1).toInt64());

  constexpr uint64_t kLargestConsecutiveDoubleInteger = uint64_t{1} << std::numeric_limits<double>::digits;
  QCOMPARE(uwf::app::wmi::WmiValue::fromUInt(kLargestConsecutiveDoubleInteger).toDouble(), static_cast<double>(kLargestConsecutiveDoubleInteger));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue::fromUInt(kLargestConsecutiveDoubleInteger + 1).toDouble());
  QCOMPARE(uwf::app::wmi::WmiValue::fromDouble(-0.0).toBool(), false);
  QCOMPARE(uwf::app::wmi::WmiValue::fromDouble(42.0).toInt64(), int64_t{42});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue::fromDouble(42.5).toInt64());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue::fromDouble(-1.0).toULongLong());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError,
                           (void)uwf::app::wmi::WmiValue::fromDouble(std::numeric_limits<double>::quiet_NaN()).toBool());

  for (const auto* value : {"true", "TRUE", "TrUe", "false", "FALSE", "FaLsE"}) {
    QCOMPARE(uwf::app::wmi::WmiValue::fromString(value).toBool(), value[0] == 't' || value[0] == 'T');
  }
  QCOMPARE(uwf::app::wmi::WmiValue::fromString("-9223372036854775808").toInt64(), std::numeric_limits<int64_t>::min());
  QCOMPARE(uwf::app::wmi::WmiValue::fromString("18446744073709551615").toULongLong(), std::numeric_limits<uint64_t>::max());
  for (const auto* value : {"", " ", "+1", "01x", "18446744073709551616", "-9223372036854775809"}) {
    QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)uwf::app::wmi::WmiValue::fromString(value).toULongLong());
  }

  const uwf::app::wmi::WmiValue missing;
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)missing.toBool());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)missing.toInt64());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)missing.toULongLong());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)missing.toDouble());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiValueConversionError, (void)missing.toString());
}

void WmiApiBehaviorTests::rowDecodingDistinguishesMissingNullWrongTypeAndEmpty() {
  uwf::app::wmi::WmiRow row{{"Bool", uwf::app::wmi::WmiValue::fromBool(true)},
                            {"Int", uwf::app::wmi::WmiValue::fromString("-42")},
                            {"UInt", uwf::app::wmi::WmiValue::fromUInt(42)},
                            {"Text", uwf::app::wmi::WmiValue::fromString("")},
                            {"Null", uwf::app::wmi::WmiValue{}}};
  QCOMPARE(uwf::app::wmi::row::requireBool(row, "Bool"), true);
  QCOMPARE(uwf::app::wmi::row::requireBoolOrFalseIfNull(row, "Bool"), true);
  QCOMPARE(uwf::app::wmi::row::requireBoolOrFalseIfNull(row, "Null"), false);
  QCOMPARE(uwf::app::wmi::row::requireInt(row, "Int"), -42);
  QCOMPARE(uwf::app::wmi::row::requireUInt(row, "UInt"), 42u);
  QVERIFY(uwf::app::wmi::row::requireString(row, "Text").empty());
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireString(row, "Text", uwf::app::wmi::row::EmptyString::Reject));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireBool(row, "Missing"));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireBoolOrFalseIfNull(row, "Missing"));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireBoolOrFalseIfNull(row, "UInt"));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireString(row, "Null"));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireUInt(row, "Bool"));
}

void WmiApiBehaviorTests::embeddedMofDecodingHandlesEscapesAndMissingFields() {
  const std::string mof = R"(instance of X { FileName = "\\Users\"quoted\".txt"; Other = "x"; };)";
  QCOMPARE(uwf::app::wmi::row::extractFromMof(mof, "FileName"), std::string("\\Users\"quoted\".txt"));
  QCOMPARE(uwf::app::wmi::row::extractFromMof(mof, "Missing"), std::string{});
  QCOMPARE(uwf::app::wmi::row::extractFromMof(R"(X { A.B = "value"; };)", "A.B"), std::string("value"));

  uwf::app::wmi::WmiRow direct{{"FileName", uwf::app::wmi::WmiValue::fromString("\\direct")}};
  QCOMPARE(uwf::app::wmi::row::requireEmbeddedString(direct, "FileName"), std::string("\\direct"));
  uwf::app::wmi::WmiRow embedded{{"__MOF", uwf::app::wmi::WmiValue::fromString(mof)}};
  QCOMPARE(uwf::app::wmi::row::requireEmbeddedString(embedded, "FileName"), std::string("\\Users\"quoted\".txt"));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireEmbeddedString(embedded, "Absent"));
}

void WmiApiBehaviorTests::embeddedMofPropertiesHaveStructuralBoundaries() {
  const std::string mof = R"mof(
    /* FileName = "comment"; FileSize = 999; */
    INSTANCE OF UWF_OverlayFile {
      OtherFileName = "prefix";
      FileName /* between name and delimiter */ = "\\Data\\FileSize = 999;.txt";
      // FileSize = 999;
      fIlEsIzE = /* before value */ "42";
    }; // trailing comment
  )mof";
  const uwf::app::wmi::WmiRow row{{"__MOF", uwf::app::wmi::WmiValue::fromString(mof)}};
  QCOMPARE(uwf::app::wmi::row::requireEmbeddedString(row, "filename"), std::string(R"(\Data\FileSize = 999;.txt)"));
  QCOMPARE(uwf::app::wmi::row::requireEmbeddedUInt64(row, "FileSize"), uint64_t{42});
  QCOMPARE(uwf::app::wmi::row::extractFromMof(R"(OtherFileName = "prefix"; FileName = "\\real.txt";)", "FileName"), std::string("\\real.txt"));
  QCOMPARE(uwf::app::wmi::row::extractFromMof(R"(OtherFileName = "prefix";)", "FileName"), std::string{});
  QCOMPARE(uwf::app::wmi::row::extractFromMof(R"(FileName = "\\Data\unknown;/*literal*/{x}\"quoted\".txt";)", "FileName"),
           std::string(R"(\Data\unknown;/*literal*/{x}"quoted".txt)"));
  const uwf::app::wmi::WmiRow wrongType{
      {"__MOF", uwf::app::wmi::WmiValue::fromString(R"(OtherFileName = "prefix"; FileName = 42;)")}};
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireEmbeddedString(wrongType, "FileName"));
}

void WmiApiBehaviorTests::embeddedMofUInt64LiteralsPreserveBoundaries() {
  for (const auto& [literal, expected] : std::vector<std::pair<std::string, uint64_t>>{
           {"0", uint64_t{0}}, {"\"0\"", uint64_t{0}}, {"42", uint64_t{42}}, {"\"42\"", uint64_t{42}},
           {"18446744073709551615", std::numeric_limits<uint64_t>::max()}, {"\"18446744073709551615\"", std::numeric_limits<uint64_t>::max()}}) {
    const uwf::app::wmi::WmiRow row{{"__MOF", uwf::app::wmi::WmiValue::fromString("FileSize = " + literal + ";")}};
    QCOMPARE(uwf::app::wmi::row::requireEmbeddedUInt64(row, "FileSize"), expected);
  }
}

void WmiApiBehaviorTests::embeddedMofRejectsMalformedAndAmbiguousProperties_data() {
  QTest::addColumn<QString>("mof");
  QTest::newRow("missing-exact-property") << QStringLiteral("OtherFileSize = 999;");
  QTest::newRow("filename-cannot-mask-invalid-size") << QStringLiteral(R"(FileName = "\\Data\\FileSize = 999;.txt"; FileSize = "bad";)");
  QTest::newRow("quoted-empty-size") << QStringLiteral("FileSize = \"\";");
  QTest::newRow("negative") << QStringLiteral("FileSize = -1;");
  QTest::newRow("positive-sign") << QStringLiteral("FileSize = +42;");
  QTest::newRow("fraction") << QStringLiteral("FileSize = 42.0;");
  QTest::newRow("bare-overflow") << QStringLiteral("FileSize = 18446744073709551616;");
  QTest::newRow("quoted-overflow") << QStringLiteral("FileSize = \"18446744073709551616\";");
  QTest::newRow("trailing-number-text") << QStringLiteral("FileSize = 42junk;");
  QTest::newRow("missing-value") << QStringLiteral("FileSize = ;");
  QTest::newRow("missing-semicolon") << QStringLiteral("FileSize = 42");
  QTest::newRow("missing-property-separator") << QStringLiteral("FileSize = 42 Other = 1;");
  QTest::newRow("duplicate-property") << QStringLiteral("FileSize = 42; FileSize = 43;");
  QTest::newRow("case-insensitive-duplicate") << QStringLiteral("FileSize = 42; filesize = 43;");
  QTest::newRow("duplicate-unrelated-property") << QStringLiteral("Other = 1; Other = 2; FileSize = 42;");
  QTest::newRow("unterminated-instance") << QStringLiteral("instance of X { FileSize = 42;");
  QTest::newRow("trailing-instance-data") << QStringLiteral("instance of X { FileSize = 42; }; Other = 1;");
  QTest::newRow("unterminated-string") << QStringLiteral("FileSize = 42; FileName = \"unterminated;");
  QTest::newRow("unterminated-comment") << QStringLiteral("FileSize = 42; /* unterminated");
  QTest::newRow("only-in-comment") << QStringLiteral("/* FileSize = 999; */ Other = 1;");
  QTest::newRow("nested-value-is-not-a-flat-property") << QStringLiteral("Other = { FileSize = 999; }; FileSize = 42;");
  QTest::newRow("malformed-unrelated-scalar") << QStringLiteral("Other = ); FileSize = 42;");
}

void WmiApiBehaviorTests::embeddedMofRejectsMalformedAndAmbiguousProperties() {
  QFETCH(QString, mof);
  const uwf::app::wmi::WmiRow row{{"__MOF", uwf::app::wmi::WmiValue::fromString(mof.toStdString())}};
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireEmbeddedUInt64(row, "FileSize"));
}

void WmiApiBehaviorTests::embeddedDirectFieldsCannotBeMaskedByMof() {
  using Value = uwf::app::wmi::WmiValue;
  const uwf::app::wmi::WmiRow direct{{"FileName", Value::fromString("\\direct")}, {"FileSize", Value::fromUInt(42)},
                                    {"__MOF", Value::fromString("not MOF")}};
  QCOMPARE(uwf::app::wmi::row::requireEmbeddedString(direct, "FileName"), std::string("\\direct"));
  QCOMPARE(uwf::app::wmi::row::requireEmbeddedUInt64(direct, "FileSize"), uint64_t{42});
  const auto mof = Value::fromString(R"(FileName = "\\fallback"; FileSize = 999;)");
  for (const auto& value : std::vector<Value>{Value{}, Value::fromBool(true), Value::fromInt(-1), Value::fromDouble(42), Value::fromString("bad")}) {
    const uwf::app::wmi::WmiRow row{{"FileSize", value}, {"__MOF", mof}};
    QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireEmbeddedUInt64(row, "FileSize"));
  }
  for (const auto& value : std::vector<Value>{Value{}, Value::fromUInt(42), Value::fromString("")}) {
    const uwf::app::wmi::WmiRow row{{"FileName", value}, {"__MOF", mof}};
    QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)uwf::app::wmi::row::requireEmbeddedString(row, "FileName"));
  }
}

void WmiApiBehaviorTests::overlayMofSizeCannotComeFromAFileName_data() {
  QTest::addColumn<QString>("literal");
  QTest::addColumn<bool>("valid");
  QTest::newRow("real-size") << QStringLiteral("42") << true;
  QTest::newRow("invalid-real-size") << QStringLiteral("\"invalid\"") << false;
}

void WmiApiBehaviorTests::overlayMofSizeCannotComeFromAFileName() {
  QFETCH(QString, literal);
  QFETCH(bool, valid);
  MemoryWmiOperations session;
  const auto mof = std::string(R"(instance of UWF_OverlayFile { FileName = "\\Data\\FileSize = 999;.txt"; FileSize = )") + literal.toStdString() + "; };";
  uwf::app::wmi::WmiMethodOutput output;
  output.arrays["OverlayFiles"] = {{{"__MOF", uwf::app::wmi::WmiValue::fromString(mof)}}};
  session.readResults.push_back(output);
  const app::provider::UwfOverlay overlay(session);
  const app::provider::OverlayRow row{"overlay"};
  if (!valid) {
    QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)overlay.getOverlayFiles(row, "C:"));
    return;
  }
  const auto files = overlay.getOverlayFiles(row, "C:");
  QCOMPARE(files.size(), std::size_t{1});
  QCOMPARE(files.front().fileName, std::string(R"(\Data\FileSize = 999;.txt)"));
  QCOMPARE(files.front().fileSize, uint64_t{42});
}

void WmiApiBehaviorTests::wmiErrorsPreserveKnownAndUnknownCodes() {
  const uwf::app::wmi::WmiError success(0);
  QVERIFY(success.isKnown());
  QVERIFY(!success.isFailure());
  QCOMPARE(success.name(), std::string_view("S_OK"));

  const uwf::app::wmi::WmiError missing(static_cast<int32_t>(WBEM_E_NOT_FOUND));
  QVERIFY(missing.isKnown());
  QVERIFY(missing.isFailure());
  QCOMPARE(missing.code(), uwf::app::wmi::WmiErrorCode::NotFound);
  QVERIFY(!missing.description().empty());

  const uwf::app::wmi::WmiError unknown(static_cast<int32_t>(0x81234567u));
  QVERIFY(!unknown.isKnown());
  QCOMPARE(unknown.name(), std::string_view("WBEM_E_UNKNOWN"));
  QVERIFY(unknown.description().empty());
  const auto code = uwf::app::wmi::makeWmiErrorCode(static_cast<int32_t>(0x81234567u));
  QVERIFY(QString::fromStdString(code.message()).contains(QStringLiteral("81234567"), Qt::CaseInsensitive));
  const uwf::app::wmi::WmiProviderError absent(static_cast<uint32_t>(WBEM_E_NOT_FOUND), "lookup");
  QVERIFY(absent.hasCode(uwf::app::wmi::WmiErrorCode::NotFound));
  QVERIFY(!absent.hasCode(uwf::app::wmi::WmiErrorCode::Failed));
  QVERIFY(!uwf::app::wmi::WmiProviderError(2, "lookup").hasCode(uwf::app::wmi::WmiErrorCode::NotFound));
}

void WmiApiBehaviorTests::apiSessionLookupHonorsSessionAndPredicate() {
  const std::vector<app::provider::VolumeRow> rows{
      {"a", true, "C:", "v1", true, false, true}, {"b", false, "C:", "v1", true, false, false}, {"c", false, "D:", "v2", true, false, false}};
  QCOMPARE(app::provider::findBySession(rows, app::provider::Session::Current)->path, std::string("a"));
  QCOMPARE(app::provider::findBySession(rows, app::provider::Session::Next, [](const app::provider::VolumeRow& row) { return row.driveLetter == "D:"; })->path,
           std::string("c"));
  QVERIFY(!app::provider::findBySession(rows, app::provider::Session::Current, [](const app::provider::VolumeRow& row) { return row.driveLetter == "Z:"; }));
}

void WmiApiBehaviorTests::filterRequiresExactlyOneRowAndConfirmsWrites() {
  MemoryWmiOperations wmi;
  app::provider::UwfFilter filter(wmi);
  wmi.queryResults.push_back({filterRow(true, false)});
  const auto row = filter.read();
  QVERIFY(row.currentEnabled);
  QVERIFY(!row.nextEnabled);
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_Filter"});
  QVERIFY(wmi.projectionQueries.empty());

  uwf::app::wmi::WmiRow unconfigured = filterRow(true, true);
  unconfigured["CurrentEnabled"] = uwf::app::wmi::WmiValue{};
  unconfigured["NextEnabled"] = uwf::app::wmi::WmiValue{};
  wmi.queryResults.push_back({unconfigured});
  const auto unconfiguredRow = filter.read();
  QVERIFY(!unconfiguredRow.currentEnabled);
  QVERIFY(!unconfiguredRow.nextEnabled);

  wmi.objectResults.push_back(filterRow(true, true));
  filter.enable(row);
  QCOMPARE(wmi.invocations.back().method, std::string("Enable"));

  wmi.objectResults.push_back(filterRow(true, true));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiStateVerificationError, filter.disable(row));
  app::provider::FilterRow noPath = row;
  noPath.path.clear();
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, filter.restartSystem(noPath));

  wmi.queryResults.push_back({});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)filter.read());
  wmi.queryResults.push_back({filterRow(true, true), filterRow(true, false, "other")});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)filter.read());
}

void WmiApiBehaviorTests::filterPowerMethodsRemainOneShotAndRequireIdentity() {
  MemoryWmiOperations wmi;
  app::provider::UwfFilter filter(wmi);
  const app::provider::FilterRow row{"filter-path", true, true};

  filter.shutdownSystem(row);
  filter.restartSystem(row);
  QCOMPARE(wmi.invocations.size(), std::size_t{2});
  QCOMPARE(wmi.invocations[0].method, std::string("ShutdownSystem"));
  QCOMPARE(wmi.invocations[1].method, std::string("RestartSystem"));
  QVERIFY(wmi.invocations[0].inputs.empty());
  QVERIFY(wmi.invocations[1].inputs.empty());
  QVERIFY(wmi.objectResults.empty());  // 电源操作没有可安全重读的通用后置条件。

  app::provider::FilterRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, filter.shutdownSystem(missingIdentity));
  QCOMPARE(wmi.invocations.size(), std::size_t{2});
}

void WmiApiBehaviorTests::overlayDecodesFilesAndRoutesCancelableReads() {
  MemoryWmiOperations wmi;
  app::provider::UwfOverlay overlay(wmi);
  wmi.queryResults.push_back({overlayRow()});
  const auto row = overlay.read();
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_Overlay"});
  QVERIFY(wmi.projectionQueries.empty());

  uwf::app::wmi::WmiMethodOutput direct;
  direct.arrays["OverlayFiles"] = {{{"FileName", uwf::app::wmi::WmiValue::fromString("\\a.txt")}, {"FileSize", uwf::app::wmi::WmiValue::fromUInt(0)}}};
  wmi.readResults.push_back(direct);
  auto files = overlay.getOverlayFiles(row, "C:");
  QCOMPARE(files.size(), std::size_t{1});
  QCOMPARE(files.front().fileSize, uint64_t{0});
  QCOMPARE(wmi.reads.back().inputs.at("Volume").toString(), std::string("C:"));

  uwf::app::wmi::WmiMethodOutput embedded;
  embedded.arrays["OverlayFiles"] = {{{"__MOF", uwf::app::wmi::WmiValue::fromString(R"(FileName = "\\b.txt"; FileSize = "18446744073709551615";)")}}};
  wmi.readResults.push_back(embedded);
  std::stop_source source;
  files = overlay.getOverlayFiles(row, "D:", source.get_token());
  QCOMPARE(files.front().fileSize, std::numeric_limits<uint64_t>::max());

  uwf::app::wmi::WmiMethodOutput malformed;
  malformed.arrays["OverlayFiles"] = {{{"__MOF", uwf::app::wmi::WmiValue::fromString(R"(FileName = "\\bad"; FileSize = "x";)")}}};
  wmi.readResults.push_back(malformed);
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)overlay.getOverlayFiles(row, "C:"));

  uwf::app::wmi::WmiMethodOutput empty;
  empty.values.emplace("OverlayFiles", uwf::app::wmi::WmiValue{});
  wmi.readResults.push_back(empty);
  QVERIFY(overlay.getOverlayFiles(row, "C:").empty());

  wmi.readResults.emplace_back();
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)overlay.getOverlayFiles(row, "C:"));
}

void WmiApiBehaviorTests::overlayThresholdWritesUseTypedInputsAndAuthoritativeState() {
  MemoryWmiOperations wmi;
  app::provider::UwfOverlay overlay(wmi);
  const app::provider::OverlayRow row{"overlay-path", 10, 90, 80, 70};

  wmi.objectResults.push_back(overlayRow(10, 90, 80, 65));
  overlay.setWarningThreshold(row, 65);
  QCOMPARE(wmi.invocations.back().method, std::string("SetWarningThreshold"));
  QCOMPARE(wmi.invocations.back().inputs.at("size").toUInt(), 65u);

  wmi.objectResults.push_back(overlayRow(10, 90, 75, 65));
  overlay.setCriticalThreshold(row, 75);
  QCOMPARE(wmi.invocations.back().method, std::string("SetCriticalThreshold"));
  QCOMPARE(wmi.invocations.back().inputs.at("size").toUInt(), 75u);

  wmi.objectResults.push_back(overlayRow(10, 90, 74, 65));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiStateVerificationError, overlay.setCriticalThreshold(row, 75));

  app::provider::OverlayRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, overlay.setWarningThreshold(missingIdentity, 50));
}

void WmiApiBehaviorTests::overlayConfigurationRejectsInvalidAndDuplicateSessions() {
  MemoryWmiOperations wmi;
  app::provider::UwfOverlayConfig config(wmi);
  wmi.queryResults.push_back({overlayConfigRow(true, 0, 1024, "current"), overlayConfigRow(false, 1, 2048, "next")});
  QCOMPARE(config.read(app::provider::Session::Next).maximumSize, 2048u);
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_OverlayConfig"});
  QVERIFY(wmi.projectionQueries.empty());

  wmi.queryResults.push_back({overlayConfigRow(true, 2, 1024, "bad")});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)config.readAll());
  wmi.queryResults.push_back({overlayConfigRow(false, 0, 1024, "a"), overlayConfigRow(false, 0, 2048, "b")});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)config.readAll());
  wmi.queryResults.push_back({overlayConfigRow(true, 0, 1024, "current")});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)config.read(app::provider::Session::Next));
}

void WmiApiBehaviorTests::overlayConfigurationWritesPreserveSessionIdentity() {
  MemoryWmiOperations wmi;
  app::provider::UwfOverlayConfig config(wmi);
  const app::provider::OverlayConfigRow row{"next-config", false, app::provider::OverlayType::RAM, 1024};

  wmi.objectResults.push_back(overlayConfigRow(false, 1, 1024, "next-config"));
  config.setType(row, app::provider::OverlayType::Disk);
  QCOMPARE(wmi.invocations.back().method, std::string("SetType"));
  QCOMPARE(wmi.invocations.back().inputs.at("type").toUInt(), 1u);

  wmi.objectResults.push_back(overlayConfigRow(false, 1, 4096, "next-config"));
  config.setMaximumSize(row, 4096);
  QCOMPARE(wmi.invocations.back().method, std::string("SetMaximumSize"));
  QCOMPARE(wmi.invocations.back().inputs.at("size").toUInt(), 4096u);

  wmi.objectResults.push_back(overlayConfigRow(true, 1, 4096, "current-config"));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiStateVerificationError, config.setMaximumSize(row, 4096));

  app::provider::OverlayConfigRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, config.setType(missingIdentity, app::provider::OverlayType::RAM));
}

void WmiApiBehaviorTests::registryFilterBuildsTypedInputsAndDecodesArrays() {
  MemoryWmiOperations wmi;
  app::provider::UwfRegistryFilter registry(wmi);
  wmi.queryResults.push_back({registryFilterRow(true, true, false, "registry-current"), registryFilterRow(false, false, true, "registry-next")});
  const auto sessionRows = registry.readAll();
  QCOMPARE(sessionRows.size(), std::size_t{2});
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_RegistryFilter"});
  QVERIFY(wmi.projectionQueries.empty());

  const app::provider::RegistryFilterRow row{"registry-path", false, false, false};
  uwf::app::wmi::WmiMethodOutput found;
  found.values.emplace("bFound", uwf::app::wmi::WmiValue::fromBool(true));
  wmi.readResults.push_back(found);
  QVERIFY(registry.findExclusion(row, "HKLM\\SOFTWARE\\Vendor"));
  QCOMPARE(wmi.reads.back().inputs.at("RegistryKey").toString(), std::string("HKLM\\SOFTWARE\\Vendor"));

  uwf::app::wmi::WmiMethodOutput exclusions;
  exclusions.arrays["ExcludedKeys"] = {{{"RegistryKey", uwf::app::wmi::WmiValue::fromString("HKLM\\SYSTEM")}},
                                       {{"__MOF", uwf::app::wmi::WmiValue::fromString(R"(RegistryKey = "HKLM\\SOFTWARE";)")}}};
  wmi.readResults.push_back(exclusions);
  const auto rows = registry.getExclusions(row);
  QCOMPARE(rows.size(), std::size_t{2});
  QCOMPARE(rows.back().registryKey, std::string("HKLM\\SOFTWARE"));

  wmi.objectResults.push_back(registryFilterRow(false, true, false, "registry-path"));
  registry.setPersistence(row, {true, false});
  QCOMPARE(wmi.puts.back().first, std::string("UWF_RegistryFilter"));
  QCOMPARE(wmi.puts.back().second, uwf::app::wmi::WmiPutMode::UpdateOnly);
  QVERIFY(wmi.putProperties.back().at("PersistDomainSecretKey").toBool());

  registry.commitRegistry(row, "HKLM\\SYSTEM", "Name");
  QCOMPARE(wmi.invocations.back().method, std::string("CommitRegistry"));
  QCOMPARE(wmi.invocations.back().inputs.at("ValueName").toString(), std::string("Name"));
}

void WmiApiBehaviorTests::registryFilterMutationsConfirmMembershipAndRouteDeletion() {
  MemoryWmiOperations wmi;
  app::provider::UwfRegistryFilter registry(wmi);
  const app::provider::RegistryFilterRow row{"registry-path", false, false, false};
  constexpr std::string_view key = "HKLM\\SOFTWARE\\Vendor";

  uwf::app::wmi::WmiMethodOutput found;
  found.values.emplace("bFound", uwf::app::wmi::WmiValue::fromBool(true));
  wmi.readResults.push_back(found);
  registry.addExclusion(row, std::string(key));
  QCOMPARE(wmi.invocations.back().method, std::string("AddExclusion"));
  QCOMPARE(wmi.invocations.back().inputs.at("RegistryKey").toString(), std::string(key));
  QCOMPARE(wmi.reads.back().method, std::string("FindExclusion"));

  uwf::app::wmi::WmiMethodOutput absent;
  absent.values.emplace("bFound", uwf::app::wmi::WmiValue::fromBool(false));
  wmi.readResults.push_back(absent);
  registry.removeExclusion(row, std::string(key));
  QCOMPARE(wmi.invocations.back().method, std::string("RemoveExclusion"));

  uwf::app::wmi::WmiMethodOutput stillPresent;
  stillPresent.values.emplace("bFound", uwf::app::wmi::WmiValue::fromBool(true));
  wmi.readResults.push_back(stillPresent);
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiStateVerificationError, registry.removeExclusion(row, std::string(key)));

  registry.commitRegistryDeletion(row, std::string(key), {});
  QCOMPARE(wmi.invocations.back().method, std::string("CommitRegistryDeletion"));
  QCOMPARE(wmi.invocations.back().inputs.at("ValueName").toString(), std::string{});

  app::provider::RegistryFilterRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, registry.addExclusion(missingIdentity, std::string(key)));
}

void WmiApiBehaviorTests::volumeDecodingSkipsUnmanagedRowsAndRejectsDuplicates() {
  MemoryWmiOperations wmi;
  app::provider::UwfVolume volume(wmi, [](const std::string&) { return std::vector<std::string>{}; });
  uwf::app::wmi::WmiRow noDrive = volumeRow(true, "C:", "Volume{none}", true, false, false, "none");
  noDrive["DriveLetter"] = uwf::app::wmi::WmiValue{};
  wmi.queryResults.push_back(
      {volumeRow(true, "c", "Volume{c}", true, false, true, "current-c"), noDrive, volumeRow(false, "D:", "Volume{d}", false, false, false, "next-d")});
  const auto rows = volume.readAll();
  QCOMPARE(rows.size(), std::size_t{2});
  QCOMPARE(rows.front().driveLetter, std::string("C:"));
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_Volume"});
  QVERIFY(wmi.projectionQueries.empty());

  uwf::app::wmi::WmiRow unconfigured = volumeRow(true, "E:", "Volume{e}", true, false, true, "current-e");
  unconfigured["Protected"] = uwf::app::wmi::WmiValue{};
  wmi.queryResults.push_back({unconfigured});
  const auto unconfiguredRows = volume.readAll();
  QCOMPARE(unconfiguredRows.size(), std::size_t{1});
  QVERIFY(!unconfiguredRows.front().isProtected);

  // 真正重复的是完整三元组；盘符、卷名大小写不影响实例身份。
  wmi.queryResults.push_back({volumeRow(false, "C:", "Volume{c}", true, false, false, "a"), volumeRow(false, "c", "volume{C}", true, false, false, "b")});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)volume.readAll());
  wmi.queryResults.push_back({volumeRow(true, "not-a-drive", "Volume{x}", true, false, false, "x")});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiDecodeError, (void)volume.readAll());
}

void WmiApiBehaviorTests::volumeMountResolutionSelectsRealInstancesRegardlessOfOrder() {
  const std::string disk = "Volume{15a398be-0000-0000-0000-200000000000}";
  const std::string cache = "Volume{77afca1a-fb20-4853-9394-21b205f1c884}";
  const auto lookup = [&](const std::string& path) -> std::vector<std::string> {
    // 目标盘符故意不放首位，确保消歧检查全部盘符，而不是只取第一个。
    if (path == R"(\\?\)" + disk + '\\') return {"G:", "D:"};
    if (path == R"(\\?\)" + cache + '\\') return {};  // 卷仍存在，但没有盘符装入点。
    throw std::runtime_error("unexpected volume mount query");
  };

  for (const bool protectedState : {false, true}) {
    std::vector<uwf::app::wmi::WmiRow> providerRows{
        volumeRow(true, "D:", disk, true, false, protectedState, "current-disk"), volumeRow(true, "d:", cache, false, false, false, "current-cache"),
        volumeRow(false, "D:", disk, true, false, false, "next-disk"), volumeRow(false, "d:", cache, false, false, true, "next-cache")};
    providerRows[1]["Protected"] = uwf::app::wmi::WmiValue{};
    // 真实卷的 NULL 也必须保留，不能用保护状态判断哪个才是目标卷。
    if (!protectedState) providerRows[0]["Protected"] = uwf::app::wmi::WmiValue{};
    std::vector<int> order{0, 1, 2, 3};
    do {
      MemoryWmiOperations wmi;
      std::vector<uwf::app::wmi::WmiRow> shuffled;
      for (const auto index : order) shuffled.push_back(providerRows[static_cast<std::size_t>(index)]);
      wmi.queryResults.push_back(std::move(shuffled));
      app::provider::UwfVolume volume(wmi, lookup);
      const auto rows = volume.readAll();
      QCOMPARE(rows.size(), std::size_t{2});
      const auto* current = app::provider::findBySession(rows, app::provider::Session::Current);
      const auto* next = app::provider::findBySession(rows, app::provider::Session::Next);
      QVERIFY(current);
      QVERIFY(next);
      QCOMPARE(current->volumeName, disk);
      QCOMPARE(current->path, std::string("current-disk"));
      QCOMPARE(current->isProtected, protectedState);
      QCOMPARE(next->volumeName, disk);
      QCOMPARE(next->path, std::string("next-disk"));
      QVERIFY(!next->isProtected);
      QVERIFY(wmi.invocations.empty());
      QVERIFY(wmi.puts.empty());
    } while (std::next_permutation(order.begin(), order.end()));
  }
}

void WmiApiBehaviorTests::volumeMountResolutionPreservesLooseNextBindings() {
  for (const bool bindByLetter : {false, true}) {
    MemoryWmiOperations wmi;
    wmi.queryResults.push_back({volumeRow(true, "C:", "Volume{current}", true, false, true, "current"),
                                volumeRow(false, "C:", "Volume{old}", bindByLetter, false, true, "next"),
                                volumeRow(false, "C:", "Volume{retired}", false, false, false, "retired")});
    app::provider::UwfVolume volume(wmi, [](const std::string& path) -> std::vector<std::string> {
      return path == R"(\\?\Volume{current}\)" ? std::vector<std::string>{"C:"} : std::vector<std::string>{};
    });
    const auto rows = volume.readAll();
    QCOMPARE(rows.size(), bindByLetter ? std::size_t{2} : std::size_t{1});
    const auto* next = app::provider::findBySession(rows, app::provider::Session::Next);
    QCOMPARE(next != nullptr, bindByLetter);
    if (next) {
      QCOMPARE(next->path, std::string("next"));
      QVERIFY(next->bindByDriveLetter);
      QVERIFY(next->isProtected);
    }
  }

  MemoryWmiOperations ordinary;
  ordinary.queryResults.push_back(
      {volumeRow(true, "E:", "Volume{ordinary}", false, false, false, "current"), volumeRow(false, "E:", "volume{ORDINARY}", false, false, true, "next")});
  app::provider::UwfVolume ordinaryVolume(ordinary, [](const std::string&) -> std::vector<std::string> {
    throw std::runtime_error("an unambiguous volume must not require an additional mount query");
  });
  QCOMPARE(ordinaryVolume.readAll().size(), std::size_t{2});

  // 不同会话的 GUID 差异本身不是重复；用户配置的紧/松绑定都必须保留。
  for (const bool bindByLetter : {false, true}) {
    ordinary.queryResults.push_back({volumeRow(true, "E:", "Volume{mounted}", false, false, true, "current"),
                                     volumeRow(false, "E:", "Volume{configured}", bindByLetter, false, true, "next")});
    const auto independent = ordinaryVolume.readAll();
    QCOMPARE(independent.size(), std::size_t{2});
    const auto* next = app::provider::findBySession(independent, app::provider::Session::Next);
    QVERIFY(next);
    QCOMPARE(next->volumeName, std::string("Volume{configured}"));
    QCOMPARE(next->bindByDriveLetter, bindByLetter);
    QVERIFY(next->isProtected);
  }
}

void WmiApiBehaviorTests::volumeMountResolutionKeepsUnconfiguredMountedDisks() {
  MemoryWmiOperations wmi;
  std::vector<uwf::app::wmi::WmiRow> raw{
      volumeRow(true, "C:", "Volume{system-cache}", false, false, false, "system-cache"),
      volumeRow(true, "F:", "Volume{data-cache}", false, false, false, "data-cache"), volumeRow(true, "C:", "Volume{system}", false, false, false, "system"),
      volumeRow(true, "F:", "Volume{data}", false, false, false, "data"), volumeRow(true, "E:", "Volume{ordinary}", false, false, false, "ordinary")};
  for (auto& row : raw) row["Protected"] = uwf::app::wmi::WmiValue{};
  wmi.queryResults.push_back(raw);
  app::provider::UwfVolume volume(wmi, [](const std::string& path) -> std::vector<std::string> {
    if (path == R"(\\?\Volume{system}\)") return {"C:"};
    if (path == R"(\\?\Volume{data}\)") return {"F:"};
    return {};
  });
  const auto rows = volume.readAll();
  QCOMPARE(rows.size(), std::size_t{3});
  QCOMPARE(rows[0].path, std::string("system"));
  QCOMPARE(rows[1].path, std::string("data"));
  QCOMPARE(rows[2].path, std::string("ordinary"));
  for (const auto& row : rows) {
    QVERIFY(row.currentSession);
    QVERIFY(!row.isProtected);
  }
}

void WmiApiBehaviorTests::volumeMountResolutionRejectsAmbiguityAndLookupFailures() {
  const std::vector<uwf::app::wmi::WmiRow> conflicting{volumeRow(true, "C:", "Volume{one}", false, false, false, "one"),
                                                       volumeRow(true, "C:", "Volume{two}", false, false, true, "two")};
  MemoryWmiOperations wmi;
  wmi.queryResults.push_back(conflicting);
  app::provider::UwfVolume ambiguous(wmi, [](const std::string&) { return std::vector<std::string>{"C:"}; });
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)ambiguous.readAll());

  wmi.queryResults.push_back(conflicting);
  app::provider::UwfVolume failedLookup(wmi, [](const std::string&) -> std::vector<std::string> {
    throw std::system_error(std::make_error_code(std::errc::permission_denied), "read mount points");
  });
  QVERIFY_THROWS_EXCEPTION(std::system_error, (void)failedLookup.readAll());

  auto orphaned = conflicting;
  orphaned.push_back(volumeRow(true, "D:", "Volume{healthy}", false, false, true, "healthy"));
  wmi.queryResults.push_back(orphaned);
  app::provider::UwfVolume unmounted(wmi, [](const std::string&) { return std::vector<std::string>{}; });
  const auto remaining = unmounted.readAll();
  QCOMPARE(remaining.size(), std::size_t{1});
  QCOMPARE(remaining.front().path, std::string("healthy"));

  wmi.queryResults.push_back(
      {volumeRow(false, "C:", "Volume{one}", true, false, false, "next-one"), volumeRow(false, "C:", "Volume{two}", false, false, true, "next-two")});
  app::provider::UwfVolume conflictingBindings(wmi, [](const std::string&) { return std::vector<std::string>{"C:"}; });
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)conflictingBindings.readAll());
  QVERIFY(wmi.invocations.empty());
  QVERIFY(wmi.puts.empty());
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)app::provider::UwfVolume(wmi, {}));
}

void WmiApiBehaviorTests::volumeMountResolutionRefreshesMountsForEachRead() {
  const std::vector<uwf::app::wmi::WmiRow> raw{
      volumeRow(true, "C:", "Volume{one}", false, false, true, "current-one"), volumeRow(false, "C:", "volume{ONE}", false, false, true, "next-one"),
      volumeRow(true, "C:", "Volume{two}", false, false, false, "current-two"), volumeRow(false, "C:", "volume{TWO}", false, false, false, "next-two")};
  MemoryWmiOperations wmi;
  bool firstMounted = true;
  std::map<std::string, int> reads;
  app::provider::UwfVolume volume(wmi, [&](const std::string& path) -> std::vector<std::string> {
    // 卷 GUID 不区分大小写；两个会话应共用本次观察，而不是各查一次。
    const auto identity = toUpperAscii(path);
    ++reads[identity];
    const bool matches = (identity == R"(\\?\VOLUME{ONE}\)") == firstMounted;
    return matches ? std::vector<std::string>{"C:"} : std::vector<std::string>{"E:"};
  });
  wmi.queryResults.push_back(raw);
  const auto first = volume.readAll();
  QCOMPARE(first.size(), std::size_t{2});
  QCOMPARE(first[0].path, std::string("current-one"));
  QCOMPARE(first[1].path, std::string("next-one"));
  QCOMPARE(reads.at(R"(\\?\VOLUME{ONE}\)"), 1);
  QCOMPARE(reads.at(R"(\\?\VOLUME{TWO}\)"), 1);

  firstMounted = false;
  wmi.queryResults.push_back(raw);
  const auto second = volume.readAll();
  QCOMPARE(second.size(), std::size_t{2});
  QCOMPARE(second[0].path, std::string("current-two"));
  QCOMPARE(second[1].path, std::string("next-two"));
  QCOMPARE(reads.at(R"(\\?\VOLUME{ONE}\)"), 2);
  QCOMPARE(reads.at(R"(\\?\VOLUME{TWO}\)"), 2);
}

void WmiApiBehaviorTests::volumeMountResolutionRoutesWritesAndRegistration() {
  const auto disk = volumeRow(true, "C:", "Volume{disk}", false, false, true, "current-disk");
  const auto cache = volumeRow(true, "C:", "Volume{cache}", false, false, true, "current-cache");
  const auto next = volumeRow(false, "C:", "Volume{disk}", true, false, false, "next-disk");
  const auto staleNext = volumeRow(false, "C:", "Volume{cache}", false, false, false, "next-cache");
  const auto otherStaleNext = volumeRow(false, "C:", "Volume{other-cache}", false, false, false, "next-other-cache");
  const auto mounts = [](const std::string& path) -> std::vector<std::string> {
    return path == R"(\\?\Volume{disk}\)" ? std::vector<std::string>{"C:"} : std::vector<std::string>{};
  };
  MemoryWmiOperations wmi;
  app::provider::UwfVolume volume(wmi, mounts);
  wmi.queryResults.push_back({cache, disk});
  const auto currentRows = volume.readAll();
  QCOMPARE(currentRows.size(), std::size_t{1});
  volume.commitFile(currentRows.front(), R"(C:\Data\state.db)");
  QCOMPARE(wmi.invocations.back().objectPath, std::string("current-disk"));
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string(R"(\Data\state.db)"));

  // 多条残留 next 行经挂载关系确认均不属于目标盘符，才能注册真实卷。
  wmi.queryResults.push_back({cache, disk, staleNext, otherStaleNext});
  wmi.objectResults.push_back(next);
  const auto registered = volume.ensureNextSessionEntry("C:");
  QCOMPARE(registered.disposition, app::provider::VolumeRegistrationDisposition::Created);
  QCOMPARE(registered.row.path, std::string("next-disk"));
  QCOMPARE(wmi.putProperties.back().at("VolumeName").toString(), std::string("Volume{disk}"));
  QCOMPARE(wmi.objectReads, std::vector<std::string>{R"(UWF_Volume.CurrentSession=FALSE,DriveLetter="C:",VolumeName="Volume{disk}")"});

  auto protectedNext = next;
  protectedNext["Protected"] = uwf::app::wmi::WmiValue::fromBool(true);
  wmi.objectResults.push_back(protectedNext);
  volume.protectVolume(registered.row);
  QCOMPARE(wmi.invocations.back().objectPath, std::string("next-disk"));
  QCOMPARE(wmi.invocations.back().method, std::string("Protect"));
  QCOMPARE(wmi.objectReads.size(), std::size_t{2});
  QCOMPARE(wmi.objectReads.back(), std::string("next-disk"));

  uwf::app::wmi::WmiMethodOutput exclusions;
  exclusions.arrays["ExcludedFiles"] = {{{"FileName", uwf::app::wmi::WmiValue::fromString(R"(\Cache)")}}};
  wmi.readResults.push_back(exclusions);
  QCOMPARE(volume.getExclusions(registered.row).front().fileName, std::string(R"(\Cache)"));
  QCOMPARE(wmi.reads.back().objectPath, std::string("next-disk"));

  wmi.queryResults.push_back({cache, disk, staleNext, next});
  const auto existing = volume.ensureNextSessionEntry("C:");
  QCOMPARE(existing.disposition, app::provider::VolumeRegistrationDisposition::AlreadyPresent);
  QCOMPARE(existing.row.path, std::string("next-disk"));
  QCOMPARE(wmi.putProperties.size(), std::size_t{1});
}

void WmiApiBehaviorTests::volumeFileOperationsNormalizePathsAndRejectCrossVolumeTargets() {
  MemoryWmiOperations wmi;
  app::provider::UwfVolume volume(wmi, [](const std::string&) { return std::vector<std::string>{}; });
  const app::provider::VolumeRow row{"volume-path", false, "C:", "Volume{c}", true, false, false};
  volume.commitFile(row, "c:\\Users\\a.txt");
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string("\\Users\\a.txt"));
  volume.commitFileDeletion(row, "\\Users\\b.txt");
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string("\\Users\\b.txt"));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, volume.commitFile(row, "D:\\other.txt"));

  uwf::app::wmi::WmiMethodOutput found;
  found.values.emplace("bFound", uwf::app::wmi::WmiValue::fromBool(false));
  wmi.readResults.push_back(found);
  QVERIFY(!volume.findExclusion(row, "C:\\Temp"));
  QCOMPARE(wmi.reads.back().inputs.at("FileName").toString(), std::string("\\Temp"));
}

void WmiApiBehaviorTests::volumeStateAndExclusionMutationsConfirmEveryPublicOperation() {
  MemoryWmiOperations wmi;
  app::provider::UwfVolume volume(wmi, [](const std::string&) { return std::vector<std::string>{}; });
  const app::provider::VolumeRow row{"volume-path", false, "C:", "Volume{c}", true, false, false};

  wmi.objectResults.push_back(volumeRow(false, "C:", "Volume{c}", true, false, true, "volume-path"));
  volume.protectVolume(row);
  QCOMPARE(wmi.invocations.back().method, std::string("Protect"));

  app::provider::VolumeRow protectedRow = row;
  protectedRow.isProtected = true;
  wmi.objectResults.push_back(volumeRow(false, "C:", "Volume{c}", true, false, false, "volume-path"));
  volume.unprotect(protectedRow);
  QCOMPARE(wmi.invocations.back().method, std::string("Unprotect"));

  wmi.objectResults.push_back(volumeRow(false, "C:", "Volume{c}", false, false, false, "volume-path"));
  volume.setBinding(row, app::provider::VolumeBinding::VolumeName);
  QCOMPARE(wmi.invocations.back().method, std::string("SetBindByDriveLetter"));
  QCOMPARE(wmi.invocations.back().inputs.at("bBindByDriveLetter").toBool(), false);

  uwf::app::wmi::WmiMethodOutput found;
  found.values.emplace("bFound", uwf::app::wmi::WmiValue::fromBool(true));
  wmi.readResults.push_back(found);
  volume.addExclusion(row, "c:\\Cache\\File.dat");
  QCOMPARE(wmi.invocations.back().method, std::string("AddExclusion"));
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string("\\Cache\\File.dat"));

  uwf::app::wmi::WmiMethodOutput absent;
  absent.values.emplace("bFound", uwf::app::wmi::WmiValue::fromBool(false));
  wmi.readResults.push_back(absent);
  volume.removeExclusion(row, "C:\\Cache\\File.dat");
  QCOMPARE(wmi.invocations.back().method, std::string("RemoveExclusion"));
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string("\\Cache\\File.dat"));

  uwf::app::wmi::WmiMethodOutput exclusions;
  exclusions.arrays["ExcludedFiles"] = {{{"FileName", uwf::app::wmi::WmiValue::fromString("\\Cache\\One")}},
                                        {{"__MOF", uwf::app::wmi::WmiValue::fromString(R"(FileName = "\\Cache\\Two";)")}}};
  wmi.readResults.push_back(exclusions);
  const auto listed = volume.getExclusions(row);
  QCOMPARE(listed.size(), std::size_t{2});
  QCOMPARE(listed.back().fileName, std::string("\\Cache\\Two"));

  uwf::app::wmi::WmiMethodOutput noExclusions;
  noExclusions.values.emplace("ExcludedFiles", uwf::app::wmi::WmiValue{});
  wmi.readResults.push_back(noExclusions);
  volume.removeAllExclusions(row);
  QCOMPARE(wmi.invocations.back().method, std::string("RemoveAllExclusions"));

  uwf::app::wmi::WmiMethodOutput remaining;
  remaining.arrays["ExcludedFiles"] = {{{"FileName", uwf::app::wmi::WmiValue::fromString("\\StillThere")}}};
  wmi.readResults.push_back(remaining);
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiStateVerificationError, volume.removeAllExclusions(row));

  app::provider::VolumeRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, volume.protectVolume(missingIdentity));
}

void WmiApiBehaviorTests::volumeRegistrationHandlesExistingCreatedAndUncertainOutcomes() {
  const uwf::app::wmi::WmiRow current = volumeRow(true, "C:", "Volume{guid}", true, false, true, "current");
  const uwf::app::wmi::WmiRow next = volumeRow(false, "C:", "Volume{guid}", true, false, false, "next");

  MemoryWmiOperations existingWmi;
  existingWmi.queryResults.push_back({current, next});
  app::provider::UwfVolume existingVolume(existingWmi, [](const std::string&) { return std::vector<std::string>{}; });
  QCOMPARE(existingVolume.ensureNextSessionEntry("c").disposition, app::provider::VolumeRegistrationDisposition::AlreadyPresent);
  QVERIFY(existingWmi.puts.empty());

  MemoryWmiOperations createdWmi;
  createdWmi.queryResults.push_back({current});
  createdWmi.objectResults.push_back(next);
  app::provider::UwfVolume createdVolume(createdWmi, [](const std::string&) { return std::vector<std::string>{}; });
  const auto created = createdVolume.ensureNextSessionEntry("C:");
  QCOMPARE(created.disposition, app::provider::VolumeRegistrationDisposition::Created);
  QCOMPARE(createdWmi.puts.size(), std::size_t{1});
  QCOMPARE(createdWmi.putProperties.front().at("CurrentSession").toBool(), false);

  MemoryWmiOperations uncertainWmi;
  uncertainWmi.queryResults.push_back({current});
  uncertainWmi.putFailure = std::make_exception_ptr(uwf::app::wmi::WmiInvocationUncertain("create volume", "transport lost"));
  uncertainWmi.objectResults.push_back(next);
  app::provider::UwfVolume uncertainVolume(uncertainWmi, [](const std::string&) { return std::vector<std::string>{}; });
  QCOMPARE(uncertainVolume.ensureNextSessionEntry("C:").disposition, app::provider::VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite);

  MemoryWmiOperations concurrentWmi;
  concurrentWmi.queryResults.push_back({current});
  concurrentWmi.putFailure =
      std::make_exception_ptr(uwf::app::wmi::WmiInfrastructureError(static_cast<std::int32_t>(WBEM_E_ALREADY_EXISTS), "create volume", "created concurrently"));
  concurrentWmi.objectResults.push_back(next);
  app::provider::UwfVolume concurrentVolume(concurrentWmi, [](const std::string&) { return std::vector<std::string>{}; });
  QCOMPARE(concurrentVolume.ensureNextSessionEntry("C:").disposition, app::provider::VolumeRegistrationDisposition::ConcurrentlyCreated);

  MemoryWmiOperations mismatchWmi;
  mismatchWmi.queryResults.push_back({current});
  mismatchWmi.objectResults.push_back(volumeRow(false, "C:", "Volume{wrong}", true, false, false, "wrong"));
  app::provider::UwfVolume mismatchVolume(mismatchWmi, [](const std::string&) { return std::vector<std::string>{}; });
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiStateVerificationError, (void)mismatchVolume.ensureNextSessionEntry("C:"));

  MemoryWmiOperations missingCurrentWmi;
  missingCurrentWmi.queryResults.push_back({});
  app::provider::UwfVolume missingCurrentVolume(missingCurrentWmi, [](const std::string&) { return std::vector<std::string>{}; });
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)missingCurrentVolume.ensureNextSessionEntry("C:"));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)missingCurrentVolume.ensureNextSessionEntry("not-a-drive"));
}

void WmiApiBehaviorTests::capabilityProbeDistinguishesAbsenceFromInfrastructureFailure() {
  MemoryWmiOperations wmi;
  QCOMPARE(uwf::app::probeUwfCapability(wmi), uwf::app::UwfCapability::Available);
  QCOMPARE(wmi.ensureConnectedCalls, 1);

  wmi.reportedClassStatus = uwf::app::wmi::WmiClassStatus::Missing;
  QCOMPARE(uwf::app::probeUwfCapability(wmi), uwf::app::UwfCapability::Unavailable);

  wmi.classStatusFailure =
      std::make_exception_ptr(uwf::app::wmi::WmiInfrastructureError(static_cast<std::int32_t>(WBEM_E_INVALID_NAMESPACE), "probe", "namespace missing"));
  QCOMPARE(uwf::app::probeUwfCapability(wmi), uwf::app::UwfCapability::Unavailable);

  wmi.classStatusFailure =
      std::make_exception_ptr(uwf::app::wmi::WmiInfrastructureError(static_cast<std::int32_t>(WBEM_E_ACCESS_DENIED), "probe", "access denied"));
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiInfrastructureError, (void)uwf::app::probeUwfCapability(wmi));
}

void WmiApiBehaviorTests::snapshotAssemblyPreservesSessionsAndSkipsUnprotectedCurrentExclusions() {
  MemoryWmiOperations unavailable;
  const auto unavailableSnapshot =
      uwf::app::readSnapshot(unavailable, uwf::app::UwfCapability::Unavailable, true, [](const std::string&) { return std::vector<std::string>{}; });
  QVERIFY(!unavailableSnapshot.uwfAvailable);
  QVERIFY(unavailableSnapshot.elevated);
  QCOMPARE(unavailableSnapshot.unavailableReason, std::string("UWF is not registered"));
  QCOMPARE(unavailable.ensureConnectedCalls, 0);

  MemoryWmiOperations wmi;
  wmi.queryResults.push_back({filterRow(true, false)});
  wmi.queryResults.push_back({overlayRow(25, 75, 80, 60)});
  wmi.queryResults.push_back({overlayConfigRow(true, 0, 100, "overlay-current"), overlayConfigRow(false, 1, 200, "overlay-next")});
  wmi.queryResults.push_back(
      {volumeRow(true, "C:", "Volume{c}", true, false, true, "current-c"), volumeRow(true, "D:", "Volume{d}", true, false, false, "current-d"),
       volumeRow(false, "C:", "Volume{c}", false, false, true, "next-c"), volumeRow(false, "D:", "Volume{d}", false, false, false, "next-d")});

  uwf::app::wmi::WmiMethodOutput currentC;
  currentC.arrays["ExcludedFiles"] = {{{"FileName", uwf::app::wmi::WmiValue::fromString("\\Current")}}};
  uwf::app::wmi::WmiMethodOutput nextC;
  nextC.arrays["ExcludedFiles"] = {{{"FileName", uwf::app::wmi::WmiValue::fromString("\\Next")}}};
  uwf::app::wmi::WmiMethodOutput nextD;
  nextD.arrays["ExcludedFiles"] = {{{"FileName", uwf::app::wmi::WmiValue::fromString("\\PreparedWhileUnprotected")}}};
  wmi.readResults.push_back(currentC);
  wmi.readResults.push_back(nextC);
  wmi.readResults.push_back(nextD);

  wmi.queryResults.push_back({registryFilterRow(true, true, false, "registry-current"), registryFilterRow(false, false, true, "registry-next")});
  uwf::app::wmi::WmiMethodOutput currentRegistry;
  currentRegistry.arrays["ExcludedKeys"] = {{{"RegistryKey", uwf::app::wmi::WmiValue::fromString("HKLM\\SOFTWARE\\Current")}}};
  uwf::app::wmi::WmiMethodOutput nextRegistry;
  nextRegistry.arrays["ExcludedKeys"] = {{{"RegistryKey", uwf::app::wmi::WmiValue::fromString("HKLM\\SOFTWARE\\Next")}}};
  wmi.readResults.push_back(currentRegistry);
  wmi.readResults.push_back(nextRegistry);

  const auto snapshot = uwf::app::readSnapshot(wmi, uwf::app::UwfCapability::Available, false, [](const std::string&) { return std::vector<std::string>{}; });
  QVERIFY(snapshot.uwfAvailable);
  QVERIFY(!snapshot.elevated);
  QCOMPARE(snapshot.current.overlay.type, core::OverlayType::RAM);
  QCOMPARE(snapshot.next.overlay.type, core::OverlayType::Disk);
  QCOMPARE(snapshot.current.overlay.warningThresholdMb, 60u);
  QCOMPARE(snapshot.next.overlay.criticalThresholdMb, 80u);
  QCOMPARE(snapshot.current.fileExclusions.at("Volume{c}").front(), std::string("C:\\Current"));
  QVERIFY(!snapshot.current.fileExclusions.contains("Volume{d}"));
  QCOMPARE(snapshot.next.fileExclusions.at("Volume{d}").front(), std::string("D:\\PreparedWhileUnprotected"));
  QCOMPARE(snapshot.current.registryExclusions.front(), std::string("HKLM\\SOFTWARE\\Current"));
  QCOMPARE(snapshot.next.registryExclusions.front(), std::string("HKLM\\SOFTWARE\\Next"));
  QCOMPARE(wmi.reads.size(), std::size_t{5});
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>({"SELECT * FROM UWF_Filter", "SELECT * FROM UWF_Overlay", "SELECT * FROM UWF_OverlayConfig",
                                                          "SELECT * FROM UWF_Volume", "SELECT * FROM UWF_RegistryFilter"}));
  QVERIFY(wmi.projectionQueries.empty());
}

void WmiApiBehaviorTests::diskInventoryClassifiesEverySupportBoundary() {
  MemoryWmiOperations wmi;
  const auto logical = [](const char* drive, const char* fs, const std::uint64_t size, const std::int32_t type) {
    return uwf::app::wmi::WmiRow{{"DeviceID", uwf::app::wmi::WmiValue::fromString(drive)},
                                 {"FileSystem", uwf::app::wmi::WmiValue::fromString(fs)},
                                 {"VolumeName", uwf::app::wmi::WmiValue::fromString(std::string("Label-") + drive)},
                                 {"Size", uwf::app::wmi::WmiValue::fromUInt(size)},
                                 {"FreeSpace", uwf::app::wmi::WmiValue::fromUInt(size / 2)},
                                 {"DriveType", uwf::app::wmi::WmiValue::fromInt(type)}};
  };
  constexpr std::uint64_t TiB = 1024ULL * 1024 * 1024 * 1024;
  wmi.queryResults.push_back(
      {logical("C:", "NTFS", TiB, 3), logical("D:", "exFAT", TiB, 3), logical("E:", "NTFS", 17 * TiB, 3), logical("F:", "FAT32", TiB, 2)});
  wmi.queryResults.push_back(
      {{{"DeviceID", uwf::app::wmi::WmiValue::fromString("\\\\?\\Volume{c}\\")}, {"DriveLetter", uwf::app::wmi::WmiValue::fromString("C:")}},
       {{"DeviceID", uwf::app::wmi::WmiValue::fromString("\\\\?\\Volume{d}\\")}, {"DriveLetter", uwf::app::wmi::WmiValue::fromString("D:")}},
       {{"DeviceID", uwf::app::wmi::WmiValue::fromString("\\\\?\\Volume{hidden}\\")}, {"DriveLetter", uwf::app::wmi::WmiValue{}}}});

  const auto disks = uwf::app::enumerateVolumes(wmi);
  QCOMPARE(disks.size(), std::size_t{4});
  QCOMPARE(disks[0].support, core::VolumeSupport::Supported);
  QCOMPARE(disks[1].support, core::VolumeSupport::FileSystemLimited);
  QCOMPARE(disks[2].support, core::VolumeSupport::ExceedsMaxSize);
  QCOMPARE(disks[3].support, core::VolumeSupport::NotFixedLocalDisk);
  QCOMPARE(disks[0].volumeName, std::string("\\\\?\\Volume{c}\\"));
  QVERIFY(disks[2].volumeName.empty());
  QCOMPARE(wmi.projectionQueries,
           std::vector<std::string>(
               {"SELECT DeviceID, FileSystem, VolumeName, Size, FreeSpace, DriveType FROM Win32_LogicalDisk WHERE DriveType = 2 OR DriveType = 3",
                "SELECT DeviceID, DriveLetter FROM Win32_Volume"}));
  QVERIFY(wmi.instanceQueries.empty());

  MemoryWmiOperations duplicate;
  duplicate.queryResults.push_back({logical("C:", "NTFS", TiB, 3), logical("c", "NTFS", TiB, 3)});
  duplicate.queryResults.push_back({});
  QVERIFY_THROWS_EXCEPTION(uwf::app::wmi::WmiProtocolError, (void)uwf::app::enumerateVolumes(duplicate));
}

}  // namespace

QTEST_APPLESS_MAIN(WmiApiBehaviorTests)

#include "WmiApiBehaviorTests.moc"
