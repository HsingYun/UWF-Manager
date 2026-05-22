#pragma once

// QMessageBox 替代品。原生 QMessageBox 的内部 label 走单独的字体路径，
// 全局 app.setFont() 上设的 hinting / styleStrategy 不会传播过去，中文
// 在 9-10pt 下渲染明显糊。这里用 QDialog + QLabel 自己拼，文字直接继承
// app font。同时 warning/information 默认让文本可选中复制（排查 WMI 错误
// 时方便贴日志），confirm 默认按钮是 Cancel（更安全）。

#include <QString>

class QWidget;

namespace uwf::ui::dialogs {

void warning(QWidget* parent, const QString& title, const QString& text);
void information(QWidget* parent, const QString& title, const QString& text);

// 二选一确认。OK / Cancel 文案走 I18n::tr 拿翻译，避免依赖 Qt 内置翻译包。
// 返回 true = 用户选了 OK；false = 用户选了 Cancel 或关掉对话框。
bool confirm(QWidget* parent, const QString& title, const QString& text);

// 提交 / 删除到磁盘前的二次确认对话框，带视觉层次：操作标题（加粗）、目标
// （等宽字体 + 描边框，单独成块）、范围说明（次要色，detail 为空则不显示）、
// 固定的"不可撤销"警示横幅。比纯文本 confirm 更适合「即将改动磁盘 / 注册表」
// 这类破坏性操作。用户点"继续"返回 true。
bool confirmCommit(QWidget* parent, const QString& title, const QString& heading, const QString& target, const QString& detail);

}  // namespace uwf::ui::dialogs
