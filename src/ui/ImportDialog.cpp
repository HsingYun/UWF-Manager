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
#include "ImportDialog.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>
#include <utility>

#include "../util/DriveLetter.h"
#include "Dialogs.h"
#include "I18n.h"
#include "PathElideDelegate.h"
#include "ThemeManager.h"

namespace uwf::ui {

QString parseErrorMessage(api::ParseError e, const QString& errorContext) {
  switch (e) {
    case api::ParseError::None:
    case api::ParseError::Comment:
      return {};  // 不应到这——caller 早就过滤掉了
    case api::ParseError::Incomplete:
      return I18n::tr("Incomplete uwfmgr command");
    case api::ParseError::MissingSizeArg:
      return I18n::tr("Missing size argument (MB)");
    case api::ParseError::InvalidSize:
      return I18n::tr("Size must be a non-negative integer in MB");
    case api::ParseError::MissingTypeArg:
      return I18n::tr("Missing overlay type argument (RAM or Disk)");
    case api::ParseError::UnknownType:
      return I18n::tr("Unknown overlay type %1 (expected RAM or Disk)").arg(errorContext);
    case api::ParseError::MissingVolumeArg:
      return I18n::tr("Missing volume argument (e.g. C:)");
    case api::ParseError::InvalidVolume:
      return I18n::tr("Volume must be a drive letter such as C:");
    case api::ParseError::MissingPathArg:
      return I18n::tr("Missing path argument");
    case api::ParseError::MissingRegistryKeyArg:
      return I18n::tr("Missing registry key argument");
    case api::ParseError::Unsupported:
      return I18n::tr("Unsupported uwfmgr command (cannot be mapped to a UI action)");
  }
  return {};
}

namespace {

// 拖入文件 / "从文件加载" 按钮共用的实现：从文件里 grep 含 "uwfmgr" 的行，
// 追加到指定文本框末尾（前面带一行 "# from: <basename>" 注释方便回查来源）。
//
// 5 MB 上限：一般日志/脚本足够；再大的多半是误拖的二进制，硬读会把 UI 卡很久。
void appendUwfmgrLinesFromFile(QPlainTextEdit* target, const QString& path) {
  QFile f(path);
  constexpr qint64 kMaxBytes = 5 * 1024 * 1024;
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    dialogs::warning(target, I18n::tr("Cannot read file"), I18n::tr("Could not open file %1: %2").arg(QFileInfo(path).fileName(), f.errorString()));
    return;
  }
  // 在打开句柄后检查并限量读取，消除“先 stat 路径、再 open”之间被替换或
  // 增长的竞态。即使文件在读取期间继续增长，也最多接收 kMaxBytes + 1。
  const QByteArray bytes = f.read(kMaxBytes + 1);
  if (f.error() != QFileDevice::NoError) {
    dialogs::warning(target, I18n::tr("Cannot read file"), I18n::tr("Could not open file %1: %2").arg(QFileInfo(path).fileName(), f.errorString()));
    return;
  }
  if (bytes.size() > kMaxBytes || !f.atEnd()) {
    dialogs::warning(target, I18n::tr("File too large"),
                     I18n::tr("File %1 is larger than 5 MB and was not parsed. Please filter it manually first.").arg(QFileInfo(path).fileName()));
    return;
  }

  QStringList matched;
  const QStringList lines = QString::fromUtf8(bytes).split('\n');
  for (const QString& line : lines) {
    if (line.contains(QStringLiteral("uwfmgr"), Qt::CaseInsensitive)) matched << line;
  }

  QString block;
  if (matched.isEmpty()) {
    // 文件里完全没找到 uwfmgr：在文本框里留个 "# (no uwfmgr lines)" 说明，
    // 让用户知道导入确实生效但内容为空，不至于以为程序坏了。
    block = QStringLiteral("# from: %1 (no uwfmgr lines found)\n").arg(QFileInfo(path).fileName());
  } else {
    block = QStringLiteral("# from: %1 (%2 line%3)\n")
                .arg(QFileInfo(path).fileName())
                .arg(matched.size())
                .arg(matched.size() == 1 ? QString() : QStringLiteral("s"));
    for (const auto& l : std::as_const(matched)) {
      block += l;
      block += QChar('\n');
    }
  }

  target->moveCursor(QTextCursor::End);
  target->insertPlainText(block);
  target->moveCursor(QTextCursor::End);
}

QString quotedFileCommand(const QString& path) { return QStringLiteral("uwfmgr file add-exclusion \"%1\"").arg(path); }

QString registryCommand(const QString& key) { return QStringLiteral("uwfmgr registry add-exclusion \"%1\"").arg(key); }

constexpr int kDefaultCeip = 1 << 0;
constexpr int kDefaultBits = 1 << 1;
constexpr int kDefaultNetwork = 1 << 2;
constexpr int kDefaultDst = 1 << 3;
constexpr int kDefaultDefender = 1 << 4;
constexpr int kDefaultScep = 1 << 5;

void appendCommandWithComment(QString& block, const QString& comment, const QString& command) {
  block += QStringLiteral(":: ");
  block += comment;
  block += QChar('\n');
  block += command;
  block += QChar('\n');
}

void appendSectionHeader(QString& block, const QString& title) {
  block += QStringLiteral(":: -- ");
  block += title;
  block += QStringLiteral(" --\n");
}

int chooseDefaultRuleSections(QWidget* parent) {
  QDialog dlg(parent);
  dlg.setWindowTitle(I18n::tr("Choose recommended configuration"));
  dlg.setMinimumWidth(400);

  auto* layout = new QVBoxLayout(&dlg);
  layout->setContentsMargins(20, 16, 20, 12);
  layout->setSpacing(10);

  auto* hint =
      new QLabel(I18n::tr("Select the recommended configuration groups to append. You can review or delete individual commands before importing."), &dlg);
  hint->setWordWrap(true);
  layout->addWidget(hint);

  auto* sourceHint = new QLabel(I18n::tr("From Microsoft official documentation."), &dlg);
  sourceHint->setWordWrap(true);
  sourceHint->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance().color(Sem::FgMuted).name()));
  layout->addWidget(sourceHint);

  auto addCheck = [&](const QString& text) {
    auto* cb = new QCheckBox(text, &dlg);
    cb->setChecked(true);
    layout->addWidget(cb);
    return cb;
  };

  auto* ceip = addCheck(I18n::tr("Customer Experience Improvement Program (CEIP)"));
  auto* bits = addCheck(I18n::tr("Background Intelligent Transfer Service (BITS)"));
  auto* network = addCheck(I18n::tr("Network profiles and policies"));
  auto* dst = addCheck(I18n::tr("Daylight saving time (DST)"));
  auto* defender = addCheck(I18n::tr("Microsoft Defender"));
  auto* scep = addCheck(I18n::tr("System Center Endpoint Protection"));

  auto* btns = new QDialogButtonBox(&dlg);
  auto* okBtn = btns->addButton(I18n::tr("OK"), QDialogButtonBox::AcceptRole);
  auto* cancelBtn = btns->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  layout->addWidget(btns);

  auto updateOk = [=]() {
    okBtn->setEnabled(ceip->isChecked() || bits->isChecked() || network->isChecked() || dst->isChecked() || defender->isChecked() || scep->isChecked());
  };
  for (auto* cb : {ceip, bits, network, dst, defender, scep}) QObject::connect(cb, &QCheckBox::toggled, &dlg, [updateOk](bool) { updateOk(); });
  QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
  updateOk();

  if (dlg.exec() != QDialog::Accepted) return 0;

  int sections = 0;
  if (ceip->isChecked()) sections |= kDefaultCeip;
  if (bits->isChecked()) sections |= kDefaultBits;
  if (network->isChecked()) sections |= kDefaultNetwork;
  if (dst->isChecked()) sections |= kDefaultDst;
  if (defender->isChecked()) sections |= kDefaultDefender;
  if (scep->isChecked()) sections |= kDefaultScep;
  return sections;
}

QString defaultRulesText(const int sections) {
  QString sys = QString::fromStdString(drive::systemLetter());
  if (sys.isEmpty()) sys = QStringLiteral("C:");

  auto win = [&](const QString& path) { return sys + QStringLiteral("\\Windows") + path; };
  auto programData = [&](const QString& path) { return sys + QStringLiteral("\\ProgramData") + path; };
  auto programFiles = [&](const QString& path) { return sys + QStringLiteral("\\Program Files") + path; };

  QString block;
  block += QStringLiteral(":: ==== ");
  block += I18n::tr("Microsoft recommended UWF exclusions");
  block += QStringLiteral(" ====\n");
  block += QStringLiteral(":: ");
  block += I18n::tr("Review these recommendations before importing; folders should exist before UWF accepts file exclusions.");
  block += QStringLiteral("\n");
  block += QStringLiteral(":: ");
  block += I18n::tr("Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.");
  block += QStringLiteral("\n\n");

  auto addFile = [&](const QString& comment, const QString& path) { appendCommandWithComment(block, comment, quotedFileCommand(path)); };
  auto addReg = [&](const QString& comment, const QString& key) { appendCommandWithComment(block, comment, registryCommand(key)); };

  if (sections & kDefaultCeip) {
    appendSectionHeader(block, I18n::tr("Customer Experience Improvement Program (CEIP)"));
    addReg(I18n::tr("CEIP: persist policy opt-in state"), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Policies\\Microsoft\\SQMClient\\Windows\\CEIPEnable"));
    addReg(I18n::tr("CEIP: persist local opt-in state"), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\SQMClient\\Windows\\CEIPEnable"));
    addReg(I18n::tr("CEIP: persist upload-disable flag"), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\SQMClient\\UploadDisableFlag"));
    block += QChar('\n');
  }

  if (sections & kDefaultBits) {
    appendSectionHeader(block, I18n::tr("Background Intelligent Transfer Service (BITS)"));
    addFile(I18n::tr("BITS: persist downloader queue files"), programData(QStringLiteral("\\Microsoft\\Network\\Downloader")));
    addReg(I18n::tr("BITS: persist transfer state index"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\BITS\\StateIndex"));
    block += QChar('\n');
  }

  if (sections & kDefaultNetwork) {
    appendSectionHeader(block, I18n::tr("Network profiles and policies"));
    addReg(I18n::tr("Wireless network GPO policy"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\Wireless\\GPTWirelessPolicy"));
    addReg(I18n::tr("Wired network GPO policy"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\WiredL2\\GP_Policy"));
    addFile(I18n::tr("Wireless network GPO policy files"), win(QStringLiteral("\\wlansvc\\Policies")));
    addFile(I18n::tr("Wired network GPO policy files"), win(QStringLiteral("\\dot2svc\\Policies")));
    addReg(I18n::tr("Wireless network interface profiles"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\wlansvc"));
    addReg(I18n::tr("Wired network interface profiles"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\dot3svc"));
    addReg(I18n::tr("Wireless service configuration"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\Wlansvc"));
    addReg(I18n::tr("Mobile broadband service configuration"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\WwanSvc"));
    addReg(I18n::tr("Wired AutoConfig service configuration"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\dot3svc"));
    block += QStringLiteral(":: ");
    block += I18n::tr("Network profile XML files are per-device; add the concrete Interfaces\\{GUID}\\{GUID}.xml paths manually if needed.");
    block += QStringLiteral("\n\n");
  }

  if (sections & kDefaultDst) {
    appendSectionHeader(block, I18n::tr("Daylight saving time (DST)"));
    addReg(I18n::tr("DST: persist time zone definitions"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"));
    addReg(I18n::tr("DST: persist selected time zone information"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"));
    block += QChar('\n');
  }

  if (sections & kDefaultDefender) {
    appendSectionHeader(block, I18n::tr("Microsoft Defender"));
    addFile(I18n::tr("Defender: persist product files and updates"), programFiles(QStringLiteral("\\Windows Defender")));
    addFile(I18n::tr("Defender: persist ProgramData signatures and state"), programData(QStringLiteral("\\Microsoft\\Windows Defender")));
    addFile(I18n::tr("Defender: persist Windows Update log"), win(QStringLiteral("\\WindowsUpdate.log")));
    addFile(I18n::tr("Defender: persist MpCmdRun log"), win(QStringLiteral("\\Temp\\MpCmdRun.log")));
    addReg(I18n::tr("Defender: persist product registry state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Defender"));
    addReg(I18n::tr("Defender: persist WdBoot service state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdBoot"));
    addReg(I18n::tr("Defender: persist WdFilter service state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdFilter"));
    addReg(I18n::tr("Defender: persist WdNisSvc service state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisSvc"));
    addReg(I18n::tr("Defender: persist WdNisDrv service state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisDrv"));
    addReg(I18n::tr("Defender: persist WinDefend service state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WinDefend"));
    block += QChar('\n');
  }

  if (sections & kDefaultScep) {
    appendSectionHeader(block, I18n::tr("System Center Endpoint Protection"));
    addFile(I18n::tr("SCEP: persist client program files"), programFiles(QStringLiteral("\\Microsoft Security Client")));
    addFile(I18n::tr("SCEP: persist Windows Update log"), win(QStringLiteral("\\Windowsupdate.log")));
    addFile(I18n::tr("SCEP: persist MpCmdRun log"), win(QStringLiteral("\\Temp\\Mpcmdrun.log")));
    addFile(I18n::tr("SCEP: persist antimalware signatures and state"), programData(QStringLiteral("\\Microsoft\\Microsoft Antimalware")));
    addReg(I18n::tr("SCEP: persist antimalware registry state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft Antimalware"));
  }

  return block;
}

void appendDefaultRules(QWidget* parent, QPlainTextEdit* target) {
  const int sections = chooseDefaultRuleSections(parent);
  if (sections == 0) return;

  target->moveCursor(QTextCursor::End);
  if (!target->toPlainText().isEmpty() && !target->toPlainText().endsWith(QChar('\n'))) target->insertPlainText(QStringLiteral("\n"));
  target->insertPlainText(defaultRulesText(sections));
  target->moveCursor(QTextCursor::End);
}

}  // namespace

ImportDialog::ImportDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle(I18n::tr("Import uwfmgr commands"));
  resize(820, 600);

  auto* layout = new QVBoxLayout(this);

  auto* desc = new QLabel(this);
  desc->setWordWrap(true);
  desc->setTextFormat(Qt::RichText);
  desc->setText(
      I18n::tr("<p>Paste or type <b>uwfmgr</b> commands below; one command per line. "
               "Supported categories: <code>filter</code> · <code>overlay</code> · <code>volume</code> · <code>file</code> · <code>registry</code>.</p>"
               "<p>Use <b>Load from file…</b> to pick any text-like file (logs, scripts, .txt, .bat, .ps1); "
               "lines containing <code>uwfmgr</code> will be appended to the box.</p>"
               "<p>Clicking <b>Import</b> turns each command into a pending UI change — <b>nothing is written to the system yet</b>. "
               "Use <b>Review and apply</b> in the toolbar to commit them.</p>"));
  layout->addWidget(desc);

  m_text = new QPlainTextEdit(this);
  m_text->setObjectName("importTextEdit");
  m_text->setPlaceholderText(
      I18n::tr("uwfmgr filter enable\n"
               "uwfmgr overlay set-type RAM\n"
               "uwfmgr volume protect C:\n"
               "uwfmgr file add-exclusion \"C:\\Users\\foo\\bar.txt\"\n"
               "uwfmgr registry add-exclusion HKLM\\Software\\MyApp"));
  // 等宽字体：路径 / 注册表键里有大量 \ 和空格，等宽下行对齐更直观。
  QFont mono = m_text->font();
  mono.setFamily(QStringLiteral("Consolas, Cascadia Mono, monospace"));
  m_text->setFont(mono);
  layout->addWidget(m_text, 1);

  m_summary = new QLabel(this);
  m_summary->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_summary->setWordWrap(true);
  m_summary->hide();
  layout->addWidget(m_summary);

  // 结果表格初始隐藏，第一次 Import 后填充并显示；用户可以再次清空文本继续导。
  m_report = new QTableWidget(0, 4, this);
  m_report->setHorizontalHeaderLabels({I18n::tr("#"), I18n::tr("Status"), I18n::tr("Command"), I18n::tr("Detail")});
  m_report->verticalHeader()->setVisible(false);
  m_report->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_report->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_report->setSelectionMode(QAbstractItemView::ExtendedSelection);
  // 命令、详情都不换行：显示不下即单行右侧截断为 "…"，完整内容由单元格
  // tooltip 查看。# / Status 两列 ResizeToContents、内容短，恒能完整显示。
  m_report->setWordWrap(false);
  m_report->setTextElideMode(Qt::ElideRight);
  m_report->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_report->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_report->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
  m_report->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  // 命令列给一个够宽的初值（Interactive 默认 section 宽度太小），用户仍可
  // 拖动；Detail（Stretch）吸收剩余宽度。
  m_report->setColumnWidth(2, 380);
  m_report->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  // Command / Detail 两列文本可能很长，套 PathElideDelegate 绕开 Qt 默认 paint
  // 在非 100% DPI 下把 elide 宽度算小的 bug（详见 PathElideDelegate.h）。
  m_report->setItemDelegateForColumn(2, new PathElideDelegate(m_report));
  m_report->setItemDelegateForColumn(3, new PathElideDelegate(m_report));
  m_report->setMinimumHeight(160);
  m_report->hide();
  layout->addWidget(m_report, 1);

  auto* buttonRow = new QHBoxLayout();
  m_importBtn = new QPushButton(I18n::tr("Import"), this);
  auto* defaultRulesBtn = new QPushButton(I18n::tr("Load recommended configuration"), this);
  auto* loadBtn = new QPushButton(I18n::tr("Load from file…"), this);
  m_importBtn->setObjectName("primaryBtn");
  // 文本框为空（或纯空白）时禁用 Import——不可点即不会进 onImportClicked，
  // 也就无需在那里弹"没有可导入内容"的模态框。
  m_importBtn->setEnabled(false);
  connect(m_text, &QPlainTextEdit::textChanged, this, [this]() { m_importBtn->setEnabled(!m_text->toPlainText().trimmed().isEmpty()); });
  auto* closeBtn = new QPushButton(I18n::tr("Close"), this);
  buttonRow->addWidget(m_importBtn);
  buttonRow->addStretch(1);
  buttonRow->addWidget(defaultRulesBtn);
  buttonRow->addWidget(loadBtn);
  buttonRow->addWidget(closeBtn);
  connect(m_importBtn, &QPushButton::clicked, this, &ImportDialog::onImportClicked);
  connect(defaultRulesBtn, &QPushButton::clicked, this, [this]() { appendDefaultRules(this, m_text); });
  connect(loadBtn, &QPushButton::clicked, this, [this]() {
    // 不限文件后缀（.txt / .bat / .ps1 / .log / 配置 dump 都可能含 uwfmgr 命令）。
    // 多选支持：用户可以一次拉一批日志进来。
    const QStringList paths = QFileDialog::getOpenFileNames(this, I18n::tr("Choose files containing uwfmgr commands"), QDir::homePath(),
                                                            I18n::tr("All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)"));
    for (const auto& p : paths) appendUwfmgrLinesFromFile(m_text, p);
  });
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
  layout->addLayout(buttonRow);
}

void ImportDialog::onImportClicked() {
  if (!m_applier) {
    dialogs::warning(this, I18n::tr("Import failed"), I18n::tr("Internal error: no applier registered."));
    return;
  }
  // 文本框为空时 Import 按钮已被禁用，到不了这里，无需再判空。
  const QString text = m_text->toPlainText();

  // 解析交给 src/uwf/api 的纯 std::string parser；UI 这边只负责 QString
  // ↔ std::string 的桥接和把注释/空行过滤掉。
  const auto parsed = api::parseUwfmgrText(text.toStdString());
  QList<api::UwfmgrCommand> realCmds;
  realCmds.reserve(static_cast<int>(parsed.size()));
  for (const auto& c : parsed) {
    if (c.parseError == api::ParseError::Comment) continue;
    realCmds.append(c);
  }
  if (realCmds.isEmpty()) {
    dialogs::warning(this, I18n::tr("Nothing to import"), I18n::tr("No uwfmgr commands found in the input."));
    return;
  }

  const auto rows = m_applier(realCmds);
  appendReport(rows);
  // 文本框清空：用户可以接着加载下一批 / 输入下一批。pending 状态由
  // m_applier 已经累加到对应控件，这里清文本框不会丢之前的修改。
  m_text->clear();
}

void ImportDialog::appendReport(const QList<ImportReportRow>& rows) {
  ++m_batchNo;
  for (const auto& r : rows) {
    switch (r.status) {
      case ImportReportRow::Status::Success:
        ++m_totalSuccess;
        break;
      case ImportReportRow::Status::Duplicate:
        ++m_totalDuplicate;
        break;
      case ImportReportRow::Status::Failed:
        ++m_totalFailed;
        break;
      case ImportReportRow::Status::Unsupported:
        ++m_totalUnsupported;
        break;
    }
  }

  const auto& tm = ThemeManager::instance();
  m_summary->setTextFormat(Qt::RichText);
  // summary 直接展示累计数字——用户关心的是"到目前为止整轮导入成果"。
  m_summary->setText(QStringLiteral("<b>%1</b> &nbsp; <span style='color:%2'>%3</span>  ·  <span style='color:%4'>%5</span>  ·  "
                                    "<span style='color:%6'>%7</span>  ·  %8")
                         .arg(I18n::tr("Cumulative after %1 batch(es):").arg(m_batchNo), tm.color(Sem::AddOk).name(),
                              I18n::tr("Applied: %1").arg(m_totalSuccess), tm.color(Sem::FgMuted).name(), I18n::tr("Duplicates: %1").arg(m_totalDuplicate),
                              tm.color(Sem::Danger).name(), I18n::tr("Failed: %1").arg(m_totalFailed), I18n::tr("Unsupported: %1").arg(m_totalUnsupported)));
  m_summary->show();

  m_report->setSortingEnabled(false);

  // 第二批起插一条横向分隔行（"── Batch N ──"），用 setSpan 横跨所有列；
  // 第一批不插，避免 "Batch 1" 这种装饰行污染单批使用的简洁视图。
  if (m_batchNo > 1) {
    const int sepRow = m_report->rowCount();
    m_report->setRowCount(sepRow + 1);
    auto* sep = new QTableWidgetItem(I18n::tr("── Batch %1 ──").arg(m_batchNo));
    QFont f = sep->font();
    f.setBold(true);
    sep->setFont(f);
    sep->setForeground(QBrush(tm.color(Sem::FgMuted)));
    sep->setTextAlignment(Qt::AlignCenter);
    // 不可选 / 不可编辑——纯展示。
    sep->setFlags(Qt::ItemIsEnabled);
    m_report->setItem(sepRow, 0, sep);
    m_report->setSpan(sepRow, 0, 1, m_report->columnCount());
  }

  // 真追加 rows——记录 startRow 起点，按相对偏移写入；不再 setRowCount(rows.size())
  // 把之前的内容清掉。
  const int startRow = m_report->rowCount();
  m_report->setRowCount(startRow + static_cast<int>(rows.size()));
  for (int i = 0; i < rows.size(); ++i) {
    const auto& r = rows[i];
    QString statusText;
    QColor statusColor;
    switch (r.status) {
      case ImportReportRow::Status::Success:
        statusText = I18n::tr("Applied");
        statusColor = tm.color(Sem::AddOk);
        break;
      case ImportReportRow::Status::Duplicate:
        statusText = I18n::tr("Duplicate");
        statusColor = tm.color(Sem::FgMuted);
        break;
      case ImportReportRow::Status::Failed:
        statusText = I18n::tr("Failed");
        statusColor = tm.color(Sem::Danger);
        break;
      case ImportReportRow::Status::Unsupported:
        statusText = I18n::tr("Unsupported");
        statusColor = tm.color(Sem::Warn);
        break;
    }

    auto* lineCell = new QTableWidgetItem(QString::number(r.lineNo));
    lineCell->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    auto* statusCell = new QTableWidgetItem(statusText);
    statusCell->setForeground(QBrush(statusColor));
    auto* cmdCell = new QTableWidgetItem(r.lineText);
    cmdCell->setToolTip(r.lineText);
    auto* detailCell = new QTableWidgetItem(r.detail);
    detailCell->setToolTip(r.detail);

    const int targetRow = startRow + i;
    m_report->setItem(targetRow, 0, lineCell);
    m_report->setItem(targetRow, 1, statusCell);
    m_report->setItem(targetRow, 2, cmdCell);
    m_report->setItem(targetRow, 3, detailCell);
  }

  m_report->show();
  // 滚到底，让用户立刻看到刚加的这批。
  m_report->scrollToBottom();
}

}  // namespace uwf::ui
