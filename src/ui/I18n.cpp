/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "I18n.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLocale>
#include <QTranslator>
#include <algorithm>
#include <array>

#include "../util/Log.h"

namespace uwf::ui {

namespace {

using Lang = I18n::Lang;
using namespace Qt::Literals::StringLiterals;

// 菜单、配置与系统匹配共用同一语言表；显示名称使用各自语言的本地写法。
constexpr std::array kLanguages{
    I18n::Language{Lang::En, "en"_L1, u"English"},
    I18n::Language{Lang::Zh_CN, "zh_CN"_L1, u"简体中文"},
    I18n::Language{Lang::Zh_TW, "zh_TW"_L1, u"繁體中文"},
    I18n::Language{Lang::Ja, "ja"_L1, u"日本語"},
    I18n::Language{Lang::Ko, "ko"_L1, u"한국어"},
    I18n::Language{Lang::De, "de"_L1, u"Deutsch"},
    I18n::Language{Lang::Fr, "fr"_L1, u"Français"},
    I18n::Language{Lang::Es, "es"_L1, u"Español"},
    I18n::Language{Lang::It, "it"_L1, u"Italiano"},
    I18n::Language{Lang::Pt_BR, "pt_BR"_L1, u"Português (Brasil)"},
    I18n::Language{Lang::Pt_PT, "pt_PT"_L1, u"Português (Portugal)"},
    I18n::Language{Lang::Ru, "ru"_L1, u"Русский"},
    I18n::Language{Lang::Uk, "uk"_L1, u"Українська"},
    I18n::Language{Lang::Pl, "pl"_L1, u"Polski"},
    I18n::Language{Lang::Nl, "nl"_L1, u"Nederlands"},
    I18n::Language{Lang::Tr, "tr"_L1, u"Türkçe"},
    I18n::Language{Lang::Id, "id"_L1, u"Bahasa Indonesia"},
    I18n::Language{Lang::Vi, "vi"_L1, u"Tiếng Việt"},
    I18n::Language{Lang::Ar, "ar"_L1, u"العربية"},
};

// 翻译器由本模块拥有；QCoreApplication 仅在安装期间引用它们。
QTranslator& translator() {
  static QTranslator t;
  return t;
}

// Qt 自带控件（标准右键菜单的 复制/全选、标准对话框按钮等）的文案由 Qt 官方
// qtbase 翻译包提供
QTranslator& qtBaseTranslator() {
  static QTranslator t;
  return t;
}

Lang applyLang(const Lang requested) {
  auto& t = translator();
  auto& qt = qtBaseTranslator();
  QCoreApplication::removeTranslator(&t);
  QCoreApplication::removeTranslator(&qt);
  const auto language = std::ranges::find(kLanguages, requested, &I18n::Language::id);
  Lang actual = Lang::En;
  if (language != kLanguages.end() && requested != Lang::En) {
    const QString code(language->code);
    if (t.load(QStringLiteral(":/i18n/%1.qm").arg(code))) {
      if (qt.load(QStringLiteral(":/i18n/qtbase_%1.qm").arg(code))) QCoreApplication::installTranslator(&qt);
      QCoreApplication::installTranslator(&t);
      actual = requested;
    } else {
      UWF_LOG_W("i18n") << "language catalog unavailable: language=" << code.toStdString();
    }
  }
  QGuiApplication::setLayoutDirection(actual == Lang::Ar ? Qt::RightToLeft : Qt::LeftToRight);
  return actual;
}

}  // namespace

I18n& I18n::instance() {
  static I18n inst;
  return inst;
}

QString I18n::applicationTitle() { return tr("Unified Write Filter (UWF) Manager"); }

QString I18n::enhancedModeServiceDescription() { return tr("UWF Manager enhanced mode helper service"); }

I18n::I18n() : m_lang(applyLang(detectSystemLang())) {}

std::span<const I18n::Language> I18n::languages() { return kLanguages; }

std::optional<I18n::Lang> I18n::parseLanguageCode(const QStringView code) {
  const QString normalized = code.trimmed().toString().replace(u'-', u'_');
  const auto language = std::ranges::find_if(kLanguages, [&](const auto& candidate) { return normalized.compare(candidate.code, Qt::CaseInsensitive) == 0; });
  if (language == kLanguages.end()) return std::nullopt;
  return language->id;
}

I18n::Lang I18n::matchLanguage(const QStringList& preferredLanguages) {
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

I18n::Lang I18n::detectSystemLang() {
  const auto locale = QLocale::system();
  const auto preferred = locale.uiLanguages();
  return matchLanguage(preferred.isEmpty() ? QStringList{locale.name()} : preferred);
}

void I18n::setLang(const Lang l) { m_lang = applyLang(l); }

void I18n::applyStartupLanguage(const std::optional<Lang> overrideLanguage) { setLang(overrideLanguage ? *overrideLanguage : detectSystemLang()); }

}  // namespace uwf::ui
