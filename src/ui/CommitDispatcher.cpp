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
#include "CommitDispatcher.h"

#include <QDialog>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QList>
#include <QProgressDialog>
#include <QStringList>
#include <QTimer>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "../util/DriveLetter.h"
#include "../util/Log.h"
#include "../util/PathMatch.h"
#include "../util/RegistryKey.h"
#include "../uwf/api/Types.h"
#include "../uwf/wmi/WmiResult.h"
#include "CommitBatch.h"
#include "Dialogs.h"
#include "I18n.h"
#include "UiUtil.h"

namespace uwf::ui {

namespace {

using uwf::ui::dialogs::confirmCommit;
using uwf::ui::dialogs::warning;

// 在作用域内暂停一个 QTimer，离开作用域时恢复（仅当它原本就在运行）。给 commit
// 这类内部会 processEvents 的操作用——防止占用刷新定时器在 WMI 写入半途触发、
// 对同一个 m_writeSession 发起重入调用（窗口模态拦不住 QTimer 超时）。
class ScopedTimerPause {
 public:
  explicit ScopedTimerPause(QTimer* timer) : m_timer(timer), m_wasActive(timer && timer->isActive()) {
    if (m_wasActive) m_timer->stop();
  }
  ~ScopedTimerPause() {
    if (m_wasActive && m_timer) m_timer->start();
  }
  ScopedTimerPause(const ScopedTimerPause&) = delete;
  ScopedTimerPause& operator=(const ScopedTimerPause&) = delete;

 private:
  QTimer* m_timer;
  bool m_wasActive;
};

// 一次注册表递归提交 / 删除批处理里的单个目标。
struct RegCommitTarget {
  std::string key;        // 归一化后的长写键路径
  std::string valueName;  // 值名；提交时空串 = (Default) 值，删除时空串 = 该键本身
  QString display;        // 报告对话框 "路径" 列的展示串
};

std::optional<std::vector<std::string>> scanRegistryKeyTreeWithProgress(QWidget* parent, const QString& title, const std::string& key) {
  constexpr int kMinVisibleMs = 1000;
  std::atomic_bool canceled{false};
  std::atomic_bool done{false};
  std::atomic<std::uint64_t> scanned{0};
  std::vector<std::string> result;
  const auto shownAt = std::chrono::steady_clock::now();
  bool closeQueued = false;

  QProgressDialog progress(I18n::tr("Scanning registry keys…"), I18n::tr("Cancel"), 0, 0, parent);
  progress.setWindowTitle(title);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  progress.setAutoClose(false);
  progress.setAutoReset(false);
  QObject::connect(&progress, &QProgressDialog::canceled, &progress, [&canceled] { canceled.store(true); });

  std::thread worker([&] {
    std::vector<std::string> local;
    if (regkey::collectKeyTree(key, canceled, scanned, local)) result = std::move(local);
    done.store(true);
  });

  QTimer poll(&progress);
  poll.setInterval(100);
  QObject::connect(&poll, &QTimer::timeout, &progress, [&] {
    if (done.load()) {
      if (!closeQueued) {
        closeQueued = true;
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - shownAt).count();
        const int delayMs = static_cast<int>(std::max<std::int64_t>(0, static_cast<std::int64_t>(kMinVisibleMs) - elapsedMs));
        QTimer::singleShot(delayMs, &progress, &QProgressDialog::accept);
      }
      return;
    }
    progress.setLabelText(I18n::tr("Scanning registry keys…\n%1 key(s) found").arg(QString::number(static_cast<qulonglong>(scanned.load()))));
  });
  poll.start();

  const int rc = progress.exec();
  if (rc != QDialog::Accepted) canceled.store(true);
  if (worker.joinable()) worker.join();

  if (canceled.load()) return std::nullopt;
  return result;
}

}  // namespace

CommitDispatcher::CommitDispatcher(WmiSession& session, const core::UwfSnapshot& snapshot, QTimer* usageTimer, QWidget* parent)
    : m_session(session), m_snapshot(snapshot), m_usageTimer(usageTimer), m_parent(parent), m_volume(m_session), m_registry(m_session) {}

void CommitDispatcher::commitFilePath(const QString& path) {
  if (path.isEmpty()) return;

  // 多文件 commit 的 QProgressDialog::setValue 会 processEvents；占用刷新定时器
  // 不受窗口模态约束，可能在 commit 半途触发、对同一个 m_writeSession 发起重入
  // WMI 调用。整段 commit 暂停该定时器，离开作用域自动恢复。
  const ScopedTimerPause usagePause(m_usageTimer);

  // 标题 / heading 提前算出来：用户面前的前置校验（排除列表、空目录等）失败时
  // 复用同一个 confirmCommit 版式，"继续"置灰、原因塞进警示区——和成功路径走
  // 一致的视觉语言，不再用一个单独的 warning() 弹窗打断。
  const QFileInfo fi(path);
  const bool isDir = fi.isDir();
  const QString title = I18n::tr("Commit to disk");
  const QString heading = isDir ? I18n::tr("Commit this folder's overlay changes to disk") : I18n::tr("Commit this file's overlay changes to disk");

  // 从路径解析盘符，定位到对应的 next-session VolumeRow。
  const QString dl = extractDriveLetter(path);
  if (dl.isEmpty()) {
    warning(m_parent, I18n::tr("Commit failed"), I18n::tr("The path has no drive letter; cannot identify the target volume."));
    return;
  }

  std::string err;
  auto volumes = m_volume.readAll(&err);
  if (!err.empty()) {
    warning(m_parent, I18n::tr("Commit failed"), I18n::tr("Failed to read volume information: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto dlStd = dl.toStdString();
  const auto* row = api::findBySession(volumes, /*wantCurrent=*/true, [&](const api::VolumeRow& v) { return v.driveLetter == dlStd; });
  if (!row) {
    warning(m_parent, I18n::tr("Commit failed"), I18n::tr("No current-session record found for volume %1.").arg(dl));
    return;
  }

  // 排除列表用 volumeName (Win32_Volume.DeviceID) 作键，按当前会话的运行态判断。
  if (auto it = m_snapshot.current.fileExclusions.find(row->volumeName); it != m_snapshot.current.fileExclusions.end()) {
    const std::string hit = findCoveringExclusion(it->second, path.toStdString());
    if (!hit.empty()) {
      confirmCommit(m_parent, title, heading, path, I18n::tr("This path is in the file exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)),
                    /*allowContinue=*/false);
      return;
    }
  }

  // UWF_Volume.CommitFile 只认单个文件条目；给目录会返回 WBEM_E_NOT_FOUND。
  // 所以目录提交 = 递归遍历目录下所有文件挨个 commit。NoSymLinks 跳过重解析点
  // （junction / mount point / 符号链接）——Win 上 QFileInfo::isSymLink 对全部
  // 重解析点返回 true，QDirIterator 据此既不列出也不递归进入，避免跨卷漫游和
  // 自引用 junction 形成的死循环。
  QStringList targets;
  if (isDir) {
    QDirIterator it(path, QDir::Files | QDir::NoSymLinks | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) targets << QDir::toNativeSeparators(it.next());
  } else {
    targets << path;
  }

  if (isDir && targets.isEmpty()) {
    confirmCommit(m_parent, title, heading, path, I18n::tr("No files were found under %1.").arg(path), /*allowContinue=*/false);
    return;
  }

  const QString detail = isDir ? I18n::tr("%1 files in this folder and all its subfolders will be committed.").arg(targets.size()) : QString();
  if (!confirmCommit(m_parent, title, heading, path, detail)) return;

  runCommitBatch(
      m_parent, I18n::tr("Commit to disk"), targets, [](const QString& f) { return f; },
      [&](const QString& f) {
        const auto res = m_volume.commitFile(*row, f.toStdString());
        if (!res.detail.empty()) {
          const char* kind = commitOutcome(res) == CommitOutcome::Skipped ? "skipped" : "failed";
          UWF_LOG_W("commit") << std::format("CommitFile {}: file={} hr=0x{:08x} rv={} detail={}", kind, f.toStdString(), static_cast<uint32_t>(res.hresult),
                                             res.returnValue, res.detail);
        }
        return res;
      });
}

void CommitDispatcher::commitFileDeletionPath(const QString& path) {
  if (path.isEmpty()) return;

  // 多目标删除会弹 QProgressDialog（setValue 内部 processEvents）——和 commitFilePath
  // 同理，整段暂停占用刷新定时器，防止半途重入 m_writeSession。
  const ScopedTimerPause usagePause(m_usageTimer);

  // 标题 / heading 提前算（fi.isDir() 在路径不存在时返回 false，正好作为"按文件
  // 删除"的默认 heading）。用户面前的前置校验失败统一走 confirmCommit + 灰按钮。
  const QFileInfo fi(path);
  const bool isDir = fi.isDir();
  const QString title = I18n::tr("Delete and commit");
  const QString heading =
      isDir ? I18n::tr("Delete this folder and its contents, and commit the deletions to disk") : I18n::tr("Delete this file, and commit the deletion to disk");

  const QString dl = extractDriveLetter(path);
  if (dl.isEmpty()) {
    warning(m_parent, I18n::tr("Commit file deletion failed"), I18n::tr("The path has no drive letter; cannot identify the target volume."));
    return;
  }

  // 核心校验：CommitFileDeletion 由方法自身执行删除，目标（文件或目录）必须**仍
  // 存在**。不存在就没有可删的东西——走灰按钮对话框告知。
  if (!fi.exists()) {
    confirmCommit(m_parent, title, heading, path, I18n::tr("This path does not exist, so there is nothing to delete."), /*allowContinue=*/false);
    return;
  }

  std::string err;
  const auto volumes = m_volume.readAll(&err);
  if (!err.empty()) {
    warning(m_parent, I18n::tr("Commit file deletion failed"), I18n::tr("Failed to read volume information: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto dlStd = dl.toStdString();
  const auto* row = api::findBySession(volumes, /*wantCurrent=*/true, [&](const api::VolumeRow& v) { return v.driveLetter == dlStd; });
  if (!row) {
    warning(m_parent, I18n::tr("Commit file deletion failed"), I18n::tr("No current-session record found for volume %1.").arg(dl));
    return;
  }

  // 落在文件排除列表里的路径，UWF 不在覆盖层维护，提交删除无意义。
  if (auto it = m_snapshot.current.fileExclusions.find(row->volumeName); it != m_snapshot.current.fileExclusions.end()) {
    const std::string hit = findCoveringExclusion(it->second, path.toStdString());
    if (!hit.empty()) {
      confirmCommit(m_parent, title, heading, path, I18n::tr("This path is in the file exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)),
                    /*allowContinue=*/false);
      return;
    }
  }

  // CommitFileDeletion 不接受非空目录、也不递归——目录删除 = 把子文件、子目录、
  // 目录本身按"最深的先删"收齐，逐个调用；删到某目录时其内容已清空。NoSymLinks
  // 见 commitFilePath 的同名注释——跳过 reparse point 避免跨卷漫游。
  QStringList targets;
  int fileCount = 0;
  int subdirCount = 0;
  if (isDir) {
    QDirIterator fileIt(path, QDir::Files | QDir::NoSymLinks | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (fileIt.hasNext()) targets << QDir::toNativeSeparators(fileIt.next());
    fileCount = static_cast<int>(targets.size());
    QDirIterator dirIt(path, QDir::Dirs | QDir::NoSymLinks | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (dirIt.hasNext()) targets << QDir::toNativeSeparators(dirIt.next());
    subdirCount = static_cast<int>(targets.size()) - fileCount;
    targets << QDir::toNativeSeparators(path);  // 目录本身——排序后最浅、最后删
    std::sort(targets.begin(), targets.end(), [](const QString& a, const QString& b) { return a.count('\\') > b.count('\\'); });
  } else {
    targets << path;
  }

  const QString detail = isDir ? I18n::tr("%1 files and %2 subfolders will be deleted.").arg(fileCount).arg(subdirCount) : QString();
  if (!confirmCommit(m_parent, title, heading, path, detail)) return;

  runCommitBatch(
      m_parent, title, targets, [](const QString& f) { return f; },
      [&](const QString& f) {
        const auto res = m_volume.commitFileDeletion(*row, f.toStdString());
        if (!res.detail.empty()) {
          const char* kind = commitOutcome(res) == CommitOutcome::Skipped ? "skipped" : "failed";
          UWF_LOG_W("commit") << std::format("CommitFileDeletion {}: file={} hr=0x{:08x} rv={} detail={}", kind, f.toStdString(),
                                             static_cast<uint32_t>(res.hresult), res.returnValue, res.detail);
        }
        return res;
      },
      [](const QString& f) { return QFileInfo::exists(f); });
}

void CommitDispatcher::commitRegistryKey(const QString& key, const QString& valueName) {
  if (key.isEmpty()) return;

  // 多目标提交会弹 QProgressDialog（setValue 内部 processEvents）——暂停占用刷新
  // 定时器，防止半途重入 m_writeSession。
  const ScopedTimerPause usagePause(m_usageTimer);

  // 注册表键先归一成长写 hive（HKLM\… → HKEY_LOCAL_MACHINE\…）：UWF 覆盖层、排除
  // 列表都按长写键路径索引，跳过归一会匹配不到。
  const std::string normKey = regkey::normalize(key.toStdString());
  if (normKey.empty()) return;
  const QString keyText = QString::fromStdString(normKey);

  // valueName 空 = 整键递归（picker 值表无选中）；非空 = 单个命名值。
  // (Default) 已在 picker 层禁选，所以单值路径下 valueName 必然是真实命名值，
  // 不会出现"key : "的裸冒号尾巴。
  const bool wholeKey = valueName.isEmpty();
  const QString title = I18n::tr("Commit to disk");
  const QString heading = wholeKey ? I18n::tr("Commit this registry key and its whole subtree to disk") : I18n::tr("Commit this registry value to disk");
  const QString target = wholeKey ? keyText : (keyText + " : " + valueName);

  // 注册表排除是全局的，比对当前运行会话即可。覆盖 = 键相等或为其祖先。
  const std::string hit = findCoveringExclusion(m_snapshot.current.registryExclusions, normKey);
  if (!hit.empty()) {
    confirmCommit(m_parent, title, heading, target, I18n::tr("This key is in the registry exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)),
                  /*allowContinue=*/false);
    return;
  }

  // 目标清单：单值 = 只提交那一个值；整键 = 递归展开整棵键子树的每一个值——
  // CommitRegistry 只能逐值提交，"提交整键"由这里展开成逐值调用。
  QList<RegCommitTarget> targets;
  if (!wholeKey) {
    if (!regkey::valueExists(normKey, valueName.toStdString())) {
      confirmCommit(m_parent, title, heading, target, I18n::tr("This registry value does not exist, so there is nothing to commit."), /*allowContinue=*/false);
      return;
    }
    targets.append({normKey, valueName.toStdString(), target});
  } else {
    if (!regkey::keyExists(normKey)) {
      confirmCommit(m_parent, title, heading, target, I18n::tr("This registry key does not exist, so there is nothing to commit."), /*allowContinue=*/false);
      return;
    }
    // UWF 的 CommitRegistry 是逐值提交——ValueName="" 提交的是键的 (Default)
    // 值，默认值不存在则返回 NOT_FOUND；UWF 没有"提交键本身（不带任何值）"的
    // 能力。这里递归展开成 valueNames 里实际存在的每一个值——valueNames 里没空串
    // （键未设过默认值）就不再硬塞 (k, "")，省一次必然 NOT_FOUND 的 WMI 调用，
    // 也让结果表里不再出现一堆无意义的 Skipped 行。代价是没值的纯结构 key
    // 整个被跳过——CommitRegistry 本来对它就什么都做不了，跳过是对的。
    for (const auto& k : regkey::collectKeyTree(normKey)) {
      const QString kText = QString::fromStdString(k);
      for (const auto& vn : regkey::valueNames(k)) {
        targets.append({k, vn, vn.empty() ? (kText + " : (Default)") : (kText + " : " + QString::fromStdString(vn))});
      }
    }
  }
  const int total = static_cast<int>(targets.size());

  const QString detail = wholeKey ? I18n::tr("%1 values in this key and all its subkeys will be committed.").arg(total) : QString();
  if (!confirmCommit(m_parent, title, heading, target, detail)) return;

  std::string err;
  auto filters = m_registry.readAll(&err);
  if (!err.empty()) {
    warning(m_parent, I18n::tr("Commit failed"), I18n::tr("Failed to read registry filter: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto* row = api::findBySession(filters, /*wantCurrent=*/true);
  if (!row) {
    warning(m_parent, I18n::tr("Commit failed"), I18n::tr("No current-session registry filter record found."));
    return;
  }

  runCommitBatch(
      m_parent, I18n::tr("Commit to disk"), targets, [](const RegCommitTarget& t) { return t.display; },
      [&](const RegCommitTarget& t) {
        const auto res = m_registry.commitRegistry(*row, t.key, t.valueName);
        if (!res.detail.empty()) {
          const char* kind = commitOutcome(res) == CommitOutcome::Skipped ? "skipped" : "failed";
          UWF_LOG_W("commit") << std::format("CommitRegistry {}: key={} value={} hr=0x{:08x} rv={} detail={}", kind, t.key, t.valueName,
                                             static_cast<uint32_t>(res.hresult), res.returnValue, res.detail);
        }
        return res;
      });
}

void CommitDispatcher::commitRegistryDeletionKey(const QString& key, const QString& valueName) {
  if (key.isEmpty()) return;

  const ScopedTimerPause usagePause(m_usageTimer);

  // 同 commitRegistryKey：先归一成长写 hive。
  const std::string normKey = regkey::normalize(key.toStdString());
  if (normKey.empty()) return;
  const QString keyText = QString::fromStdString(normKey);

  // valueName 空 = 整键递归；非空 = 单个命名值。(Default) 已在 picker 层禁选。
  const bool wholeKey = valueName.isEmpty();
  const QString title = I18n::tr("Delete and commit");
  const QString heading = wholeKey ? I18n::tr("Delete this registry key and its whole subtree, and commit the deletions to disk")
                                   : I18n::tr("Delete this registry value, and commit the deletion to disk");
  const QString target = wholeKey ? keyText : (keyText + " : " + valueName);

  const std::string hit = findCoveringExclusion(m_snapshot.current.registryExclusions, normKey);
  if (!hit.empty()) {
    confirmCommit(m_parent, title, heading, target, I18n::tr("This key is in the registry exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)),
                  /*allowContinue=*/false);
    return;
  }

  // 目标清单：单值 = 只删那一个值；整键 = 递归整棵键子树。CommitRegistryDeletion
  // 由方法自身执行删除，目标必须仍存在；它不递归——CommitRegistryDeletion(key,"")
  // 只能删叶子键，故按 collectKeyTree 的后序（最深子键在前）逐个删。
  QList<RegCommitTarget> targets;
  QStringList previewKeys;
  if (!wholeKey) {
    if (!regkey::valueExists(normKey, valueName.toStdString())) {
      confirmCommit(m_parent, title, heading, target, I18n::tr("This registry value does not exist, so there is nothing to delete."), /*allowContinue=*/false);
      return;
    }
    targets.append({normKey, valueName.toStdString(), target});
  } else {
    if (!regkey::keyExists(normKey)) {
      confirmCommit(m_parent, title, heading, target, I18n::tr("This registry key does not exist, so there is nothing to delete."), /*allowContinue=*/false);
      return;
    }
    auto keys = scanRegistryKeyTreeWithProgress(m_parent, title, normKey);
    if (!keys) return;
    for (const auto& k : *keys) {
      const QString display = QString::fromStdString(k);
      targets.append({k, std::string{}, display});
      previewKeys << display;
    }
  }
  const int total = static_cast<int>(targets.size());

  const QString detail = wholeKey ? I18n::tr("%1 keys and all values they contain will be recursively deleted.").arg(total) : QString();
  if (wholeKey) {
    if (!confirmCommit(m_parent, title, heading, target, detail, previewKeys)) return;
  } else if (!confirmCommit(m_parent, title, heading, target, detail)) {
    return;
  }

  std::string err;
  auto filters = m_registry.readAll(&err);
  if (!err.empty()) {
    warning(m_parent, I18n::tr("Commit failed"), I18n::tr("Failed to read registry filter: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto* row = api::findBySession(filters, /*wantCurrent=*/true);
  if (!row) {
    warning(m_parent, I18n::tr("Commit failed"), I18n::tr("No current-session registry filter record found."));
    return;
  }

  runCommitBatch(
      m_parent, I18n::tr("Delete and commit"), targets, [](const RegCommitTarget& t) { return t.display; },
      [&](const RegCommitTarget& t) {
        const auto res = m_registry.commitRegistryDeletion(*row, t.key, t.valueName);
        if (!res.detail.empty()) {
          const char* kind = commitOutcome(res) == CommitOutcome::Skipped ? "skipped" : "failed";
          UWF_LOG_W("commit") << std::format("CommitRegistryDeletion {}: key={} value={} hr=0x{:08x} rv={} detail={}", kind, t.key, t.valueName,
                                             static_cast<uint32_t>(res.hresult), res.returnValue, res.detail);
        }
        return res;
      },
      [](const RegCommitTarget& t) { return t.valueName.empty() ? regkey::keyExists(t.key) : regkey::valueExists(t.key, t.valueName); });
}

}  // namespace uwf::ui
