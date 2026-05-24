#pragma once

// "About UWF Manager" 对话框：logo + 标题 + 版本号 + GPL 说明 + UWF 依赖说明。
// 从 MainWindow::showAbout 拆出来——版面 + HTML 字符串占 100 行，没有业务逻辑，
// 留在主窗口里只是噪音。MainWindow::showAbout 现在退化成两行 dlg.exec()。

#include <QDialog>

namespace uwf::ui {

class AboutDialog : public QDialog {
  Q_OBJECT
 public:
  explicit AboutDialog(QWidget* parent = nullptr);
};

}  // namespace uwf::ui
