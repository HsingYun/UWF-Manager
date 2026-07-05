/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "UiUtil.h"

#include <shlobj.h>
#include <windows.h>

#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QVariant>
#include <QWidget>
#include <string>

#include "../util/DriveLetter.h"
#include "../util/Log.h"
#include "I18n.h"
#include "ThemeManager.h"

namespace uwf::ui {

void revealInExplorer(const QString& path) {
  const QString abs = QDir::toNativeSeparators(path);

  // 路径存在：用 Shell API 打开父目录并高亮目标，不走 explorer.exe /select 的命令
  // 行形式——后者在路径含空格时引号位置会让 explorer 解析失败、回退到默认目录。
  if (QFileInfo::exists(abs)) {
    const std::wstring wide = abs.toStdWString();
    if (PIDLIST_ABSOLUTE pidl = ILCreateFromPathW(wide.c_str())) {
      // cidl=0 + apidl=NULL：选中 pidl 自身（文件选文件、目录选目录，落在各自父目录里）。
      (void)SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
      ILFree(pidl);
      return;
    }
  }

  // 路径不存在或拿不到 PIDL：沿父目录回溯到第一个真实存在的文件夹直接打开（不选中）。
  QString folder = QFileInfo(abs).absolutePath();
  if (folder.isEmpty()) folder = abs;
  while (!folder.isEmpty() && !QFileInfo::exists(folder)) {
    const QString up = QFileInfo(folder).absolutePath();
    if (up == folder) break;
    folder = up;
  }
  if (!folder.isEmpty() && QFileInfo::exists(folder)) {
    const std::wstring wf = QDir::toNativeSeparators(folder).toStdWString();
    // 走 explorer.exe + 文件夹参数，避免 ShellExecute "open" 在某些机器上落到默认
    // 文件管理器替身的问题。
    ShellExecuteW(nullptr, L"open", L"explorer.exe", wf.c_str(), nullptr, SW_SHOWNORMAL);
  }
}

QString extractDriveLetter(const QString& path) {
  std::string err;
  const std::string dl = drive::fromPath(path.toStdString(), &err);
  if (dl.empty() && !err.empty()) UWF_LOG_W("ui") << "extractDriveLetter: " << err;
  return QString::fromStdString(dl);
}

void setComboValue(QComboBox* combo, const QVariant& value) {
  for (int i = 0; i < combo->count(); ++i) {
    if (combo->itemData(i) == value) {
      combo->setCurrentIndex(i);
      return;
    }
  }
}

void markDirty(QWidget* w, bool dirty) {
  w->setProperty("dirty", dirty);
  w->style()->unpolish(w);
  w->style()->polish(w);
}

QString enabledStateLabel(bool enabled) {
  const QString color = ThemeManager::instance().color(enabled ? Sem::AddOk : Sem::Danger).name();
  const QString text = enabled ? I18n::tr("Enabled") : I18n::tr("Disabled");
  return QStringLiteral("<span style=\"color:%1\">%2</span>").arg(color, text.toHtmlEscaped());
}

QWidget* makeSessionChip(const QString& caption, const QString& captionTooltip, QWidget* value) {
  // 两张 chip 装的值控件高度不一（"本次"是文字 QLabel，"下次"是 24px 高的
  // SwitchButton），不统一的话 chip 各自按内容收缩、外框高度对不齐。把值控件
  // 的最小高度一律抬到开关高度（SwitchButton::sizeHint 的 kTrackH+4=24），让两张
  // chip 的内容行等高、外框自然等高。
  constexpr int kChipValueH = 24;
  value->setMinimumHeight(kChipValueH);

  auto* card = new QFrame();
  card->setObjectName("statusChip");
  auto* h = new QHBoxLayout(card);
  h->setContentsMargins(11, 5, 12, 5);  // 上下对称，caption / value 在框内垂直居中
  h->setSpacing(8);
  auto* cap = new QLabel(caption);
  cap->setObjectName("statusChipCaption");
  if (!captionTooltip.isEmpty()) {
    cap->setToolTip(captionTooltip);
    // 也挂到卡片上：hover 标题左右的留白区也能看到说明（值控件区域仍显示值自己
    // 的 tooltip）。
    card->setToolTip(captionTooltip);
  }
  h->addWidget(cap, 0, Qt::AlignVCenter);
  h->addWidget(value, 0, Qt::AlignVCenter);
  return card;
}

}  // namespace uwf::ui
