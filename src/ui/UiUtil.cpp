#include "UiUtil.h"

#include <shlobj.h>
#include <windows.h>

#include <QComboBox>
#include <QDir>
#include <QFileInfo>
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

}  // namespace uwf::ui
