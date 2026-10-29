// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RegistryPickerViewModel.h"

#include <algorithm>
#include <system_error>

#include "core/RegistryExclusionPolicy.h"
#include "presentation/common/ItemRoles.h"
#include "shared/RegistryPath.h"
namespace uwf::presentation {
RegistryPickerViewModel::RegistryPickerViewModel(Mode mode, const app::RegistryBrowser& browser)
    : m_values(mode != Mode::Exclusion), m_mode(mode), m_browser(browser) {
  resetTree();
}
RegistryPickerViewModel::Node* RegistryPickerViewModel::node(const QModelIndex& index) const {
  return index.isValid() ? (index.model() == this ? static_cast<Node*>(index.internalPointer()) : nullptr) : const_cast<Node*>(&m_root);
}
QModelIndex RegistryPickerViewModel::index(int row, int column, const QModelIndex& parent) const {
  const auto* owner = node(parent);
  if (!owner || column != 0 || row < 0 || (parent.isValid() && parent.column() != 0) || static_cast<size_t>(row) >= owner->children.size()) return {};
  return createIndex(row, column, owner->children[static_cast<size_t>(row)].get());
}
QModelIndex RegistryPickerViewModel::parent(const QModelIndex& index) const {
  if (!index.isValid() || index.model() != this) return {};
  const auto* child = node(index);
  auto* owner = child->parent;
  return owner && owner != &m_root ? createIndex(owner->row, 0, owner) : QModelIndex{};
}
int RegistryPickerViewModel::rowCount(const QModelIndex& parent) const {
  const auto* owner = node(parent);
  return !owner || (parent.isValid() && parent.column() != 0) ? 0 : static_cast<int>(owner->children.size());
}
int RegistryPickerViewModel::columnCount(const QModelIndex& parent) const { return parent.isValid() && parent.column() != 0 ? 0 : 1; }
QVariant RegistryPickerViewModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.column() != 0) return {};
  const auto* item = node(index);
  if (role == Qt::DisplayRole) return item->name;
  if (role == PathRole) return item->path;
  if (role == IconResourceRole) return QStringLiteral(":/icons/registry.svg");
  if (role == StatusRole && item->availability != KeyAvailability::Selectable) return static_cast<int>(ItemStatus::Muted);
  return {};
}
Qt::ItemFlags RegistryPickerViewModel::flags(const QModelIndex& index) const {
  if (!index.isValid() || index.model() != this) return Qt::NoItemFlags;
  const auto availability = node(index)->availability;
  if (availability == KeyAvailability::Pruned) return Qt::ItemNeverHasChildren;
  if (availability == KeyAvailability::ContainerOnly) return Qt::ItemIsEnabled;
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
bool RegistryPickerViewModel::hasChildren(const QModelIndex& parent) const {
  const auto* owner = node(parent);
  if (!owner || (parent.isValid() && parent.column() != 0)) return false;
  if (!parent.isValid()) return !m_root.children.empty();
  return owner->availability != KeyAvailability::Pruned && (!owner->children.empty() || (!owner->loaded && owner->subkeys != SubkeyState::Absent));
}
bool RegistryPickerViewModel::canFetchMore(const QModelIndex& parent) const {
  const auto* item = node(parent);
  return parent.isValid() && parent.column() == 0 && item && !item->loaded && !item->failed && item->availability != KeyAvailability::Pruned &&
         item->subkeys != SubkeyState::Absent;
}
void RegistryPickerViewModel::fetchMore(const QModelIndex& parent) { loadChildren(parent); }
QModelIndex RegistryPickerViewModel::locateKey(const QString& path) {
  const auto normalized = QString::fromStdString(regkey::normalize(path.toStdString()));
  const auto parts = normalized.split('\\', Qt::SkipEmptyParts);
  if (parts.isEmpty()) return {};
  QModelIndex current;
  for (qsizetype part = 0; part < parts.size(); ++part) {
    if (current.isValid() && !loadChildren(current)) break;
    QModelIndex found;
    for (int row = 0; row < rowCount(current); ++row) {
      const auto candidate = index(row, 0, current);
      if (candidate.data().toString().compare(parts[part], Qt::CaseInsensitive) == 0) {
        found = candidate;
        break;
      }
    }
    if (!found.isValid()) break;
    current = found;
  }
  return current;
}
bool RegistryPickerViewModel::loadChildren(const QModelIndex& parent) {
  if (!parent.isValid() || parent.model() != this) return false;
  auto* owner = node(parent);
  if (owner->loaded || owner->availability == KeyAvailability::Pruned) return true;
  std::vector<std::unique_ptr<Node>> items;
  try {
    for (const auto& description : children(owner->path)) {
      auto item = std::make_unique<Node>();
      item->name = QString::fromStdString(description.name);
      item->path = owner->path + '\\' + item->name;
      item->parent = owner;
      item->row = static_cast<int>(items.size());
      item->availability = availabilityOf(item->path);
      item->subkeys = description.subkeys;
      items.push_back(std::move(item));
    }
  } catch (const std::exception& error) {
    owner->failed = true;
    const auto message = QString::fromUtf8(error.what());
    m_keyErrors.insert(owner->path, message);
    m_values.setValues({});
    m_accessError = AccessError{AccessError::Operation::Keys, owner->path, message};
    emit accessStateChanged();
    emit childrenLoadFailed(parent, message);
    return false;
  }
  owner->failed = false;
  const bool recovered = m_keyErrors.remove(owner->path);
  owner->loaded = true;
  if (!items.empty()) {
    beginInsertRows(parent, 0, static_cast<int>(items.size()) - 1);
    owner->children = std::move(items);
    endInsertRows();
  } else {
    emit dataChanged(parent, parent);
  }
  if (recovered && m_accessError && m_accessError->operation == AccessError::Operation::Keys && m_accessError->key == owner->path) loadValues(m_detailKey);
  return true;
}
void RegistryPickerViewModel::resetTree() {
  std::vector<std::unique_ptr<Node>> roots;
  for (const auto& hive : rootHives()) {
    auto item = std::make_unique<Node>();
    item->name = item->path = QString::fromStdString(hive);
    item->parent = &m_root;
    item->row = static_cast<int>(roots.size());
    item->availability = availabilityOf(item->path);
    item->subkeys = SubkeyState::Present;
    roots.push_back(std::move(item));
  }
  beginResetModel();
  m_keyErrors.clear();
  m_detailKey.clear();
  m_accessError.reset();
  m_values.setValues({});
  m_root.children = std::move(roots);
  endResetModel();
  emit accessStateChanged();
}
void RegistryPickerViewModel::loadValues(const QString& key) {
  m_detailKey = key;
  m_accessError.reset();
  m_values.setValues({});
  if (const auto failed = m_keyErrors.constFind(key); failed != m_keyErrors.cend()) {
    m_accessError = AccessError{AccessError::Operation::Keys, key, *failed};
  } else if (!key.isEmpty()) {
    try {
      m_values.setValues(values(key));
    } catch (const std::exception& error) {
      const auto message = QString::fromUtf8(error.what());
      m_accessError = AccessError{AccessError::Operation::Values, key, message};
      emit valuesLoadFailed(message);
    }
  }
  emit accessStateChanged();
}
std::vector<RegistryPickerViewModel::ChildDescription> RegistryPickerViewModel::children(const QString& path) const {
  // 隐藏 UWF 内部使用的 HKLM\PERSISTENT_* 直通配置单元。
  const bool atHklmRoot = path == QLatin1String("HKEY_LOCAL_MACHINE");
  std::vector<ChildDescription> children;
  for (const auto& name : m_browser.subkeyNames(path.toStdString())) {
    if (atHklmRoot && name.starts_with("PERSISTENT_")) continue;
    const QString childPath = path + '\\' + QString::fromStdString(name);
    SubkeyState subkeys = SubkeyState::Unknown;
    try {
      subkeys = m_browser.hasSubkeys(childPath.toStdString()) ? SubkeyState::Present : SubkeyState::Absent;
    } catch (const std::system_error&) {
      // 父键枚举已经证明该子键存在；仅子键的可展开性尚不可知。
      // 不能因为一个受限键就丢弃父键其余已完整枚举的兄弟节点。
      subkeys = SubkeyState::Unknown;
    }
    children.push_back({name, subkeys});
  }
  return children;
}
std::vector<app::RegistryValueInfo> RegistryPickerViewModel::values(const QString& key) const {
  auto items = m_browser.values(key.toStdString());
  // 默认值置于首行；未设置时补充 REG_SZ 空值占位。
  auto def = std::find_if(items.begin(), items.end(), [](const auto& v) { return v.name.empty(); });
  if (def != items.end()) {
    std::rotate(items.begin(), def, def + 1);
  } else {
    items.insert(items.begin(), {std::string{}, 1u, std::vector<uint8_t>{}});  // REG_SZ 空值占位
  }
  return items;
}
RegistryPickerViewModel::KeyAvailability RegistryPickerViewModel::availabilityOf(const QString& key) const {
  return m_availability ? m_availability(key) : standardAvailability(key);
}
RegistryPickerViewModel::KeyAvailability RegistryPickerViewModel::standardAvailability(const QString& key) {
  // 枚举路径已满足语法约束；排除资格与录入页共用 classifyRegistryExclusion。
  switch (core::classifyRegistryExclusion(key.toStdString())) {
    case core::RegExclusionClass::Allowed:
      return KeyAvailability::Selectable;
    case core::RegExclusionClass::Container:
      return KeyAvailability::ContainerOnly;
    case core::RegExclusionClass::MachineAccount:  // 域机器账户密钥：UWF 明确禁排，所有模式一律 Pruned
    case core::RegExclusionClass::OutOfScope:
      return KeyAvailability::Pruned;
  }
  return KeyAvailability::Pruned;
}
bool RegistryPickerViewModel::canAccept(const QString& key) const {
  if (key.isEmpty()) return false;
  return m_mode != Mode::Exclusion || availabilityOf(key) == KeyAvailability::Selectable;
}
void RegistryPickerViewModel::accept(QString key, QString valueName) {
  m_result = Result{std::move(key), supportsValueSelection() ? std::move(valueName) : QString{}};
}
}  // namespace uwf::presentation

#include "moc_RegistryPickerViewModel.cpp"
