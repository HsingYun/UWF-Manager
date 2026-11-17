// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/appearance/I18n.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLocale>
#include <QTranslator>
#include <algorithm>

#include "presentation/appearance/AppearanceTransition.h"
#include "foundation/logging/Log.h"

namespace uwf::ui {

namespace {

using Lang = I18n::Lang;
using namespace Qt::Literals::StringLiterals;

// Installed translators remain owned by this module.
QTranslator& translator() {
  static QTranslator t;
  return t;
}

// Standard editor menus use a separate Qt translation catalog.
QTranslator& qtBaseTranslator() {
  static QTranslator t;
  return t;
}

Lang applyLang(const Lang requested) {
  auto& t = translator();
  auto& qt = qtBaseTranslator();
  QCoreApplication::removeTranslator(&t);
  QCoreApplication::removeTranslator(&qt);
  const auto language = std::ranges::find(app::languages(), requested, &I18n::Language::id);
  Lang actual = Lang::En;
  if (language != app::languages().end() && requested != Lang::En) {
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

LanguageEvents& I18n::events() {
  static LanguageEvents events;
  return events;
}

void I18n::bind(QObject* owner, std::function<void()> render) {
  QObject::connect(&events(), &LanguageEvents::changed, owner, render);
  render();
}

QString I18n::applicationTitle() { return tr("Unified Write Filter (UWF) Manager"); }

I18n::I18n() : m_lang(applyLang(detectSystemLang())) {}

std::span<const I18n::Language> I18n::languages() { return app::languages(); }

std::optional<I18n::Lang> I18n::parseLanguageCode(const QStringView code) { return app::parseLanguageCode(code); }

I18n::Lang I18n::matchLanguage(const QStringList& preferredLanguages) { return app::matchLanguage(preferredLanguages); }

I18n::Lang I18n::detectSystemLang() {
  const auto locale = QLocale::system();
  const auto preferred = locale.uiLanguages();
  return matchLanguage(preferred.isEmpty() ? QStringList{locale.name()} : preferred);
}

void I18n::setLang(const Lang l) {
  if (m_lang == l) return;
  m_lang = applyLang(l);
  emit events().changed();
}

void I18n::requestLang(const Lang l) {
  AppearanceTransition::request(&events(), [this, l] { setLang(l); }, [this, l] { return m_lang != l; });
}

void I18n::applyStartupLanguage(const std::optional<Lang> overrideLanguage) { setLang(overrideLanguage ? *overrideLanguage : detectSystemLang()); }

}  // namespace uwf::ui
#include "moc_I18n.cpp"
