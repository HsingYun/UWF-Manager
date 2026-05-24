#include "AboutDialog.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "I18n.h"
#include "ThemeManager.h"
#include "uwf_version.h"

namespace uwf::ui {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
  // 改用普通 QDialog 而非 QMessageBox：QMessageBox 内部 label 走另一条
  // 字体路径，全局 app.setFont() 设置的 hinting / styleStrategy 不会传播过去，
  // 中文渲染会"糊"。QDialog + QLabel 跟其它对话框一样能继承 app font。
  setWindowTitle(I18n::tr("About UWF Manager"));
  setMinimumWidth(520);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(20, 16, 20, 12);
  layout->setSpacing(10);

  // 头部：左侧软件 logo，右侧标题 + 版本号竖排。
  auto* header = new QHBoxLayout();
  header->setSpacing(14);

  // app.svg 是矢量 logo（同时用作窗口 / 托盘图标），渲染成 64×64 放在左侧。
  auto* logo = new QLabel(this);
  logo->setPixmap(QIcon(QStringLiteral(":/icons/app.svg")).pixmap(64, 64));
  logo->setFixedSize(64, 64);
  header->addWidget(logo);

  auto* titleBox = new QVBoxLayout();
  titleBox->setSpacing(2);

  // 标题：手动用 QLabel + 大字号 + bold（YaHei 真实字重 700）替代 <h3>，
  // 避免 QTextDocument 的 <h3> 默认合成粗体（同样的 hinting 问题）。
  auto* title = new QLabel(I18n::tr("Unified Write Filter (UWF) Manager"), this);
  QFont titleFont = title->font();
  titleFont.setBold(true);
  titleFont.setPointSizeF(titleFont.pointSizeF() + 3);
  title->setFont(titleFont);
  title->setTextInteractionFlags(Qt::TextSelectableByMouse);
  titleBox->addWidget(title);

  // 版本号紧贴标题下方、弱化显示。UWF_VER_STRING 由 cmake/GitVersion.cmake
  // 在构建期注入 git 短哈希（无 git 仓库时回退为 "1.0.0.0"）。用内联 color
  // 走富文本，理由同 body：避开 QSS / palette 对 QLabel 文字色的干扰。
  auto* version = new QLabel(this);
  version->setTextFormat(Qt::RichText);
  version->setTextInteractionFlags(Qt::TextSelectableByMouse);
  // 版本号与 "powered by Qt …" 都不进 tr：版本号无需翻译，"powered by Qt" 是
  // Qt 官方品牌标语。
  const QString verText =
      QStringLiteral("Version %1").arg(QString::fromLatin1(UWF_VER_STRING)) + QStringLiteral(" · powered by Qt %1").arg(QString::fromLatin1(qVersion()));
  version->setText(QStringLiteral("<span style=\"color:%1\">%2</span>").arg(ThemeManager::instance().color(Sem::FgMuted).name(), verText));
  titleBox->addWidget(version);

  header->addLayout(titleBox, 1);
  layout->addLayout(header);

  auto* body = new QLabel(this);
  body->setTextFormat(Qt::RichText);
  body->setTextInteractionFlags(Qt::TextBrowserInteraction);
  body->setOpenExternalLinks(true);
  body->setWordWrap(true);
  // 之前试过 QPalette::Link 设主题 accent，但 Qt 在 light 主题下 QLabel 的
  // 富文本链接颜色经常被 QTextDocument 的默认值覆盖（看着仍是无对比度的浅蓝）。
  // 改用 inline `style="color:..."` 注到每个 <a> 标签，绕开 palette / QSS 的所有
  // 干扰。<code> 标签去掉的原因同上：会切到 Courier New 的中文 fallback 渲染糊。
  QString html = I18n::tr(
                     "<p>A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after "
                     "the next reboot.</p>"
                     "<p>Source code: <a href=\"%3\">%3</a></p>"
                     "<p>Copyright © 2026 HsingYun &lt;<a href=\"mailto:%1\">%1</a>&gt;</p>"
                     "<p>This program is released under the <a href=\"%2\">GNU General Public License v3.0</a>; the full license text is included in the "
                     "LICENSE file shipped with this program.</p>"
                     "<p>This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided \"as is\", "
                     "without any warranty.</p>")
                     .arg("iakext@gmail.com", "https://www.gnu.org/licenses/gpl-3.0.html", "https://github.com/HsingYun/UWF-Manager");
  const QString linkColor = ThemeManager::instance().color(Sem::Accent).name();
  html.replace(QStringLiteral("<a "), QStringLiteral("<a style=\"color:%1\" ").arg(linkColor));
  body->setText(html);
  // body 吸收纵向拉伸：对话框拉高时多余空间归 body，header（logo + 标题 + 版本）保持紧凑、不被拉开。
  layout->addWidget(body, 1);

  // UWF 行为提示：说明本程序只是 UWF 的图形配置前端（写入过滤由系统 UWF 完成、且依赖
  // 系统已装并启用 UWF），以及 UWF 首次启用会对系统做的更改。单独一个 label，顶部描边与正文分块。
  auto* uwfNote = new QLabel(this);
  uwfNote->setTextFormat(Qt::RichText);
  uwfNote->setTextInteractionFlags(Qt::TextSelectableByMouse);
  uwfNote->setWordWrap(true);
  uwfNote->setText(
      I18n::tr("<p><b>This program depends on the Windows Unified Write Filter (UWF).</b> UWF Manager does not perform write "
               "filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program "
               "only configures and manages UWF, and requires UWF to be installed and enabled on the system.</p>"
               "<p>When UWF is first enabled on a device, it makes the following changes to the system to improve UWF "
               "performance:</p>"
               "<ul>"
               "<li>Paging files are disabled.</li>"
               "<li>System Restore is disabled.</li>"
               "<li>SuperFetch is disabled.</li>"
               "<li>The file indexing service is turned off.</li>"
               "<li>The defragmentation service is turned off.</li>"
               "<li>Fast boot is disabled.</li>"
               "<li>The BCD setting bootstatuspolicy is set to ignoreallfailures.</li>"
               "</ul>"
               "<p>After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved "
               "to an unprotected volume and paging re-enabled.</p>"));
  uwfNote->setStyleSheet(QStringLiteral("QLabel { border-top: 1px solid %1; padding-top: 10px; }").arg(ThemeManager::instance().color(Sem::FgMuted).name()));
  layout->addWidget(uwfNote);

  auto* btns = new QDialogButtonBox(this);
  auto* closeBtn = btns->addButton(I18n::tr("Close"), QDialogButtonBox::AcceptRole);
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  layout->addWidget(btns);
}

}  // namespace uwf::ui
