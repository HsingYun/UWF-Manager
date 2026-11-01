// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractItemModel>
#include <QMetaType>
#include <cstdint>
#include <map>
#include <memory>
#include <utility>

#include "application/identity/contracts/ObjectIdentity.h"
#include "application/registry/contracts/RegistryBrowser.h"
#include "presentation/viewmodels/common/AsyncTask.h"
#include "presentation/viewmodels/registry/RegistryValueModel.h"
namespace uwf::presentation {
class RegistryPickerViewModel final : public QAbstractItemModel {
  Q_OBJECT
 public:
  enum class Mode {
    CommitValue,  // Exact values, including the default value, or a whole key.
    DeleteValue,  // Named values or a whole key; excludes default-value deletion.
    Exclusion,    // Key exclusions only.
  };

  struct Result {
    QString key;                       // Expands the hive name without rewriting key components.
    std::optional<QString> valueName;  // nullopt selects the key; an empty string selects its default value.
  };

  enum class KeyAvailability {
    Selectable,     // Selectable and expandable.
    ContainerOnly,  // Expandable container for admissible descendants; not selectable.
    Pruned,         // Neither selectable nor expandable.
  };

  // The checker receives the normalized full key path.
  using AvailabilityChecker = std::function<KeyAvailability(const QString& normalizedKey)>;

  enum class SubkeyState { Absent, Present, Unknown };
  struct AccessError {
    enum class Operation { Keys, Values };
    Operation operation;
    QString key;
    QString message;
  };
  RegistryPickerViewModel(Mode mode, std::shared_ptr<const app::RegistryBrowser> browser);
  ~RegistryPickerViewModel() override;
  QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool hasChildren(const QModelIndex& parent = {}) const override;
  bool canFetchMore(const QModelIndex& parent) const override;
  void fetchMore(const QModelIndex& parent) override;
  void loadChildren(const QModelIndex& parent);
  // Navigation is asynchronous and selects the deepest accessible ancestor.
  // Only published model indexes are delivered, on this model's thread.
  void navigateTo(const QString& path);
  void resetTree();
  void loadValues(const QString& key);
  [[nodiscard]] RegistryValueModel& valueModel() { return m_values; }
  [[nodiscard]] const std::optional<AccessError>& accessError() const { return m_accessError; }
  [[nodiscard]] Mode mode() const { return m_mode; }
  [[nodiscard]] bool supportsValueSelection() const { return m_mode != Mode::Exclusion; }
  [[nodiscard]] std::vector<std::string> rootHives() const;
  [[nodiscard]] bool rootsReady() const { return m_rootsReady; }
  [[nodiscard]] bool valuesLoading() const { return m_valuesTask.isRunning(); }
  [[nodiscard]] bool childrenLoading(const QModelIndex& parent) const;
  [[nodiscard]] KeyAvailability availabilityOf(const QString& key) const;
  void setAvailabilityChecker(AvailabilityChecker checker) { m_availability = std::move(checker); }
  // Explicit values must be selectable observations of the current key;
  // whole-key and exclusion selection retain their independent admission rules.
  [[nodiscard]] bool canAccept(const QString& key, const std::optional<QString>& valueName = {}) const;
  void accept(QString key, std::optional<QString> valueName);
  void cancel();
  [[nodiscard]] const std::optional<Result>& selection() const { return m_result; }

 signals:
  void rootsLoaded();
  void childrenLoaded(const QModelIndex& parent);
  void navigationFinished(const QModelIndex& index);
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
    bool loading = false;
    std::unique_ptr<AsyncTask> loader;
    std::vector<std::unique_ptr<Node>> children;
  };
  [[nodiscard]] Node* node(const QModelIndex& index) const;
  [[nodiscard]] KeyAvailability standardAvailability(const QString& key) const;
  void continueNavigation();
  void cancelChildren(Node& parent);
  void finishTreePublication();
  void finishValuePublication();
  void resumePendingRequests();
  void beginValueRequest(std::uint64_t ticket);
  Node m_root;
  RegistryValueModel m_values;
  Mode m_mode;
  std::shared_ptr<const app::RegistryBrowser> m_browser;
  AsyncTask m_rootsTask{this};
  AsyncTask m_valuesTask{this};
  std::uint64_t m_treeGeneration = 0;
  std::uint64_t m_valueGeneration = 0;
  bool m_treePublishing = false;
  bool m_resetPending = false;
  bool m_resetScheduled = false;
  bool m_valuesPublishing = false;
  bool m_valueRequestPending = false;
  bool m_rootsReady = false;
  bool m_navigating = false;
  bool m_advancingNavigation = false;
  QString m_navigationPath;
  AvailabilityChecker m_availability;
  QString m_detailKey;
  QString m_valuesKey;
  std::map<QString, QString, app::ObjectNameLess> m_keyErrors;
  std::optional<AccessError> m_accessError;
  std::optional<Result> m_result;
};
}  // namespace uwf::presentation
Q_DECLARE_METATYPE(std::optional<QString>)
