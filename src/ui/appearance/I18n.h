// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 源文案使用英文 I18n::tr，lupdate/lrelease 生成并嵌入翻译目录。
// 缺少译文时回退源文案；动态参数使用 %1、%2，允许译文调整顺序。

#include <QCoreApplication>
#include <QLatin1StringView>
#include <QObject>
#include <QString>
#include <QStringView>
#include <functional>
#include <optional>
#include <span>

#include "app/settings/UiPreferences.h"

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
  // 按首选顺序匹配语言与脚本/地区；所有首选语言均不支持时使用英文。
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
  [[nodiscard]] static QString enhancedModeServiceDescription();
  [[nodiscard]] Lang lang() const { return m_lang; }
  void setLang(Lang l);
  void requestLang(Lang l);
  // 只用于启动初始化；未指定时跟随系统，不保存工具栏中的临时选择。
  void applyStartupLanguage(std::optional<Lang> overrideLanguage);

  static Lang detectSystemLang();

 private:
  I18n();
  Lang m_lang;
};

}  // namespace uwf::ui
