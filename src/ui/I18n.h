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
#pragma once

// Qt 风格 i18n：对外暴露 I18n::tr(...)（由 Q_DECLARE_TR_FUNCTIONS(uwf) 生成
// 的静态方法，等价于 QCoreApplication::translate("uwf", ...)），返回 QString。
//
// 工作流：
//   - 源语言：英文。代码里所有 UI 文案都写英文 source。
//   - lupdate 扫源码识别 I18n::tr("...")（因为它走 QCoreApplication::translate
//     的标准路径），把 source 提到 resources/i18n/zh_CN.ts；译者填中文
//     translation；lrelease 编出 .qm 嵌进资源。
//   - 运行时按系统语言加载相应 .qm；加载失败或没匹配项时 translate() 会
//     回落到 source（即英文），这正是我们要的 fallback。
//
// 用法：
//   I18n::tr("Filter")                       // → QString
//   I18n::tr("Has %1 items").arg(n)          // 用 QString::arg 注入参数
//   I18n::tr("%1 / %2").arg(a).arg(b)        // 多参数 chain
//
// 占位符：QString::arg 用 %1/%2/...，不再用 std::format 的 {0}/{1}。
// 不同语言可以在 .ts 里自由调换 %1/%2 的位置实现重排。

#include <QCoreApplication>
#include <QString>

namespace uwf::ui {

class I18n {
  Q_DECLARE_TR_FUNCTIONS(uwf)

 public:
  enum class Lang {
    En,     // English (源语言，不需要 .qm)
    Zh_CN,  // 简体中文
    // 新增语言：在这里加 enum、扩 detectSystemLang/loadLang，并在 .ts 旁边
    // 放对应 locale 的 .ts 文件。
  };

  static I18n& instance();
  [[nodiscard]] Lang lang() const { return m_lang; }
  void setLang(Lang l);

  static Lang detectSystemLang();

 private:
  I18n();
  Lang m_lang;
};

}  // namespace uwf::ui
