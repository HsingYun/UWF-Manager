// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <vector>

#include "app/api/RegistryCommitTypes.h"

namespace uwf::app {

// 计划生成直接复用 regkey::collectKeyTree/valueNames 的生产递归能力；这里不
// 实现第二棵注册表遍历器。计划与执行分离，使服务删除前可以先冻结键清单，
// 再在 SCM 标记删除后、最终释放删除屏障前按同一清单提交删除。
[[nodiscard]] std::vector<RegistryCommitTarget> planRegistryTreeCommit(const std::string& key);
[[nodiscard]] std::vector<RegistryCommitTarget> planRegistryTreeDeletion(const std::string& key);

}  // namespace uwf::app
