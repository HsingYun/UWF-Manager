// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// English source text is the fallback for missing catalog entries.
// Positional placeholders allow translations to reorder arguments.

#include <QCoreApplication>
#include <QLatin1StringView>
#include <QObject>
#include <QString>
#include <QStringView>
#include <functional>
#include <optional>
#include <span>

#include "application/settings/UiPreferences.h"

namespace uwf::ui {

class LanguageEvents final : public QObject {
  Q_OBJECT
 signals:
  void changed();
};

class I18n {
  Q_DECLARE_TR_FUNCTIONS(uwf)

 public:
  using Lang = app::LanguageId;
  using Language = app::Language;

  [[nodiscard]] static std::span<const Language> languages();
  [[nodiscard]] static std::optional<Lang> parseLanguageCode(QStringView code);
  // Matches language, script and region in preference order; falls back to English.
  [[nodiscard]] static Lang matchLanguage(const QStringList& preferredLanguages);

  static I18n& instance();
  static LanguageEvents& events();
  // Bind display updates to the owner's lifetime. Rendering must not reset editors.
  static void bind(QObject* owner, std::function<void()> render);
  template <typename Target, typename Render>
  static Target* text(Target* target, Render render) {
    bind(target, [target, render] { target->setText(render()); });
    return target;
  }
  [[nodiscard]] static QString applicationTitle();
  [[nodiscard]] Lang lang() const { return m_lang; }
  void setLang(Lang l);
  void requestLang(Lang l);
  // Applies the startup override or system preference without persisting toolbar selections.
  void applyStartupLanguage(std::optional<Lang> overrideLanguage);

  static Lang detectSystemLang();

 private:
  I18n();
  Lang m_lang;
};

}  // namespace uwf::ui
