#include "ImportDialog.h"

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
#include <QStringConverter>
#include <QStringList>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>


#include "I18n.h"
#include "MessageDialog.h"
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
  if (f.size() > kMaxBytes) {
    dialogs::warning(target, I18n::tr("File too large"),
                     I18n::tr("File %1 is larger than 5 MB and was not parsed. Please filter it manually first.").arg(QFileInfo(path).fileName()));
    return;
  }
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    dialogs::warning(target, I18n::tr("Cannot read file"),
                     I18n::tr("Could not open file %1: %2").arg(QFileInfo(path).fileName(), f.errorString()));
    return;
  }
  QTextStream ts(&f);
  ts.setEncoding(QStringConverter::Utf8);

  QStringList matched;
  while (!ts.atEnd()) {
    const QString line = ts.readLine();
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
    for (const auto& l : matched) {
      block += l;
      block += QChar('\n');
    }
  }

  target->moveCursor(QTextCursor::End);
  target->insertPlainText(block);
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
  desc->setText(I18n::tr(
      "<p>Paste or type <b>uwfmgr</b> commands below; one command per line. "
      "Supported categories: <code>filter</code> · <code>overlay</code> · <code>volume</code> · <code>file</code> · <code>registry</code>.</p>"
      "<p>Use <b>Load from file…</b> to pick any text-like file (logs, scripts, .txt, .bat, .ps1); "
      "lines containing <code>uwfmgr</code> will be appended to the box.</p>"
      "<p>Clicking <b>Import</b> turns each command into a pending UI change — <b>nothing is written to the system yet</b>. "
      "Use <b>Review and apply</b> in the toolbar to commit them.</p>"));
  layout->addWidget(desc);

  m_text = new QPlainTextEdit(this);
  m_text->setObjectName("importTextEdit");
  m_text->setPlaceholderText(I18n::tr(
      "uwfmgr filter enable\n"
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
  m_report->setWordWrap(true);
  m_report->setTextElideMode(Qt::ElideNone);
  m_report->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_report->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_report->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
  m_report->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  m_report->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_report->setMinimumHeight(160);
  m_report->hide();
  layout->addWidget(m_report, 1);

  auto* btns = new QDialogButtonBox(this);
  // ActionRole 在 WindowsLayout 下落在最左侧，跟 Import / Close 自然分组。
  auto* loadBtn = btns->addButton(I18n::tr("Load from file…"), QDialogButtonBox::ActionRole);
  m_importBtn = btns->addButton(I18n::tr("Import"), QDialogButtonBox::AcceptRole);
  m_importBtn->setObjectName("primaryBtn");
  btns->addButton(I18n::tr("Close"), QDialogButtonBox::RejectRole);
  connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(m_importBtn, &QPushButton::clicked, this, &ImportDialog::onImportClicked);
  connect(loadBtn, &QPushButton::clicked, this, [this]() {
    // 不限文件后缀（.txt / .bat / .ps1 / .log / 配置 dump 都可能含 uwfmgr 命令）。
    // 多选支持：用户可以一次拉一批日志进来。
    const QStringList paths = QFileDialog::getOpenFileNames(this, I18n::tr("Choose files containing uwfmgr commands"), QDir::homePath(),
                                                            I18n::tr("All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)"));
    for (const auto& p : paths) appendUwfmgrLinesFromFile(m_text, p);
  });
  layout->addWidget(btns);
}

void ImportDialog::onImportClicked() {
  if (!m_applier) {
    dialogs::warning(this, I18n::tr("Import failed"), I18n::tr("Internal error: no applier registered."));
    return;
  }
  const QString text = m_text->toPlainText();
  if (text.trimmed().isEmpty()) {
    dialogs::warning(this, I18n::tr("Nothing to import"), I18n::tr("The text area is empty."));
    return;
  }

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
      case ImportReportRow::Status::Success: ++m_totalSuccess; break;
      case ImportReportRow::Status::Duplicate: ++m_totalDuplicate; break;
      case ImportReportRow::Status::Failed: ++m_totalFailed; break;
      case ImportReportRow::Status::Unsupported: ++m_totalUnsupported; break;
    }
  }

  const auto& tm = ThemeManager::instance();
  m_summary->setTextFormat(Qt::RichText);
  // summary 直接展示累计数字——用户关心的是"到目前为止整轮导入成果"。
  m_summary->setText(QStringLiteral("<b>%1</b> &nbsp; <span style='color:%2'>%3</span>  ·  <span style='color:%4'>%5</span>  ·  "
                                    "<span style='color:%6'>%7</span>  ·  %8")
                         .arg(I18n::tr("Cumulative after %1 batch(es):").arg(m_batchNo),
                              tm.color(Sem::AddOk).name(), I18n::tr("Applied: %1").arg(m_totalSuccess),
                              tm.color(Sem::FgMuted).name(), I18n::tr("Duplicates: %1").arg(m_totalDuplicate),
                              tm.color(Sem::Danger).name(), I18n::tr("Failed: %1").arg(m_totalFailed),
                              I18n::tr("Unsupported: %1").arg(m_totalUnsupported)));
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
