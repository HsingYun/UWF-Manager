/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest>
#include <cmath>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "uwf/UwfSnapshot.h"
#include "uwf/api/Types.h"
#include "uwf/api/UwfFilter.h"
#include "uwf/api/UwfOverlay.h"
#include "uwf/api/UwfOverlayConfig.h"
#include "uwf/api/UwfRegistryFilter.h"
#include "uwf/api/UwfVolume.h"
#include "uwf/wmi/WmiError.h"
#include "uwf/wmi/WmiException.h"
#include "uwf/wmi/WmiRowUtil.h"

namespace {

using namespace uwf;

struct MethodCall {
  std::string objectPath;
  std::string method;
  WmiRow inputs;
};

class MemoryWmiOperations final : public WmiOperations {
 public:
  mutable int ensureConnectedCalls = 0;
  mutable std::vector<std::string> projectionQueries;
  mutable std::vector<std::string> instanceQueries;
  mutable std::deque<std::vector<WmiRow>> queryResults;
  mutable std::deque<WmiRow> objectResults;
  mutable std::deque<WmiMethodOutput> readResults;
  mutable std::vector<MethodCall> invocations;
  mutable std::vector<MethodCall> reads;
  mutable std::vector<std::pair<std::string, WmiPutMode>> puts;
  mutable std::vector<WmiRow> putProperties;
  mutable std::exception_ptr putFailure;
  WmiClassStatus reportedClassStatus = WmiClassStatus::Present;
  mutable std::exception_ptr classStatusFailure;

  void ensureConnected() const override { ++ensureConnectedCalls; }

  [[nodiscard]] std::vector<WmiRow> query(const std::string& wql) const override {
    projectionQueries.push_back(wql);
    return takeQuery();
  }
  [[nodiscard]] std::vector<WmiRow> queryInstances(const std::string& wql) const override {
    instanceQueries.push_back(wql);
    return takeQuery();
  }
  [[nodiscard]] WmiClassStatus classStatus(const std::string&) const override {
    if (classStatusFailure) std::rethrow_exception(classStatusFailure);
    return reportedClassStatus;
  }

  [[nodiscard]] WmiRow getObject(const std::string&) const override {
    if (objectResults.empty()) throw std::runtime_error("no object result queued");
    WmiRow row = std::move(objectResults.front());
    objectResults.pop_front();
    return row;
  }

  void invokeMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs) const override {
    invocations.push_back({objectPath, methodName, inputs});
  }

  [[nodiscard]] WmiMethodOutput callMethodRead(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs) const override {
    reads.push_back({objectPath, methodName, inputs});
    return takeReadResult();
  }

  [[nodiscard]] WmiMethodOutput callMethodReadCancelable(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs,
                                                         std::stop_token) const override {
    reads.push_back({objectPath, methodName, inputs});
    return takeReadResult();
  }

  void putInstance(const std::string& className, const WmiRow& props, const WmiPutMode mode) const override {
    puts.emplace_back(className, mode);
    putProperties.push_back(props);
    if (putFailure) std::rethrow_exception(putFailure);
  }

 private:
  [[nodiscard]] std::vector<WmiRow> takeQuery() const {
    if (queryResults.empty()) throw std::runtime_error("no query result queued");
    auto rows = std::move(queryResults.front());
    queryResults.pop_front();
    return rows;
  }

  [[nodiscard]] WmiMethodOutput takeReadResult() const {
    if (readResults.empty()) throw std::runtime_error("no method result queued");
    auto output = std::move(readResults.front());
    readResults.pop_front();
    return output;
  }
};

WmiRow filterRow(const bool currentEnabled, const bool nextEnabled, const std::string& path = "filter-path") {
  return {{"__PATH", WmiValue::fromString(path)}, {"CurrentEnabled", WmiValue::fromBool(currentEnabled)}, {"NextEnabled", WmiValue::fromBool(nextEnabled)}};
}

WmiRow overlayRow(const uint32_t consumption = 10, const uint32_t available = 90, const uint32_t critical = 80, const uint32_t warning = 70,
                  const std::string& path = "overlay-path") {
  return {{"__PATH", WmiValue::fromString(path)},
          {"OverlayConsumption", WmiValue::fromUInt(consumption)},
          {"AvailableSpace", WmiValue::fromUInt(available)},
          {"CriticalOverlayThreshold", WmiValue::fromUInt(critical)},
          {"WarningOverlayThreshold", WmiValue::fromUInt(warning)}};
}

WmiRow overlayConfigRow(const bool current, const uint32_t type, const uint32_t maximum, const std::string& path) {
  return {{"__PATH", WmiValue::fromString(path)},
          {"CurrentSession", WmiValue::fromBool(current)},
          {"Type", WmiValue::fromUInt(type)},
          {"MaximumSize", WmiValue::fromUInt(maximum)}};
}

WmiRow registryFilterRow(const bool current, const bool domain, const bool tscal, const std::string& path) {
  return {{"__PATH", WmiValue::fromString(path)},
          {"CurrentSession", WmiValue::fromBool(current)},
          {"PersistDomainSecretKey", WmiValue::fromBool(domain)},
          {"PersistTSCAL", WmiValue::fromBool(tscal)}};
}

WmiRow volumeRow(const bool current, const std::string& drive, const std::string& volume, const bool bind, const bool pending, const bool protectedState,
                 const std::string& path) {
  return {{"__PATH", WmiValue::fromString(path)},           {"CurrentSession", WmiValue::fromBool(current)}, {"DriveLetter", WmiValue::fromString(drive)},
          {"VolumeName", WmiValue::fromString(volume)},     {"BindByDriveLetter", WmiValue::fromBool(bind)}, {"CommitPending", WmiValue::fromBool(pending)},
          {"Protected", WmiValue::fromBool(protectedState)}};
}

class WmiApiBehaviorTests final : public QObject {
  Q_OBJECT

 private slots:
  void wmiValueConversionsRejectLossAndNonFiniteInputs();
  void wmiValueConversionMatrixCoversKindsAndBoundaries();
  void rowDecodingDistinguishesMissingNullWrongTypeAndEmpty();
  void embeddedMofDecodingHandlesEscapesAndMissingFields();
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
  void volumeFileOperationsNormalizePathsAndRejectCrossVolumeTargets();
  void volumeStateAndExclusionMutationsConfirmEveryPublicOperation();
  void volumeRegistrationHandlesExistingCreatedAndUncertainOutcomes();
  void capabilityProbeDistinguishesAbsenceFromInfrastructureFailure();
  void snapshotAssemblyPreservesSessionsAndSkipsUnprotectedCurrentExclusions();
  void diskInventoryClassifiesEverySupportBoundary();
};

void WmiApiBehaviorTests::wmiValueConversionsRejectLossAndNonFiniteInputs() {
  QCOMPARE(WmiValue::fromBool(true).toBool(), true);
  QCOMPARE(WmiValue::fromInt(-1).toInt(), -1);
  QCOMPARE(WmiValue::fromUInt(std::numeric_limits<uint32_t>::max()).toUInt(), std::numeric_limits<uint32_t>::max());
  QCOMPARE(WmiValue::fromString("18446744073709551615").toULongLong(), std::numeric_limits<uint64_t>::max());
  QCOMPARE(WmiValue::fromString("false").toBool(), false);
  QCOMPARE(WmiValue::fromDouble(1.5).toDouble(), 1.5);

  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue{}.toString());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromInt(-1).toUInt());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromUInt(static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1).toUInt());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromString("12x").toInt());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromString("nan").toDouble());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromDouble(std::numeric_limits<double>::infinity()).toString());
}

void WmiApiBehaviorTests::wmiValueConversionMatrixCoversKindsAndBoundaries() {
  QCOMPARE(WmiValue::fromBool(false).toInt64(), int64_t{0});
  QCOMPARE(WmiValue::fromBool(true).toULongLong(), uint64_t{1});
  QCOMPARE(WmiValue::fromBool(true).toDouble(), 1.0);
  QCOMPARE(WmiValue::fromBool(false).toString(), std::string("false"));

  QCOMPARE(WmiValue::fromInt(std::numeric_limits<int32_t>::min()).toInt(), std::numeric_limits<int32_t>::min());
  QCOMPARE(WmiValue::fromInt(std::numeric_limits<int32_t>::max()).toInt(), std::numeric_limits<int32_t>::max());
  QCOMPARE(WmiValue::fromInt(std::numeric_limits<int64_t>::min()).toInt64(), std::numeric_limits<int64_t>::min());
  QCOMPARE(WmiValue::fromInt(std::numeric_limits<int64_t>::max()).toInt64(), std::numeric_limits<int64_t>::max());
  QCOMPARE(WmiValue::fromUInt(std::numeric_limits<uint64_t>::max()).toULongLong(), std::numeric_limits<uint64_t>::max());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromInt(std::numeric_limits<int64_t>::min()).toULongLong());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromUInt(static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1).toInt64());

  constexpr uint64_t kLargestConsecutiveDoubleInteger = uint64_t{1} << std::numeric_limits<double>::digits;
  QCOMPARE(WmiValue::fromUInt(kLargestConsecutiveDoubleInteger).toDouble(), static_cast<double>(kLargestConsecutiveDoubleInteger));
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromUInt(kLargestConsecutiveDoubleInteger + 1).toDouble());
  QCOMPARE(WmiValue::fromDouble(-0.0).toBool(), false);
  QCOMPARE(WmiValue::fromDouble(42.0).toInt64(), int64_t{42});
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromDouble(42.5).toInt64());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromDouble(-1.0).toULongLong());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromDouble(std::numeric_limits<double>::quiet_NaN()).toBool());

  for (const std::string value : {"true", "TRUE", "TrUe", "false", "FALSE", "FaLsE"}) {
    QCOMPARE(WmiValue::fromString(value).toBool(), value[0] == 't' || value[0] == 'T');
  }
  QCOMPARE(WmiValue::fromString("-9223372036854775808").toInt64(), std::numeric_limits<int64_t>::min());
  QCOMPARE(WmiValue::fromString("18446744073709551615").toULongLong(), std::numeric_limits<uint64_t>::max());
  for (const std::string value : {"", " ", "+1", "01x", "18446744073709551616", "-9223372036854775809"}) {
    QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)WmiValue::fromString(value).toULongLong());
  }

  const WmiValue missing;
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)missing.toBool());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)missing.toInt64());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)missing.toULongLong());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)missing.toDouble());
  QVERIFY_THROWS_EXCEPTION(WmiValueConversionError, (void)missing.toString());
}

void WmiApiBehaviorTests::rowDecodingDistinguishesMissingNullWrongTypeAndEmpty() {
  WmiRow row{{"Bool", WmiValue::fromBool(true)},
             {"Int", WmiValue::fromString("-42")},
             {"UInt", WmiValue::fromUInt(42)},
             {"Text", WmiValue::fromString("")},
             {"Null", WmiValue{}}};
  QCOMPARE(rowutil::requireBool(row, "Bool"), true);
  QCOMPARE(rowutil::requireInt(row, "Int"), -42);
  QCOMPARE(rowutil::requireUInt(row, "UInt"), 42u);
  QVERIFY(rowutil::requireString(row, "Text").empty());
  QVERIFY_THROWS_EXCEPTION(WmiDecodeError, (void)rowutil::requireString(row, "Text", rowutil::EmptyString::Reject));
  QVERIFY_THROWS_EXCEPTION(WmiDecodeError, (void)rowutil::requireBool(row, "Missing"));
  QVERIFY_THROWS_EXCEPTION(WmiDecodeError, (void)rowutil::requireString(row, "Null"));
  QVERIFY_THROWS_EXCEPTION(WmiDecodeError, (void)rowutil::requireUInt(row, "Bool"));
}

void WmiApiBehaviorTests::embeddedMofDecodingHandlesEscapesAndMissingFields() {
  const std::string mof = R"(instance of X { FileName = "\\Users\"quoted\".txt"; Other = "x"; };)";
  QCOMPARE(rowutil::extractFromMof(mof, "FileName"), std::string("\\Users\"quoted\".txt"));
  QCOMPARE(rowutil::extractFromMof(mof, "Missing"), std::string{});
  QCOMPARE(rowutil::extractFromMof(R"(X { A.B = "value"; };)", "A.B"), std::string("value"));

  WmiRow direct{{"FileName", WmiValue::fromString("\\direct")}};
  QCOMPARE(rowutil::requireEmbeddedString(direct, "FileName"), std::string("\\direct"));
  WmiRow embedded{{"__MOF", WmiValue::fromString(mof)}};
  QCOMPARE(rowutil::requireEmbeddedString(embedded, "FileName"), std::string("\\Users\"quoted\".txt"));
  QVERIFY_THROWS_EXCEPTION(WmiDecodeError, (void)rowutil::requireEmbeddedString(embedded, "Absent"));
}

void WmiApiBehaviorTests::wmiErrorsPreserveKnownAndUnknownCodes() {
  const WmiError success(0);
  QVERIFY(success.isKnown());
  QVERIFY(!success.isFailure());
  QCOMPARE(success.name(), std::string_view("S_OK"));

  const WmiError missing(static_cast<int32_t>(WBEM_E_NOT_FOUND));
  QVERIFY(missing.isKnown());
  QVERIFY(missing.isFailure());
  QCOMPARE(missing.code(), WmiErrorCode::NotFound);
  QVERIFY(!missing.description().empty());

  const WmiError unknown(static_cast<int32_t>(0x81234567u));
  QVERIFY(!unknown.isKnown());
  QCOMPARE(unknown.name(), std::string_view("WBEM_E_UNKNOWN"));
  QVERIFY(unknown.description().empty());
  const auto code = makeWmiErrorCode(static_cast<int32_t>(0x81234567u));
  QVERIFY(QString::fromStdString(code.message()).contains(QStringLiteral("81234567"), Qt::CaseInsensitive));
}

void WmiApiBehaviorTests::apiSessionLookupHonorsSessionAndPredicate() {
  const std::vector<api::VolumeRow> rows{
      {"a", true, "C:", "v1", true, false, true}, {"b", false, "C:", "v1", true, false, false}, {"c", false, "D:", "v2", true, false, false}};
  QCOMPARE(api::findBySession(rows, api::Session::Current)->path, std::string("a"));
  QCOMPARE(api::findBySession(rows, api::Session::Next, [](const api::VolumeRow& row) { return row.driveLetter == "D:"; })->path, std::string("c"));
  QVERIFY(!api::findBySession(rows, api::Session::Current, [](const api::VolumeRow& row) { return row.driveLetter == "Z:"; }));
}

void WmiApiBehaviorTests::filterRequiresExactlyOneRowAndConfirmsWrites() {
  MemoryWmiOperations wmi;
  api::UwfFilter filter(wmi);
  wmi.queryResults.push_back({filterRow(true, false)});
  const auto row = filter.read();
  QVERIFY(row.currentEnabled);
  QVERIFY(!row.nextEnabled);
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_Filter"});
  QVERIFY(wmi.projectionQueries.empty());

  wmi.objectResults.push_back(filterRow(true, true));
  filter.enable(row);
  QCOMPARE(wmi.invocations.back().method, std::string("Enable"));

  wmi.objectResults.push_back(filterRow(true, true));
  QVERIFY_THROWS_EXCEPTION(WmiStateVerificationError, filter.disable(row));
  api::FilterRow noPath = row;
  noPath.path.clear();
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, filter.restartSystem(noPath));

  wmi.queryResults.push_back({});
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, (void)filter.read());
  wmi.queryResults.push_back({filterRow(true, true), filterRow(true, false, "other")});
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, (void)filter.read());
}

void WmiApiBehaviorTests::filterPowerMethodsRemainOneShotAndRequireIdentity() {
  MemoryWmiOperations wmi;
  api::UwfFilter filter(wmi);
  const api::FilterRow row{"filter-path", true, true};

  filter.shutdownSystem(row);
  filter.restartSystem(row);
  QCOMPARE(wmi.invocations.size(), std::size_t{2});
  QCOMPARE(wmi.invocations[0].method, std::string("ShutdownSystem"));
  QCOMPARE(wmi.invocations[1].method, std::string("RestartSystem"));
  QVERIFY(wmi.invocations[0].inputs.empty());
  QVERIFY(wmi.invocations[1].inputs.empty());
  QVERIFY(wmi.objectResults.empty());  // 电源操作没有可安全重读的通用后置条件。

  api::FilterRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, filter.shutdownSystem(missingIdentity));
  QCOMPARE(wmi.invocations.size(), std::size_t{2});
}

void WmiApiBehaviorTests::overlayDecodesFilesAndRoutesCancelableReads() {
  MemoryWmiOperations wmi;
  api::UwfOverlay overlay(wmi);
  wmi.queryResults.push_back({overlayRow()});
  const auto row = overlay.read();
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_Overlay"});
  QVERIFY(wmi.projectionQueries.empty());

  WmiMethodOutput direct;
  direct.arrays["OverlayFiles"] = {{{"FileName", WmiValue::fromString("\\a.txt")}, {"FileSize", WmiValue::fromUInt(0)}}};
  wmi.readResults.push_back(direct);
  auto files = overlay.getOverlayFiles(row, "C:");
  QCOMPARE(files.size(), std::size_t{1});
  QCOMPARE(files.front().fileSize, uint64_t{0});
  QCOMPARE(wmi.reads.back().inputs.at("Volume").toString(), std::string("C:"));

  WmiMethodOutput embedded;
  embedded.arrays["OverlayFiles"] = {{{"__MOF", WmiValue::fromString(R"(FileName = "\\b.txt"; FileSize = "18446744073709551615";)")}}};
  wmi.readResults.push_back(embedded);
  std::stop_source source;
  files = overlay.getOverlayFiles(row, "D:", source.get_token());
  QCOMPARE(files.front().fileSize, std::numeric_limits<uint64_t>::max());

  WmiMethodOutput malformed;
  malformed.arrays["OverlayFiles"] = {{{"__MOF", WmiValue::fromString(R"(FileName = "\\bad"; FileSize = "x";)")}}};
  wmi.readResults.push_back(malformed);
  QVERIFY_THROWS_EXCEPTION(WmiDecodeError, (void)overlay.getOverlayFiles(row, "C:"));

  WmiMethodOutput empty;
  empty.values.emplace("OverlayFiles", WmiValue{});
  wmi.readResults.push_back(empty);
  QVERIFY(overlay.getOverlayFiles(row, "C:").empty());

  wmi.readResults.emplace_back();
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, (void)overlay.getOverlayFiles(row, "C:"));
}

void WmiApiBehaviorTests::overlayThresholdWritesUseTypedInputsAndAuthoritativeState() {
  MemoryWmiOperations wmi;
  api::UwfOverlay overlay(wmi);
  const api::OverlayRow row{"overlay-path", 10, 90, 80, 70};

  wmi.objectResults.push_back(overlayRow(10, 90, 80, 65));
  overlay.setWarningThreshold(row, 65);
  QCOMPARE(wmi.invocations.back().method, std::string("SetWarningThreshold"));
  QCOMPARE(wmi.invocations.back().inputs.at("size").toUInt(), 65u);

  wmi.objectResults.push_back(overlayRow(10, 90, 75, 65));
  overlay.setCriticalThreshold(row, 75);
  QCOMPARE(wmi.invocations.back().method, std::string("SetCriticalThreshold"));
  QCOMPARE(wmi.invocations.back().inputs.at("size").toUInt(), 75u);

  wmi.objectResults.push_back(overlayRow(10, 90, 74, 65));
  QVERIFY_THROWS_EXCEPTION(WmiStateVerificationError, overlay.setCriticalThreshold(row, 75));

  api::OverlayRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, overlay.setWarningThreshold(missingIdentity, 50));
}

void WmiApiBehaviorTests::overlayConfigurationRejectsInvalidAndDuplicateSessions() {
  MemoryWmiOperations wmi;
  api::UwfOverlayConfig config(wmi);
  wmi.queryResults.push_back({overlayConfigRow(true, 0, 1024, "current"), overlayConfigRow(false, 1, 2048, "next")});
  QCOMPARE(config.read(api::Session::Next).maximumSize, 2048u);
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_OverlayConfig"});
  QVERIFY(wmi.projectionQueries.empty());

  wmi.queryResults.push_back({overlayConfigRow(true, 2, 1024, "bad")});
  QVERIFY_THROWS_EXCEPTION(WmiDecodeError, (void)config.readAll());
  wmi.queryResults.push_back({overlayConfigRow(false, 0, 1024, "a"), overlayConfigRow(false, 0, 2048, "b")});
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, (void)config.readAll());
  wmi.queryResults.push_back({overlayConfigRow(true, 0, 1024, "current")});
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, (void)config.read(api::Session::Next));
}

void WmiApiBehaviorTests::overlayConfigurationWritesPreserveSessionIdentity() {
  MemoryWmiOperations wmi;
  api::UwfOverlayConfig config(wmi);
  const api::OverlayConfigRow row{"next-config", false, api::OverlayType::RAM, 1024};

  wmi.objectResults.push_back(overlayConfigRow(false, 1, 1024, "next-config"));
  config.setType(row, api::OverlayType::Disk);
  QCOMPARE(wmi.invocations.back().method, std::string("SetType"));
  QCOMPARE(wmi.invocations.back().inputs.at("type").toUInt(), 1u);

  wmi.objectResults.push_back(overlayConfigRow(false, 1, 4096, "next-config"));
  config.setMaximumSize(row, 4096);
  QCOMPARE(wmi.invocations.back().method, std::string("SetMaximumSize"));
  QCOMPARE(wmi.invocations.back().inputs.at("size").toUInt(), 4096u);

  wmi.objectResults.push_back(overlayConfigRow(true, 1, 4096, "current-config"));
  QVERIFY_THROWS_EXCEPTION(WmiStateVerificationError, config.setMaximumSize(row, 4096));

  api::OverlayConfigRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, config.setType(missingIdentity, api::OverlayType::RAM));
}

void WmiApiBehaviorTests::registryFilterBuildsTypedInputsAndDecodesArrays() {
  MemoryWmiOperations wmi;
  api::UwfRegistryFilter registry(wmi);
  wmi.queryResults.push_back({registryFilterRow(true, true, false, "registry-current"), registryFilterRow(false, false, true, "registry-next")});
  const auto sessionRows = registry.readAll();
  QCOMPARE(sessionRows.size(), std::size_t{2});
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_RegistryFilter"});
  QVERIFY(wmi.projectionQueries.empty());

  const api::RegistryFilterRow row{"registry-path", false, false, false};
  WmiMethodOutput found;
  found.values.emplace("bFound", WmiValue::fromBool(true));
  wmi.readResults.push_back(found);
  QVERIFY(registry.findExclusion(row, "HKLM\\SOFTWARE\\Vendor"));
  QCOMPARE(wmi.reads.back().inputs.at("RegistryKey").toString(), std::string("HKLM\\SOFTWARE\\Vendor"));

  WmiMethodOutput exclusions;
  exclusions.arrays["ExcludedKeys"] = {{{"RegistryKey", WmiValue::fromString("HKLM\\SYSTEM")}},
                                       {{"__MOF", WmiValue::fromString(R"(RegistryKey = "HKLM\\SOFTWARE";)")}}};
  wmi.readResults.push_back(exclusions);
  const auto rows = registry.getExclusions(row);
  QCOMPARE(rows.size(), std::size_t{2});
  QCOMPARE(rows.back().registryKey, std::string("HKLM\\SOFTWARE"));

  wmi.objectResults.push_back(registryFilterRow(false, true, false, "registry-path"));
  registry.setPersistence(row, {true, false});
  QCOMPARE(wmi.puts.back().first, std::string("UWF_RegistryFilter"));
  QCOMPARE(wmi.puts.back().second, WmiPutMode::UpdateOnly);
  QVERIFY(wmi.putProperties.back().at("PersistDomainSecretKey").toBool());

  registry.commitRegistry(row, "HKLM\\SYSTEM", "Name");
  QCOMPARE(wmi.invocations.back().method, std::string("CommitRegistry"));
  QCOMPARE(wmi.invocations.back().inputs.at("ValueName").toString(), std::string("Name"));
}

void WmiApiBehaviorTests::registryFilterMutationsConfirmMembershipAndRouteDeletion() {
  MemoryWmiOperations wmi;
  api::UwfRegistryFilter registry(wmi);
  const api::RegistryFilterRow row{"registry-path", false, false, false};
  constexpr std::string_view key = "HKLM\\SOFTWARE\\Vendor";

  WmiMethodOutput found;
  found.values.emplace("bFound", WmiValue::fromBool(true));
  wmi.readResults.push_back(found);
  registry.addExclusion(row, std::string(key));
  QCOMPARE(wmi.invocations.back().method, std::string("AddExclusion"));
  QCOMPARE(wmi.invocations.back().inputs.at("RegistryKey").toString(), std::string(key));
  QCOMPARE(wmi.reads.back().method, std::string("FindExclusion"));

  WmiMethodOutput absent;
  absent.values.emplace("bFound", WmiValue::fromBool(false));
  wmi.readResults.push_back(absent);
  registry.removeExclusion(row, std::string(key));
  QCOMPARE(wmi.invocations.back().method, std::string("RemoveExclusion"));

  WmiMethodOutput stillPresent;
  stillPresent.values.emplace("bFound", WmiValue::fromBool(true));
  wmi.readResults.push_back(stillPresent);
  QVERIFY_THROWS_EXCEPTION(WmiStateVerificationError, registry.removeExclusion(row, std::string(key)));

  registry.commitRegistryDeletion(row, std::string(key), {});
  QCOMPARE(wmi.invocations.back().method, std::string("CommitRegistryDeletion"));
  QCOMPARE(wmi.invocations.back().inputs.at("ValueName").toString(), std::string{});

  api::RegistryFilterRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, registry.addExclusion(missingIdentity, std::string(key)));
}

void WmiApiBehaviorTests::volumeDecodingSkipsUnmanagedRowsAndRejectsDuplicates() {
  MemoryWmiOperations wmi;
  api::UwfVolume volume(wmi);
  WmiRow noDrive = volumeRow(true, "C:", "Volume{none}", true, false, false, "none");
  noDrive["DriveLetter"] = WmiValue{};
  wmi.queryResults.push_back(
      {volumeRow(true, "c", "Volume{c}", true, false, true, "current-c"), noDrive, volumeRow(false, "D:", "Volume{d}", false, false, false, "next-d")});
  const auto rows = volume.readAll();
  QCOMPARE(rows.size(), std::size_t{2});
  QCOMPARE(rows.front().driveLetter, std::string("C:"));
  QCOMPARE(wmi.instanceQueries, std::vector<std::string>{"SELECT * FROM UWF_Volume"});
  QVERIFY(wmi.projectionQueries.empty());

  wmi.queryResults.push_back({volumeRow(false, "C:", "Volume{c}", true, false, false, "a"), volumeRow(false, "c", "Volume{other}", true, false, false, "b")});
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, (void)volume.readAll());
  wmi.queryResults.push_back({volumeRow(true, "not-a-drive", "Volume{x}", true, false, false, "x")});
  QVERIFY_THROWS_EXCEPTION(WmiDecodeError, (void)volume.readAll());
}

void WmiApiBehaviorTests::volumeFileOperationsNormalizePathsAndRejectCrossVolumeTargets() {
  MemoryWmiOperations wmi;
  api::UwfVolume volume(wmi);
  const api::VolumeRow row{"volume-path", false, "C:", "Volume{c}", true, false, false};
  volume.commitFile(row, "c:\\Users\\a.txt");
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string("\\Users\\a.txt"));
  volume.commitFileDeletion(row, "\\Users\\b.txt");
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string("\\Users\\b.txt"));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, volume.commitFile(row, "D:\\other.txt"));

  WmiMethodOutput found;
  found.values.emplace("bFound", WmiValue::fromBool(false));
  wmi.readResults.push_back(found);
  QVERIFY(!volume.findExclusion(row, "C:\\Temp"));
  QCOMPARE(wmi.reads.back().inputs.at("FileName").toString(), std::string("\\Temp"));
}

void WmiApiBehaviorTests::volumeStateAndExclusionMutationsConfirmEveryPublicOperation() {
  MemoryWmiOperations wmi;
  api::UwfVolume volume(wmi);
  const api::VolumeRow row{"volume-path", false, "C:", "Volume{c}", true, false, false};

  wmi.objectResults.push_back(volumeRow(false, "C:", "Volume{c}", true, false, true, "volume-path"));
  volume.protectVolume(row);
  QCOMPARE(wmi.invocations.back().method, std::string("Protect"));

  api::VolumeRow protectedRow = row;
  protectedRow.isProtected = true;
  wmi.objectResults.push_back(volumeRow(false, "C:", "Volume{c}", true, false, false, "volume-path"));
  volume.unprotect(protectedRow);
  QCOMPARE(wmi.invocations.back().method, std::string("Unprotect"));

  wmi.objectResults.push_back(volumeRow(false, "C:", "Volume{c}", false, false, false, "volume-path"));
  volume.setBinding(row, api::VolumeBinding::VolumeName);
  QCOMPARE(wmi.invocations.back().method, std::string("SetBindByDriveLetter"));
  QCOMPARE(wmi.invocations.back().inputs.at("bBindByDriveLetter").toBool(), false);

  WmiMethodOutput found;
  found.values.emplace("bFound", WmiValue::fromBool(true));
  wmi.readResults.push_back(found);
  volume.addExclusion(row, "c:\\Cache\\File.dat");
  QCOMPARE(wmi.invocations.back().method, std::string("AddExclusion"));
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string("\\Cache\\File.dat"));

  WmiMethodOutput absent;
  absent.values.emplace("bFound", WmiValue::fromBool(false));
  wmi.readResults.push_back(absent);
  volume.removeExclusion(row, "C:\\Cache\\File.dat");
  QCOMPARE(wmi.invocations.back().method, std::string("RemoveExclusion"));
  QCOMPARE(wmi.invocations.back().inputs.at("FileName").toString(), std::string("\\Cache\\File.dat"));

  WmiMethodOutput exclusions;
  exclusions.arrays["ExcludedFiles"] = {{{"FileName", WmiValue::fromString("\\Cache\\One")}},
                                        {{"__MOF", WmiValue::fromString(R"(FileName = "\\Cache\\Two";)")}}};
  wmi.readResults.push_back(exclusions);
  const auto listed = volume.getExclusions(row);
  QCOMPARE(listed.size(), std::size_t{2});
  QCOMPARE(listed.back().fileName, std::string("\\Cache\\Two"));

  WmiMethodOutput noExclusions;
  noExclusions.values.emplace("ExcludedFiles", WmiValue{});
  wmi.readResults.push_back(noExclusions);
  volume.removeAllExclusions(row);
  QCOMPARE(wmi.invocations.back().method, std::string("RemoveAllExclusions"));

  WmiMethodOutput remaining;
  remaining.arrays["ExcludedFiles"] = {{{"FileName", WmiValue::fromString("\\StillThere")}}};
  wmi.readResults.push_back(remaining);
  QVERIFY_THROWS_EXCEPTION(WmiStateVerificationError, volume.removeAllExclusions(row));

  api::VolumeRow missingIdentity = row;
  missingIdentity.path.clear();
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, volume.protectVolume(missingIdentity));
}

void WmiApiBehaviorTests::volumeRegistrationHandlesExistingCreatedAndUncertainOutcomes() {
  const WmiRow current = volumeRow(true, "C:", "Volume{guid}", true, false, true, "current");
  const WmiRow next = volumeRow(false, "C:", "Volume{guid}", true, false, false, "next");

  MemoryWmiOperations existingWmi;
  existingWmi.queryResults.push_back({current, next});
  api::UwfVolume existingVolume(existingWmi);
  QCOMPARE(existingVolume.ensureNextSessionEntry("c").disposition, api::VolumeRegistrationDisposition::AlreadyPresent);
  QVERIFY(existingWmi.puts.empty());

  MemoryWmiOperations createdWmi;
  createdWmi.queryResults.push_back({current});
  createdWmi.objectResults.push_back(next);
  api::UwfVolume createdVolume(createdWmi);
  const auto created = createdVolume.ensureNextSessionEntry("C:");
  QCOMPARE(created.disposition, api::VolumeRegistrationDisposition::Created);
  QCOMPARE(createdWmi.puts.size(), std::size_t{1});
  QCOMPARE(createdWmi.putProperties.front().at("CurrentSession").toBool(), false);

  MemoryWmiOperations uncertainWmi;
  uncertainWmi.queryResults.push_back({current});
  uncertainWmi.putFailure = std::make_exception_ptr(WmiInvocationUncertain("create volume", "transport lost"));
  uncertainWmi.objectResults.push_back(next);
  api::UwfVolume uncertainVolume(uncertainWmi);
  QCOMPARE(uncertainVolume.ensureNextSessionEntry("C:").disposition, api::VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite);

  MemoryWmiOperations concurrentWmi;
  concurrentWmi.queryResults.push_back({current});
  concurrentWmi.putFailure =
      std::make_exception_ptr(WmiInfrastructureError(static_cast<std::int32_t>(WBEM_E_ALREADY_EXISTS), "create volume", "created concurrently"));
  concurrentWmi.objectResults.push_back(next);
  api::UwfVolume concurrentVolume(concurrentWmi);
  QCOMPARE(concurrentVolume.ensureNextSessionEntry("C:").disposition, api::VolumeRegistrationDisposition::ConcurrentlyCreated);

  MemoryWmiOperations mismatchWmi;
  mismatchWmi.queryResults.push_back({current});
  mismatchWmi.objectResults.push_back(volumeRow(false, "C:", "Volume{wrong}", true, false, false, "wrong"));
  api::UwfVolume mismatchVolume(mismatchWmi);
  QVERIFY_THROWS_EXCEPTION(WmiStateVerificationError, (void)mismatchVolume.ensureNextSessionEntry("C:"));

  MemoryWmiOperations missingCurrentWmi;
  missingCurrentWmi.queryResults.push_back({});
  api::UwfVolume missingCurrentVolume(missingCurrentWmi);
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, (void)missingCurrentVolume.ensureNextSessionEntry("C:"));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)missingCurrentVolume.ensureNextSessionEntry("not-a-drive"));
}

void WmiApiBehaviorTests::capabilityProbeDistinguishesAbsenceFromInfrastructureFailure() {
  MemoryWmiOperations wmi;
  QCOMPARE(probeUwfCapability(wmi), UwfCapability::Available);
  QCOMPARE(wmi.ensureConnectedCalls, 1);

  wmi.reportedClassStatus = WmiClassStatus::Missing;
  QCOMPARE(probeUwfCapability(wmi), UwfCapability::Unavailable);

  wmi.classStatusFailure = std::make_exception_ptr(WmiInfrastructureError(static_cast<std::int32_t>(WBEM_E_INVALID_NAMESPACE), "probe", "namespace missing"));
  QCOMPARE(probeUwfCapability(wmi), UwfCapability::Unavailable);

  wmi.classStatusFailure = std::make_exception_ptr(WmiInfrastructureError(static_cast<std::int32_t>(WBEM_E_ACCESS_DENIED), "probe", "access denied"));
  QVERIFY_THROWS_EXCEPTION(WmiInfrastructureError, (void)probeUwfCapability(wmi));
}

void WmiApiBehaviorTests::snapshotAssemblyPreservesSessionsAndSkipsUnprotectedCurrentExclusions() {
  MemoryWmiOperations unavailable;
  const auto unavailableSnapshot = readSnapshot(unavailable, UwfCapability::Unavailable, true);
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

  WmiMethodOutput currentC;
  currentC.arrays["ExcludedFiles"] = {{{"FileName", WmiValue::fromString("\\Current")}}};
  WmiMethodOutput nextC;
  nextC.arrays["ExcludedFiles"] = {{{"FileName", WmiValue::fromString("\\Next")}}};
  WmiMethodOutput nextD;
  nextD.arrays["ExcludedFiles"] = {{{"FileName", WmiValue::fromString("\\PreparedWhileUnprotected")}}};
  wmi.readResults.push_back(currentC);
  wmi.readResults.push_back(nextC);
  wmi.readResults.push_back(nextD);

  wmi.queryResults.push_back({registryFilterRow(true, true, false, "registry-current"), registryFilterRow(false, false, true, "registry-next")});
  WmiMethodOutput currentRegistry;
  currentRegistry.arrays["ExcludedKeys"] = {{{"RegistryKey", WmiValue::fromString("HKLM\\SOFTWARE\\Current")}}};
  WmiMethodOutput nextRegistry;
  nextRegistry.arrays["ExcludedKeys"] = {{{"RegistryKey", WmiValue::fromString("HKLM\\SOFTWARE\\Next")}}};
  wmi.readResults.push_back(currentRegistry);
  wmi.readResults.push_back(nextRegistry);

  const auto snapshot = readSnapshot(wmi, UwfCapability::Available, false);
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
    return WmiRow{{"DeviceID", WmiValue::fromString(drive)},
                  {"FileSystem", WmiValue::fromString(fs)},
                  {"VolumeName", WmiValue::fromString(std::string("Label-") + drive)},
                  {"Size", WmiValue::fromUInt(size)},
                  {"FreeSpace", WmiValue::fromUInt(size / 2)},
                  {"DriveType", WmiValue::fromInt(type)}};
  };
  constexpr std::uint64_t TiB = 1024ULL * 1024 * 1024 * 1024;
  wmi.queryResults.push_back(
      {logical("C:", "NTFS", TiB, 3), logical("D:", "exFAT", TiB, 3), logical("E:", "NTFS", 17 * TiB, 3), logical("F:", "FAT32", TiB, 2)});
  wmi.queryResults.push_back({{{"DeviceID", WmiValue::fromString("\\\\?\\Volume{c}\\")}, {"DriveLetter", WmiValue::fromString("C:")}},
                              {{"DeviceID", WmiValue::fromString("\\\\?\\Volume{d}\\")}, {"DriveLetter", WmiValue::fromString("D:")}},
                              {{"DeviceID", WmiValue::fromString("\\\\?\\Volume{hidden}\\")}, {"DriveLetter", WmiValue{}}}});

  const auto disks = enumerateDisks(wmi);
  QCOMPARE(disks.size(), std::size_t{4});
  QCOMPARE(disks[0].support, core::DiskSupport::Supported);
  QCOMPARE(disks[1].support, core::DiskSupport::FileSystemLimited);
  QCOMPARE(disks[2].support, core::DiskSupport::ExceedsMaxSize);
  QCOMPARE(disks[3].support, core::DiskSupport::NotFixedLocalDisk);
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
  QVERIFY_THROWS_EXCEPTION(WmiProtocolError, (void)enumerateDisks(duplicate));
}

}  // namespace

QTEST_APPLESS_MAIN(WmiApiBehaviorTests)

#include "WmiApiBehaviorTests.moc"
