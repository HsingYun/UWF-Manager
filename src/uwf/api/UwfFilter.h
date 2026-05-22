#pragma once

// UWF_Filter —— 筛选器单例。读 CurrentEnabled / NextEnabled；
// 写接口：Enable / Disable / ResetSettings / ShutdownSystem / RestartSystem。
// 所有写接口都真实调用 WMI ExecMethod（非 dry-run）。

#include <optional>
#include <string>

#include "../wmi/WmiClient.h"
#include "Types.h"

namespace uwf {

class UwfFilter {
 public:
  explicit UwfFilter(WmiSession& session) : m_session(session) {}

  // 查询 UWF_Filter 单例；没有行或查询失败返回 nullopt。
  [[nodiscard]] std::optional<api::FilterRow> read(std::string* error = nullptr) const;

  // 在下次重启时启用 UWF。
  bool enable(const api::FilterRow& row, std::string* error = nullptr) const;
  // 在下次重启时禁用 UWF。
  bool disable(const api::FilterRow& row, std::string* error = nullptr) const;
  // 将 UWF 配置重置为原始设置。
  bool resetSettings(const api::FilterRow& row, std::string* error = nullptr) const;
  // 安全关闭受 UWF 保护的系统（即使 overlay 已满）。
  bool shutdownSystem(const api::FilterRow& row, std::string* error = nullptr) const;
  // 安全重启受 UWF 保护的系统（即使 overlay 已满）。
  bool restartSystem(const api::FilterRow& row, std::string* error = nullptr) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf
