// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RegistryValueModel.h"

#include <algorithm>

#include "app/registry/RegistryValueFormat.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PresentationText.h"
namespace uwf::presentation {
namespace {
enum RegistryType : uint32_t {
  REG_NONE = 0,
  REG_SZ = 1,
  REG_EXPAND_SZ = 2,
  REG_BINARY = 3,
  REG_DWORD = 4,
  REG_DWORD_BIG_ENDIAN = 5,
  REG_LINK = 6,
  REG_MULTI_SZ = 7,
  REG_RESOURCE_LIST = 8,
  REG_FULL_RESOURCE_DESCRIPTOR = 9,
  REG_RESOURCE_REQUIREMENTS_LIST = 10,
  REG_QWORD = 11
};
// 按 Win32 类型生成单行预览，最多显示 200 个字符。
constexpr int kPreviewMaxChars = 200;
QString truncatedAt(QString s, int max = kPreviewMaxChars) {
  if (s.size() > max) {
    s.resize(max);
    s.append(QStringLiteral("…"));
  }
  return s;
}

QString decodeUtf16Le(const std::vector<uint8_t>& data) {
  QString text;
  text.reserve(static_cast<qsizetype>((data.size() + 1) / 2));
  for (size_t index = 0; index + 1 < data.size();) {
    const auto codeUnit = static_cast<char16_t>(static_cast<uint16_t>(data[index]) | (static_cast<uint16_t>(data[index + 1]) << 8));
    index += 2;
    if (QChar::isHighSurrogate(codeUnit)) {
      if (index + 1 < data.size()) {
        const auto low = static_cast<char16_t>(static_cast<uint16_t>(data[index]) | (static_cast<uint16_t>(data[index + 1]) << 8));
        if (QChar::isLowSurrogate(low)) {
          text.append(QChar(codeUnit));
          text.append(QChar(low));
          index += 2;
          continue;
        }
      }
      text.append(QChar(QChar::ReplacementCharacter));
    } else if (QChar::isLowSurrogate(codeUnit)) {
      text.append(QChar(QChar::ReplacementCharacter));
    } else {
      text.append(QChar(codeUnit));
    }
  }
  if (data.size() % 2 != 0) text.append(QChar(QChar::ReplacementCharacter));
  return text;
}

QString formatValuePreview(uint32_t type, const std::vector<uint8_t>& data) {
  if (data.empty()) return type == REG_SZ ? PresentationText::tr("(value not set)") : PresentationText::tr("(empty)");

  switch (type) {
    case REG_SZ:
    case REG_EXPAND_SZ:
    case REG_LINK: {
      // UTF-16 LE 字节流；末尾的 null terminator（一对 0x00 0x00）要去掉。
      QString s = decodeUtf16Le(data);
      while (s.endsWith(QChar(u'\0'))) s.chop(1);
      return truncatedAt(s);
    }
    case REG_MULTI_SZ: {
      const QString decoded = decodeUtf16Le(data);
      const QStringView view(decoded);
      QStringList parts;
      qsizetype begin = 0;
      for (qsizetype i = 0; i < view.size(); ++i) {
        if (view[i] == QChar(u'\0')) {
          if (i > begin) parts.append(view.mid(begin, i - begin).toString());
          begin = i + 1;
        }
      }
      return truncatedAt(parts.join(QStringLiteral(" · ")));
    }
    case REG_DWORD: {
      if (data.size() < 4) return PresentationText::tr("(empty)");
      const uint32_t v = static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) | (static_cast<uint32_t>(data[2]) << 16) |
                         (static_cast<uint32_t>(data[3]) << 24);
      return QString::asprintf("0x%08X (%u)", v, v);
    }
    case REG_DWORD_BIG_ENDIAN: {
      if (data.size() < 4) return PresentationText::tr("(empty)");
      const uint32_t v = (static_cast<uint32_t>(data[0]) << 24) | (static_cast<uint32_t>(data[1]) << 16) | (static_cast<uint32_t>(data[2]) << 8) |
                         static_cast<uint32_t>(data[3]);
      return QString::asprintf("0x%08X (%u)", v, v);
    }
    case REG_QWORD: {
      if (data.size() < 8) return PresentationText::tr("(empty)");
      uint64_t v = 0;
      for (size_t i = 0; i < 8; ++i) v |= static_cast<uint64_t>(data[i]) << (i * 8);
      return QString::asprintf("0x%016llX (%llu)", static_cast<unsigned long long>(v), static_cast<unsigned long long>(v));
    }
    default: {
      // 二进制预览最多读取 64 字节，剩余内容以省略号表示。
      constexpr size_t kMaxBytes = 64;
      const size_t shown = std::min(data.size(), kMaxBytes);
      QString hex;
      hex.reserve(static_cast<qsizetype>(shown * 3));
      for (size_t i = 0; i < shown; ++i) {
        if (i > 0) hex.append(' ');
        hex.append(QString::asprintf("%02X", data[i]));
      }
      if (data.size() > kMaxBytes) hex.append(QStringLiteral(" …"));
      return hex;
    }
  }
}

}  // namespace
RegistryValueModel::RegistryValueModel(bool selectable, QObject* parent) : QAbstractTableModel(parent), m_selectable(selectable) {}
int RegistryValueModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_rows.size()); }
int RegistryValueModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 3; }
QVariant RegistryValueModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0 || section >= 3) return {};
  const QStringList headers{PresentationText::tr("Name"), PresentationText::tr("Type"), PresentationText::tr("Data")};
  return headers[section];
}
QVariant RegistryValueModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= 3) return {};
  const auto& row = m_rows[index.row()];
  if (role == NameRole && index.column() == 0) return row.name;
  if (role == MonospaceRole) return index.column() == 1;
  if (role == StatusRole && index.column() == 1) return static_cast<int>(ItemStatus::Muted);
  if (role != Qt::DisplayRole && role != Qt::ToolTipRole) return {};
  return index.column() == 0 ? row.displayName : index.column() == 1 ? row.type : row.preview;
}
Qt::ItemFlags RegistryValueModel::flags(const QModelIndex& index) const {
  if (!index.isValid() || index.model() != this || index.row() >= rowCount() || index.row() < 0) return Qt::NoItemFlags;
  return m_selectable && !m_rows[index.row()].name.isEmpty() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}
void RegistryValueModel::setValues(const std::vector<app::RegistryValueInfo>& values) {
  QList<Row> rows;
  rows.reserve(static_cast<qsizetype>(values.size()));
  for (const auto& value : values) {
    const QString name = QString::fromStdString(value.name);
    rows.append({name, name.isEmpty() ? PresentationText::tr("(Default)") : name, QString::fromStdString(app::valueTypeName(value.type)),
                 value.data ? formatValuePreview(value.type, *value.data) : PresentationText::tr("Preview unavailable (value is too large)")});
  }
  beginResetModel();
  m_rows = std::move(rows);
  endResetModel();
}
}  // namespace uwf::presentation

#include "moc_RegistryValueModel.cpp"
