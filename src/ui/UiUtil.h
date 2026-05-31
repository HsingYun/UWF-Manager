#pragma once

// 跨控件复用的小 UI helper——原本在多个文件里各写一份（逐字重复），收口到此。
// 都是薄封装：包一层 QString 边界 / Win32 Shell / 表单控件样式，无业务逻辑。

#include <QString>

class QComboBox;
class QVariant;
class QWidget;

namespace uwf::ui {

// 在资源管理器中定位并高亮 path：存在则打开父目录并选中目标，否则沿父目录回溯
// 到第一个真实存在的文件夹直接打开。内部统一转成本地分隔符，调用方不必预处理。
// ExclusionListWidget / OverlayFilesDialog 共用。
void revealInExplorer(const QString& path);

// QString 边界适配：从完整路径取归一化盘符（"C:"）；取不到（如卷 GUID 路径解析
// 失败）返回空串并把原因写日志。CommitDispatcher / ImportApplier 共用（路由到
// 对应 DiskTab）。盘符逻辑本身全在 uwf::drive。
[[nodiscard]] QString extractDriveLetter(const QString& path);

// 把 combo 的当前项切到 itemData == value 的那项；找不到则不动。两个状态面板共用。
void setComboValue(QComboBox* combo, const QVariant& value);

// 切换控件的 "dirty" 动态属性并重跑 style polish，让 QSS 据此变色。两个状态面板共用。
void markDirty(QWidget* w, bool dirty);

}  // namespace uwf::ui
