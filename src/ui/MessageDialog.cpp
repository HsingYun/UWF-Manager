#include "MessageDialog.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>

#include "I18n.h"

namespace uwf::ui::dialogs {

namespace {

// 统一构造 dialog 主体：标题、可选中正文 label、底部按钮区。caller 把按钮
// 自己 add 到返回的 QDialogButtonBox 上，再 connect 进 dlg.accept/reject。
struct DialogParts {
  QDialog* dlg;
  QLabel* body;
  QDialogButtonBox* btns;
};

DialogParts build(QWidget* parent, const QString& title, const QString& text) {
  auto* dlg = new QDialog(parent);
  dlg->setWindowTitle(title);

  auto* layout = new QVBoxLayout(dlg);
  layout->setContentsMargins(20, 16, 20, 12);
  layout->setSpacing(12);

  auto* body = new QLabel(text, dlg);
  body->setWordWrap(true);
  // 排错场景常需要复制文字进日志，所有 dialog 默认开启选中。
  body->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  layout->addWidget(body, 1);

  // 对话框宽度跟随内容：按最宽一行（整行不折所需的宽度）定下限，夹在
  // [360, 760]。够宽时 wordWrap 自然不折——长注册表路径之类的单行内容因而
  // 完整显示；只有极长文本才会折到 760 上限，避免对话框宽到溢出屏幕。
  const QFontMetrics fm(body->fontMetrics());
  int widest = 0;
  for (const auto& line : text.split('\n')) widest = std::max(widest, fm.horizontalAdvance(line));
  const auto margins = layout->contentsMargins();
  dlg->setMinimumWidth(std::clamp(widest + margins.left() + margins.right(), 360, 760));

  auto* btns = new QDialogButtonBox(dlg);
  layout->addWidget(btns);
  return {dlg, body, btns};
}

}  // namespace

void warning(QWidget* parent, const QString& title, const QString& text) {
  auto [dlg, body, btns] = build(parent, title, text);
  auto* ok = btns->addButton(I18n::tr("OK"), QDialogButtonBox::AcceptRole);
  QObject::connect(ok, &QPushButton::clicked, dlg, &QDialog::accept);
  dlg->exec();
  delete dlg;
}

void information(QWidget* parent, const QString& title, const QString& text) {
  // 当前实现 warning / information 完全等价：都是单 OK 按钮 + 可选中正文。
  // 留两个名字是为了 caller 表达意图（语义性 > DRY）。
  warning(parent, title, text);
}

bool confirm(QWidget* parent, const QString& title, const QString& text) {
  auto [dlg, body, btns] = build(parent, title, text);
  auto* okBtn = btns->addButton(I18n::tr("OK"), QDialogButtonBox::AcceptRole);
  auto* cancelBtn = btns->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  QObject::connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
  QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
  // 默认焦点在 Cancel 上：误按 Enter 不会触发危险动作。
  cancelBtn->setDefault(true);
  cancelBtn->setFocus();
  const bool accepted = dlg->exec() == QDialog::Accepted;
  delete dlg;
  return accepted;
}

}  // namespace uwf::ui::dialogs
