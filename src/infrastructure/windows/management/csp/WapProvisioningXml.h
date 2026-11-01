// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QStringList>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

namespace uwf::windows::csp {
enum class WapAction { GetValue, GetChildren, PutValue, AddCharacteristic, RemoveValue, RemoveCharacteristic };
struct WapValue {
  QString datatype;
  QString text;
};
struct WapCommand {
  WapAction action;
  QStringList parents;
  QString name;
  std::optional<WapValue> value;
};
struct WapNode {
  QString element;
  QString name;
  std::optional<WapValue> value;
  std::vector<WapNode> children;
};
struct WapProviderError {
  std::optional<std::uint32_t> code;
  QString element;
  QString name;
  QString description;
};
struct WapDocument {
  WapNode root;
  std::vector<WapProviderError> errors;
};
// A local command cannot be represented without changing its original text.
// Callers must reject it before invoking a transport, independently of errors
// reported by a provider response.
class WapEncodingError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};
class WapProtocolError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

[[nodiscard]] QString encodeWapCommand(const WapCommand& command);
[[nodiscard]] WapDocument decodeWapDocument(const QString& xml);
// Missing and duplicate nodes are protocol failures, never empty collections.
[[nodiscard]] const WapNode& wapCharacteristic(const WapDocument& document, const QStringList& path);
[[nodiscard]] const WapValue& wapParameter(const WapNode& characteristic, const QString& name);
}  // namespace uwf::windows::csp
