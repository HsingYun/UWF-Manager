/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// 注册表键 / 值选择对话框：树形浏览 + 值列表 + 地址栏跳转 + 实时校验。
//
// 三种模式分别对应 UWF 注册表的三个写入入口：
//   - CommitValue —— UWF_RegistryFilter::CommitRegistry。可选值名；不选 = 整键
//                    递归（上层把它展开成"每个子键的每个值"逐个 commit）。
//   - DeleteValue —— UWF_RegistryFilter::CommitRegistryDeletion。可选值名；不选
//                    = 整键递归（上层按"最深子键先删"的顺序逐个 delete-commit）。
//                    注意 WMI 层 (key, "") 是删键本身（非默认值），递归交给上层。
//   - Exclusion   —— UWF_RegistryFilter::AddExclusion。WMI 不收值名，所以值表
//                    只展示、整表禁用选择，OK 永远返回 (key, "")。
//
// **三种模式下 (Default) 行恒禁用**——
//   CommitValue: 默认值很少有人单独维护，不暴露成单选项；要 commit 走整键递归
//   DeleteValue: WMI 的 (key, "") 不是删默认值而是删键本身，单选默认值语义会误导
//   Exclusion:   值表整表禁选，连带也无法选默认值
//
// 三种模式默认都套同一个 availability checker（standardAvailability）——
// UWF 的 CommitRegistry / CommitRegistryDeletion / AddExclusion 实测都
// 只接受相同的 6 前缀白名单（HKLM\BCD00000000 / SYSTEM / SOFTWARE / SAM /
// SECURITY / COMPONENTS 下的子键），所以 picker 在 UI 层就把不可能的路径
// 灰掉，避免让用户提交后才看到 cryptic 的 0x80041008。树里每个节点都跑一次，
// 按三态结果决定该节点的可选性 / 可展开性：
//   - Selectable   —— 路径合法，可选 + 可展开
//   - ContainerOnly —— 路径不合法，但子树里可能含合法 key，可展开但不可选
//   - Pruned       —— 路径不合法且子树里也不可能有合法 key，灰字 + 禁选 + 禁展开
// 调用方通常无需自带 checker；只有当某场景需要更严的规则（比如再叠加
// 用户级 deny list）才走 setAvailabilityChecker。三态视觉反馈足够直观，
// 本对话框不再额外画 hint 横幅。
//
// 静态便捷 pick() 一行调用 + 阻塞执行；要更细的控制（pre-select 某个 key、
// 自定义按钮等）可以直接 new + exec()，取 result()。

#include <QDialog>
#include <QString>
#include <functional>
#include <optional>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace uwf::ui {

class RegistryPickerDialog : public QDialog {
  Q_OBJECT
 public:
  enum class Mode {
    CommitValue,  // 配 CommitRegistry：可选值名；不选 = 整键递归
    DeleteValue,  // 配 CommitRegistryDeletion：可选值名；不选 = 整键递归
    Exclusion,    // 配 AddExclusion：值表只展示、整表禁选，结果永远 (key, "")
  };

  struct Result {
    QString key;        // 归一长写键路径（HKEY_LOCAL_MACHINE\...）
    QString valueName;  // 选中行的真实值名；空串 = 用户意图"整键递归"。
                        // (Default) 在所有模式都禁选，故 valueName 非空时必然是真实
                        // 命名值——"空串=整键递归"的语义因此无歧义，不再需要单独的
                        // wholeKey 字段。Exclusion 模式永远空（值表整表禁选）。
  };

  // 注册表树里每个 key 的三态可用性。详见类注释。
  enum class KeyAvailability {
    Selectable,     // 路径合法：可选 + 可展开
    ContainerOnly,  // 路径不合法但子树含合法 key：可展开、不可选
    Pruned,         // 路径不合法且不含合法 key：灰字 + 禁选 + 禁展开
  };

  // 用 AvailabilityChecker 类型而不是裸函数指针：调用方常用 lambda 包一层（捕获
  // widget 自己的盘符等上下文）。入参是树节点的完整长写键路径。
  using AvailabilityChecker = std::function<KeyAvailability(const QString& normalizedKey)>;

  RegistryPickerDialog(Mode mode, const QString& title, QWidget* parent = nullptr);

  // 覆盖默认的 standardAvailability。populateRootHives / loadChildren 每次新建
  // 节点时都会调一次（无注入时跑 standardAvailability），决定该节点的 flags +
  // 是否带占位子节点；OK 按钮则按当前选中节点的判定结果决定可用性。
  void setAvailabilityChecker(AvailabilityChecker checker);

  // 默认 checker：6 前缀白名单 + $MACHINE.ACC 黑名单，与 UWF 三大写入入口
  // (CommitRegistry / CommitRegistryDeletion / AddExclusion) 的合法路径口径
  // 一致。除非要叠加更严的判定，调用方一般不必关心这个函数。
  [[nodiscard]] static KeyAvailability standardAvailability(const QString& key);

  // 预先选中一个键路径——节点不存在时尽量展开到最深可达层。
  // 在 show() 前调用最稳；运行中也可以用，会触发地址栏 + 树同步。
  void preselectKey(const QString& key);

  // 用户点 OK 后的结果。Cancel 或还没 exec 完返回 nullopt。
  [[nodiscard]] std::optional<Result> result() const { return m_result; }

  // 静态便捷：阻塞执行 + 取结果一步完成。checker 仅 Exclusion 用。
  [[nodiscard]] static std::optional<Result> pick(Mode mode, const QString& title, QWidget* parent = nullptr, AvailabilityChecker checker = {},
                                                  const QString& preselectKey = {});

 private:
  void buildUi();
  void populateRootHives();
  void loadChildren(QTreeWidgetItem* item);  // 把 item 下面的占位 "..." 换成真实子键
  // 给一个新建的树节点按 availability 设 flags + 决定是否带占位子节点。
  // path 为完整长写键路径，hasRealSubkeys 为实机注册表枚举结果。
  void applyAvailability(QTreeWidgetItem* item, const QString& path, bool hasRealSubkeys);
  // 调一次 m_availability；未注入则默认 Selectable。
  [[nodiscard]] KeyAvailability availabilityOf(const QString& key) const;
  void onItemExpanded(QTreeWidgetItem* item);
  void onTreeSelectionChanged();
  void onValueSelectionChanged();
  void onAddressBarReturnPressed();
  void onRefresh();
  void refreshValueTable(const QString& keyPath);
  void navigateTo(const QString& keyPath);  // 展开树到 keyPath 并选中
  void refreshSelectionLabels();            // 重画底部"当前键 / 值"两行展示
  void updateOkButton();                    // 跑 validator + 决定 OK 是否可用

  [[nodiscard]] QString currentKeyPath() const;    // 当前选中节点的完整长写路径，无选中返回空
  [[nodiscard]] QString currentValueName() const;  // 当前值表选中的值名；无选中或 Exclusion 返回空

  // 三个模式各自暴露给值表 / 结果构造的小判定，把 mode switch 集中到一处。
  [[nodiscard]] bool supportsValueSelection() const { return m_mode != Mode::Exclusion; }

 protected:
  // 值表 viewport 上空白处单击 → 清掉选中（valueName 回到空，即"整键递归"）。
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  Mode m_mode;
  AvailabilityChecker m_availability;
  std::optional<Result> m_result;

  // UI（buildUi 填充）
  QLineEdit* m_addressBar = nullptr;
  QTreeWidget* m_tree = nullptr;
  QTableWidget* m_valueTable = nullptr;
  QLabel* m_keyLabel = nullptr;    // 底部"键：……"行
  QLabel* m_valueLabel = nullptr;  // 底部"值：……"行（仅支持值选择的模式创建）
  QPushButton* m_okBtn = nullptr;
};

}  // namespace uwf::ui
