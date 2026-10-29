// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UiPreferences.h"

#include <QLocale>
#include <algorithm>
#include <array>

namespace uwf::app {

namespace {
using Lang = LanguageId;
using namespace Qt::Literals::StringLiterals;
// 菜单、配置与系统匹配共用同一语言表；显示名称使用各自语言的本地写法。
constexpr std::array kLanguages{
    Language{Lang::En, "en"_L1, u"English"},
    Language{Lang::Zh_CN, "zh_CN"_L1, u"简体中文"},
    Language{Lang::Zh_TW, "zh_TW"_L1, u"繁體中文"},
    Language{Lang::Ja, "ja"_L1, u"日本語"},
    Language{Lang::Ko, "ko"_L1, u"한국어"},
    Language{Lang::De, "de"_L1, u"Deutsch"},
    Language{Lang::Fr, "fr"_L1, u"Français"},
    Language{Lang::Es, "es"_L1, u"Español"},
    Language{Lang::It, "it"_L1, u"Italiano"},
    Language{Lang::Pt_BR, "pt_BR"_L1, u"Português (Brasil)"},
    Language{Lang::Pt_PT, "pt_PT"_L1, u"Português (Portugal)"},
    Language{Lang::Ru, "ru"_L1, u"Русский"},
    Language{Lang::Uk, "uk"_L1, u"Українська"},
    Language{Lang::Pl, "pl"_L1, u"Polski"},
    Language{Lang::Nl, "nl"_L1, u"Nederlands"},
    Language{Lang::Tr, "tr"_L1, u"Türkçe"},
    Language{Lang::Id, "id"_L1, u"Bahasa Indonesia"},
    Language{Lang::Vi, "vi"_L1, u"Tiếng Việt"},
    Language{Lang::Ar, "ar"_L1, u"العربية"},
};

}  // namespace

std::span<const Language> languages() { return kLanguages; }

std::optional<LanguageId> parseLanguageCode(const QStringView code) {
  const QString normalized = code.trimmed().toString().replace(u'-', u'_');
  const auto language = std::ranges::find_if(kLanguages, [&](const auto& candidate) { return normalized.compare(candidate.code, Qt::CaseInsensitive) == 0; });
  if (language == kLanguages.end()) return std::nullopt;
  return language->id;
}

LanguageId matchLanguage(const QStringList& preferredLanguages) {
  for (const auto& tag : preferredLanguages) {
    const QLocale locale(tag);
    switch (locale.language()) {
      case QLocale::Chinese:
        return locale.script() == QLocale::TraditionalHanScript ? Lang::Zh_TW : Lang::Zh_CN;
      case QLocale::Portuguese:
        return locale.territory() == QLocale::Brazil ? Lang::Pt_BR : Lang::Pt_PT;
      default:
        for (const auto& candidate : kLanguages) {
          if (candidate.id == Lang::Zh_CN || candidate.id == Lang::Zh_TW || candidate.id == Lang::Pt_BR || candidate.id == Lang::Pt_PT) continue;
          if (QLocale(QString(candidate.code)).language() == locale.language()) return candidate.id;
        }
    }
  }
  return Lang::En;
}

}  // namespace uwf::app
