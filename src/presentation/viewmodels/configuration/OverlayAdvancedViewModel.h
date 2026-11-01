// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QObject>
#include <QStringList>
#include <memory>

#include "application/configuration/overlay/OverlayPolicyEditor.h"

namespace uwf::presentation {
class OverlayAdvancedViewModel final : public QObject {
  Q_OBJECT
 public:
  using Field = app::OverlayPolicyField;
  using Edit = app::OverlayPolicyEdit;
  enum class ValueState { Unknown, Off, On };
  Q_ENUM(ValueState)
  struct Condition {
    QString title;
    QString text;
  };
  struct FeatureDetails {
    QString officialName;
    QString description;
    QList<Condition> conditions;
  };
  explicit OverlayAdvancedViewModel(std::shared_ptr<app::OverlayPolicyEditor> editor, QObject* parent = nullptr);
  [[nodiscard]] const std::shared_ptr<app::OverlayPolicyEditor>& editor() const { return m_editor; }
  [[nodiscard]] bool available() const { return m_editor->available(); }
  [[nodiscard]] bool readOnly() const { return m_editor->readOnly(); }
  [[nodiscard]] Edit beginEdit(bool baselineOnly = false) const { return m_editor->beginEdit(baselineOnly); }
  [[nodiscard]] int pendingCount(const Edit& edit) const { return m_editor->pendingCount(edit); }
  [[nodiscard]] bool hasPendingChanges() const { return !m_editor->pendingChanges().empty(); }
  [[nodiscard]] bool changed(const Edit& edit, Field field) const;
  [[nodiscard]] bool canCommit(const Edit& edit) const { return m_editor->canCommit(edit); }
  [[nodiscard]] ValueState currentState(Field field) const;
  [[nodiscard]] ValueState currentFilterState() const;
  [[nodiscard]] QString currentText(Field field) const;
  [[nodiscard]] bool canEditField(const Edit& edit, Field field) const { return m_editor->disabledReason(edit, field) == app::OverlayPolicyIssue::None; }
  [[nodiscard]] QString disabledReason(const Edit& edit, Field field) const;
  [[nodiscard]] QStringList conflicts(const Edit& edit) const;
  [[nodiscard]] QString currentFilterText() const;
  [[nodiscard]] QString currentTypeText() const;
  [[nodiscard]] QString targetTypeText() const;
  [[nodiscard]] QString targetTypeText(const Edit& edit) const;
  [[nodiscard]] static QString fieldName(Field field);
  [[nodiscard]] static QString officialName(Field field);
  [[nodiscard]] static QString description(Field field);
  [[nodiscard]] static FeatureDetails featureDetails(Field field);
  [[nodiscard]] static std::optional<bool> value(const Edit& edit, Field field) { return app::OverlayPolicyEditor::value(edit.values, field); }
  static void setValue(Edit& edit, Field field, bool enabled) { app::OverlayPolicyEditor::setValue(edit, field, enabled); }
  [[nodiscard]] static QString conflictText(app::OverlayPolicyIssue issue);
  bool commit(const Edit& edit) { return m_editor->commit(edit); }

 signals:
  void stateChanged();
  void pendingChanged();

 private:
  [[nodiscard]] static QString issueText(app::OverlayPolicyIssue issue);
  std::shared_ptr<app::OverlayPolicyEditor> m_editor;
};
}  // namespace uwf::presentation
