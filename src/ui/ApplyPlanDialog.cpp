#include "ApplyPlanDialog.h"

#include <QClipboard>
#include <QColor>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSaveFile>
#include <QStringConverter>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <algorithm>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include "../uwf/api/UwfmgrCli.h"
#include "DiskTab.h"
#include "GlobalStatusPanel.h"
#include "I18n.h"
#include "MessageDialog.h"
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
    for (const QAction* act : menu->actions()) {
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
    QStringList kept;
    for (const QString& line : sel.split('\n')) {
      if (line.trimmed().startsWith("::")) continue;
      kept << line;
    }
    QGuiApplication::clipboard()->setText(kept.join('\n'));
  }
};

// 配置类写操作（Protect/Unprotect/SetBindByDriveLetter/AddExclusion/
// RemoveExclusion 等）要发给"下一次会话" (CurrentSession=false) 的实例。
const api::VolumeRow* findNextVolume(const std::vector<api::VolumeRow>& rows, const std::string& driveLetter) {
  for (const auto& r : rows) {
    if (!r.currentSession && r.driveLetter == driveLetter) return &r;
  }
  return nullptr;
}

// SetType / SetMaximumSize 写入 CurrentSession=false 的实例（下次会话）。
// 注意：这两个方法还要求 UWF_Filter.CurrentEnabled==false，否则 WMI 会返回
// WBEM_E_INVALID_PARAMETER (0x80041008)。
const api::OverlayConfigRow* findNextOverlayConfig(const std::vector<api::OverlayConfigRow>& rows) {
  for (const auto& r : rows) {
    if (!r.currentSession) return &r;
  }
  return nullptr;
}

const api::RegistryFilterRow* findNextRegistryFilter(const std::vector<api::RegistryFilterRow>& rows) {
  for (const auto& r : rows) {
    if (!r.currentSession) return &r;
  }
  return nullptr;
}

api::OverlayType coreTypeToApi(core::OverlayType t) { return t == core::OverlayType::Disk ? api::OverlayType::Disk : api::OverlayType::RAM; }

}  // namespace

ApplyPlanDialog::ApplyPlanDialog(GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs, const core::UwfSnapshot& snapshot,
                                 WmiSession& writeSession, QWidget* parent)
    : QDialog(parent),
      m_session(writeSession),
      m_snapshot(snapshot),
      m_filter(m_session),
      m_overlay(m_session),
      m_overlayConfig(m_session),
      m_volume(m_session),
      m_registry(m_session) {
  // 一条变更或一条快照配置的命令文本一律走 src/uwf/api/UwfmgrCli 渲染——
  // quoting / 各 verb 的拼接全在那里，UI 这边只负责拼"comment"和决定哪些
  // PendingChanges 字段要不要进入显示列表。args 列表为空时按 0 个参数处理
  // （filter enable/disable）。
  auto cli = [](api::UwfmgrKind k, std::vector<std::string> args = {}) {
    api::UwfmgrCommand c;
    c.kind = k;
    c.args = std::move(args);
    return api::renderCommand(c);
  };

  // ── 收集 user 待应用的改动 ──────────────────────────
  if (auto v = global->pendingFilterEnabled()) {
    m_changes.setFilterEnabled = *v;
    m_changeCmds.push_back({I18n::tr("· Filter (global) %1").arg(*v ? I18n::tr("Enable") : I18n::tr("Disable")).toStdString(),
                            cli(*v ? api::UwfmgrKind::FilterEnable : api::UwfmgrKind::FilterDisable)});
  }
  {
    const auto d = global->pendingOverlay();
    m_changes.setOverlay = d;
    if (d.type) {
      const char* typeStr = *d.type == core::OverlayType::RAM ? "RAM" : "Disk";
      m_changeCmds.push_back({I18n::tr("· Overlay type → %1").arg(typeStr).toStdString(), cli(api::UwfmgrKind::OverlaySetType, {typeStr})});
    }
    if (d.maximumSizeMb) {
      m_changeCmds.push_back({I18n::tr("· Overlay maximum size → %1 MB").arg(*d.maximumSizeMb).toStdString(),
                              cli(api::UwfmgrKind::OverlaySetSize, {std::to_string(*d.maximumSizeMb)})});
    }
    if (d.warningThresholdMb) {
      m_changeCmds.push_back({I18n::tr("· Overlay warning threshold → %1 MB").arg(*d.warningThresholdMb).toStdString(),
                              cli(api::UwfmgrKind::OverlaySetWarningThreshold, {std::to_string(*d.warningThresholdMb)})});
    }
    if (d.criticalThresholdMb) {
      m_changeCmds.push_back({I18n::tr("· Overlay critical threshold → %1 MB").arg(*d.criticalThresholdMb).toStdString(),
                              cli(api::UwfmgrKind::OverlaySetCriticalThreshold, {std::to_string(*d.criticalThresholdMb)})});
    }
    if (d.touchesOverlayConfig() && m_snapshot.current.filter.enabled) {
      m_changeCmds.push_back(
          {I18n::tr("⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.").toStdString(), ""});
    }
  }

  for (auto& t : diskTabs) {
    if (!t || !t->supported()) continue;
    const std::string dlStd = t->driveLetter().toStdString();

    if (auto v = t->pendingVolumeProtected()) {
      m_changes.volumeProtect[dlStd] = *v;
      m_changeCmds.push_back(
          {I18n::tr("· Volume %1 protection %2").arg(QString::fromStdString(dlStd), *v ? I18n::tr("Enable") : I18n::tr("Disable")).toStdString(),
           cli(*v ? api::UwfmgrKind::VolumeProtect : api::UwfmgrKind::VolumeUnprotect, {dlStd})});
    }
    if (auto v = t->pendingBindByVolumeName()) {
      m_changes.volumeBindByVolumeName[dlStd] = *v;
      // uwfmgr CLI 没有 SetBindByDriveLetter 对应命令，只能走本程序的 WMI 写入。
      // cmd 留空 → 仅渲染 comment 行。
      m_changeCmds.push_back({I18n::tr("· Volume %1 bind by → %2 (no CLI equivalent; this program only)")
                                  .arg(QString::fromStdString(dlStd), *v ? I18n::tr("volume ID") : I18n::tr("drive letter"))
                                  .toStdString(),
                              ""});
    }
    // 注意只在有 pending 时才 access map[dlStd]——map 的 operator[] 会无端
    // 插入空 entry，commit 分支后续 for-each 会因此误以为这个卷有变更并尝试
    // 注册它（"为何改 D: 时连 F: 也被注册"的根因）。
    if (const auto added = t->pendingFileAdded(); !added.isEmpty()) {
      auto& addBucket = m_changes.addFileExclusions[dlStd];
      for (const auto& p : added) {
        const std::string ps = p.toStdString();
        addBucket.push_back(ps);
        m_changeCmds.push_back(
            {I18n::tr("+ File exclusion  %1  %2").arg(QString::fromStdString(dlStd), p).toStdString(), cli(api::UwfmgrKind::FileAddExclusion, {ps})});
      }
    }
    if (const auto removed = t->pendingFileRemoved(); !removed.isEmpty()) {
      auto& rmBucket = m_changes.removeFileExclusions[dlStd];
      for (const auto& p : removed) {
        const std::string ps = p.toStdString();
        rmBucket.push_back(ps);
        m_changeCmds.push_back(
            {I18n::tr("− File exclusion  %1  %2").arg(QString::fromStdString(dlStd), p).toStdString(), cli(api::UwfmgrKind::FileRemoveExclusion, {ps})});
      }
    }
    for (const auto& p : t->pendingRegAdded()) {
      const std::string ps = p.toStdString();
      m_changes.addRegistryExclusions.push_back(ps);
      m_changeCmds.push_back({I18n::tr("+ Registry exclusion  %1").arg(p).toStdString(), cli(api::UwfmgrKind::RegistryAddExclusion, {ps})});
    }
    for (const auto& p : t->pendingRegRemoved()) {
      const std::string ps = p.toStdString();
      m_changes.removeRegistryExclusions.push_back(ps);
      m_changeCmds.push_back({I18n::tr("− Registry exclusion  %1").arg(p).toStdString(), cli(api::UwfmgrKind::RegistryRemoveExclusion, {ps})});
    }
  }

  // ── 收集当前快照配置（基于 current session：现在 UWF 真实在跑的状态）──
  // 用 current 而非 next 是因为：next 是"上次应用过、等下次重启生效"的配置，
  // 普通用户更想看到"现在 UWF 实际在做什么"，那就是 current。
  const auto& cur = m_snapshot.current;
  m_snapshotCmds.push_back({I18n::tr("Filter (global) %1").arg(cur.filter.enabled ? I18n::tr("Enabled") : I18n::tr("Disabled")).toStdString(),
                            cli(cur.filter.enabled ? api::UwfmgrKind::FilterEnable : api::UwfmgrKind::FilterDisable)});
  {
    const auto& o = cur.overlay;
    const char* typeStr = o.type == core::OverlayType::RAM ? "RAM" : "Disk";
    m_snapshotCmds.push_back({I18n::tr("Overlay type → %1").arg(typeStr).toStdString(), cli(api::UwfmgrKind::OverlaySetType, {typeStr})});
    m_snapshotCmds.push_back(
        {I18n::tr("Overlay maximum size → %1 MB").arg(o.maximumSizeMb).toStdString(), cli(api::UwfmgrKind::OverlaySetSize, {std::to_string(o.maximumSizeMb)})});
    m_snapshotCmds.push_back({I18n::tr("Overlay warning threshold → %1 MB").arg(o.warningThresholdMb).toStdString(),
                              cli(api::UwfmgrKind::OverlaySetWarningThreshold, {std::to_string(o.warningThresholdMb)})});
    m_snapshotCmds.push_back({I18n::tr("Overlay critical threshold → %1 MB").arg(o.criticalThresholdMb).toStdString(),
                              cli(api::UwfmgrKind::OverlaySetCriticalThreshold, {std::to_string(o.criticalThresholdMb)})});
  }
  for (const auto& v : cur.volumes) {
    if (v.driveLetter.empty()) continue;
    m_snapshotCmds.push_back({I18n::tr("Volume %1 protection %2")
                                  .arg(QString::fromStdString(v.driveLetter), v.isProtected ? I18n::tr("Enabled") : I18n::tr("Disabled"))
                                  .toStdString(),
                              cli(v.isProtected ? api::UwfmgrKind::VolumeProtect : api::UwfmgrKind::VolumeUnprotect, {v.driveLetter})});
  }
  // fileExclusions 的 key 是 volumeName，渲染时回查 driveLetter 让 comment 更可读
  // （CLI 命令本身不需要——路径自带盘符就够 UWF 定位了）。
  for (const auto& [vname, paths] : cur.fileExclusions) {
    std::string dl;
    for (const auto& vol : cur.volumes) {
      if (vol.volumeName == vname) {
        dl = vol.driveLetter;
        break;
      }
    }
    for (const auto& p : paths) {
      m_snapshotCmds.push_back({I18n::tr("File exclusion %1 %2").arg(QString::fromStdString(dl.empty() ? vname : dl), QString::fromStdString(p)).toStdString(),
                                cli(api::UwfmgrKind::FileAddExclusion, {p})});
    }
  }
  for (const auto& k : cur.registryExclusions) {
    m_snapshotCmds.push_back({I18n::tr("Registry exclusion %1").arg(QString::fromStdString(k)).toStdString(), cli(api::UwfmgrKind::RegistryAddExclusion, {k})});
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

  auto formatBlockHtml = [&](const QString& title, const std::vector<Cmd>& items) {
    QString html;
    // 段标题：弱化处理（淡灰底 + 灰字 + 不加粗 + 小字号），让"命令"成为
    // 视觉主角。plain-text 形式以 ":: " 起头让 CommandTextEdit 复制时跳过。
    html += QString("<div style='background:%1;color:%2;padding:3px 10px;border-radius:3px;font-size:9pt;margin:8px 0 2px 0'>:: %3</div>")
                .arg(mutedTint, mutedFaint, title.toHtmlEscaped());
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

  QString defaultHtml;
  // 外层 wrapper 显式声明 font-family，让所有内部 div 继承——不写的话
  // QTextEdit 的 RichText 引擎对中文字符会按字体表自由 fallback，多段中
  // 后面的段标题偶尔会落到宋体，跟其它段视觉上不一致。
  defaultHtml += "<div style=\"font-family:'Segoe UI','Microsoft YaHei UI','Microsoft YaHei',sans-serif\">";
  if (!m_changeCmds.empty()) {
    defaultHtml += formatBlockHtml(I18n::tr("Pending changes"), m_changeCmds);
  }
  defaultHtml += formatBlockHtml(I18n::tr("Current session configuration"), m_snapshotCmds);
  defaultHtml += "</div>";

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

  auto* text = new CommandTextEdit(this);
  text->setReadOnly(true);
  text->setObjectName("planText");
  text->setLineWrapMode(QTextEdit::NoWrap);
  text->setHtml(defaultHtml);
  layout->addWidget(text, 1);

  auto* box = new QDialogButtonBox(this);
  // 导出按钮：把对话框里展示的所有命令写到一个文件，注释（":: ..." 行）剔掉，
  // 只保留可执行的 uwfmgr 命令。ActionRole 在 WindowsLayout 下落在左侧，
  // 与 Apply / Close 自然分组。
  auto* exportBtn = box->addButton(I18n::tr("Export commands…"), QDialogButtonBox::ActionRole);
  auto* commitBtn = box->addButton(I18n::tr("Apply"), QDialogButtonBox::AcceptRole);
  commitBtn->setObjectName("primaryBtn");
  // 不用 QDialogButtonBox::Close 标准按钮——它走 Qt 内置翻译，没加载中文
  // 翻译包时会显示 "Close"。直接用自定义按钮加 RejectRole 保留关闭语义。
  box->addButton(I18n::tr("Close"), QDialogButtonBox::RejectRole);

  // 无变更时禁用"应用"，只能关闭。
  const bool hasChanges = !m_changeCmds.empty();
  commitBtn->setEnabled(hasChanges);

  // 导出：先 changeCmds 再 snapshotCmds，跳过 cmd 为空的纯注释行（这些在
  // 收集时只是给视觉做提示，没有可执行 CLI 对应）。两段之间空一行隔开。
  connect(exportBtn, &QPushButton::clicked, this, [this]() {
    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    const QString suggested = QString("uwfmgr-commands-%1.txt").arg(stamp);
    const QString path = QFileDialog::getSaveFileName(this, I18n::tr("Export commands to file"), QDir::home().filePath(suggested),
                                                      I18n::tr("Text files (*.txt);;All files (*)"));
    if (path.isEmpty()) return;

    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      warning(this, I18n::tr("Export failed"), I18n::tr("Could not open file for writing: %1").arg(out.errorString()));
      return;
    }
    QTextStream ts(&out);
    ts.setEncoding(QStringConverter::Utf8);
    int written = 0;
    for (const auto& c : m_changeCmds) {
      if (c.cmd.empty()) continue;
      ts << QString::fromStdString(c.cmd) << '\n';
      ++written;
    }
    if (written > 0 && std::ranges::any_of(m_snapshotCmds, [](const auto& c) { return !c.cmd.empty(); })) {
      ts << '\n';
    }
    for (const auto& c : m_snapshotCmds) {
      if (c.cmd.empty()) continue;
      ts << QString::fromStdString(c.cmd) << '\n';
      ++written;
    }
    if (!out.commit()) {
      warning(this, I18n::tr("Export failed"), I18n::tr("Could not write file: %1").arg(out.errorString()));
      return;
    }
    information(this, I18n::tr("Export finished"), I18n::tr("Exported %1 commands to:\n%2").arg(written).arg(QDir::toNativeSeparators(path)));
  });

  connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(commitBtn, &QPushButton::clicked, this, [this, text, commitBtn, formatBlockPlain, joinLines]() {
    // 真实写入前再弹一次二次确认，避免误点。
    const QString warn2 = ThemeManager::instance().color(Sem::Warn).name();
    if (!confirm(this, I18n::tr("Confirm apply"),
                 I18n::tr("These changes will be <span style='color:%1'>written to the system</span>; most take effect after the next reboot.<br><br>Continue?")
                     .arg(warn2)))
      return;
    // 一个对话框只应用一次：确认后立即禁用 Apply。applied() 会触发宿主 refresh
    // 重新读快照并改写 m_snapshot（本对话框按引用持有它）；若不禁用，用户再点
    // 一次会把同一批 m_changes 对着已刷新的快照重放。
    commitBtn->setEnabled(false);
    std::vector<std::string> outcome;

    // 每一步单独收集错误，不因单点失败终止其它写入。
    auto note = [&](const std::string& line) { outcome.push_back(line); };

    if (!m_session.isConnected()) {
      std::string err;
      if (!m_session.connect(api::kWmiNamespace, &err)) {
        note(I18n::tr("✘ Failed to connect to the system: %1").arg(QString::fromStdString(err)).toStdString());
        const std::string body = formatBlockPlain(I18n::tr("Applied changes").toStdString(), m_changeCmds) + "\n:: ==== " + I18n::tr("Result").toStdString() +
                                 " ====\n" + joinLines(outcome);
        text->setPlainText(QString::fromStdString(body));
        return;
      }
    }

    // ── UWF_Filter ───────────────────────────────────────
    if (m_changes.setFilterEnabled) {
      std::string err;
      auto row = m_filter.read(&err);
      if (!row) {
        note(I18n::tr("✘ Failed to read filter state: %1").arg(QString::fromStdString(err)).toStdString());
      } else {
        const bool ok = *m_changes.setFilterEnabled ? m_filter.enable(*row, &err) : m_filter.disable(*row, &err);
        note(ok ? I18n::tr("✓ Filter: %1").arg(*m_changes.setFilterEnabled ? I18n::tr("Enabled") : I18n::tr("Disabled")).toStdString()
                : I18n::tr("✘ Failed to %1 filter: %2")
                      .arg(*m_changes.setFilterEnabled ? I18n::tr("enable") : I18n::tr("disable"), QString::fromStdString(err))
                      .toStdString());
      }
    }

    // ── UWF_Overlay (阈值) ─────────────────────────────────
    // 阈值无 session 区分，也不需要先禁用筛选器。只下发被改的字段。
    if (m_changes.setOverlay.warningThresholdMb || m_changes.setOverlay.criticalThresholdMb) {
      std::string err;
      if (auto overlay = m_overlay.read(&err)) {
        if (const auto v = m_changes.setOverlay.warningThresholdMb) {
          if (m_overlay.setWarningThreshold(*overlay, *v, &err))
            note(I18n::tr("✓ Overlay warning threshold set to %1 MB").arg(*v).toStdString());
          else
            note(I18n::tr("✘ Failed to set warning threshold: %1").arg(QString::fromStdString(err)).toStdString());
        }
        if (const auto v = m_changes.setOverlay.criticalThresholdMb) {
          if (m_overlay.setCriticalThreshold(*overlay, *v, &err))
            note(I18n::tr("✓ Overlay critical threshold set to %1 MB").arg(*v).toStdString());
          else
            note(I18n::tr("✘ Failed to set critical threshold: %1").arg(QString::fromStdString(err)).toStdString());
        }
      } else {
        note(I18n::tr("✘ Failed to read overlay state: %1").arg(QString::fromStdString(err)).toStdString());
      }
    }

    // ── UWF_OverlayConfig (next, 类型 / 最大大小) ─────────
    // 前提：UWF_Filter.CurrentEnabled 必须为 false，否则 WMI 直接拒绝。
    if (m_changes.setOverlay.touchesOverlayConfig()) {
      if (m_snapshot.current.filter.enabled) {
        note(I18n::tr("✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.").toStdString());
      } else {
        std::string err;
        const auto configs = m_overlayConfig.readAll(&err);
        if (const auto* next = findNextOverlayConfig(configs)) {
          if (const auto t = m_changes.setOverlay.type) {
            const char* tStr = *t == core::OverlayType::RAM ? "RAM" : "Disk";
            if (m_overlayConfig.setType(*next, coreTypeToApi(*t), &err))
              note(I18n::tr("✓ Overlay type set to %1").arg(tStr).toStdString());
            else
              note(I18n::tr("✘ Failed to set overlay type: %1").arg(QString::fromStdString(err)).toStdString());
          }
          if (const auto v = m_changes.setOverlay.maximumSizeMb) {
            // 基于磁盘的覆盖层要求最大大小至少 1024 MB。type 未在本次 delta 中
            // 改动时，沿用 next 会话的基线类型判断。
            const auto effType = m_changes.setOverlay.type.value_or(m_snapshot.next.overlay.type);
            if (effType == core::OverlayType::Disk && *v < core::kDiskOverlayMinSizeMb) {
              note(I18n::tr("✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.").arg(core::kDiskOverlayMinSizeMb).toStdString());
            } else if (m_overlayConfig.setMaximumSize(*next, *v, &err)) {
              note(I18n::tr("✓ Overlay maximum size set to %1 MB").arg(*v).toStdString());
            } else {
              note(I18n::tr("✘ Failed to set maximum size: %1").arg(QString::fromStdString(err)).toStdString());
            }
          }
        } else {
          note(I18n::tr("✘ Failed to read overlay configuration: %1").arg(QString::fromStdString(err)).toStdString());
        }
      }
    }

    // ── UWF_Volume ───────────────────────────────────────
    if (!m_changes.volumeProtect.empty() || !m_changes.volumeBindByVolumeName.empty() || !m_changes.addFileExclusions.empty() ||
        !m_changes.removeFileExclusions.empty()) {
      std::string err;
      auto volumes = m_volume.readAll(&err);

      // 找 next session row；找不到就让 ensureNextSessionEntry 从同卷的
      // current session 行复制 VolumeName 创建一份 next session 实例。
      // 返回 by value，同时把新 row append 到 volumes 让后续的 caller 也
      // 能命中（避免对同一卷的多个 pending 改动重复触发 PutInstance）。
      auto getOrCreateNextVolume = [&](const std::string& dl) -> std::optional<api::VolumeRow> {
        if (const auto* hit = findNextVolume(volumes, dl)) return *hit;
        std::string e;
        auto created = m_volume.ensureNextSessionEntry(dl, &e);
        if (!created) {
          note(I18n::tr("✘ Volume %1: failed to register with UWF: %2").arg(QString::fromStdString(dl), QString::fromStdString(e)).toStdString());
          return std::nullopt;
        }
        volumes.push_back(*created);
        return created;
      };

      for (const auto& [dl, wantProtect] : m_changes.volumeProtect) {
        auto v = getOrCreateNextVolume(dl);
        if (!v) continue;
        const bool ok = wantProtect ? m_volume.protectVolume(*v, &err) : m_volume.unprotect(*v, &err);
        note(ok ? I18n::tr("✓ Volume %1 protection: %2").arg(QString::fromStdString(dl), wantProtect ? I18n::tr("Enabled") : I18n::tr("Disabled")).toStdString()
                : I18n::tr("✘ Failed to %1 protection on volume %2: %3")
                      .arg(wantProtect ? I18n::tr("enable") : I18n::tr("disable"), QString::fromStdString(dl), QString::fromStdString(err))
                      .toStdString());
      }

      for (const auto& [dl, byVolumeName] : m_changes.volumeBindByVolumeName) {
        auto v = getOrCreateNextVolume(dl);
        if (!v) continue;
        std::string e;
        // changes 里以"按卷 ID 绑定"为语义（byVolumeName）；UWF_Volume.SetBindByDriveLetter
        // 的入参 bBindByDriveLetter 语义相反，传参时取反。
        const bool ok = m_volume.setBindByDriveLetter(*v, !byVolumeName, &e);
        note(ok ? I18n::tr("✓ Volume %1 bind by: %2")
                      .arg(QString::fromStdString(dl), byVolumeName ? I18n::tr("volume ID") : I18n::tr("drive letter"))
                      .toStdString()
                : I18n::tr("✘ Failed to set binding for volume %1: %2").arg(QString::fromStdString(dl), QString::fromStdString(e)).toStdString());
      }

      for (const auto& [dl, paths] : m_changes.addFileExclusions) {
        if (paths.empty()) continue;
        auto v = getOrCreateNextVolume(dl);
        if (!v) continue;
        for (const auto& path : paths) {
          std::string e;
          if (m_volume.addExclusion(*v, path, &e))
            note(I18n::tr("✓ Volume %1 added file exclusion: %2").arg(QString::fromStdString(dl), QString::fromStdString(path)).toStdString());
          else
            note(I18n::tr("✘ Volume %1 failed to add file exclusion %2: %3")
                     .arg(QString::fromStdString(dl), QString::fromStdString(path), QString::fromStdString(e))
                     .toStdString());
        }
      }
      for (const auto& [dl, paths] : m_changes.removeFileExclusions) {
        if (paths.empty()) continue;
        // remove 分支不能走 getOrCreateNextVolume：如果 next-session 行
        // 还不存在，那这个卷在 next session 里压根没有任何排除项，本来
        // 就没东西可删；不应该为了"删一条不存在的排除"而触发
        // PutInstance 把卷无故注册进 UWF。直接跳过即可。
        const auto* v = findNextVolume(volumes, dl);
        if (!v) continue;
        for (const auto& path : paths) {
          std::string e;
          if (m_volume.removeExclusion(*v, path, &e))
            note(I18n::tr("✓ Volume %1 removed file exclusion: %2").arg(QString::fromStdString(dl), QString::fromStdString(path)).toStdString());
          else
            note(I18n::tr("✘ Volume %1 failed to remove file exclusion %2: %3")
                     .arg(QString::fromStdString(dl), QString::fromStdString(path), QString::fromStdString(e))
                     .toStdString());
        }
      }
    }

    // ── UWF_RegistryFilter ───────────────────────────────
    if (!m_changes.addRegistryExclusions.empty() || !m_changes.removeRegistryExclusions.empty()) {
      std::string err;
      const auto regs = m_registry.readAll(&err);
      const auto* next = findNextRegistryFilter(regs);
      if (!next) {
        note(I18n::tr("✘ Failed to read registry filter: %1").arg(QString::fromStdString(err)).toStdString());
      } else {
        for (const auto& k : m_changes.addRegistryExclusions) {
          std::string e;
          if (m_registry.addExclusion(*next, k, &e))
            note(I18n::tr("✓ Added registry exclusion: %1").arg(QString::fromStdString(k)).toStdString());
          else
            note(I18n::tr("✘ Failed to add registry exclusion %1: %2").arg(QString::fromStdString(k), QString::fromStdString(e)).toStdString());
        }
        for (const auto& k : m_changes.removeRegistryExclusions) {
          std::string e;
          if (m_registry.removeExclusion(*next, k, &e))
            note(I18n::tr("✓ Removed registry exclusion: %1").arg(QString::fromStdString(k)).toStdString());
          else
            note(I18n::tr("✘ Failed to remove registry exclusion %1: %2").arg(QString::fromStdString(k), QString::fromStdString(e)).toStdString());
        }
      }
    }

    const std::string body = formatBlockPlain(I18n::tr("Applied changes").toStdString(), m_changeCmds) + "\n:: ==== " + I18n::tr("Result").toStdString() +
                             " ====\n" + joinLines(outcome);
    text->setPlainText(QString::fromStdString(body));

    // 写完要立刻重新读一次快照并刷新 UI：
    // - next-session 的排除列表、保护状态、overlay 配置可能都变了；
    // - 各 DiskTab 的 pending 状态要清零，否则看起来"还没提交"。
    // 宿主用 QueuedConnection 接 applied()，等这波对话框里的事件循环回落
    // 再做，避免在回调里递归进 refresh 的弹窗 / WMI 读。
    emit applied();
  });
  layout->addWidget(box);
}

}  // namespace uwf::ui
