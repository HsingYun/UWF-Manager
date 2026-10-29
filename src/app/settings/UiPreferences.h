// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QLatin1StringView>
#include <QStringList>
#include <QStringView>
#include <optional>
#include <span>

namespace uwf::app {

enum class Theme { Dark, Light };

enum class LanguageId {
  En,
  Zh_CN,
  Zh_TW,
  Ja,
  Ko,
  De,
  Fr,
  Es,
  It,
  Pt_BR,
  Pt_PT,
  Ru,
  Uk,
  Pl,
  Nl,
  Tr,
  Id,
  Vi,
  Ar,
};

struct Language {
  LanguageId id;
  QLatin1StringView code;
  QStringView nativeName;
};

[[nodiscard]] std::span<const Language> languages();
[[nodiscard]] std::optional<LanguageId> parseLanguageCode(QStringView code);
[[nodiscard]] LanguageId matchLanguage(const QStringList& preferredLanguages);

}  // namespace uwf::app
