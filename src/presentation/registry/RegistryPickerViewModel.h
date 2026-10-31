// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractItemModel>
#include <QHash>
#include <memory>
#include <utility>

#include "RegistryValueModel.h"
#include "app/ports/RegistryBrowser.h"
namespace uwf::presentation {
class RegistryPickerViewModel final : public QAbstractItemModel {
  Q_OBJECT
 public:
  enum class Mode {
    CommitValue,  // 提交命名值或整键
    DeleteValue,  // 删除命名值或整键
    Exclusion,    // 仅添加键排除项
  };

  struct Result {
    QString key;        // 归一长写键路径（HKEY_LOCAL_MACHINE\...）
    QString valueName;  // 空串表示整键递归；Exclusion 模式固定为空。
  };

  enum class KeyAvailability {
    Selectable,     // 路径合法：可选 + 可展开
    ContainerOnly,  // 路径不合法但子树含合法 key：可展开、不可选
    Pruned,         // 路径不合法且不含合法 key：灰字 + 禁选 + 禁展开
  };

  // 检查器接收归一化完整键名，可捕获调用方上下文。
  using AvailabilityChecker = std::function<KeyAvailability(const QString& normalizedKey)>;

  enum class SubkeyState { Absent, Present, Unknown };
  struct AccessError {
    enum class Operation { Keys, Values };
    Operation operation;
    QString key;
    QString message;
  };
  struct ChildDescription {
    std::string name;
    SubkeyState subkeys;
  };
  RegistryPickerViewModel(Mode mode, const app::RegistryBrowser& browser);
  QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool hasChildren(const QModelIndex& parent = {}) const override;
  bool canFetchMore(const QModelIndex& parent) const override;
  void fetchMore(const QModelIndex& parent) override;
  bool loadChildren(const QModelIndex& parent);
  // Resolve aliases and descend as far as accessible; the view only expands
  // ancestors and selects the returned index.
  [[nodiscard]] QModelIndex locateKey(const QString& path);
  void resetTree();
  void loadValues(const QString& key);
  [[nodiscard]] RegistryValueModel& valueModel() { return m_values; }
  [[nodiscard]] const std::optional<AccessError>& accessError() const { return m_accessError; }
  [[nodiscard]] Mode mode() const { return m_mode; }
  [[nodiscard]] bool supportsValueSelection() const { return m_mode != Mode::Exclusion; }
  [[nodiscard]] std::vector<std::string> rootHives() const { return m_browser.rootHiveLongNames(); }
  [[nodiscard]] std::vector<ChildDescription> children(const QString& key) const;
  [[nodiscard]] std::vector<app::RegistryValueInfo> values(const QString& key) const;
  [[nodiscard]] KeyAvailability availabilityOf(const QString& key) const;
  [[nodiscard]] static KeyAvailability standardAvailability(const QString& key);
  void setAvailabilityChecker(AvailabilityChecker checker) { m_availability = std::move(checker); }
  [[nodiscard]] bool canAccept(const QString& key) const;
  void accept(QString key, QString valueName);
  void cancel() { m_result.reset(); }
  [[nodiscard]] const std::optional<Result>& selection() const { return m_result; }

 signals:
  void childrenLoadFailed(const QModelIndex& parent, const QString& message);
  void accessStateChanged();

 private:
  struct Node {
    QString name;
    QString path;
    Node* parent = nullptr;
    int row = 0;
    KeyAvailability availability = KeyAvailability::Pruned;
    SubkeyState subkeys = SubkeyState::Absent;
    bool loaded = false;
    // Keep the expansion affordance after failure; retry explicitly instead of on each layout.
    bool failed = false;
    std::vector<std::unique_ptr<Node>> children;
  };
  [[nodiscard]] Node* node(const QModelIndex& index) const;
  Node m_root;
  RegistryValueModel m_values;
  Mode m_mode;
  const app::RegistryBrowser& m_browser;
  AvailabilityChecker m_availability;
  QString m_detailKey;
  QHash<QString, QString> m_keyErrors;
  std::optional<AccessError> m_accessError;
  std::optional<Result> m_result;
};
}  // namespace uwf::presentation
