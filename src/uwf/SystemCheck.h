#pragma once

// SystemCheck：启动时的前置检查。
// 返回的结果是"结构化"的——具体的中文描述由 UI 层（main.cpp）负责组合，
// 这样核心层不引入任何显示相关的文本。

#include <string>

namespace uwf {

enum class CheckStatus {
  Ok,                  // 一切就绪
  UnsupportedEdition,  // 不是 Enterprise / Education / IoT Enterprise
  UwfNotInstalled,     // 没装 UWF 功能（找不到 uwfmgr.exe）
};

struct SystemCheckResult {
  CheckStatus status = CheckStatus::Ok;
  std::string editionId;    // 读自注册表 EditionID，比如 "Enterprise"
  std::string productName;  // 读自注册表 ProductName，比如 "Windows 11 Enterprise"
};

SystemCheckResult runSystemChecks();
bool isElevated();
std::string uwfmgrPath();

}  // namespace uwf
