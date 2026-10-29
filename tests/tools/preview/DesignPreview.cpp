// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QFocusFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardItemModel>
#include <QStyleOptionButton>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>
#include <memory>

#include "ui/design/DesignStyle.h"
#include "ui/design/DesignSystem.h"
#include "ui/widgets/BusyIndicator.h"
#include "ui/widgets/SwitchButton.h"
#include "ui/widgets/TextLabel.h"

namespace {
using namespace uwf::ui;
class Surface : public QWidget {
 public:
  explicit Surface(QWidget* parent = nullptr, bool footer = false) : QWidget(parent), m_footer(footer) {}

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    const auto& theme = DesignSystem::instance();
    painter.fillRect(rect(), theme.color(m_footer ? design::Color::Footer : design::Color::Window));
    if (m_footer) {
      painter.setPen(theme.color(design::Color::Line));
      painter.drawLine(rect().topLeft(), rect().topRight());
    }
  }

 private:
  bool m_footer;
};
class SampleButton : public QPushButton {
 public:
  SampleButton(const QString& text, const QString& role, int state, QWidget* parent) : QPushButton(text, parent), m_state(state) {
    setProperty("dialogAction", role);
    setProperty("designHover", state == 1 ? 1.0 : 0.0);
    setFixedHeight(36);
    setEnabled(state != 4);
  }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    if (m_state == 1) option.state |= QStyle::State_MouseOver;
    if (m_state == 2) option.state |= QStyle::State_Sunken;
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
  }

 private:
  int m_state;
};
QLabel* label(const QString& text, int size = 14, bool bold = false, QWidget* parent = nullptr) {
  auto* result = new TextLabel(text,
                               size == 25   ? TextStyle::Title
                               : size == 11 ? TextStyle::Eyebrow
                               : size == 13 ? TextStyle::Label
                               : size == 12 ? TextStyle::Caption
                                            : TextStyle::Body,
                               parent);
  QFont font = result->font();
  font.setPixelSize(size);
  font.setWeight(bold ? QFont::DemiBold : QFont::Normal);
  result->setFont(font);
  return result;
}
QWidget* gallery() {
  auto* root = new Surface();
  root->setObjectName(QStringLiteral("designPreview"));
  root->setWindowTitle(QStringLiteral("UWF Manager · 控件规范"));
  root->setWindowFlag(Qt::FramelessWindowHint);
  root->setFixedWidth(1096);
  auto* all = new QVBoxLayout(root);
  all->setContentsMargins(0, 0, 0, 0);
  all->setSpacing(0);
  auto* header = new QVBoxLayout();
  header->setContentsMargins(32, 29, 32, 23);
  header->setSpacing(8);
  header->addWidget(label(QStringLiteral("UWF Manager"), 11));
  header->addWidget(label(QStringLiteral("控件规范"), 25, true));
  header->addWidget(label(QStringLiteral("同一视觉参数覆盖所有页面与交互状态。"), 13));
  all->addLayout(header);
  auto* body = new QVBoxLayout();
  body->setContentsMargins(32, 0, 32, 26);
  body->setSpacing(24);
  all->addLayout(body);
  auto* matrix = new QGridLayout();
  matrix->setVerticalSpacing(18);
  matrix->setHorizontalSpacing(16);
  matrix->setColumnMinimumWidth(0, 110);
  const QStringList states{QStringLiteral("默认"), QStringLiteral("悬停"), QStringLiteral("按下"), QStringLiteral("键盘焦点"), QStringLiteral("禁用")};
  for (int column = 0; column < states.size(); ++column) {
    matrix->addWidget(label(states[column], 12), 0, column + 1);
    matrix->setColumnStretch(column + 1, 1);
  }
  const QStringList names{QStringLiteral("次级动作"), QStringLiteral("主要动作"), QStringLiteral("危险动作"), QStringLiteral("轻量动作")};
  const QStringList texts{QStringLiteral("取消"), QStringLiteral("应用更改"), QStringLiteral("删除并提交"), QStringLiteral("复制详情")};
  const QStringList roles{QStringLiteral("secondary"), QStringLiteral("primary"), QStringLiteral("destructive"), QStringLiteral("quiet")};
  for (int row = 0; row < 4; ++row) {
    matrix->addWidget(label(names[row], 12), row + 1, 0);
    for (int state = 0; state < 5; ++state) {
      auto* button = new SampleButton(texts[row], roles[row], state, root);
      matrix->addWidget(button, row + 1, state + 1, Qt::AlignLeft);
      if (state == 3) {
        auto* frame = new QFocusFrame(button);
        frame->setWidget(button);
        frame->show();
      }
    }
  }
  body->addWidget(label(QStringLiteral("按钮"), 13, true));
  body->addLayout(matrix);
  body->addWidget(label(QStringLiteral("输入与选择"), 13, true));
  auto* fields = new QHBoxLayout();
  fields->setSpacing(22);
  auto* path = new QLineEdit(QStringLiteral("C:\\ProgramData\\MyApp"), root);
  path->setFixedSize(220, 36);
  fields->addWidget(path);
  auto* combo = new QComboBox(root);
  combo->addItems({QStringLiteral("RAM"), QStringLiteral("Disk")});
  combo->setFixedSize(160, 36);
  fields->addWidget(combo);
  auto* active = new SwitchButton(root);
  active->setChecked(true);
  active->setProperty("accessibleName", QStringLiteral("启用筛选器"));
  fields->addWidget(active);
  fields->addWidget(label(QStringLiteral("启用筛选器"), 13));
  auto* inactive = new SwitchButton(root);
  inactive->setEnabled(false);
  inactive->setProperty("accessibleName", QStringLiteral("不可用"));
  fields->addWidget(inactive);
  fields->addWidget(label(QStringLiteral("不可用"), 13));
  fields->addStretch();
  body->addLayout(fields);
  body->addWidget(label(QStringLiteral("列表"), 13, true));
  auto* table = new QTableView(root);
  auto* model = new QStandardItemModel(3, 2, table);
  model->setHorizontalHeaderLabels({QStringLiteral("文件或文件夹"), QStringLiteral("状态")});
  const QStringList paths{QStringLiteral("C:\\ProgramData\\MyApp\\Logs"), QStringLiteral("C:\\Users\\Public\\Documents"),
                          QStringLiteral("C:\\ProgramData\\MyApp\\Cache")};
  for (int row = 0; row < 3; ++row) {
    model->setItem(row, 0, new QStandardItem(paths[row]));
    model->setItem(row, 1, new QStandardItem(row == 2 ? QStringLiteral("新增") : QStringLiteral("已配置")));
  }
  table->setModel(model);
  table->setShowGrid(false);
  table->verticalHeader()->hide();
  table->verticalHeader()->setDefaultSectionSize(42);
  table->horizontalHeader()->setFixedHeight(38);
  table->horizontalHeader()->setStretchLastSection(false);
  table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table->setColumnWidth(1, 120);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  table->selectRow(1);
  table->setFixedHeight(166);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setFrameShape(QFrame::NoFrame);
  body->addWidget(table);
  auto* progress = new QProgressBar(root);
  progress->setRange(0, 0);
  progress->setTextVisible(false);
  progress->setFixedHeight(4);
  body->addWidget(label(QStringLiteral("正在读取覆盖层…"), 13));
  body->addWidget(progress);
  auto* footer = new Surface(root, true);
  footer->setFixedHeight(76);
  auto* actions = new QHBoxLayout(footer);
  actions->setContentsMargins(28, 18, 28, 18);
  actions->setSpacing(8);
  actions->addWidget(label(QStringLiteral("按钮默认焦点与主要动作分别定义"), 12));
  actions->addStretch();
  auto* apply = new QPushButton(QStringLiteral("应用更改"), footer);
  apply->setProperty("dialogAction", "primary");
  auto* cancel = new QPushButton(QStringLiteral("取消"), footer);
  actions->addWidget(apply);
  actions->addWidget(cancel);
  all->addWidget(footer);
  root->adjustSize();
  return root;
}
}  // namespace
int main(int argc, char** argv) {
  QApplication application(argc, argv);
  const auto args = application.arguments();
  auto& theme = uwf::ui::DesignSystem::instance();
  theme.setLight(!args.contains(QStringLiteral("--dark")));
  application.setStyle(new uwf::ui::DesignStyle());
  application.setPalette(theme.palette());
  application.setFont(uwf::ui::DesignSystem::defaultFont());
  std::unique_ptr<QWidget> window(gallery());
  window->show();
  const auto output = args.indexOf(QStringLiteral("--output"));
  if (output >= 0 && output + 1 < args.size())
    QTimer::singleShot(250, &application, [&] {
      const QString path = args[output + 1];
      QDir().mkpath(QFileInfo(path).absolutePath());
      const bool saved = window->grab().toImage().scaled(window->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation).save(path);
      application.exit(saved ? 0 : 1);
    });
  return application.exec();
}
