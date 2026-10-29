// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 注册表树、值预览与地址栏导航。空值名表示整键递归，命名值表示单值操作。
// Exclusion 模式只返回键；默认值行始终不可选，避免与整键删除的空值名语义冲突。
// 三种模式共用排除资格策略；可注入更严格的节点可用性检查。

#include <QString>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "presentation/registry/RegistryPickerViewModel.h"
#include "ui/dialogs/PanelDialog.h"

class QLineEdit;
class QPushButton;
class QTableView;
class QTreeView;
class QStackedWidget;

namespace uwf::ui {

class ContentHintBox;
class VerticalScrollArea;
class ContentState;
class TextLabel;

// 只读浏览接口，所有方法在 GUI 线程同步调用。对话框借用数据源，调用方须保证其生命周期。

class RegistryPickerDialog : public PanelDialog {
  Q_OBJECT
 public:
  using Mode = presentation::RegistryPickerViewModel::Mode;
  using Result = presentation::RegistryPickerViewModel::Result;
  using KeyAvailability = presentation::RegistryPickerViewModel::KeyAvailability;
  using AvailabilityChecker = presentation::RegistryPickerViewModel::AvailabilityChecker;

  RegistryPickerDialog(std::shared_ptr<presentation::RegistryPickerViewModel> model, const QString& title, QWidget* parent = nullptr);

  // 覆盖默认策略，控制节点选择、展开和确认按钮可用性。
  void setAvailabilityChecker(AvailabilityChecker checker);

  // 默认策略与共享排除规则一致，允许指定子树并禁止 $MACHINE.ACC。
  [[nodiscard]] static KeyAvailability standardAvailability(const QString& key);

  // 预选路径；目标不可达时停在最深可达节点。
  void preselectKey(const QString& key);

  // 接受后返回选择，取消返回 nullopt。
  [[nodiscard]] std::optional<Result> selection() const { return m_viewModel->selection(); }

  // 阻塞执行选择器并返回结果；三种模式均支持自定义 checker。
  [[nodiscard]] static std::optional<Result> pick(std::shared_ptr<presentation::RegistryPickerViewModel> model, const QString& title, QWidget* parent = nullptr,
                                                  AvailabilityChecker checker = {}, const QString& preselectKey = {});

 private:
  bool requestRejection() override;
  using SubkeyState = presentation::RegistryPickerViewModel::SubkeyState;

  void buildUi();
  void populateRootHives();
  void onTreeSelectionChanged();
  void onValueSelectionChanged();
  void onAddressBarReturnPressed();
  void onRefresh();
  void refreshValueTable(const QString& keyPath);
  void renderAccessState();
  void navigateTo(const QString& keyPath);  // 展开树到 keyPath 并选中
  void refreshSelectionLabels();
  void updateOkButton();

  [[nodiscard]] QString currentKeyPath() const;    // 当前选中节点的完整长写路径，无选中返回空
  [[nodiscard]] QString currentValueName() const;  // 当前值表选中的值名；无选中或 Exclusion 返回空

  [[nodiscard]] bool supportsValueSelection() const { return m_viewModel->supportsValueSelection(); }

 protected:
  // 值表 viewport 上空白处单击 → 清掉选中（valueName 回到空，即"整键递归"）。
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  std::shared_ptr<presentation::RegistryPickerViewModel> m_viewModel;

  // UI（buildUi 填充）
  QLineEdit* m_addressBar = nullptr;
  QTreeView* m_tree = nullptr;
  QTableView* m_valueTable = nullptr;
  QStackedWidget* m_details = nullptr;
  ContentState* m_accessError = nullptr;
  ContentHintBox* m_errorPath = nullptr;
  ContentHintBox* m_errorDetail = nullptr;
  TextLabel* m_keyLabel = nullptr;
  TextLabel* m_valueLabel = nullptr;
  QPushButton* m_wholeKey = nullptr;
  QPushButton* m_okBtn = nullptr;
};

}  // namespace uwf::ui
