// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QIODevice>
#include <QStringList>
#include <map>
#include <tuple>

namespace uwf::test::tools {

using MessageKey = std::tuple<QString, QString, QString>;

struct TranslationMessage {
  QString status;
  QStringList forms;
  bool plural = false;
};

struct TranslationCatalog {
  QString language;
  std::map<MessageKey, TranslationMessage> messages;
};

TranslationCatalog readCatalog(QIODevice& input);
TranslationCatalog readCatalog(const QString& path);
QStringList checkMessage(const MessageKey& key, const TranslationMessage& message, const QString& language, bool qtSupplement = false);
QStringList checkCatalog(const TranslationCatalog& catalog, const TranslationCatalog& reference, const QString& language, bool qtSupplement = false);

}  // namespace uwf::test::tools
