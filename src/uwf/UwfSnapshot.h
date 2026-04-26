#pragma once

// UwfSnapshot —— "快照"入口：把 UwfFilter / UwfVolume / UwfOverlay /
// UwfOverlayConfig / UwfRegistryFilter 五个子模块读到的数据
// 组合成一份 core::UwfSnapshot。调用方不需要知道每个 WMI 类的细节。
//
// 另外附带 enumerateDisks()：它不属于任何 UWF WMI 类（查的是 root\cimv2 的
// Win32_LogicalDisk），但"快照一次系统当前状态"的入口天然需要它。

#include <string>
#include <vector>

#include "../core/UwfConfig.h"

namespace uwf {

// 读取完整 UWF 状态（filter / overlay / volumes / registry exclusions / overlay
// files）。 如果 UWF 命名空间不可用，uwfAvailable=false，rawError
// 中会有原始错误。
core::UwfSnapshot readSnapshot(std::string* error = nullptr);

// 枚举本机所有可见的磁盘卷，并对每个卷计算 DiskSupport 判定。
std::vector<core::DiskInfo> enumerateDisks(std::string* error = nullptr);

}  // namespace uwf
