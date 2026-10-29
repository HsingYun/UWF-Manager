// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingDraft.h"

#include <QSet>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>

#include "FileStagingPath.h"
#include "app/api/OperationError.h"
#include "app/common/ApplicationText.h"

namespace uwf::app {
namespace {
using EntryKeys = QSet<std::pair<FileStagingKind, QString>>;
EntryKeys entryKeys(const QList<FileStagingEntry>& entries) {
  EntryKeys keys;
  keys.reserve(entries.size());
  for (const auto& entry : entries) keys.insert(fileStagingEntryKey(entry));
  return keys;
}
bool sameEntries(const QList<FileStagingEntry>& first, const QList<FileStagingEntry>& second) {
  return first.size() == second.size() && entryKeys(first) == entryKeys(second);
}
QList<FileStagingEntry> difference(const QList<FileStagingEntry>& first, const QList<FileStagingEntry>& second) {
  QList<FileStagingEntry> result;
  const auto keys = entryKeys(second);
  for (const auto& entry : first)
    if (!keys.contains(fileStagingEntryKey(entry))) result.append(entry);
  return result;
}
}  // namespace

QList<FileStagingEntry> FileStagingChanges::added() const { return difference(after, before); }
QList<FileStagingEntry> FileStagingChanges::removed() const { return difference(before, after); }
std::size_t FileStagingChanges::count() const {
  const auto edits = static_cast<std::size_t>(added().size() + removed().size());
  return edits == 0 && persistencePending ? 1 : edits;
}

FileStagingDraft::FileStagingDraft(std::shared_ptr<FileStagingStore> store) : m_store(std::move(store)) {}
void FileStagingDraft::ensureLoaded() const {
  if (m_loaded) return;
  const auto entries = m_store->load();
  m_baseline = entries;
  m_entries = entries;
  m_loaded = true;
}
QList<FileStagingEntry> FileStagingDraft::load() const {
  ensureLoaded();
  if (!m_baselineKnown)
    throw std::runtime_error(
        ApplicationText::tr("Review the operation details. Read the current system state before retrying changes that may have been written.").toStdString());
  return m_entries;
}
QList<FileStagingEntry> FileStagingDraft::baseline() const { return m_baseline; }
void FileStagingDraft::replace(const QList<FileStagingEntry>& entries) {
  static_cast<void>(load());
  if (sameEntries(m_entries, entries)) return;
  m_entries = entries;
  emit changed();
}
std::optional<FileStagingChanges> FileStagingDraft::changes() const {
  if (!m_loaded || (!m_persistencePending && sameEntries(m_baseline, m_entries))) return std::nullopt;
  return FileStagingChanges{m_baseline, m_entries, m_persistencePending};
}
void FileStagingDraft::reload() {
  const auto entries = m_store->load();
  m_baseline = entries;
  m_entries = entries;
  m_loaded = true;
  m_baselineKnown = true;
  m_persistencePending = false;
  emit changed();
}
void FileStagingDraft::reconcile() {
  const auto entries = m_store->load();
  m_baseline = entries;
  m_baselineKnown = true;
  emit changed();
}
FileStagingWriteOperation FileStagingDraft::prepareWrite(const FileStagingChanges& edits) const {
  ensureLoaded();
  if (!sameEntries(m_entries, edits.after))
    throw std::runtime_error(ApplicationText::tr("The file staging draft changed after this plan was opened. Review a new plan.").toStdString());
  return [store = m_store, edits] {
    FileStagingWriteResult result;
    try {
      const auto current = store->load();
      if (!sameEntries(current, edits.before) && !sameEntries(current, edits.after))
        throw std::runtime_error(
            ApplicationText::tr("The saved file staging list changed after this plan was opened. Refresh and review the changes.").toStdString());
      result.attempted = true;
      store->replace(edits.after);
      result.observed = store->load();
      if (!sameEntries(*result.observed, edits.after))
        throw std::runtime_error(ApplicationText::tr("The saved file staging list does not match the accepted plan.").toStdString());
    } catch (...) {
      result.failure = std::current_exception();
      if (result.attempted) {
        try {
          result.observed = store->load();
        } catch (...) {
          result.observed.reset();
        }
        try {
          std::rethrow_exception(result.failure);
        } catch (const std::exception& error) {
          result.failure =
              std::make_exception_ptr(OperationError(std::make_error_code(std::errc::state_not_recoverable), error.what(), WriteEffect::Uncertain));
        } catch (...) {
          result.failure =
              std::make_exception_ptr(OperationError(std::make_error_code(std::errc::state_not_recoverable),
                                                     ApplicationText::tr("File staging could not be saved").toStdString(), WriteEffect::Uncertain));
        }
      }
    }
    return result;
  };
}
void FileStagingDraft::acceptWrite(const FileStagingWriteResult& result) {
  if (!result.attempted) return;
  if (result.observed) m_baseline = *result.observed;
  m_baselineKnown = result.observed.has_value();
  m_persistencePending = result.failure != nullptr;
  emit changed();
}
void FileStagingDraft::apply(const FileStagingChanges& edits) {
  const auto result = prepareWrite(edits)();
  acceptWrite(result);
  if (result.failure) std::rethrow_exception(result.failure);
}
}  // namespace uwf::app
#include "moc_FileStagingDraft.cpp"
