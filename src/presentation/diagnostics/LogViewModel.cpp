// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "LogViewModel.h"

#include <QPointer>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include "presentation/common/ItemRoles.h"
#include "presentation/common/PresentationText.h"
#include "shared/Log.h"
namespace uwf::presentation {
namespace {
LogViewModel::LogRow parseLine(const std::string& line) {
  if (line.size() > 2 && line.front() == '[') {
    const auto rb = line.find(']');
    if (rb != std::string::npos) {
      const std::string head = line.substr(1, rb - 1);
      const std::string msg = line.substr(rb + 1);
      const auto sp1 = head.find(' ');
      const auto sp2 = sp1 == std::string::npos ? sp1 : head.find(' ', sp1 + 1);
      if (sp1 != std::string::npos && sp2 != std::string::npos) {
        return {QString::fromStdString(head.substr(0, sp1)), QString::fromStdString(head.substr(sp1 + 1, sp2 - sp1 - 1)),
                QString::fromStdString(head.substr(sp2 + 1)), QString::fromStdString(msg).trimmed()};
      }
    }
  }
  return {QString(), QString(), QString(), QString::fromStdString(line)};
}

}  // namespace
LogViewModel::LogViewModel(std::shared_ptr<app::LogSource> source) : m_source(std::move(source)), m_filtered(std::make_unique<LogFilterModel>(this)) {
  if (!m_source) throw std::invalid_argument("log source must be provided");
  m_filtered->setSourceModel(this);
}
LogViewModel::~LogViewModel() = default;
void LogViewModel::reload() {
  m_loader.cancel();
  QPointer<LogViewModel> self(this);
  const auto generation = m_loading.begin();
  m_state = ContentLoadState::Loading;
  m_error.clear();
  emit loadingStarted();
  if (!self || !m_loading.isCurrent(generation)) return;
  m_loader.start(
      [source = m_source](std::stop_token token) {
        if (token.stop_requested()) return std::vector<LogRow>{};
        auto raw = source->recentLines();
        std::vector<LogRow> entries;
        entries.reserve(raw.size());
        for (const auto& line : raw) {
          if (token.stop_requested()) break;
          entries.push_back(parseLine(line));
        }
        return entries;
      },
      [this, self, generation](AsyncResult<std::vector<LogRow>> result) mutable {
        m_loading.publish(generation, [self, generation, result = std::move(result)]() mutable {
          if (self) self->onLoadFinished(generation, std::move(result));
        });
      });
}
void LogViewModel::onLoadFinished(const LoadingGate::Ticket generation, AsyncResult<std::vector<LogRow>> result) {
  if (const auto* failure = std::get_if<std::exception_ptr>(&result)) {
    try {
      std::rethrow_exception(*failure);
    } catch (const std::exception& error) {
      m_error = PresentationText::tr("Failed to load logs: %1").arg(QString::fromUtf8(error.what()));
    } catch (...) {
      m_error = PresentationText::tr("Failed to load logs: unexpected error.");
    }
    m_state = ContentLoadState::Failed;
    emit loadFailed(m_error);
    return;
  }
  QPointer<LogViewModel> self(this);
  auto entries = std::get<std::vector<LogRow>>(std::move(result));
  const int total = static_cast<int>(entries.size());
  beginResetModel();
  if (!self) return;
  if (!m_loading.isCurrent(generation)) {
    endResetModel();
    return;
  }
  m_entries = std::move(entries);
  endResetModel();
  if (!self || !m_loading.isCurrent(generation)) return;
  m_state = total ? ContentLoadState::Ready : ContentLoadState::Empty;
  emit entriesChanged();
  if (self && m_loading.isCurrent(generation)) emit loadingFinished(total);
}
void LogViewModel::cancelLoading() {
  if (!loading()) return;
  m_loading.cancel();
  m_loader.cancel();
  m_state = ContentLoadState::Canceled;
  emit loadingCanceled();
}
int LogViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(entries().size()); }
int LogViewModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 4; }
QVariant LogViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0 || section >= 4) return {};
  const QStringList headers{PresentationText::tr("Time"), PresentationText::tr("Level"), PresentationText::tr("Tag"), PresentationText::tr("Message")};
  return headers[section];
}
QVariant LogViewModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= 4) return {};
  const auto& row = entries()[static_cast<std::size_t>(index.row())];
  if (role == FullTextRole) return row[static_cast<std::size_t>(index.column())];
  if (role == MonospaceRole) return index.column() == 0 || index.column() == 2;
  if (role == RowStatusRole) return static_cast<int>(row[1] == QLatin1String("E") || row[1] == QLatin1String("F") ? ItemStatus::Error : ItemStatus::Normal);
  if (role == StatusRole && (index.column() == 0 || index.column() == 2)) return static_cast<int>(ItemStatus::Muted);
  if (role == Qt::DisplayRole || (role == Qt::ToolTipRole && index.column() == 3)) return row[static_cast<std::size_t>(index.column())];
  if (index.column() == 1) {
    if (role == StatusIconRole) return true;
    if (role == StatusRole)
      return static_cast<int>(row[1] == QLatin1String("E") || row[1] == QLatin1String("F") ? ItemStatus::Error
                              : row[1] == QLatin1String("W")                               ? ItemStatus::Warning
                              : row[1] == QLatin1String("I")                               ? ItemStatus::Muted
                                                                                           : ItemStatus::Faint);
  }
  return {};
}
void LogViewModel::clear() {
  try {
    m_source->clear();
    reload();
  } catch (const std::exception& error) {
    m_loading.cancel();
    m_loader.cancel();
    m_error = PresentationText::tr("Failed to clear logs: %1").arg(QString::fromUtf8(error.what()));
    m_state = ContentLoadState::Failed;
    emit loadFailed(m_error);
  } catch (...) {
    m_loading.cancel();
    m_loader.cancel();
    m_error = PresentationText::tr("Failed to clear logs: unexpected error.");
    m_state = ContentLoadState::Failed;
    emit loadFailed(m_error);
  }
}
QString LogViewModel::allRowsToText() const {
  QString out;
  out.reserve(static_cast<qsizetype>(m_entries.size()) * 64);
  QStringList headers;
  for (int column = 0; column < columnCount(); ++column) headers.append(headerData(column, Qt::Horizontal, Qt::DisplayRole).toString());
  out += headers.join('\t') + '\n';
  for (const auto& f : m_entries) {
    out += f[0];
    out += QChar('\t');
    out += f[1];
    out += QChar('\t');
    out += f[2];
    out += QChar('\t');
    out += f[3];
    out += QChar('\n');
  }
  return out;
}
}  // namespace uwf::presentation

#include "moc_LogViewModel.cpp"
