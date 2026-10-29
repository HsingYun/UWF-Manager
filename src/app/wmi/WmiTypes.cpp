// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WmiTypes.h"

#include <bit>
#include <charconv>
#include <cmath>
#include <format>
#include <limits>
#include <utility>

#include "shared/StringUtil.h"

namespace uwf::app::wmi {

namespace {
bool isExactlyRepresentableAsDouble(const uint64_t magnitude) noexcept {
  if (magnitude == 0) return true;
  constexpr int kDoubleSignificandBits = std::numeric_limits<double>::digits;
  const int significantBits = std::numeric_limits<uint64_t>::digits - std::countl_zero(magnitude);
  if (significantBits <= kDoubleSignificandBits) return true;
  const int discardedBits = significantBits - kDoubleSignificandBits;
  const uint64_t discardedMask = (uint64_t{1} << discardedBits) - 1;
  return (magnitude & discardedMask) == 0;
}

uint64_t magnitudeOf(const int64_t value) noexcept { return value < 0 ? static_cast<uint64_t>(-(value + 1)) + 1 : static_cast<uint64_t>(value); }

}  // namespace

WmiValue WmiValue::fromBool(bool v) {
  WmiValue x;
  x.m_kind = Kind::Bool;
  x.m_bool = v;
  return x;
}
WmiValue WmiValue::fromInt(int64_t v) {
  WmiValue x;
  x.m_kind = Kind::Int;
  x.m_int = v;
  return x;
}
WmiValue WmiValue::fromUInt(uint64_t v) {
  WmiValue x;
  x.m_kind = Kind::UInt;
  x.m_uint = v;
  return x;
}
WmiValue WmiValue::fromDouble(double v) {
  WmiValue x;
  x.m_kind = Kind::Double;
  x.m_double = v;
  return x;
}
WmiValue WmiValue::fromString(std::string v) {
  WmiValue x;
  x.m_kind = Kind::String;
  x.m_string = std::move(v);
  return x;
}

bool WmiValue::toBool() const {
  switch (m_kind) {
    case Kind::Bool:
      return m_bool;
    case Kind::Int:
      return m_int != 0;
    case Kind::UInt:
      return m_uint != 0;
    case Kind::Double:
      if (!std::isfinite(m_double)) throw WmiValueConversionError("non-finite Double is not a boolean");
      return m_double != 0.0;
    case Kind::String: {
      const std::string folded = toLowerAscii(m_string);
      if (folded == "true" || folded == "1") return true;
      if (folded == "false" || folded == "0") return false;
      throw WmiValueConversionError(std::format("'{}' is not a boolean", m_string));
    }
    default:
      throw WmiValueConversionError("null WMI value is not a boolean");
  }
}

int32_t WmiValue::toInt() const {
  const int64_t v = toInt64();
  if (v < INT32_MIN || v > INT32_MAX) throw WmiValueConversionError(std::format("{} is outside the Int32 range", v));
  return static_cast<int32_t>(v);
}

uint32_t WmiValue::toUInt() const {
  const uint64_t v = toULongLong();
  if (v > UINT32_MAX) throw WmiValueConversionError(std::format("{} is outside the UInt32 range", v));
  return static_cast<uint32_t>(v);
}

int64_t WmiValue::toInt64() const {
  switch (m_kind) {
    case Kind::Bool:
      return m_bool ? 1 : 0;
    case Kind::Int:
      return m_int;
    case Kind::UInt:
      if (m_uint > static_cast<uint64_t>(INT64_MAX)) throw WmiValueConversionError(std::format("{} is outside the Int64 range", m_uint));
      return static_cast<int64_t>(m_uint);
    case Kind::Double:
      if (!std::isfinite(m_double) || std::trunc(m_double) != m_double || m_double < static_cast<double>(INT64_MIN) ||
          m_double >= -static_cast<double>(INT64_MIN)) {
        throw WmiValueConversionError(std::format("{} is not an exact Int64", m_double));
      }
      return static_cast<int64_t>(m_double);
    case Kind::String: {
      int64_t value = 0;
      const auto [end, error] = std::from_chars(m_string.data(), m_string.data() + m_string.size(), value, 10);
      if (error != std::errc{} || end != m_string.data() + m_string.size()) {
        throw WmiValueConversionError(std::format("'{}' is not an Int64", m_string));
      }
      return value;
    }
    default:
      throw WmiValueConversionError("null WMI value is not an Int64");
  }
}

uint64_t WmiValue::toULongLong() const {
  switch (m_kind) {
    case Kind::Bool:
      return m_bool ? 1u : 0u;
    case Kind::Int:
      if (m_int < 0) throw WmiValueConversionError(std::format("{} is outside the UInt64 range", m_int));
      return static_cast<uint64_t>(m_int);
    case Kind::UInt:
      return m_uint;
    case Kind::Double:
      if (!std::isfinite(m_double) || std::trunc(m_double) != m_double || m_double < 0 || m_double >= -static_cast<double>(INT64_MIN) * 2.0) {
        throw WmiValueConversionError(std::format("{} is not an exact UInt64", m_double));
      }
      return static_cast<uint64_t>(m_double);
    case Kind::String: {
      uint64_t value = 0;
      const auto [end, error] = std::from_chars(m_string.data(), m_string.data() + m_string.size(), value, 10);
      if (error != std::errc{} || end != m_string.data() + m_string.size()) {
        throw WmiValueConversionError(std::format("'{}' is not a UInt64", m_string));
      }
      return value;
    }
    default:
      throw WmiValueConversionError("null WMI value is not a UInt64");
  }
}

double WmiValue::toDouble() const {
  switch (m_kind) {
    case Kind::Bool:
      return m_bool ? 1.0 : 0.0;
    case Kind::Int:
      if (!isExactlyRepresentableAsDouble(magnitudeOf(m_int))) {
        throw WmiValueConversionError(std::format("{} cannot be represented exactly as a Double", m_int));
      }
      return static_cast<double>(m_int);
    case Kind::UInt:
      if (!isExactlyRepresentableAsDouble(m_uint)) {
        throw WmiValueConversionError(std::format("{} cannot be represented exactly as a Double", m_uint));
      }
      return static_cast<double>(m_uint);
    case Kind::Double:
      if (!std::isfinite(m_double)) throw WmiValueConversionError("non-finite WMI value is not a Double");
      return m_double;
    case Kind::String: {
      double value = 0.0;
      const auto [end, error] = std::from_chars(m_string.data(), m_string.data() + m_string.size(), value, std::chars_format::general);
      if (error != std::errc{} || end != m_string.data() + m_string.size() || !std::isfinite(value)) {
        throw WmiValueConversionError(std::format("'{}' is not a Double", m_string));
      }
      return value;
    }
    default:
      throw WmiValueConversionError("null WMI value is not a Double");
  }
}

std::string WmiValue::toString() const {
  switch (m_kind) {
    case Kind::Bool:
      return m_bool ? "true" : "false";
    case Kind::Int:
      return std::format("{}", m_int);
    case Kind::UInt:
      return std::format("{}", m_uint);
    case Kind::Double:
      if (!std::isfinite(m_double)) throw WmiValueConversionError("non-finite WMI value is not a String");
      return std::format("{}", m_double);
    case Kind::String:
      return m_string;
    default:
      throw WmiValueConversionError("null WMI value is not a String");
  }
}

}  // namespace uwf::app::wmi
