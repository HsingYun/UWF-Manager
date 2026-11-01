// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPointer>
#include <QSignalBlocker>
#include <functional>
#include <initializer_list>
#include <optional>
#include <utility>
#include <vector>

namespace uwf::app {
// Signal suppression and guarded publication are one protocol. Editors choose
// their business notifications; the session resumes every participant first.
template <class Editor>
class EditorUpdate {
 public:
  explicit EditorUpdate(Editor& editor, std::initializer_list<QObject*> children = {}) : m_editor(&editor), m_blocker(std::in_place, editor) {
    m_blocks.reserve(children.size());
    for (auto* child : children)
      if (child) m_blocks.emplace_back(*child);
  }
  EditorUpdate(EditorUpdate&& other) noexcept
      : m_editor(std::exchange(other.m_editor, {})), m_blocker(std::move(other.m_blocker)), m_blocks(std::move(other.m_blocks)) {}
  EditorUpdate(const EditorUpdate&) = delete;
  EditorUpdate& operator=(const EditorUpdate&) = delete;
  ~EditorUpdate() { resume(); }
  void resume() {
    if (!m_editor && m_blocker) m_blocker->dismiss();
    m_blocker.reset();
    for (auto& block : m_blocks) {
      if (!block.object && block.blocker) block.blocker->dismiss();
      block.blocker.reset();
    }
    m_blocks.clear();
  }

 protected:
  using Continue = std::function<bool()>;
  [[nodiscard]] bool current(const Continue& guard) const { return m_editor && (!guard || guard()) && m_editor; }
  template <class... Notifications>
  [[nodiscard]] bool publish(const Continue& guard, Notifications&&... notifications) {
    const auto one = [&](auto&& notification) {
      if (!current(guard)) return false;
      std::forward<decltype(notification)>(notification)();
      return current(guard);
    };
    return (one(std::forward<Notifications>(notifications)) && ...);
  }
  QPointer<Editor> m_editor;

 private:
  struct Block {
    explicit Block(QObject& owner) : object(&owner), blocker(std::in_place, owner) {}
    QPointer<QObject> object;
    std::optional<QSignalBlocker> blocker;
  };
  std::optional<QSignalBlocker> m_blocker;
  std::vector<Block> m_blocks;
};
}  // namespace uwf::app
