#pragma once

#include <QAbstractButton>

namespace uwf::ui {

// 仿 Windows 11 风格的纯开关控件（只画 pill + 圆点，不带文字）。
// 通过 property("dirty") 可以让 dirty 状态高亮。
class SwitchButton : public QAbstractButton {
  Q_OBJECT
 public:
  explicit SwitchButton(QWidget* parent = nullptr);

  [[nodiscard]] QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent*) override;
};

}  // namespace uwf::ui
