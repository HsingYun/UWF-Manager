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
//                    只展示、整表禁用选择，OK 永远返回 (key, "", wholeKey=false)。
//
// **三种模式下 (Default) 行恒禁用**——
//   CommitValue: 默认值很少有人单独维护，不暴露成单选项；要 commit 走整键递归
//   DeleteValue: WMI 的 (key, "") 不是删默认值而是删键本身，单选默认值语义会误导
//   Exclusion:   值表整表禁选，连带也无法选默认值
//
// Exclusion 模式可注入 availability checker：树里每个节点都跑一次，按三态结果
// 决定该节点的可选性 / 可展开性：
//   - Selectable   —— 路径合法，可选 + 可展开
//   - ContainerOnly —— 路径不合法，但子树里可能含合法 key，可展开但不可选
//   - Pruned       —— 路径不合法且子树里也不可能有合法 key，灰字 + 禁选 + 禁展开
// 调用方通常用 forbidRegExclusionReason + UWF 白名单覆盖判定实现。CommitValue /
// DeleteValue 模式的 checker 忽略——合法性 UWF 自己回报，树节点全 Selectable。
// 三态的视觉反馈是足够直观的入场教育，本对话框不再额外画 hint 横幅。
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
    CommitValue,   // 配 CommitRegistry：可选值名；不选 = 整键递归
    DeleteValue,   // 配 CommitRegistryDeletion：可选值名；不选 = 整键递归
    Exclusion,     // 配 AddExclusion：值表只展示、整表禁选，结果永远 (key, "")
  };

  struct Result {
    QString key;        // 归一长写键路径（HKEY_LOCAL_MACHINE\...）
    QString valueName;  // 选中行的真实值名；wholeKey=true 时无意义。
                        // Exclusion 永远空；(Default) 在所有模式都禁选，故不会出现空串。
    bool wholeKey = false;  // 值表无选中 = 用户意图"整键递归"（CommitValue / DeleteValue
                            // 时这种语义和"提交单个空名值"不同，必须靠这个 bool 区分）。
                            // Exclusion 永远 false（值表禁选、用不到）。
  };

  // 注册表树里每个 key 的三态可用性。详见类注释。
  enum class KeyAvailability {
    Selectable,      // 路径合法：可选 + 可展开
    ContainerOnly,   // 路径不合法但子树含合法 key：可展开、不可选
    Pruned,          // 路径不合法且不含合法 key：灰字 + 禁选 + 禁展开
  };

  // 用 AvailabilityChecker 类型而不是裸函数指针：调用方常用 lambda 包一层（捕获
  // widget 自己的盘符等上下文）。入参是树节点的完整长写键路径。
  using AvailabilityChecker = std::function<KeyAvailability(const QString& normalizedKey)>;

  RegistryPickerDialog(Mode mode, const QString& title, QWidget* parent = nullptr);

  // 注入三态可用性判定（仅 Exclusion mode 生效）。populateRootHives / loadChildren
  // 每次新建节点时调一次，决定该节点的 flags + 是否带占位子节点；OK 按钮则按
  // 当前选中节点的判定结果决定可用性。
  void setAvailabilityChecker(AvailabilityChecker checker);

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
