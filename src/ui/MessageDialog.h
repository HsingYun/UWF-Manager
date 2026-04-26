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

}  // namespace uwf::ui::dialogs
