#include "CenteredTextStyle.h"

#include <QPalette>
#include <QStyleOptionTab>
#include <QTabWidget>
#include <QWidget>

namespace uwf::ui {

void CenteredTextStyle::drawItemText(QPainter* painter, const QRect& rect, const int flags, const QPalette& pal, const bool enabled, const QString& text,
                                     const QPalette::ColorRole textRole) const {
  QRect r = rect;
  // ButtonText 由 QToolButton / QTabBar 等绘制时使用；WindowText 默认角色
  // 也会在多处按钮场景出现。这里统一往上抬 1px 抵消基线偏移。
  if (textRole == QPalette::ButtonText || textRole == QPalette::WindowText || textRole == QPalette::NoRole) {
    r.translate(0, -1);
  }
  QProxyStyle::drawItemText(painter, r, flags, pal, enabled, text, textRole);
}

void CenteredTextStyle::drawControl(const ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const {
  if (ce == CE_TabBarTabLabel) {
    if (const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(opt)) {
      // 把整块 label 区域（图标 + 文字）向上挪 1px。
      // drawItemText 还会再给文字多抬 1px，于是文字相对图标上移 1px，
      // 视觉上图标和文字中线对齐。
      QStyleOptionTab shifted = *tab;
      shifted.rect.translate(0, -1);
      QProxyStyle::drawControl(ce, &shifted, p, w);
      return;
    }
  }
  QProxyStyle::drawControl(ce, opt, p, w);
}

void CenteredTextStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const {
  // Windows 11 样式会在 C++ 层给 QTabWidget 外层画圆角边框，并在 QTabBar 下
  // 画一条分隔线。这些不受 QSS border-radius / border 控制。只对最外层的
  // 盘符 TAB (objectName=mainTabs) 跳过这两个原语；内层 innerTabs 保留原生
  // 圆角外观。PE_FrameTabBarBase 传入的 w 可能是 QTabBar，要向上爬到
  // QTabWidget 再判断。
  if (pe == PE_FrameTabWidget || pe == PE_FrameTabBarBase) {
    const QWidget* tw = w;
    while (tw && !qobject_cast<const QTabWidget*>(tw)) {
      tw = tw->parentWidget();
    }
    if (tw && tw->objectName() == QLatin1String("mainTabs")) return;
  }
  QProxyStyle::drawPrimitive(pe, opt, p, w);
}

}  // namespace uwf::ui
