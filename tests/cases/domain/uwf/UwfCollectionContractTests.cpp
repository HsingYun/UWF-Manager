// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "domain/uwf/contracts/Backend.h"

namespace {
using namespace uwf::compat;
constexpr auto volumeA = "aaaaaaaa-1111-2222-3333-444444444444";
constexpr auto volumeB = "bbbbbbbb-1111-2222-3333-444444444444";
constexpr auto volumeC = "cccccccc-1111-2222-3333-444444444444";

// Assign equal values one identity to enumerate distinct orders without duplicate permutations.
// Expected ordering is literal and independent of production comparison helpers.
template <class T>
std::vector<std::vector<T>> allInputOrders(const std::vector<T>& values) {
  std::vector<std::size_t> indices(values.size());
  for (std::size_t index = 0; index < values.size(); ++index)
    indices[index] = static_cast<std::size_t>(std::find(values.begin(), values.begin() + static_cast<std::ptrdiff_t>(index), values[index]) - values.begin());
  std::sort(indices.begin(), indices.end());
  std::vector<std::vector<T>> orders;
  do {
    std::vector<T> order;
    order.reserve(values.size());
    for (const auto index : indices) order.push_back(values[index]);
    orders.push_back(std::move(order));
  } while (std::next_permutation(indices.begin(), indices.end()));
  return orders;
}

struct MalformedCollection {
  const char* name;
  Request request;
  Value value;
};
const auto& malformedCollections() {
  static const std::array cases{
      MalformedCollection{"inventory-invalid-utf8-id", GetVolumes{}, std::vector<Volume>{{"\xc3", {}, false, false, {}}}},
      MalformedCollection{"inventory-nul-id", GetVolumes{}, std::vector<Volume>{{std::string(volumeA) + '\0' + "suffix", {}, false, false, {}}}},
      MalformedCollection{"inventory-duplicate-canonical-id", GetVolumes{},
                          std::vector<Volume>{{volumeA, {}, false, false, {}}, {"Volume{AAAAAAAA-1111-2222-3333-444444444444}", "C:", true, true, false}}},
      MalformedCollection{"inventory-invalid-utf8-drive", GetVolumes{}, std::vector<Volume>{{volumeA, "\xc3", false, false, {}}}},
      MalformedCollection{"inventory-nul-drive", GetVolumes{}, std::vector<Volume>{{volumeA, std::string("C:\0tail", 7), false, false, {}}}},
      MalformedCollection{"inventory-not-a-drive-letter", GetVolumes{}, std::vector<Volume>{{volumeA, "C:\\Data", false, false, {}}}},
      MalformedCollection{"file-list-invalid-utf8", GetFileExclusions{volumeA}, std::vector<std::string>{"\\ok", "\\bad\xc3"}},
      MalformedCollection{"file-list-nul", GetFileExclusions{volumeA}, std::vector<std::string>{"\\ok", std::string("\\ok\0tail", 8)}},
      MalformedCollection{"registry-list-invalid-utf8", GetRegistryExclusions{}, std::vector<std::string>{"HKLM\\ok", "HKLM\\bad\xc3"}},
      MalformedCollection{"registry-list-nul", GetRegistryExclusions{}, std::vector<std::string>{"HKLM\\ok", std::string("HKLM\\ok\0tail", 12)}},
      MalformedCollection{"overlay-invalid-utf8", GetOverlayFiles{volumeA}, std::vector<OverlayFile>{{"\\ok", 0}, {"\\bad\xc3", 4096}}},
      MalformedCollection{"overlay-nul", GetOverlayFiles{volumeA}, std::vector<OverlayFile>{{"\\ok", 0}, {std::string("\\ok\0tail", 8), 4096}}},
      MalformedCollection{"inventory-wrong-variant", GetVolumes{}, false},
      MalformedCollection{"file-list-wrong-variant", GetFileExclusions{volumeA}, false},
      MalformedCollection{"registry-list-wrong-variant", GetRegistryExclusions{}, false},
      MalformedCollection{"overlay-wrong-variant", GetOverlayFiles{volumeA}, false}};
  return cases;
}
}  // namespace

class UwfCollectionContractTests final : public QObject {
  Q_OBJECT
 private slots:
  void everyInventoryPermutationHasCanonicalIdentityAndPreservesOptionalKnowledge_data() {
    QTest::addColumn<bool>("current");
    QTest::newRow("Current") << true;
    QTest::newRow("Next") << false;
  }
  void everyInventoryPermutationHasCanonicalIdentityAndPreservesOptionalKnowledge() {
    QFETCH(bool, current);
    const Request request = GetVolumes{current ? Session::Current : Session::Next};
    const std::vector<Volume> input{{volumeC, "f:", false, true, true},
                                    {"\\\\?\\Volume{BBBBBBBB-1111-2222-3333-444444444444}\\", "", true, false, false},
                                    {"Volume{AAAAAAAA-1111-2222-3333-444444444444}", "d:", false, false, {}}};
    const std::vector<Volume> expected{{volumeA, "D:", false, false, {}}, {volumeB, "", true, false, false}, {volumeC, "F:", false, true, true}};
    const auto orders = allInputOrders(input);
    QCOMPARE(orders.size(), std::size_t{6});
    for (const auto& order : orders) {
      const auto normalized = normalizeResponse(request, Response::success(order));
      QVERIFY(normalized.succeeded());
      QCOMPARE(normalized.effect, Effect::None);
      QVERIFY(std::get<std::vector<Volume>>(normalized.value) == expected);
      const auto again = normalizeResponse(request, normalized);
      QVERIFY(again.succeeded());
      QCOMPARE(again.effect, Effect::None);
      QVERIFY(std::get<std::vector<Volume>>(again.value) == expected);
    }
  }

  void everyUserListPermutationHasLiteralAsciiOrderWithoutRewritingOrDeduplication_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("current");
    for (const bool registry : {false, true})
      for (const bool current : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(registry ? QStringLiteral("registry") : QStringLiteral("file"),
                                                                 current ? QStringLiteral("Current") : QStringLiteral("Next"))))
            << registry << current;
  }
  void everyUserListPermutationHasLiteralAsciiOrderWithoutRewritingOrDeduplication() {
    QFETCH(bool, registry);
    QFETCH(bool, current);
    const auto session = current ? Session::Current : Session::Next;
    const Request request = registry ? Request{GetRegistryExclusions{session}} : Request{GetFileExclusions{volumeA, session}};
    const std::vector<std::string> fileInput{"\\\xc3\xa4", "\\alpha\\", "\\alpha", "\\\xc3\x84", "\\Alpha", "\\alpha"};
    const std::vector<std::string> fileExpected{"\\Alpha", "\\alpha", "\\alpha", "\\alpha\\", "\\\xc3\x84", "\\\xc3\xa4"};
    const std::vector<std::string> registryInput{"HKLM\\\xc3\xa4", "HKLM\\alpha\\", "HKLM\\alpha", "HKLM\\\xc3\x84", "HKLM\\Alpha", "HKLM\\alpha"};
    const std::vector<std::string> registryExpected{"HKLM\\Alpha", "HKLM\\alpha", "HKLM\\alpha", "HKLM\\alpha\\", "HKLM\\\xc3\x84", "HKLM\\\xc3\xa4"};
    const auto& expected = registry ? registryExpected : fileExpected;
    const auto orders = allInputOrders(registry ? registryInput : fileInput);
    QCOMPARE(orders.size(), std::size_t{360});
    for (const auto& order : orders) {
      auto source = Response::success(order);
      source.warnings = {"provider observation retained"};
      const auto normalized = normalizeResponse(request, std::move(source));
      QVERIFY(normalized.succeeded());
      QCOMPARE(normalized.effect, Effect::None);
      QVERIFY(std::get<std::vector<std::string>>(normalized.value) == expected);
      QCOMPARE(normalized.warnings, std::vector<std::string>{"provider observation retained"});
      const auto again = normalizeResponse(request, normalized);
      QVERIFY(again.succeeded());
      QVERIFY(std::get<std::vector<std::string>>(again.value) == expected);
      QCOMPARE(again.warnings, normalized.warnings);
    }
  }

  void everyOverlayRecordPermutationKeepsStreamsDuplicatesAndUnsignedByteCounts() {
    const Request request = GetOverlayFiles{volumeA};
    constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
    const std::vector<OverlayFile> input{{"\\alpha", maximum}, {"\\Alpha", 4096}, {"\\alpha:stream", 1024}, {"\\alpha", 0}, {"\\alpha", 0}, {"\\\xc3\x84", 0}};
    const std::vector<OverlayFile> expected{{"\\Alpha", 4096},    {"\\alpha", 0},           {"\\alpha", 0},
                                            {"\\alpha", maximum}, {"\\alpha:stream", 1024}, {"\\\xc3\x84", 0}};
    const auto orders = allInputOrders(input);
    QCOMPARE(orders.size(), std::size_t{360});
    for (const auto& order : orders) {
      const auto normalized = normalizeResponse(request, Response::success(order));
      QVERIFY(normalized.succeeded());
      QCOMPARE(normalized.effect, Effect::None);
      QVERIFY(std::get<std::vector<OverlayFile>>(normalized.value) == expected);
      const auto again = normalizeResponse(request, normalized);
      QVERIFY(again.succeeded());
      QVERIFY(std::get<std::vector<OverlayFile>>(again.value) == expected);
    }
  }

  void malformedCollectionsStayProtocolFailuresInsteadOfPartialSuccess_data() {
    QTest::addColumn<int>("caseIndex");
    for (std::size_t index = 0; index < malformedCollections().size(); ++index) QTest::newRow(malformedCollections()[index].name) << static_cast<int>(index);
  }
  void malformedCollectionsStayProtocolFailuresInsteadOfPartialSuccess() {
    QFETCH(int, caseIndex);
    const auto& entry = malformedCollections()[static_cast<std::size_t>(caseIndex)];
    const auto normalized = normalizeResponse(entry.request, Response{entry.value, {}, Effect::None, {}, {}});
    QVERIFY(normalized.failure);
    QCOMPARE(normalized.failure->kind, ErrorKind::Protocol);
    QCOMPARE(normalized.effect, Effect::None);
    QCOMPARE(normalized.failure->effect, Effect::None);
  }

  void emptyCollectionsRemainValidWithoutInventedMetadata() {
    const std::array requests{Request{GetVolumes{Session::Current}},
                              Request{GetVolumes{Session::Next}},
                              Request{GetFileExclusions{volumeA, Session::Current}},
                              Request{GetFileExclusions{volumeA, Session::Next}},
                              Request{GetRegistryExclusions{Session::Current}},
                              Request{GetRegistryExclusions{Session::Next}},
                              Request{GetOverlayFiles{volumeA}}};
    for (const auto& request : requests) {
      const Value empty = std::holds_alternative<GetVolumes>(request)        ? Value{std::vector<Volume>{}}
                          : std::holds_alternative<GetOverlayFiles>(request) ? Value{std::vector<OverlayFile>{}}
                                                                             : Value{std::vector<std::string>{}};
      const auto normalized = normalizeResponse(request, Response{empty, {}, Effect::None, {}, {}});
      QVERIFY(normalized.succeeded());
      QCOMPARE(normalized.effect, Effect::None);
      QVERIFY(normalized.value == empty);
      QVERIFY(normalizeResponse(request, normalized).value == empty);
    }
  }
};

QTEST_APPLESS_MAIN(UwfCollectionContractTests)
#include "UwfCollectionContractTests.moc"
