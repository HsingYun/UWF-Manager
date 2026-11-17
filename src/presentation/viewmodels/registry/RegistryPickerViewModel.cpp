// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/registry/RegistryPickerViewModel.h"

#include <QPersistentModelIndex>
#include <QPointer>
#include <QScopedValueRollback>
#include <QTimer>
#include <system_error>

#include "application/identity/contracts/ObjectIdentity.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/policies/RegistryCommitPolicy.h"
#include "domain/uwf/policies/RegistryExclusionPolicy.h"
#include "foundation/text/Utf8.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/PresentationText.h"

namespace uwf::presentation {
namespace {
bool validRegistryText(const QString& text) { return !text.contains(QChar::Null) && isValidUtf16(QStringView(text)); }
struct ChildDescription {
  std::string name;
  RegistryPickerViewModel::SubkeyState subkeys;
};
QString observationError(const std::exception_ptr& failure) {
  try {
    std::rethrow_exception(failure);
  } catch (const std::exception& error) {
    return QString::fromUtf8(error.what());
  } catch (...) {
    return PresentationText::tr("The operation failed with an unknown error.");
  }
}
std::vector<ChildDescription> readChildren(const app::RegistryBrowser& browser, const QString& path, std::stop_token stop) {
  using Subkeys = RegistryPickerViewModel::SubkeyState;
  const bool machineRoot = path == QLatin1String("HKEY_LOCAL_MACHINE");
  std::vector<ChildDescription> children;
  for (const auto& name : browser.subkeyNames(path.toStdString(), stop)) {
    if (stop.stop_requested()) return {};
    if (machineRoot && name.starts_with("PERSISTENT_")) continue;
    Subkeys subkeys = Subkeys::Unknown;
    try {
      subkeys = browser.hasSubkeys((path + '\\' + QString::fromStdString(name)).toStdString(), stop) ? Subkeys::Present : Subkeys::Absent;
    } catch (const std::system_error&) {
      // The parent enumeration established existence, not child readability.
    }
    children.push_back({name, subkeys});
  }
  return children;
}
}  // namespace
RegistryPickerViewModel::RegistryPickerViewModel(Mode mode, std::shared_ptr<const app::RegistryBrowser> browser)
    : m_values(mode == Mode::CommitValue   ? RegistryValueModel::Selection::AllValues
               : mode == Mode::DeleteValue ? RegistryValueModel::Selection::NamedValues
                                           : RegistryValueModel::Selection::None),
      m_mode(mode),
      m_browser(std::move(browser)) {
  if (!m_browser) throw std::invalid_argument("Registry browsing requires an owned browser");
  resetTree();
}
RegistryPickerViewModel::~RegistryPickerViewModel() { cancel(); }
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
  return parent.isValid() && parent.column() == 0 && item && !item->loaded && !item->loading && !item->failed &&
         item->availability != KeyAvailability::Pruned && item->subkeys != SubkeyState::Absent;
}
void RegistryPickerViewModel::fetchMore(const QModelIndex& parent) { loadChildren(parent); }
void RegistryPickerViewModel::navigateTo(const QString& path) {
  m_result.reset();
  if (!validRegistryText(path)) {
    m_navigationPath.clear();
    m_navigating = false;
    emit navigationFinished({});
    return;
  }
  m_navigationPath = app::paths::expandRegistryHive(path);
  m_navigating = true;
  continueNavigation();
}
void RegistryPickerViewModel::continueNavigation() {
  if (!m_navigating || !m_rootsReady || m_advancingNavigation) return;
  QModelIndex selected;
  {
    const QScopedValueRollback guard(m_advancingNavigation, true);
    const auto parts = m_navigationPath.split('\\', Qt::SkipEmptyParts);
    QModelIndex current;
    for (qsizetype part = 0; part < parts.size(); ++part) {
      if (current.isValid()) {
        const auto* item = node(current);
        if (!item->loaded && !item->failed && item->availability != KeyAvailability::Pruned) {
          loadChildren(current);
          return;
        }
        if (item->failed) break;
      }
      QModelIndex found;
      for (int row = 0; row < rowCount(current); ++row) {
        const auto candidate = index(row, 0, current);
        if (app::equalObjectNames(candidate.data().toString(), parts[part])) {
          found = candidate;
          break;
        }
      }
      if (!found.isValid()) break;
      current = found;
    }
    m_navigating = false;
    selected = current;
  }
  emit navigationFinished(selected);
}
bool RegistryPickerViewModel::childrenLoading(const QModelIndex& parent) const {
  const auto* item = node(parent);
  return item && item->loading;
}
void RegistryPickerViewModel::loadChildren(const QModelIndex& parent) {
  if (!m_rootsReady || m_treePublishing || !parent.isValid() || parent.model() != this) return;
  auto* owner = node(parent);
  if (owner->loaded || owner->loading || owner->availability == KeyAvailability::Pruned) return;
  owner->loading = true;
  owner->failed = false;
  if (!owner->loader) owner->loader = std::make_unique<AsyncTask>(this);
  const QPersistentModelIndex target(parent);
  const auto generation = m_treeGeneration;
  const auto path = owner->path;
  owner->loader->start(
      [browser = m_browser, path](std::stop_token stop) { return readChildren(*browser, path, stop); },
      [this, target, generation, path](AsyncResult<std::vector<ChildDescription>> outcome) {
        if (generation != m_treeGeneration || !target.isValid()) return;
        auto* item = node(target);
        item->loading = false;
        if (const auto* failure = std::get_if<std::exception_ptr>(&outcome)) {
          const auto message = observationError(*failure);
          item->failed = true;
          m_keyErrors.insert_or_assign(path, message);
          const QPointer self(this);
          if (app::equalObjectNames(m_detailKey, path))
            loadValues(path);
          else if (m_detailKey.isEmpty()) {
            m_accessError = AccessError{AccessError::Operation::Keys, path, message};
            emit accessStateChanged();
          }
          if (!self || generation != m_treeGeneration || !target.isValid()) return;
          emit childrenLoadFailed(target, message);
          if (self && generation == m_treeGeneration) continueNavigation();
          return;
        }
        const auto descriptions = std::get<std::vector<ChildDescription>>(std::move(outcome));
        std::vector<std::unique_ptr<Node>> items;
        for (const auto& description : descriptions) {
          auto child = std::make_unique<Node>();
          child->name = QString::fromStdString(description.name);
          child->path = path + '\\' + child->name;
          child->parent = item;
          child->row = static_cast<int>(items.size());
          child->availability = availabilityOf(child->path);
          child->subkeys = description.subkeys;
          items.push_back(std::move(child));
        }
        const bool recovered = m_keyErrors.erase(path) != 0;
        item->failed = false;
        item->loaded = true;
        const QPointer self(this);
        if (!items.empty()) {
          m_treePublishing = true;
          beginInsertRows(target, 0, static_cast<int>(items.size()) - 1);
          if (!self) return;
          // A reentrant reset is deferred until this promised insertion closes.
          item->children = std::move(items);
          endInsertRows();
          if (!self) return;
          finishTreePublication();
        } else
          emit dataChanged(target, target);
        if (!self || generation != m_treeGeneration || !target.isValid()) return;
        if (recovered && m_accessError && m_accessError->operation == AccessError::Operation::Keys && app::equalObjectNames(m_accessError->key, path))
          loadValues(m_detailKey);
        if (!self || generation != m_treeGeneration) return;
        emit childrenLoaded(target);
        if (self && generation == m_treeGeneration) continueNavigation();
      });
}
std::vector<std::string> RegistryPickerViewModel::rootHives() const {
  std::vector<std::string> result;
  for (const auto& item : m_root.children) result.push_back(item->path.toStdString());
  return result;
}
void RegistryPickerViewModel::finishTreePublication() {
  m_treePublishing = false;
  resumePendingRequests();
}
void RegistryPickerViewModel::finishValuePublication() {
  m_valuesPublishing = false;
  resumePendingRequests();
}
void RegistryPickerViewModel::resumePendingRequests() {
  if (m_treePublishing || m_valuesPublishing) return;
  if (m_resetPending) {
    if (!m_resetScheduled) {
      m_resetScheduled = true;
      QTimer::singleShot(0, this, [this] {
        m_resetScheduled = false;
        if (m_resetPending) resetTree();
      });
    }
    return;
  }
  if (m_valueRequestPending) beginValueRequest(m_valueGeneration);
}
void RegistryPickerViewModel::resetTree() {
  ++m_treeGeneration;
  ++m_valueGeneration;
  m_valuesKey.clear();
  m_valueRequestPending = false;
  m_rootsTask.cancel();
  m_valuesTask.cancel();
  cancelChildren(m_root);
  m_rootsReady = false;
  m_navigating = false;
  if (m_treePublishing || m_valuesPublishing) {
    m_resetPending = true;
    return;
  }
  m_resetPending = false;
  const auto generation = m_treeGeneration;
  const QPointer self(this);
  m_treePublishing = true;
  beginResetModel();
  if (!self) return;
  m_root.children.clear();
  m_keyErrors.clear();
  m_detailKey.clear();
  m_accessError.reset();
  m_valuesPublishing = true;
  m_values.clear();
  if (!self) return;
  finishValuePublication();
  if (!self) return;
  endResetModel();
  if (!self) return;
  finishTreePublication();
  if (!self || generation != m_treeGeneration) return;
  m_rootsTask.start(
      [browser = m_browser](std::stop_token stop) { return stop.stop_requested() ? std::vector<std::string>{} : browser->rootHiveLongNames(stop); },
      [this, generation](AsyncResult<std::vector<std::string>> outcome) {
        if (generation != m_treeGeneration) return;
        std::vector<std::unique_ptr<Node>> roots;
        if (const auto* failure = std::get_if<std::exception_ptr>(&outcome))
          m_accessError = AccessError{AccessError::Operation::Keys, {}, observationError(*failure)};
        else
          for (const auto& hive : std::get<std::vector<std::string>>(outcome)) {
            auto item = std::make_unique<Node>();
            item->name = item->path = QString::fromStdString(hive);
            item->parent = &m_root;
            item->row = static_cast<int>(roots.size());
            item->availability = availabilityOf(item->path);
            item->subkeys = SubkeyState::Present;
            roots.push_back(std::move(item));
          }
        const QPointer self(this);
        m_treePublishing = true;
        beginResetModel();
        if (!self) return;
        m_root.children = std::move(roots);
        m_rootsReady = generation == m_treeGeneration;
        endResetModel();
        if (!self) return;
        finishTreePublication();
        if (!self || generation != m_treeGeneration) return;
        emit accessStateChanged();
        if (!self || generation != m_treeGeneration) return;
        emit rootsLoaded();
        if (self && generation == m_treeGeneration) continueNavigation();
      });
  emit accessStateChanged();
}
void RegistryPickerViewModel::loadValues(const QString& key) {
  const auto ticket = ++m_valueGeneration;
  m_valuesTask.cancel();
  m_valuesKey.clear();
  m_detailKey = key;
  if (m_treePublishing || m_valuesPublishing || m_resetPending) {
    m_valueRequestPending = true;
    return;
  }
  beginValueRequest(ticket);
}
void RegistryPickerViewModel::beginValueRequest(const std::uint64_t ticket) {
  if (ticket != m_valueGeneration) return;
  if (m_treePublishing || m_valuesPublishing || m_resetPending) {
    m_valueRequestPending = true;
    return;
  }
  m_valueRequestPending = false;
  const auto key = m_detailKey;
  const auto generation = m_treeGeneration;
  m_accessError.reset();
  if (const auto failed = m_keyErrors.find(key); failed != m_keyErrors.cend()) m_accessError = AccessError{AccessError::Operation::Keys, key, failed->second};
  if (!validRegistryText(key)) m_accessError = AccessError{AccessError::Operation::Values, key, PresentationText::tr("Unable to access this registry key")};
  const QPointer self(this);
  m_valuesPublishing = true;
  m_values.clear();
  if (!self) return;
  finishValuePublication();
  if (!self || ticket != m_valueGeneration || generation != m_treeGeneration) return;
  if (!key.isEmpty() && !m_accessError) {
    m_valuesTask.start(
        [browser = m_browser, key](std::stop_token stop) {
          return stop.stop_requested() ? std::vector<app::RegistryValueInfo>{} : browser->values(key.toStdString(), stop);
        },
        [this, key, generation, ticket](AsyncResult<std::vector<app::RegistryValueInfo>> outcome) {
          if (generation != m_treeGeneration || ticket != m_valueGeneration) return;
          const QPointer self(this);
          if (const auto* failure = std::get_if<std::exception_ptr>(&outcome))
            m_accessError = AccessError{AccessError::Operation::Values, key, observationError(*failure)};
          else {
            m_valuesKey = app::paths::expandRegistryHive(key);
            m_valuesPublishing = true;
            m_values.setValues(std::get<std::vector<app::RegistryValueInfo>>(std::move(outcome)));
            if (!self) return;
            finishValuePublication();
          }
          if (self && ticket == m_valueGeneration && generation == m_treeGeneration) emit accessStateChanged();
        });
  }
  emit accessStateChanged();
}
void RegistryPickerViewModel::cancelChildren(Node& parent) {
  for (auto& child : parent.children) {
    if (child->loader) child->loader->cancel();
    child->loading = false;
    cancelChildren(*child);
  }
}
void RegistryPickerViewModel::cancel() {
  m_result.reset();
  m_valuesKey.clear();
  ++m_treeGeneration;
  m_resetPending = false;
  ++m_valueGeneration;
  m_valueRequestPending = false;
  m_navigating = false;
  m_rootsTask.cancel();
  m_valuesTask.cancel();
  cancelChildren(m_root);
}
RegistryPickerViewModel::KeyAvailability RegistryPickerViewModel::availabilityOf(const QString& key) const {
  if (!validRegistryText(key)) return KeyAvailability::Pruned;
  if (m_mode != Mode::Exclusion && !core::preservesRegistryCommitTarget(key.toStdString())) return KeyAvailability::Pruned;
  return m_availability ? m_availability(key) : standardAvailability(key);
}
RegistryPickerViewModel::KeyAvailability RegistryPickerViewModel::standardAvailability(const QString& key) const {
  if (m_mode != Mode::Exclusion) return core::isRegistryCommitTargetAllowed(key.toStdString(), true) ? KeyAvailability::Selectable : KeyAvailability::Pruned;
  // Enumerated paths are syntactically valid; exclusion admission uses the shared policy.
  switch (core::classifyRegistryExclusion(key.toStdString())) {
    case core::RegExclusionClass::Allowed:
      return KeyAvailability::Selectable;
    case core::RegExclusionClass::Container:
      return KeyAvailability::ContainerOnly;
    case core::RegExclusionClass::MachineAccount:  // Domain secret key persistence uses its dedicated editor action.
    case core::RegExclusionClass::OutOfScope:
      return KeyAvailability::Pruned;
  }
  return KeyAvailability::Pruned;
}
bool RegistryPickerViewModel::canAccept(const QString& key, const std::optional<QString>& valueName) const {
  if (!validRegistryText(key) || (valueName && !validRegistryText(*valueName))) return false;
  if (key.isEmpty() || availabilityOf(key) != KeyAvailability::Selectable) return false;
  if (m_mode == Mode::Exclusion) return true;
  if (valueName)
    return !m_valuesKey.isEmpty() && app::equalObjectNames(app::paths::expandRegistryHive(key), m_valuesKey) && m_values.canSelectValue(*valueName);
  // An ordinary whole-key commit is application traversal followed by exact
  // value commits. Deletion also submits the selected key itself at the end,
  // so a known reserved key must not start that destructive partial workflow.
  return m_mode != Mode::DeleteValue || core::isRegistryCommitTargetAllowed(key.toStdString(), false);
}
void RegistryPickerViewModel::accept(QString key, std::optional<QString> valueName) {
  m_result.reset();
  if (!validRegistryText(key) || (valueName && !validRegistryText(*valueName))) return;
  key = app::paths::expandRegistryHive(key);
  if (!supportsValueSelection()) valueName.reset();
  if (canAccept(key, valueName)) m_result = Result{std::move(key), std::move(valueName)};
}
}  // namespace uwf::presentation

#include "moc_RegistryPickerViewModel.cpp"
