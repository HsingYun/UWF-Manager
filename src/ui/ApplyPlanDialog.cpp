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
#include "ApplyPlanDialog.h"

#include <QClipboard>
#include <QColor>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QDir>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSaveFile>
#include <QSplitter>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <exception>
#include <format>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../core/Config.h"
#include "../util/Log.h"
#include "../uwf/api/UwfmgrCli.h"
#include "../uwf/wmi/WmiException.h"
#include "Dialogs.h"
#include "DiskTab.h"
#include "GlobalStatusPanel.h"
#include "I18n.h"
#include "PendingCollect.h"
#include "ThemeManager.h"

namespace uwf::ui {

namespace {

using uwf::ui::dialogs::confirm;
using uwf::ui::dialogs::information;
using uwf::ui::dialogs::warning;

// 预览并应用对话框用的 QTextEdit 子类：复制（Ctrl+C 或右键菜单）时把
// 注释行（以 "::" 开头）过滤掉，让用户 Ctrl+A 全选 + 复制时只把 uwfmgr
// 命令带到剪贴板，注释只用于显示——粘到 cmd 里直接可执行，不会带噪音。
class CommandTextEdit : public QTextEdit {
 public:
  using QTextEdit::QTextEdit;

 protected:
  void keyPressEvent(QKeyEvent* e) override {
    if (e->matches(QKeySequence::Copy)) {
      copyFiltered();
      return;
    }
    QTextEdit::keyPressEvent(e);
  }

  void contextMenuEvent(QContextMenuEvent* e) override {
    QMenu* menu = createStandardContextMenu();
    const auto actions = menu->actions();
    for (const QAction* act : actions) {
      if (act->shortcut() == QKeySequence::Copy) {
        disconnect(act, nullptr, this, nullptr);
        QObject::connect(act, &QAction::triggered, this, &CommandTextEdit::copyFiltered);
        break;
      }
    }
    menu->exec(e->globalPos());
    delete menu;
  }

 private:
  void copyFiltered() const {
    const auto cur = textCursor();
    if (!cur.hasSelection()) return;
    // selection().toPlainText() 用 \n 做段落分隔，比 selectedText() 的
    // U+2029 PARAGRAPH SEPARATOR 直接好用。
    const QString sel = cur.selection().toPlainText();
    const QStringList lines = sel.split('\n');
    QStringList kept;
    for (const QString& line : lines) {
      if (line.trimmed().startsWith("::")) continue;
      kept << line;
    }
    QGuiApplication::clipboard()->setText(kept.join('\n'));
  }
};

// 配置类写操作（Protect / Unprotect / SetBindByDriveLetter / AddExclusion /
// RemoveExclusion / SetType / SetMaximumSize 等）一律发给 next session 实例。
// SetType / SetMaximumSize 还要求 UWF_Filter.CurrentEnabled==false，否则
// WMI 返回 WBEM_E_INVALID_PARAMETER (0x80041008)。下面 3 处查询都用统一的
// api::findBySession（见 Types.h）。
const api::VolumeRow* findNextVolume(const std::vector<api::VolumeRow>& rows, const std::string& driveLetter) {
  return api::findBySession(rows, api::Session::Next, [&](const api::VolumeRow& v) { return v.driveLetter == driveLetter; });
}

api::OverlayType coreTypeToApi(core::OverlayType t) { return t == core::OverlayType::Disk ? api::OverlayType::Disk : api::OverlayType::RAM; }

struct ApplyMessages {
  QString success;
  QString failure;
};

class ApplyJournal final {
 public:
  enum class StepResult { Confirmed, Failed };

  template <typename Action>
  StepResult execute(const QString& failureContext, Action&& action) {
    try {
      action();
      m_anyWriteConfirmed = true;
      m_reconciliationRequired = true;
      return StepResult::Confirmed;
    } catch (const WmiWriteOutcomeError& error) {
      m_reconciliationRequired = true;
      recordError(failureContext, error);
    } catch (const std::exception& error) {
      recordError(failureContext, error);
    } catch (...) {
      const QString message = failureContext + I18n::tr(": unknown error");
      UWF_LOG_E("apply") << "operation failed: error=" << message.toStdString();
      recordFailure(message);
    }
    return StepResult::Failed;
  }

  template <typename Action>
  void apply(const ApplyMessages& messages, Action&& action) {
    if (execute(messages.failure, std::forward<Action>(action)) == StepResult::Confirmed) recordSuccess(messages.success);
  }

  void recordSuccess(const QString& message) { m_lines.push_back(message.toStdString()); }

  void recordFailure(const QString& message) { m_lines.push_back(message.toStdString()); }

  void recordError(const QString& context, const std::exception& error) {
    UWF_LOG_E("apply") << "operation failed: context=" << context.toStdString() << " error=" << error.what();
    recordFailure(context + ": " + QString::fromUtf8(error.what()));
  }

  void registrationObserved(const api::VolumeRegistrationDisposition disposition) {
    // ensureNextSessionEntry 只在本轮基线没有 next-session 行时调用。无论条目
    // 来自本次写入还是并发创建，都必须重读；只有本进程创建或确认了不确定写，
    // 才算本批次确认成功并显示重启入口。
    m_reconciliationRequired = true;
    if (disposition == api::VolumeRegistrationDisposition::Created || disposition == api::VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite) {
      m_anyWriteConfirmed = true;
    }
  }

  void requireReconciliation() { m_reconciliationRequired = true; }

  [[nodiscard]] bool anyWriteConfirmed() const noexcept { return m_anyWriteConfirmed; }
  [[nodiscard]] bool reconciliationRequired() const noexcept { return m_reconciliationRequired; }
  [[nodiscard]] const std::vector<std::string>& lines() const noexcept { return m_lines; }

 private:
  std::vector<std::string> m_lines;
  bool m_anyWriteConfirmed = false;
  bool m_reconciliationRequired = false;
};

// 把一条已渲染好的 uwfmgr 命令翻成"待变更"段的中文 comment。命令映射的决策
// （哪个字段 → 哪条命令、参数怎么拼）全在 api::renderPendingChanges 里，这里
// 只按 kind 出文案，不再重复那套映射。新增 UwfmgrKind 时这个 switch 少 case 会
// 被 -Werror（-Wswitch）顶出来，强制同步——比之前"显示/导出两份映射静默漂移"
// 安全。a0 = 命令首个参数（类型字符串 / MB / 盘符 / 路径 / 键）。
QString pendingComment(const api::UwfmgrCommand& c) {
  const QString a0 = c.args.empty() ? QString() : QString::fromStdString(c.args[0]);
  switch (c.kind) {
    case api::UwfmgrKind::FilterEnable:
      return I18n::tr("· Filter (global) %1").arg(I18n::tr("Enable"));
    case api::UwfmgrKind::FilterDisable:
      return I18n::tr("· Filter (global) %1").arg(I18n::tr("Disable"));
    case api::UwfmgrKind::OverlaySetType:
      return I18n::tr("· Overlay type → %1").arg(a0);
    case api::UwfmgrKind::OverlaySetSize:
      return I18n::tr("· Overlay maximum size → %1 MB").arg(a0);
    case api::UwfmgrKind::OverlaySetWarningThreshold:
      return I18n::tr("· Overlay warning threshold → %1 MB").arg(a0);
    case api::UwfmgrKind::OverlaySetCriticalThreshold:
      return I18n::tr("· Overlay critical threshold → %1 MB").arg(a0);
    case api::UwfmgrKind::VolumeProtect:
      return I18n::tr("· Volume %1 protection %2").arg(a0, I18n::tr("Enable"));
    case api::UwfmgrKind::VolumeUnprotect:
      return I18n::tr("· Volume %1 protection %2").arg(a0, I18n::tr("Disable"));
    case api::UwfmgrKind::FileAddExclusion:
      return I18n::tr("+ File exclusion  %1").arg(a0);
    case api::UwfmgrKind::FileRemoveExclusion:
      return I18n::tr("− File exclusion  %1").arg(a0);
    case api::UwfmgrKind::RegistryAddExclusion:
      return I18n::tr("+ Registry exclusion  %1").arg(a0);
    case api::UwfmgrKind::RegistryRemoveExclusion:
      return I18n::tr("− Registry exclusion  %1").arg(a0);
    case api::UwfmgrKind::Unknown:
      return {};
  }
  return {};
}

// 当前会话配置段的 comment：同理只翻 kind，但文案是"陈述现状"（Enabled/
// Disabled、无前缀点），且 renderSession 只会产出 add/enable 方向的命令。
QString snapshotComment(const api::UwfmgrCommand& c) {
  const QString a0 = c.args.empty() ? QString() : QString::fromStdString(c.args[0]);
  switch (c.kind) {
    case api::UwfmgrKind::FilterEnable:
      return I18n::tr("Filter (global) %1").arg(I18n::tr("Enabled"));
    case api::UwfmgrKind::FilterDisable:
      return I18n::tr("Filter (global) %1").arg(I18n::tr("Disabled"));
    case api::UwfmgrKind::OverlaySetType:
      return I18n::tr("Overlay type → %1").arg(a0);
    case api::UwfmgrKind::OverlaySetSize:
      return I18n::tr("Overlay maximum size → %1 MB").arg(a0);
    case api::UwfmgrKind::OverlaySetWarningThreshold:
      return I18n::tr("Overlay warning threshold → %1 MB").arg(a0);
    case api::UwfmgrKind::OverlaySetCriticalThreshold:
      return I18n::tr("Overlay critical threshold → %1 MB").arg(a0);
    case api::UwfmgrKind::VolumeProtect:
      return I18n::tr("Volume %1 protection %2").arg(a0, I18n::tr("Enabled"));
    case api::UwfmgrKind::VolumeUnprotect:
      return I18n::tr("Volume %1 protection %2").arg(a0, I18n::tr("Disabled"));
    case api::UwfmgrKind::FileAddExclusion:
    case api::UwfmgrKind::FileRemoveExclusion:
      return I18n::tr("File exclusion %1").arg(a0);
    case api::UwfmgrKind::RegistryAddExclusion:
    case api::UwfmgrKind::RegistryRemoveExclusion:
      return I18n::tr("Registry exclusion %1").arg(a0);
    case api::UwfmgrKind::Unknown:
      return {};
  }
  return {};
}

}  // namespace

ApplyPlanDialog::ApplyPlanDialog(GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs, const core::UwfSnapshot& snapshot,
                                 WmiOperations& writeSession, QWidget* parent)
    : ApplyPlanDialog(global, diskTabs, snapshot, ApplyPlanServices{writeSession, dialogs::systemFileDialogs()}, parent) {}

ApplyPlanDialog::ApplyPlanDialog(GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs, const core::UwfSnapshot& snapshot,
                                 ApplyPlanServices services, QWidget* parent)
    : QDialog(parent),
      m_session(services.wmi),
      m_fileDialogs(services.fileDialogs),
      m_snapshot(snapshot),
      m_filter(m_session),
      m_overlay(m_session),
      m_overlayConfig(m_session),
      m_volume(m_session),
      m_registry(m_session) {
  // 收集待应用的改动到 core::PendingChanges——只采集"和基线不同"的数据。
  // 遍历逻辑与 MainWindow 状态栏计数共用 collectPending（见 PendingCollect）。
  // 至于每个字段映射成哪条 uwfmgr 命令、参数怎么拼，全部交给下面的
  // api::renderPendingChanges（命令映射的唯一真相源，导出按钮走的也是它）。
  m_changes = collectPending(global, diskTabs);

  // ── 待变更命令行 ──────────────────────────────────
  // CLI 可表达的改动全部经 api::renderPendingChanges 渲染，UI 只把 kind 翻成中文
  // comment（pendingComment）。
  for (const auto& c : api::renderPendingChanges(m_changes)) {
    m_changeCmds.push_back({pendingComment(c).toStdString(), api::renderCommand(c)});
  }
  // 以下三类没有 uwfmgr 命令对应，renderPendingChanges 不会输出，补成纯 comment
  // 行（cmd 留空）：覆盖层类型/大小在筛选器开启时改不了的告警、绑定方式切换、
  // 两个注册表持久化开关。
  if (m_changes.setOverlay.touchesOverlayConfig() && m_snapshot.current.filter.enabled) {
    m_changeCmds.push_back(
        {I18n::tr("⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.").toStdString(), ""});
  }
  for (const auto& [dl, byVolumeName] : m_changes.volumeBindByVolumeName) {
    m_changeCmds.push_back({I18n::tr("· Volume %1 bind by → %2 (no CLI equivalent; this program only)")
                                .arg(QString::fromStdString(dl), byVolumeName ? I18n::tr("volume ID") : I18n::tr("drive letter"))
                                .toStdString(),
                            ""});
  }
  if (m_changes.setPersistDomainSecretKey) {
    m_changeCmds.push_back(
        {I18n::tr("· %1 persistence %2")
             .arg(I18n::tr("Domain Secret Key (DomainSecretKey)"), *m_changes.setPersistDomainSecretKey ? I18n::tr("Enable") : I18n::tr("Disable"))
             .toStdString(),
         ""});
  }
  if (m_changes.setPersistTSCAL) {
    m_changeCmds.push_back(
        {I18n::tr("· %1 persistence %2")
             .arg(I18n::tr("Terminal Services Client Access License (TSCAL)"), *m_changes.setPersistTSCAL ? I18n::tr("Enable") : I18n::tr("Disable"))
             .toStdString(),
         ""});
  }

  // ── 当前会话配置 ──────────────────────────────────
  // 基于 current session（现在 UWF 真实在跑的状态，比 next 更直观），同样全程
  // 走 api::renderSession，UI 只翻 comment（snapshotComment）。
  for (const auto& c : api::renderSession(m_snapshot.current)) {
    m_snapshotCmds.push_back({snapshotComment(c).toStdString(), api::renderCommand(c)});
  }

  // ── 拼装 HTML 块 ─────────────────────────────────
  // 用 RichText 渲染：段标题色块、注释灰、命令等宽 + 主前景色。
  // 同时也准备一份 plain-text 形式（带 :: 注释），给"应用结果"段用，因为
  // 那段拼了大量 ✓✘ 行，纯文本更清晰也方便粘贴日志。
  const QString accent = ThemeManager::instance().color(Sem::Accent).name();
  const QColor mutedColor = ThemeManager::instance().color(Sem::FgMuted);
  // 比 muted 再淡一档，给段标题色块底和注释行用——让命令突出，注释退后。
  const QString mutedFaint = QString("rgba(%1,%2,%3,0.55)").arg(mutedColor.red()).arg(mutedColor.green()).arg(mutedColor.blue());
  const QString mutedTint = QString("rgba(%1,%2,%3,0.06)").arg(mutedColor.red()).arg(mutedColor.green()).arg(mutedColor.blue());
  const QString fg = ThemeManager::instance().color(Sem::Fg).name();

  // titleColor 决定标题字色：待变更段用强调色（蓝）让它跳出来，当前配置段
  // 仍用灰；其余样式两段完全一致。
  auto formatBlockHtml = [&](const QString& title, const std::vector<Cmd>& items, const QString& titleColor) {
    QString html;
    // 段标题：淡灰底 + 小字号，让"命令"成为视觉主角。plain-text 形式以
    // ":: " 起头让 CommandTextEdit 复制时跳过。
    html += QString("<div style='background:%1;color:%2;padding:3px 10px;border-radius:3px;font-size:9pt;margin:8px 0 2px 0'>:: %3</div>")
                .arg(mutedTint, titleColor, title.toHtmlEscaped());
    for (const auto& c : items) {
      if (!c.comment.empty()) {
        // 注释行用更淡的灰色 + 小字号，肉眼一眼能区分这是装饰文字而非命令。
        html +=
            QString("<div style='color:%1;font-size:9pt;margin:4px 0 0 12px'>:: %2</div>").arg(mutedFaint, QString::fromStdString(c.comment).toHtmlEscaped());
      }
      if (!c.cmd.empty()) {
        html += QString("<div style='font-family:Consolas,Cascadia Mono,monospace;color:%1;margin:1px 0 4px 12px'>%2</div>")
                    .arg(fg, QString::fromStdString(c.cmd).toHtmlEscaped());
      }
    }
    return html;
  };

  // 同样的内容也提供一份纯文本视图（commit 完成后的"应用结果"用）。
  auto formatBlockPlain = [](const std::string& title, const std::vector<Cmd>& items) {
    std::string out = std::format(":: ==== {} ====\n", title);
    for (const auto& c : items) {
      if (!c.comment.empty()) out += std::format(":: {}\n", c.comment);
      if (!c.cmd.empty()) out += c.cmd + "\n";
    }
    return out;
  };

  auto wrapHtml = [](const QString& body) {
    return QString("<div style=\"font-family:'Segoe UI','Microsoft YaHei UI','Microsoft YaHei',sans-serif\">%1</div>").arg(body);
  };
  // 外层 wrapper 显式声明 font-family，让所有内部 div 继承——不写的话
  // QTextEdit 的 RichText 引擎对中文字符会按字体表自由 fallback，多段中
  // 后面的段标题偶尔会落到宋体，跟其它段视觉上不一致。
  const QString pendingHtml = wrapHtml(formatBlockHtml(I18n::tr("Pending changes (%1)").arg(m_changes.count()), m_changeCmds, accent));
  const QString sessionHtml = wrapHtml(formatBlockHtml(I18n::tr("Current session configuration"), m_snapshotCmds, mutedFaint));

  auto joinLines = [](const std::vector<std::string>& lines) {
    std::string out;
    for (size_t i = 0; i < lines.size(); ++i) {
      if (i) out.push_back('\n');
      out += lines[i];
    }
    return out;
  };

  setWindowTitle(I18n::tr("Review and apply changes"));
  resize(820, 560);
  auto* layout = new QVBoxLayout(this);
  // 不用 <b>：Qt RichText 在 Segoe UI 优先的 fallback 链下，给中文字符合成
  // 粗体（synthetic bold），渲染发虚。改用主题色强调取代粗体。accent 在
  // 上面 formatBlockHtml 那段已经定义。
  const QString warn = ThemeManager::instance().color(Sem::Warn).name();
  auto* info = new QLabel(
      I18n::tr("Below is the full configuration in uwfmgr command form. <span style='color:%1'>Pending changes</span>, if any, are shown in a separate section "
               "first. Click <span style='color:%2'>Apply</span> to write the changes to the system (most take effect after the next reboot).")
          .arg(warn, accent),
      this);
  info->setWordWrap(true);
  info->setTextFormat(Qt::RichText);
  layout->addWidget(info);

  auto configureText = [](CommandTextEdit* edit) {
    edit->setReadOnly(true);
    edit->setObjectName("planText");
    edit->setLineWrapMode(QTextEdit::NoWrap);
  };

  auto* pendingText = new CommandTextEdit(this);
  configureText(pendingText);
  pendingText->setHtml(pendingHtml);

  auto* sessionText = new CommandTextEdit(this);
  configureText(sessionText);
  sessionText->setHtml(sessionHtml);

  auto* splitter = new QSplitter(Qt::Vertical, this);
  splitter->setChildrenCollapsible(false);
  splitter->addWidget(pendingText);
  splitter->addWidget(sessionText);
  splitter->setStretchFactor(0, m_changeCmds.empty() ? 0 : 1);
  splitter->setStretchFactor(1, 1);
  splitter->setSizes(m_changeCmds.empty() ? QList<int>{120, 360} : QList<int>{220, 260});
  layout->addWidget(splitter, 1);

  auto* buttonRow = new QHBoxLayout();
  // 导出按钮：把对话框里展示的所有命令写到一个文件，注释（":: ..." 行）剔掉，
  // 只保留可执行的 uwfmgr 命令。Apply 固定在左侧，与 Import 对话框的主按钮位置一致；
  // 导出 / 关闭留在右侧。
  auto* commitBtn = new QPushButton(I18n::tr("Apply"), this);
  commitBtn->setObjectName("primaryBtn");
  auto* restartBtn = new QPushButton(I18n::tr("Safe restart"), this);
  restartBtn->setObjectName("restartBtn");
  restartBtn->setVisible(false);
  auto* exportBtn = new QPushButton(I18n::tr("Export commands…"), this);
  auto* closeBtn = new QPushButton(I18n::tr("Close"), this);
  buttonRow->addWidget(commitBtn);
  buttonRow->addWidget(restartBtn);
  buttonRow->addStretch(1);
  buttonRow->addWidget(exportBtn);
  buttonRow->addWidget(closeBtn);

  // 无变更时禁用"应用"，只能关闭。
  const bool hasChanges = !m_changeCmds.empty();
  commitBtn->setEnabled(hasChanges);

  // 导出：先 changeCmds 再 snapshotCmds，跳过 cmd 为空的纯注释行（这些在
  // 收集时只是给视觉做提示，没有可执行 CLI 对应）。两段之间空一行隔开。
  connect(exportBtn, &QPushButton::clicked, this, [this]() {
    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    const QString suggested = QString("uwfmgr-commands-%1.txt").arg(stamp);
    const QString path =
        m_fileDialogs.saveFile(this, {I18n::tr("Export commands to file"), QDir::home().filePath(suggested), I18n::tr("Text files (*.txt);;All files (*)")});
    if (path.isEmpty()) return;

    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      warning(this, I18n::tr("Export failed"), I18n::tr("Could not open file for writing: %1").arg(out.errorString()));
      return;
    }
    const auto changeCmds = api::renderPendingChanges(m_changes);
    const auto snapshotCmds = api::renderSession(m_snapshot.current);
    QString content;
    int written = 0;
    {
      QTextStream ts(&content);
      // 走 src/uwf/api/UwfmgrCli 的集中渲染器，"PendingChanges / SessionSnapshot →
      // uwfmgr 命令" 的映射只在那里维护。m_changeCmds / m_snapshotCmds 还要给 UI
      // 展示交错塞中文 comment，导出纯命令时不再从它们抠 cmd 字段抄一遍同样的映射。
      // 注：多盘 pending 改动时，导出按"命令类型"聚合（renderPendingChanges 遍历
      // PendingChanges 的 std::map），与展示按 tab 聚合的顺序略不同；命令集合与
      // 回放效果完全一致。
      for (const auto& c : changeCmds) {
        ts << QString::fromStdString(api::renderCommand(c)) << '\n';
        ++written;
      }
      if (!changeCmds.empty() && !snapshotCmds.empty()) {
        ts << '\n';
      }
      for (const auto& c : snapshotCmds) {
        ts << QString::fromStdString(api::renderCommand(c)) << '\n';
        ++written;
      }
    }
    const QByteArray encoded = content.toUtf8();
    if (out.write(encoded) != encoded.size()) {
      const QString error = out.errorString();
      out.cancelWriting();
      warning(this, I18n::tr("Export failed"), I18n::tr("Could not write file: %1").arg(error));
      return;
    }
    if (!out.commit()) {
      warning(this, I18n::tr("Export failed"), I18n::tr("Could not write file: %1").arg(out.errorString()));
      return;
    }
    information(this, I18n::tr("Export finished"), I18n::tr("Exported %1 commands to:\n%2").arg(written).arg(QDir::toNativeSeparators(path)));
  });

  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
  connect(restartBtn, &QPushButton::clicked, this, &ApplyPlanDialog::safeRestartRequested);
  connect(commitBtn, &QPushButton::clicked, this, [this, pendingText, commitBtn, restartBtn, formatBlockPlain, joinLines]() {
    // 真实写入前再弹一次二次确认，避免误点。
    const QString warn2 = ThemeManager::instance().color(Sem::Warn).name();
    if (!confirm(this, I18n::tr("Confirm apply"),
                 I18n::tr("These changes will be <span style='color:%1'>written to the system</span>; most take effect after the next reboot.<br><br>Continue?")
                     .arg(warn2)))
      return;
    // 一个对话框只应用一次：确认后立即禁用 Apply。对账请求会触发宿主 refresh
    // 重新读快照并改写 m_snapshot（本对话框按引用持有它）；若不禁用，用户再点
    // 一次会把同一批 m_changes 对着已刷新的快照重放。
    commitBtn->setEnabled(false);
    ApplyJournal journal;

    try {
      m_session.ensureConnected();
    } catch (const std::exception& error) {
      journal.recordError(I18n::tr("✘ Failed to connect to the system"), error);
      const std::string body = formatBlockPlain(I18n::tr("Applied changes").toStdString(), m_changeCmds) + "\n:: ==== " + I18n::tr("Result").toStdString() +
                               " ====\n" + joinLines(journal.lines());
      pendingText->setPlainText(QString::fromStdString(body));
      commitBtn->setEnabled(true);
      return;
    }

    // ── UWF_Filter ───────────────────────────────────────
    if (m_changes.setFilterEnabled) {
      try {
        const auto row = m_filter.read();
        const bool enable = *m_changes.setFilterEnabled;
        journal.apply({.success = I18n::tr("✓ Filter: %1").arg(enable ? I18n::tr("Enabled") : I18n::tr("Disabled")),
                       .failure = I18n::tr("✘ Failed to %1 filter").arg(enable ? I18n::tr("enable") : I18n::tr("disable"))},
                      [&] {
                        if (enable)
                          m_filter.enable(row);
                        else
                          m_filter.disable(row);
                      });
      } catch (const std::exception& error) {
        journal.recordError(I18n::tr("✘ Failed to read filter state"), error);
      }
    }

    // ── UWF_Overlay (阈值) ─────────────────────────────────
    // 阈值无 session 区分，也不需要先禁用筛选器。只下发被改的字段。
    if (m_changes.setOverlay.warningThresholdMb || m_changes.setOverlay.criticalThresholdMb) {
      try {
        const auto overlay = m_overlay.read();
        if (const auto v = m_changes.setOverlay.warningThresholdMb) {
          journal.apply({.success = I18n::tr("✓ Overlay warning threshold set to %1 MB").arg(*v), .failure = I18n::tr("✘ Failed to set warning threshold")},
                        [&] { m_overlay.setWarningThreshold(overlay, *v); });
        }
        if (const auto v = m_changes.setOverlay.criticalThresholdMb) {
          journal.apply({.success = I18n::tr("✓ Overlay critical threshold set to %1 MB").arg(*v), .failure = I18n::tr("✘ Failed to set critical threshold")},
                        [&] { m_overlay.setCriticalThreshold(overlay, *v); });
        }
      } catch (const std::exception& error) {
        journal.recordError(I18n::tr("✘ Failed to read overlay state"), error);
      }
    }

    // ── UWF_OverlayConfig (next, 类型 / 最大大小) ─────────
    // 前提：UWF_Filter.CurrentEnabled 必须为 false，否则 WMI 直接拒绝。
    if (m_changes.setOverlay.touchesOverlayConfig()) {
      if (m_snapshot.current.filter.enabled) {
        journal.recordFailure(I18n::tr("✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first."));
      } else {
        try {
          const auto next = m_overlayConfig.read(api::Session::Next);
          if (const auto t = m_changes.setOverlay.type) {
            const char* tStr = *t == core::OverlayType::RAM ? "RAM" : "Disk";
            journal.apply({.success = I18n::tr("✓ Overlay type set to %1").arg(tStr), .failure = I18n::tr("✘ Failed to set overlay type")},
                          [&] { m_overlayConfig.setType(next, coreTypeToApi(*t)); });
          }
          if (const auto v = m_changes.setOverlay.maximumSizeMb) {
            // 基于磁盘的覆盖层要求最大大小至少 1024 MB。type 未在本次 delta 中
            // 改动时，沿用 next 会话的基线类型判断。
            const auto effType = m_changes.setOverlay.type.value_or(m_snapshot.next.overlay.type);
            if (effType == core::OverlayType::Disk && *v < config::kDiskOverlayMinSizeMb) {
              journal.recordFailure(I18n::tr("✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.").arg(config::kDiskOverlayMinSizeMb));
            } else {
              journal.apply({.success = I18n::tr("✓ Overlay maximum size set to %1 MB").arg(*v), .failure = I18n::tr("✘ Failed to set maximum size")},
                            [&] { m_overlayConfig.setMaximumSize(next, *v); });
            }
          }
        } catch (const std::exception& error) {
          journal.recordError(I18n::tr("✘ Failed to read overlay configuration"), error);
        }
      }
    }

    // ── UWF_Volume ───────────────────────────────────────
    if (!m_changes.volumeProtect.empty() || !m_changes.volumeBindByVolumeName.empty() || !m_changes.addFileExclusions.empty() ||
        !m_changes.removeFileExclusions.empty()) {
      try {
        auto volumes = m_volume.readAll();
        std::set<std::string> registrationFailures;
        // 找 next session row；找不到就让 ensureNextSessionEntry 从同卷的
        // current session 行复制 VolumeName 创建一份 next session 实例。
        // 返回 by value，同时把新 row append 到 volumes 让后续的 caller 也
        // 能命中（避免对同一卷的多个 pending 改动重复触发 PutInstance）。
        // [&] 同步辅助与 journal / volumes 同一栈帧；clazy 在每个
        // captured-by-ref 变量的使用行报警，因此逐行抑制。
        auto getOrCreateNextVolume = [&](const std::string& dl) -> std::optional<api::VolumeRow> {
          if (const auto* hit = findNextVolume(volumes, dl)) return *hit;  // clazy:exclude=lambda-in-connect
          // 同一批次绝不隐式重放失败或未确认的注册。该卷后续操作统一跳过，
          // 等宿主完成权威对账后再由用户明确发起新批次。
          if (registrationFailures.contains(dl)) return std::nullopt;  // clazy:exclude=lambda-in-connect
          try {
            auto registration = m_volume.ensureNextSessionEntry(dl);
            journal.registrationObserved(registration.disposition);  // clazy:exclude=lambda-in-connect
            volumes.push_back(registration.row);                     // clazy:exclude=lambda-in-connect
            return registration.row;
          } catch (const WmiWriteOutcomeError& error) {
            // PutInstance 成功、不确定，或遇到并发 AlreadyExists 后，确认读取
            // 失败都意味着外层快照不再足以继续写；必须对账。
            journal.requireReconciliation();  // clazy:exclude=lambda-in-connect
            registrationFailures.insert(dl);  // clazy:exclude=lambda-in-connect
            journal.recordError(I18n::tr("✘ Volume %1: failed to register with UWF").arg(QString::fromStdString(dl)),
                                error);  // clazy:exclude=lambda-in-connect
          } catch (const std::exception& error) {
            registrationFailures.insert(dl);  // clazy:exclude=lambda-in-connect
            journal.recordError(I18n::tr("✘ Volume %1: failed to register with UWF").arg(QString::fromStdString(dl)),
                                error);  // clazy:exclude=lambda-in-connect
          }
          return std::nullopt;
        };

        for (const auto& [dl, wantProtect] : m_changes.volumeProtect) {
          auto v = getOrCreateNextVolume(dl);
          if (!v) continue;
          journal.apply(
              {.success = I18n::tr("✓ Volume %1 protection: %2").arg(QString::fromStdString(dl), wantProtect ? I18n::tr("Enabled") : I18n::tr("Disabled")),
               .failure =
                   I18n::tr("✘ Failed to %1 protection on volume %2").arg(wantProtect ? I18n::tr("enable") : I18n::tr("disable"), QString::fromStdString(dl))},
              [&] {
                if (wantProtect)
                  m_volume.protectVolume(*v);
                else
                  m_volume.unprotect(*v);
              });
        }

        for (const auto& [dl, byVolumeName] : m_changes.volumeBindByVolumeName) {
          auto v = getOrCreateNextVolume(dl);
          if (!v) continue;
          const auto binding = byVolumeName ? api::VolumeBinding::VolumeName : api::VolumeBinding::DriveLetter;
          journal.apply(
              {.success = I18n::tr("✓ Volume %1 bind by: %2").arg(QString::fromStdString(dl), byVolumeName ? I18n::tr("volume ID") : I18n::tr("drive letter")),
               .failure = I18n::tr("✘ Failed to set binding for volume %1").arg(QString::fromStdString(dl))},
              [&] { m_volume.setBinding(*v, binding); });
        }

        for (const auto& [dl, paths] : m_changes.addFileExclusions) {
          if (paths.empty()) continue;
          auto v = getOrCreateNextVolume(dl);
          if (!v) continue;
          for (const auto& path : paths) {
            journal.apply({.success = I18n::tr("✓ Volume %1 added file exclusion: %2").arg(QString::fromStdString(dl), QString::fromStdString(path)),
                           .failure = I18n::tr("✘ Volume %1 failed to add file exclusion %2").arg(QString::fromStdString(dl), QString::fromStdString(path))},
                          [&] { m_volume.addExclusion(*v, path); });
          }
        }
        for (const auto& [dl, paths] : m_changes.removeFileExclusions) {
          if (paths.empty()) continue;
          // remove 分支不能走 getOrCreateNextVolume：如果 next-session 行
          // 还不存在，那这个卷在 next session 里压根没有任何排除项，本来
          // 就没东西可删；不应该为了"删一条不存在的排除"而触发
          // PutInstance 把卷无故注册进 UWF。直接跳过即可。
          const auto* v = findNextVolume(volumes, dl);
          if (!v) {
            // 本轮重新读取已确认该 next-session 实例不存在，所有待删路径都
            // 已经收敛到目标状态。无需制造实例再删除，但必须刷新宿主快照，
            // 否则旧 pending 会一直留在 UI 中。
            journal.requireReconciliation();
            for (const auto& path : paths) {
              journal.recordSuccess(I18n::tr("✓ Volume %1 file exclusion already absent: %2").arg(QString::fromStdString(dl), QString::fromStdString(path)));
            }
            continue;
          }
          for (const auto& path : paths) {
            journal.apply({.success = I18n::tr("✓ Volume %1 removed file exclusion: %2").arg(QString::fromStdString(dl), QString::fromStdString(path)),
                           .failure = I18n::tr("✘ Volume %1 failed to remove file exclusion %2").arg(QString::fromStdString(dl), QString::fromStdString(path))},
                          [&] { m_volume.removeExclusion(*v, path); });
          }
        }
      } catch (const std::exception& error) {
        journal.recordError(I18n::tr("✘ Failed to read volume configuration"), error);
      }
    }

    // ── UWF_RegistryFilter ───────────────────────────────
    if (!m_changes.addRegistryExclusions.empty() || !m_changes.removeRegistryExclusions.empty() || m_changes.setPersistDomainSecretKey ||
        m_changes.setPersistTSCAL) {
      try {
        const auto next = m_registry.read(api::Session::Next);
        for (const auto& k : m_changes.addRegistryExclusions) {
          journal.apply({.success = I18n::tr("✓ Added registry exclusion: %1").arg(QString::fromStdString(k)),
                         .failure = I18n::tr("✘ Failed to add registry exclusion %1").arg(QString::fromStdString(k))},
                        [&] { m_registry.addExclusion(next, k); });
        }
        for (const auto& k : m_changes.removeRegistryExclusions) {
          journal.apply({.success = I18n::tr("✓ Removed registry exclusion: %1").arg(QString::fromStdString(k)),
                         .failure = I18n::tr("✘ Failed to remove registry exclusion %1").arg(QString::fromStdString(k))},
                        [&] { m_registry.removeExclusion(next, k); });
        }
        // 两个持久化开关整实例 PutInstance：未改动的那个用 next 行现值兜底。
        if (m_changes.setPersistDomainSecretKey || m_changes.setPersistTSCAL) {
          const api::RegistryPersistence persistence{.domainSecretKey = m_changes.setPersistDomainSecretKey.value_or(next.persistDomainSecretKey),
                                                     .terminalServicesClientAccessLicense = m_changes.setPersistTSCAL.value_or(next.persistTSCAL)};
          if (journal.execute(I18n::tr("✘ Failed to update registry persistence switches"), [&] { m_registry.setPersistence(next, persistence); }) ==
              ApplyJournal::StepResult::Confirmed) {
            if (m_changes.setPersistDomainSecretKey)
              journal.recordSuccess(
                  I18n::tr("✓ %1 persistence: %2")
                      .arg(I18n::tr("Domain Secret Key (DomainSecretKey)"), *m_changes.setPersistDomainSecretKey ? I18n::tr("Enabled") : I18n::tr("Disabled")));
            if (m_changes.setPersistTSCAL)
              journal.recordSuccess(I18n::tr("✓ %1 persistence: %2")
                                        .arg(I18n::tr("Terminal Services Client Access License (TSCAL)"),
                                             *m_changes.setPersistTSCAL ? I18n::tr("Enabled") : I18n::tr("Disabled")));
          }
        }
      } catch (const std::exception& error) {
        journal.recordError(I18n::tr("✘ Failed to read registry filter"), error);
      }
    }

    const std::string body = formatBlockPlain(I18n::tr("Applied changes").toStdString(), m_changeCmds) + "\n:: ==== " + I18n::tr("Result").toStdString() +
                             " ====\n" + joinLines(journal.lines());
    pendingText->setPlainText(QString::fromStdString(body));
    restartBtn->setVisible(journal.anyWriteConfirmed());
    // 所有前置读取都失败或前置条件不满足时，系统没有收到任何写请求，可以
    // 安全重试；只要写请求实际提交给 provider，或重新读取已确认目标状态本就
    // 收敛，就交回宿主刷新并保持禁用，避免重放部分成功或已经过期的批次。
    if (!journal.reconciliationRequired()) commitBtn->setEnabled(true);

    // 写完要立刻重新读一次快照并刷新 UI：
    // - next-session 的排除列表、保护状态、overlay 配置可能都变了；
    // - 各 DiskTab 的 pending 状态要清零，否则看起来"还没提交"。
    // 宿主用 QueuedConnection 接对账请求，等这波对话框里的事件循环回落
    // 再做，避免在回调里递归进 refresh 的弹窗 / WMI 读。
    if (journal.reconciliationRequired()) emit reconciliationRequired();
  });
  layout->addLayout(buttonRow);
}

}  // namespace uwf::ui
