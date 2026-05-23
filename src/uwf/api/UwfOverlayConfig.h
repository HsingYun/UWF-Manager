#pragma once

// UWF_OverlayConfig —— 按 CurrentSession 存在 2 个实例
// （true 为当前会话，false 为下次会话）。
//
// 字段：Type / MaximumSize
// 方法：SetType(UInt32) / SetMaximumSize(UInt32)

#include <optional>
#include <string>
#include <vector>

#include "../wmi/WmiClient.h"
#include "../wmi/WmiResult.h"
#include "Types.h"

namespace uwf {

class UwfOverlayConfig {
 public:
  explicit UwfOverlayConfig(WmiSession& session) : m_session(session) {}

  // 读取全部实例（通常 2 条）。
  std::vector<api::OverlayConfigRow> readAll(std::string* error = nullptr) const;

  // 读取某一会话的配置；找不到返回 nullopt。
  [[nodiscard]] std::optional<api::OverlayConfigRow> read(bool currentSession, std::string* error = nullptr) const;

  [[nodiscard]] WmiResult setType(const api::OverlayConfigRow& row, api::OverlayType type) const;
  [[nodiscard]] WmiResult setMaximumSize(const api::OverlayConfigRow& row, uint32_t sizeMb) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf
