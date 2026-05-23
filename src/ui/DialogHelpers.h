#pragma once

// ui 层小工具：跨多个 widget 共用、又不值得各自做完整组件的拼接。

#include <QDir>
#include <QString>

namespace uwf::ui {

// 文件 / 文件夹选择对话框的默认 base 路径：盘符已知就用其根目录，否则用用户
// home。DiskTab 的 4 个 commit 入口、ExclusionListWidget 的 2 个添加按钮共用——
// 4 个文件、6 处调用各写一份容易漏；这里集中。
[[nodiscard]] inline QString dialogBasePath(const QString& driveLetter) { return driveLetter.isEmpty() ? QDir::homePath() : driveLetter + "\\"; }

}  // namespace uwf::ui
