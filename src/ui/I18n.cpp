#include "I18n.h"

#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

namespace uwf::ui {

namespace {

// 单例 QTranslator，install 一次后由 QCoreApplication 持有引用；切换语言时
// load 新的 .qm 即可，不需要 uninstall（load 会替换内部表）。
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

// 把 translator 切到目标语言：始终先 removeTranslator 让状态归零，再按
// 目标语言决定要不要重新 install。
//   - En：英文是源语言，不需要 .qm；remove 之后 tr() 直接返回 source。
//   - Zh_CN：load .qm 成功后 install；失败也保持 remove 状态走 source。
//
// 不缓存 "已 install" 的状态：早期版本用 static bool 标记一次 install，但
// remove 之后那个标记没复位，回切语言时就再也 install 不上了——表现为
// "切到英文后切不回中文"。每次都 remove + install 简单可靠，install 同一
// 个 translator 多次 Qt 自己会处理（先 remove 等价于幂等）。
void applyLang(const I18n::Lang l) {
  auto& t = translator();
  auto& qt = qtBaseTranslator();
  QCoreApplication::removeTranslator(&t);
  QCoreApplication::removeTranslator(&qt);
  switch (l) {
    case I18n::Lang::En:
      return;
    case I18n::Lang::Zh_CN:
      if (qt.load(":/i18n/qt/zh_CN.qm")) {
        QCoreApplication::installTranslator(&qt);
      }
      if (t.load(":/i18n/zh_CN.qm")) {
        QCoreApplication::installTranslator(&t);
      }
      return;
  }
}

}  // namespace

I18n& I18n::instance() {
  static I18n inst;
  return inst;
}

I18n::I18n() : m_lang(detectSystemLang()) { applyLang(m_lang); }

I18n::Lang I18n::detectSystemLang() {
  // 严格只对"中文 + 简体脚本"（zh_CN / zh_Hans_* / zh_SG）启用我们的中文翻译。
  // 用 QLocale::script() 判断而不是 name().startsWith("zh")——后者会把繁体
  // （zh_TW / zh_Hant_* / zh_HK / zh_MO）也匹配进来，给繁体用户套上简体词
  // 语序对不上的翻译，反而比直接显示英文源语言更难用。
  // QLocale 即便 name() 里没显式带 script tag 也能从 country code 推断出
  // 默认 script（zh_CN → SimplifiedHan, zh_TW → TraditionalHan），所以这条
  // 判断可靠。
  const QLocale loc = QLocale::system();
  if (loc.language() == QLocale::Chinese && loc.script() == QLocale::SimplifiedHanScript) {
    return Lang::Zh_CN;
  }
  return Lang::En;
}

void I18n::setLang(const Lang l) {
  m_lang = l;
  applyLang(l);
}

}  // namespace uwf::ui
