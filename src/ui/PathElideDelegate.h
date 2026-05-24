#pragma once

// 自定义文本 delegate：接管 paint 里的 elision，绕开 Qt 默认路径在某些环境
// （实测 Win11 + 125% DPI）下把 elide 宽度算成远小于 cell 实际宽度的 bug。
//
// 用法：
//   table->setItemDelegateForColumn(col, new PathElideDelegate(table));
//
// 根因：Qt 默认 `style->drawControl(CE_ItemViewItem)` 内部会用
// `subElementRect(SE_ItemViewItemText)` 再扣一道子部件（icon area / focus 边框 /
// indicator margin）才得到文本绘制 rect；在非 100% DPI 下这一刀扣得过狠，
// 文本宽度远小于 cell 实际宽度，结果长内容被压缩成 "C:..." 而 cell 还很宽。
//
// 工作方式：
//   1. 用默认 drawControl 画背景 / 选中 / hover，但 opt.text 清空让它不画文本；
//   2. 自己按 opt.rect.width() 减去 QSS 配的 6×2 padding 算 elide 宽度，
//      调 QFontMetrics::elidedText 产生省略文本，然后 painter->drawText 绘到
//      cell 内——elide 宽度严格 = cell 实际宽度（扣 padding），跟 subElementRect
//      解耦。

#include <QApplication>
#include <QFontMetrics>
#include <QModelIndex>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

namespace uwf::ui {

class PathElideDelegate : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QString fullText = opt.text;
    opt.text.clear();  // 让默认路径不绘文本

    // 默认背景 / 选中 / hover 由 Qt 画
    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    // 自己绘文本——elide 宽度严格按 cell 实际宽度（扣 6px×2 内边距，与 QSS 一致）
    const int kPad = 6;
    const int textWidth = std::max(0, opt.rect.width() - kPad * 2);
    const QString elided = opt.fontMetrics.elidedText(fullText, opt.textElideMode, textWidth);

    const QRect textRect = opt.rect.adjusted(kPad, 0, -kPad, 0);
    const QPalette::ColorRole role = (opt.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text;
    painter->setPen(opt.palette.color(role));
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elided);
  }
};

}  // namespace uwf::ui
