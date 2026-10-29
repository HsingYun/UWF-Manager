// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 启动期系统检查返回结构化结果，本地化说明由 UI 层生成。

#include <string>

namespace uwf::windows {

enum class CheckStatus {
  Ok,                 // Windows 10/11 客户端且 Edition 受 UWF 支持
  UnsupportedSystem,  // 系统家族或 Edition 不在支持范围
};

struct SystemCheckResult {
  CheckStatus status = CheckStatus::Ok;
  std::string editionId;    // 注册表 EditionID，比如 "Enterprise"
  std::string productName;  // 已按真实系统家族修正的展示名
};

SystemCheckResult runSystemChecks();
bool isElevated();
std::string uwfmgrPath();

}  // namespace uwf::windows
