// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace uwf::app::wmi {

class WmiValueConversionError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class WmiValue {
 public:
  enum class Kind { None, Bool, Int, UInt, Double, String };

  WmiValue() = default;
  static WmiValue fromBool(bool v);
  static WmiValue fromInt(int64_t v);
  static WmiValue fromUInt(uint64_t v);
  static WmiValue fromDouble(double v);
  static WmiValue fromString(std::string v);

  [[nodiscard]] Kind kind() const { return m_kind; }
  [[nodiscard]] bool isValid() const { return m_kind != Kind::None; }

  // 各种 to*() 都会做可验证的类型转换：比如 string "1" 可转为整数。
  // 无值、格式错误或越界抛出 WmiValueConversionError，不用 ok/def
  // 参数把数据损坏静默折叠成业务值。
  [[nodiscard]] bool toBool() const;
  [[nodiscard]] int32_t toInt() const;
  [[nodiscard]] uint32_t toUInt() const;
  [[nodiscard]] int64_t toInt64() const;
  [[nodiscard]] uint64_t toULongLong() const;
  [[nodiscard]] double toDouble() const;
  [[nodiscard]] std::string toString() const;

 private:
  Kind m_kind = Kind::None;
  bool m_bool = false;
  int64_t m_int = 0;
  uint64_t m_uint = 0;
  double m_double = 0.0;
  std::string m_string;
};

using WmiRow = std::map<std::string, WmiValue>;

struct WmiMethodOutput {
  WmiRow values;
  std::map<std::string, std::vector<WmiRow>> arrays;
};

enum class WmiClassStatus { Present, Missing };
enum class WmiPutMode { CreateOnly, UpdateOnly };

}  // namespace uwf::app::wmi
