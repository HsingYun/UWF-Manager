// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <qt_windows.h>
#include <wbemcli.h>

#include <QTest>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/windows/management/wmi/WmiError.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/management/wmi/WmiRowUtil.h"

namespace {
class WmiApiBehaviorTests final : public QObject {
  Q_OBJECT
 private slots:
  void wmiValueConversionsRejectLossAndNonFiniteInputs();
  void wmiValueConversionMatrixCoversKindsAndBoundaries();
  void rowDecodingDistinguishesMissingNullWrongTypeAndEmpty();
  void embeddedMofDecodingHandlesEscapesAndMissingFields();
  void embeddedMofStringsPreserveSerializedCharacters_data();
  void embeddedMofStringsPreserveSerializedCharacters();
  void embeddedMofPropertiesHaveStructuralBoundaries();
  void embeddedMofUInt64LiteralsPreserveBoundaries();
  void embeddedMofRejectsMalformedAndAmbiguousProperties_data();
  void embeddedMofRejectsMalformedAndAmbiguousProperties();
  void embeddedDirectFieldsCannotBeMaskedByMof();
  void overlayMofSizeCannotComeFromAFileName_data();
  void overlayMofSizeCannotComeFromAFileName();
  void wmiErrorsPreserveKnownAndUnknownCodes();
};
void WmiApiBehaviorTests::wmiValueConversionsRejectLossAndNonFiniteInputs() {
  QCOMPARE(uwf::windows::wmi::WmiValue::fromBool(true).toBool(), true);
  QCOMPARE(uwf::windows::wmi::WmiValue::fromInt(-1).toInt(), -1);
  QCOMPARE(uwf::windows::wmi::WmiValue::fromUInt(std::numeric_limits<uint32_t>::max()).toUInt(), std::numeric_limits<uint32_t>::max());
  QCOMPARE(uwf::windows::wmi::WmiValue::fromString("18446744073709551615").toULongLong(), std::numeric_limits<uint64_t>::max());
  QCOMPARE(uwf::windows::wmi::WmiValue::fromString("false").toBool(), false);
  QCOMPARE(uwf::windows::wmi::WmiValue::fromDouble(1.5).toDouble(), 1.5);

  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)uwf::windows::wmi::WmiValue{}.toString());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)uwf::windows::wmi::WmiValue::fromInt(-1).toUInt());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError,
                           (void)uwf::windows::wmi::WmiValue::fromUInt(static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1).toUInt());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)uwf::windows::wmi::WmiValue::fromString("12x").toInt());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)uwf::windows::wmi::WmiValue::fromString("nan").toDouble());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError,
                           (void)uwf::windows::wmi::WmiValue::fromDouble(std::numeric_limits<double>::infinity()).toString());
}

void WmiApiBehaviorTests::wmiValueConversionMatrixCoversKindsAndBoundaries() {
  QCOMPARE(uwf::windows::wmi::WmiValue::fromBool(false).toInt64(), int64_t{0});
  QCOMPARE(uwf::windows::wmi::WmiValue::fromBool(true).toULongLong(), uint64_t{1});
  QCOMPARE(uwf::windows::wmi::WmiValue::fromBool(true).toDouble(), 1.0);
  QCOMPARE(uwf::windows::wmi::WmiValue::fromBool(false).toString(), std::string("false"));

  QCOMPARE(uwf::windows::wmi::WmiValue::fromInt(std::numeric_limits<int32_t>::min()).toInt(), std::numeric_limits<int32_t>::min());
  QCOMPARE(uwf::windows::wmi::WmiValue::fromInt(std::numeric_limits<int32_t>::max()).toInt(), std::numeric_limits<int32_t>::max());
  QCOMPARE(uwf::windows::wmi::WmiValue::fromInt(std::numeric_limits<int64_t>::min()).toInt64(), std::numeric_limits<int64_t>::min());
  QCOMPARE(uwf::windows::wmi::WmiValue::fromInt(std::numeric_limits<int64_t>::max()).toInt64(), std::numeric_limits<int64_t>::max());
  QCOMPARE(uwf::windows::wmi::WmiValue::fromUInt(std::numeric_limits<uint64_t>::max()).toULongLong(), std::numeric_limits<uint64_t>::max());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError,
                           (void)uwf::windows::wmi::WmiValue::fromInt(std::numeric_limits<int64_t>::min()).toULongLong());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError,
                           (void)uwf::windows::wmi::WmiValue::fromUInt(static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1).toInt64());

  constexpr uint64_t kLargestConsecutiveDoubleInteger = uint64_t{1} << std::numeric_limits<double>::digits;
  QCOMPARE(uwf::windows::wmi::WmiValue::fromUInt(kLargestConsecutiveDoubleInteger).toDouble(), static_cast<double>(kLargestConsecutiveDoubleInteger));
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError,
                           (void)uwf::windows::wmi::WmiValue::fromUInt(kLargestConsecutiveDoubleInteger + 1).toDouble());
  QCOMPARE(uwf::windows::wmi::WmiValue::fromDouble(-0.0).toBool(), false);
  QCOMPARE(uwf::windows::wmi::WmiValue::fromDouble(42.0).toInt64(), int64_t{42});
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)uwf::windows::wmi::WmiValue::fromDouble(42.5).toInt64());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)uwf::windows::wmi::WmiValue::fromDouble(-1.0).toULongLong());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError,
                           (void)uwf::windows::wmi::WmiValue::fromDouble(std::numeric_limits<double>::quiet_NaN()).toBool());

  for (const auto* value : {"true", "TRUE", "TrUe", "false", "FALSE", "FaLsE"}) {
    QCOMPARE(uwf::windows::wmi::WmiValue::fromString(value).toBool(), value[0] == 't' || value[0] == 'T');
  }
  QCOMPARE(uwf::windows::wmi::WmiValue::fromString("-9223372036854775808").toInt64(), std::numeric_limits<int64_t>::min());
  QCOMPARE(uwf::windows::wmi::WmiValue::fromString("18446744073709551615").toULongLong(), std::numeric_limits<uint64_t>::max());
  for (const auto* value : {"", " ", "+1", "01x", "18446744073709551616", "-9223372036854775809"}) {
    QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)uwf::windows::wmi::WmiValue::fromString(value).toULongLong());
  }

  const uwf::windows::wmi::WmiValue missing;
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)missing.toBool());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)missing.toInt64());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)missing.toULongLong());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)missing.toDouble());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiValueConversionError, (void)missing.toString());
}

void WmiApiBehaviorTests::rowDecodingDistinguishesMissingNullWrongTypeAndEmpty() {
  uwf::windows::wmi::WmiRow row{{"Bool", uwf::windows::wmi::WmiValue::fromBool(true)},
                                {"Int", uwf::windows::wmi::WmiValue::fromString("-42")},
                                {"UInt", uwf::windows::wmi::WmiValue::fromUInt(42)},
                                {"Text", uwf::windows::wmi::WmiValue::fromString("")},
                                {"Null", uwf::windows::wmi::WmiValue{}}};
  QCOMPARE(uwf::windows::wmi::row::requireBool(row, "Bool"), true);
  QCOMPARE(uwf::windows::wmi::row::requireInt(row, "Int"), -42);
  QCOMPARE(uwf::windows::wmi::row::requireUInt(row, "UInt"), 42u);
  QVERIFY(uwf::windows::wmi::row::requireString(row, "Text").empty());
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError,
                           (void)uwf::windows::wmi::row::requireString(row, "Text", uwf::windows::wmi::row::EmptyString::Reject));
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireBool(row, "Missing"));
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireBool(row, "Null"));
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireBool(row, "UInt"));
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireString(row, "Null"));
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireUInt(row, "Bool"));
}

void WmiApiBehaviorTests::embeddedMofDecodingHandlesEscapesAndMissingFields() {
  const std::string mof = R"(instance of X { FileName = "\\Users\"quoted\".txt"; Other = "x"; };)";
  QCOMPARE(uwf::windows::wmi::row::extractFromMof(mof, "FileName"), std::string("\\Users\"quoted\".txt"));
  QCOMPARE(uwf::windows::wmi::row::extractFromMof(mof, "Missing"), std::string{});
  QCOMPARE(uwf::windows::wmi::row::extractFromMof(R"(X { A.B = "value"; };)", "A.B"), std::string("value"));

  uwf::windows::wmi::WmiRow direct{{"FileName", uwf::windows::wmi::WmiValue::fromString("\\direct")}};
  QCOMPARE(uwf::windows::wmi::row::requireEmbeddedString(direct, "FileName"), std::string("\\direct"));
  uwf::windows::wmi::WmiRow embedded{{"__MOF", uwf::windows::wmi::WmiValue::fromString(mof)}};
  QCOMPARE(uwf::windows::wmi::row::requireEmbeddedString(embedded, "FileName"), std::string("\\Users\"quoted\".txt"));
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireEmbeddedString(embedded, "Absent"));
}

void WmiApiBehaviorTests::embeddedMofStringsPreserveSerializedCharacters_data() {
  QTest::addColumn<QString>("serialized");
  QTest::addColumn<QString>("expected");
  // Captured from an in-memory IWbemClassObject::GetObjectText probe. The
  // provider's escaped text is distinct from a name containing a backslash.
  QTest::newRow("tab") << QStringLiteral("A\\tB") << QStringLiteral("A\tB");
  QTest::newRow("line-feed") << QStringLiteral("A\\nB") << QStringLiteral("A\nB");
  QTest::newRow("carriage-return") << QStringLiteral("A\rB") << QStringLiteral("A\rB");
  QTest::newRow("backspace") << QStringLiteral("A\bB") << QStringLiteral("A\bB");
  QTest::newRow("form-feed") << QStringLiteral("A\fB") << QStringLiteral("A\fB");
  QTest::newRow("vertical-tab") << QStringLiteral("A\vB") << QStringLiteral("A\vB");
  QTest::newRow("control-one") << QStringLiteral("A\001B") << QStringLiteral("A\001B");
  QTest::newRow("control-thirty-one") << QStringLiteral("A\037B") << QStringLiteral("A\037B");
  QTest::newRow("delete-character") << QStringLiteral("A\177B") << QStringLiteral("A\177B");
  QTest::newRow("literal-backslash-n") << QStringLiteral("A\\\\nB") << QStringLiteral("A\\nB");
  QTest::newRow("literal-backslash-t") << QStringLiteral("A\\\\tB") << QStringLiteral("A\\tB");
  QTest::newRow("quote") << QStringLiteral("A\\\"B") << QStringLiteral("A\"B");
  QTest::newRow("unicode") << QStringLiteral("A\u00e9\u2028B") << QStringLiteral("A\u00e9\u2028B");
}

void WmiApiBehaviorTests::embeddedMofStringsPreserveSerializedCharacters() {
  QFETCH(QString, serialized);
  QFETCH(QString, expected);
  const auto mof = QStringLiteral("instance of X { KeyName = \"") + serialized + QStringLiteral("\"; };");
  const auto expectedText = expected.toStdString();
  QCOMPARE(uwf::windows::wmi::row::extractFromMof(mof.toStdString(), "KeyName"), expectedText);
  const uwf::windows::wmi::WmiRow embedded{{"__MOF", uwf::windows::wmi::WmiValue::fromString(mof.toStdString())}};
  QCOMPARE(uwf::windows::wmi::row::requireEmbeddedString(embedded, "KeyName"), expectedText);
  const uwf::windows::wmi::WmiRow direct{{"KeyName", uwf::windows::wmi::WmiValue::fromString(expectedText)}};
  QCOMPARE(uwf::windows::wmi::row::requireEmbeddedString(direct, "KeyName"), expectedText);
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
  const uwf::windows::wmi::WmiRow row{{"__MOF", uwf::windows::wmi::WmiValue::fromString(mof)}};
  QCOMPARE(uwf::windows::wmi::row::requireEmbeddedString(row, "filename"), std::string(R"(\Data\FileSize = 999;.txt)"));
  QCOMPARE(uwf::windows::wmi::row::requireEmbeddedUInt64(row, "FileSize"), uint64_t{42});
  QCOMPARE(uwf::windows::wmi::row::extractFromMof(R"(OtherFileName = "prefix"; FileName = "\\real.txt";)", "FileName"), std::string("\\real.txt"));
  QCOMPARE(uwf::windows::wmi::row::extractFromMof(R"(OtherFileName = "prefix";)", "FileName"), std::string{});
  QCOMPARE(uwf::windows::wmi::row::extractFromMof(R"(FileName = "\\Data\unknown;/*literal*/{x}\"quoted\".txt";)", "FileName"),
           std::string(R"(\Data\unknown;/*literal*/{x}"quoted".txt)"));
  const uwf::windows::wmi::WmiRow wrongType{{"__MOF", uwf::windows::wmi::WmiValue::fromString(R"(OtherFileName = "prefix"; FileName = 42;)")}};
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireEmbeddedString(wrongType, "FileName"));
}

void WmiApiBehaviorTests::embeddedMofUInt64LiteralsPreserveBoundaries() {
  for (const auto& [literal, expected] : std::vector<std::pair<std::string, uint64_t>>{{"0", uint64_t{0}},
                                                                                       {"\"0\"", uint64_t{0}},
                                                                                       {"42", uint64_t{42}},
                                                                                       {"\"42\"", uint64_t{42}},
                                                                                       {"18446744073709551615", std::numeric_limits<uint64_t>::max()},
                                                                                       {"\"18446744073709551615\"", std::numeric_limits<uint64_t>::max()}}) {
    const uwf::windows::wmi::WmiRow row{{"__MOF", uwf::windows::wmi::WmiValue::fromString("FileSize = " + literal + ";")}};
    QCOMPARE(uwf::windows::wmi::row::requireEmbeddedUInt64(row, "FileSize"), expected);
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
  const uwf::windows::wmi::WmiRow row{{"__MOF", uwf::windows::wmi::WmiValue::fromString(mof.toStdString())}};
  QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireEmbeddedUInt64(row, "FileSize"));
}

void WmiApiBehaviorTests::embeddedDirectFieldsCannotBeMaskedByMof() {
  using Value = uwf::windows::wmi::WmiValue;
  const uwf::windows::wmi::WmiRow direct{
      {"FileName", Value::fromString("\\direct")}, {"FileSize", Value::fromUInt(42)}, {"__MOF", Value::fromString("not MOF")}};
  QCOMPARE(uwf::windows::wmi::row::requireEmbeddedString(direct, "FileName"), std::string("\\direct"));
  QCOMPARE(uwf::windows::wmi::row::requireEmbeddedUInt64(direct, "FileSize"), uint64_t{42});
  const auto mof = Value::fromString(R"(FileName = "\\fallback"; FileSize = 999;)");
  for (const auto& value : std::vector<Value>{Value{}, Value::fromBool(true), Value::fromInt(-1), Value::fromDouble(42), Value::fromString("bad")}) {
    const uwf::windows::wmi::WmiRow row{{"FileSize", value}, {"__MOF", mof}};
    QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireEmbeddedUInt64(row, "FileSize"));
  }
  for (const auto& value : std::vector<Value>{Value{}, Value::fromUInt(42), Value::fromString("")}) {
    const uwf::windows::wmi::WmiRow row{{"FileName", value}, {"__MOF", mof}};
    QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)uwf::windows::wmi::row::requireEmbeddedString(row, "FileName"));
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
  const auto mof = std::string(R"(instance of UWF_OverlayFile { FileName = "\\Data\\FileSize = 999;.txt"; FileSize = )") + literal.toStdString() + "; };";
  uwf::windows::wmi::WmiMethodOutput output;
  output.arrays["OverlayFiles"] = {{{"__MOF", uwf::windows::wmi::WmiValue::fromString(mof)}}};
  const auto decode = [&] {
    return uwf::windows::wmi::row::readArrayOutput<std::pair<std::string, std::uint64_t>>(output, "OverlayFiles", [](const auto& row) {
      return std::pair{uwf::windows::wmi::row::requireEmbeddedString(row, "FileName"), uwf::windows::wmi::row::requireEmbeddedUInt64(row, "FileSize")};
    });
  };
  if (!valid) {
    QVERIFY_THROWS_EXCEPTION(uwf::windows::wmi::WmiDecodeError, (void)decode());
    return;
  }
  const auto files = decode();
  QCOMPARE(files.size(), std::size_t{1});
  QCOMPARE(files.front().first, std::string(R"(\Data\FileSize = 999;.txt)"));
  QCOMPARE(files.front().second, uint64_t{42});
}

void WmiApiBehaviorTests::wmiErrorsPreserveKnownAndUnknownCodes() {
  const uwf::windows::wmi::WmiError success(0);
  QVERIFY(success.isKnown());
  QVERIFY(!success.isFailure());
  QCOMPARE(success.name(), std::string_view("S_OK"));

  const uwf::windows::wmi::WmiError missing(static_cast<int32_t>(WBEM_E_NOT_FOUND));
  QVERIFY(missing.isKnown());
  QVERIFY(missing.isFailure());
  QCOMPARE(missing.code(), uwf::windows::wmi::WmiErrorCode::NotFound);
  QVERIFY(!missing.description().empty());

  const uwf::windows::wmi::WmiError unknown(static_cast<int32_t>(0x81234567u));
  QVERIFY(!unknown.isKnown());
  QCOMPARE(unknown.name(), std::string_view("WBEM_E_UNKNOWN"));
  QVERIFY(unknown.description().empty());
  const auto code = uwf::windows::wmi::makeWmiErrorCode(static_cast<int32_t>(0x81234567u));
  QVERIFY(QString::fromStdString(code.message()).contains(QStringLiteral("81234567"), Qt::CaseInsensitive));
  const uwf::windows::wmi::WmiProviderError absent(static_cast<uint32_t>(WBEM_E_NOT_FOUND), "lookup");
  QVERIFY(absent.hasCode(uwf::windows::wmi::WmiErrorCode::NotFound));
  QVERIFY(!absent.hasCode(uwf::windows::wmi::WmiErrorCode::Failed));
  QVERIFY(!uwf::windows::wmi::WmiProviderError(2, "lookup").hasCode(uwf::windows::wmi::WmiErrorCode::NotFound));

  // Generic CIM/WMI callers retain standard exception handling and raw facts;
  // UWF-specific error kinds are assigned only by the UWF backend adapter.
  const uwf::windows::wmi::WmiInfrastructureError infrastructure(static_cast<int32_t>(0x81234567u), "read CIM inventory", "original diagnostic");
  const std::system_error& standard = infrastructure;
  QVERIFY(QString::fromUtf8(standard.what()).contains(u"original diagnostic"));
  QCOMPARE(static_cast<uint32_t>(standard.code().value()), uint32_t{0x81234567});
  QCOMPARE(infrastructure.diagnostic().code, static_cast<int32_t>(0x81234567u));
  QCOMPARE(infrastructure.diagnostic().result, uint32_t{0});
  QCOMPARE(infrastructure.writeEffect(), uwf::windows::wmi::WriteEffect::None);
  const uwf::windows::wmi::WmiProviderError provider(87, "invoke CIM method", "provider diagnostic", uwf::windows::wmi::WriteEffect::Uncertain);
  QCOMPARE(provider.diagnostic().code, int32_t{0});
  QCOMPARE(provider.diagnostic().result, uint32_t{87});
  QCOMPARE(provider.writeEffect(), uwf::windows::wmi::WriteEffect::Uncertain);
  const uwf::windows::wmi::WmiCancelled canceled("enumerate CIM inventory");
  QVERIFY(canceled.code() == std::errc::operation_canceled);
  QCOMPARE(canceled.writeEffect(), uwf::windows::wmi::WriteEffect::None);
}

}  // namespace
QTEST_APPLESS_MAIN(WmiApiBehaviorTests)
#include "WmiApiBehaviorTests.moc"
