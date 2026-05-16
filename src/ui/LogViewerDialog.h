#pragma once

// UWF Manager 进程内日志查看器。
//
// 进程内日志写在一个环形缓冲里（见 src/util/Log.h），本对话框把它解析成
// 四列表格（时间 / 级别 / 标签 / 消息）分页展示，并提供刷新 / 复制 / 清空。
// 完全自包含——构造即建好整套 UI 并在后台线程异步拉起首批日志。

#include <QDialog>

namespace uwf::ui {

class LogViewerDialog : public QDialog {
  Q_OBJECT
 public:
  explicit LogViewerDialog(QWidget* parent = nullptr);
};

}  // namespace uwf::ui
