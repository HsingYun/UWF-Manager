#include "MainWindow.h"

#include <windows.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QHoverEvent>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QProgressDialog>
#include <QPushButton>
#include <QSet>
#include <QShortcut>
#include <QStatusBar>
#include <QStyle>
#include <QStyleOption>
#include <QSvgRenderer>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <chrono>
#include <format>
#include <memory>
#include <string_view>

#include "../core/Config.h"
#include "../util/DriveLetter.h"
#include "../util/Log.h"
#include "../util/PathMatch.h"
#include "../util/RegistryKey.h"
#include "../uwf/UwfSnapshot.h"
#include "../uwf/api/UwfmgrCli.h"
#include "../uwf/wmi/WmiError.h"
#include "../uwf/wmi/WmiResult.h"
#include "ApplyPlanDialog.h"
#include "DiskTab.h"
#include "GlobalStatusPanel.h"
#include "I18n.h"
#include "ImportDialog.h"
#include "LogViewerDialog.h"
#include "MessageDialog.h"
#include "TableText.h"
#include "ThemeManager.h"
#include "TrayController.h"
#include "uwf_version.h"

namespace uwf::ui {

namespace {

// 旧的 warnSelectable / confirmYesNo helper 已迁移到 ui::dialogs（QDialog 实现，
// 走 app font，避免 QMessageBox 的中文渲染糊问题）。
using uwf::ui::dialogs::confirm;
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

// QToolBar 溢出时最右侧的扩展按钮（qt_toolbar_ext_button）：被 QSS 的
// `QToolBar QToolButton` 规则命中后转由 QStyleSheetStyle 渲染，后者既不画
// PE_IndicatorToolBarExtension 雪佛龙、又因该按钮自绘 paintEvent 而忽略 setIcon，
// 于是只剩一个空白方块。QSS background-image 能补图标，但 SVG 被位图栅格化、
// 高分屏放大后发糊。这里用事件过滤器接管它的 Paint：先用 PE_Widget 画出 QSS
// 背景（保留 hover），再用 QSvgRenderer 矢量绘制雪佛龙——任意 DPI 都清晰。
class ToolbarExtIcon : public QObject {
 public:
  using QObject::QObject;

  bool eventFilter(QObject* obj, QEvent* ev) override {
    if (ev->type() != QEvent::Paint) return QObject::eventFilter(obj, ev);
    auto* w = qobject_cast<QWidget*>(obj);
    if (!w) return false;
    QPainter p(w);
    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, w);  // QSS 背景（含 hover）
    static QSvgRenderer svg{QStringLiteral(":/icons/arrow_right.svg")};
    constexpr qreal kSide = 14.0;
    svg.render(&p, QRectF((w->width() - kSide) / 2.0, (w->height() - kSide) / 2.0, kSide, kSide));
    return true;  // 自己画完，吃掉默认那次空白绘制
  }
};

// 批量提交 / 删除结果里单个目标的记录——成功、跳过、失败全都进这里。
struct CommitReportRow {
  QString category;   // "成功" / "跳过" / "失败"
  QString path;       // 完整路径 / 注册表键
  QString errorCode;  // "0x80041001" 之类，成功为 "-"
  QString reason;     // 面向普通用户的解释，成功为 "-"
  // 仅删除操作填充：执行前 / 后目标是否存在。提交操作留 nullopt，结果表不显示这两列。
  std::optional<bool> existedBefore;
  std::optional<bool> existsAfter;
};

// 一次注册表递归提交 / 删除批处理里的单个目标。
struct RegCommitTarget {
  std::string key;        // 归一化后的长写键路径
  std::string valueName;  // 值名；提交时空串 = (Default) 值，删除时空串 = 该键本身
  QString display;        // 报告对话框 "路径" 列的展示串
};

// 把 HRESULT / UWF returnValue 翻译成普通用户看得懂的一句话。
// 不在这里暴露 "WBEM_E_*" / "ExecMethod" 这些实现术语——那些进日志。
// isDeletion 区分 Commit{File,Registry}（=false）vs Commit{File,Registry}Deletion
// （=true）——两类操作下同一个 HRESULT 含义不同，文案得分别写：
//   * NotFound 对 Commit：overlay 里没有该目标的待提交改动。
//   * NotFound 对 Deletion：目标不在物理盘 / 持久化 hive 上——只活在 overlay 里，
//     重启就没；本来就不需要也无法用提交删除。
QString explainCommitFailure(int32_t hresult, uint32_t returnValue, bool isDeletion) {
  if (hresult != 0) {
    switch (uwf::WmiError(hresult).code()) {
      // WBEM_E_FAILED（0x80041001）是一般性失败码，没有确定原因。实测最常见
      // 的一种诱因：目标被其他进程占着 handle（例如资源管理器正在浏览该目录、
      // 文本编辑器打开了文件、regedit 打开了键、AV 在扫描），UWF 看到非
      // 0 引用计数就拒。措辞要带"可能"避免误导——其它原因（权限、UWF 内部
      // 状态等）也会落到同一个 HRESULT，无法仅凭代码区分。
      case uwf::WmiErrorCode::Failed:
        return I18n::tr(
            "The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any "
            "program holding it and try again.");
      case uwf::WmiErrorCode::NotFound:
        if (isDeletion) {
          return I18n::tr(
              "Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It "
              "will disappear on reboot; commit-delete is neither needed nor possible.");
        }
        return I18n::tr("The target was not found; there is nothing in the overlay to commit.");
      case uwf::WmiErrorCode::InvalidParameter:
        return I18n::tr("A parameter was rejected by the system (invalid path or argument).");
      default:
        return I18n::tr("The operation failed (see log for details).");
    }
  }
  if (returnValue != 0) return I18n::tr("Operation rejected (code %1).").arg(returnValue);
  return I18n::tr("Unknown cause.");
}

QString formatErrorCode(int32_t hresult, uint32_t returnValue) {
  // %08X：大写 8 位定宽十六进制；显示 "0x" 小写前缀更符合 Win32 文档惯例。
  if (hresult != 0) return QString::asprintf("0x%08X", static_cast<uint32_t>(hresult));
  if (returnValue != 0) return QString("rv=%1").arg(returnValue);
  return "-";
}

// 结果对话框：每次提交 / 删除后都弹出，无论成功失败都列出全部目标。
// 列：类别 / 路径 / [删除操作额外的"执行前存在""执行后存在"] / 错误码 / 原因。
// 大批量（十万条级）分页展示，每页 kReportPageSize 行。用普通 QDialog 而非
// QMessageBox，避免 Windows 提示音。
void showCommitReport(QWidget* parent, const QList<CommitReportRow>& rows, int canceledRemaining = 0) {
  constexpr int kReportPageSize = 200;

  // 删除操作才填了"执行前 / 后是否存在"；提交操作为 nullopt，那两列不出现。
  // 提前算：决定要不要多两列、影响默认窗宽。下方表头处复用同一份判定。
  const bool showExistence = !rows.isEmpty() && rows.front().existedBefore.has_value();

  QDialog dlg(parent);
  dlg.setWindowTitle(canceledRemaining > 0 ? I18n::tr("Commit canceled") : I18n::tr("Commit result"));
  dlg.resize(showExistence ? 1300 : 1180, 520);
  auto* lay = new QVBoxLayout(&dlg);

  const QString okLabel = I18n::tr("Succeeded");
  const QString skipLabel = I18n::tr("Skipped");
  int okN = 0, skipN = 0, failN = 0;
  for (const auto& r : rows) {
    if (r.category == okLabel)
      ++okN;
    else if (r.category == skipLabel)
      ++skipN;
    else
      ++failN;
  }
  QString summaryText = I18n::tr("%1 succeeded; %2 skipped; %3 failed.").arg(okN).arg(skipN).arg(failN);
  if (canceledRemaining > 0) summaryText += I18n::tr("\nCanceled by user; %1 entries not processed.").arg(canceledRemaining);
  auto* summary = new QLabel(summaryText);
  summary->setTextInteractionFlags(Qt::TextSelectableByMouse);
  lay->addWidget(summary);

  QStringList headers{I18n::tr("Category"), I18n::tr("Path")};
  if (showExistence) headers << I18n::tr("Existed before") << I18n::tr("Exists after");
  headers << I18n::tr("Error code") << I18n::tr("Reason");
  const int colCount = static_cast<int>(headers.size());

  auto* table = new QTableWidget(0, colCount, &dlg);
  table->setHorizontalHeaderLabels(headers);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  table->setTextElideMode(Qt::ElideMiddle);
  table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  auto* hh = table->horizontalHeader();
  // 全部 Interactive：QHeaderView 文档明确 Stretch / ResizeToContents 都禁止用户拖动
  // 列宽；只有 Interactive / Fixed 让用户拖。给每列一个合理初值（按内容典型宽度），
  // 路径列默认给得最宽。Reason 不能再用 ResizeToContents——它有时是一长串中文
  // （"目标可能正被其他程序占用…"）会把整张表撑爆，挤掉路径列变成 "C..."。
  // 横向滚动模式已开（ScrollPerPixel），列宽总和 > 表宽时用户可以横向滚动看全。
  hh->setSectionResizeMode(QHeaderView::Interactive);
  hh->setStretchLastSection(false);
  int initialCol = 0;
  hh->resizeSection(initialCol++, 70);   // Category：「成功」/「跳过」/「失败」
  hh->resizeSection(initialCol++, 520);  // Path：大头——典型 Windows 路径
  if (showExistence) {
    hh->resizeSection(initialCol++, 100);  // Existed before：「是」/「否」+ 表头宽度
    hh->resizeSection(initialCol++, 100);  // Exists after
  }
  hh->resizeSection(initialCol++, 120);  // Error code：「0x80041001」
  hh->resizeSection(initialCol++, 360);  // Reason：可能很长，给 360 起；用户可拖宽
  lay->addWidget(table, 1);

  const QString yes = I18n::tr("Yes");
  const QString no = I18n::tr("No");
  const int pageCount = rows.isEmpty() ? 1 : (static_cast<int>(rows.size()) + kReportPageSize - 1) / kReportPageSize;

  auto fillPage = [&](int page) {
    const int start = page * kReportPageSize;
    const int end = std::min<int>(start + kReportPageSize, static_cast<int>(rows.size()));
    table->setRowCount(end - start);
    for (int i = start; i < end; ++i) {
      const auto& r = rows[i];
      const int vr = i - start;
      int c = 0;
      table->setItem(vr, c++, new QTableWidgetItem(r.category));
      auto* pathItem = new QTableWidgetItem(r.path);
      pathItem->setToolTip(r.path);
      table->setItem(vr, c++, pathItem);
      if (showExistence) {
        table->setItem(vr, c++, new QTableWidgetItem(r.existedBefore.value_or(false) ? yes : no));
        table->setItem(vr, c++, new QTableWidgetItem(r.existsAfter.value_or(false) ? yes : no));
      }
      table->setItem(vr, c++, new QTableWidgetItem(r.errorCode));
      auto* reasonItem = new QTableWidgetItem(r.reason);
      reasonItem->setToolTip(r.reason);
      table->setItem(vr, c++, reasonItem);
    }
  };

  // 分页栏常驻显示——即使只有一页也保留，让"分页 + 总条数"这两条信息一直可见，
  // 同时和 LogViewerDialog 的体感一致。按钮按状态置灰，标签固定写"第 X / Y 页 · 共 N 条"。
  int currentPage = 0;
  auto* pageBar = new QHBoxLayout();
  auto* prevBtn = new QPushButton(I18n::tr("Previous page"), &dlg);
  auto* nextBtn = new QPushButton(I18n::tr("Next page"), &dlg);
  auto* pageLabel = new QLabel(&dlg);
  pageBar->addStretch();
  pageBar->addWidget(prevBtn);
  pageBar->addWidget(pageLabel);
  pageBar->addWidget(nextBtn);
  pageBar->addStretch();
  lay->addLayout(pageBar);

  const int totalRows = static_cast<int>(rows.size());
  auto refreshPage = [&]() {
    fillPage(currentPage);
    pageLabel->setText(I18n::tr("Page %1 / %2 · %3 entries total").arg(currentPage + 1).arg(pageCount).arg(totalRows));
    prevBtn->setEnabled(currentPage > 0);
    nextBtn->setEnabled(currentPage + 1 < pageCount);
  };
  QObject::connect(prevBtn, &QPushButton::clicked, &dlg, [&] {
    if (currentPage > 0) {
      --currentPage;
      refreshPage();
    }
  });
  QObject::connect(nextBtn, &QPushButton::clicked, &dlg, [&] {
    if (currentPage + 1 < pageCount) {
      ++currentPage;
      refreshPage();
    }
  });
  refreshPage();

  // "复制全部"复制所有行（不止当前页），直接从 rows 拼 TSV。
  auto allRowsToText = [&] {
    QString out = headers.join('\t');
    for (const auto& r : rows) {
      out += '\n' + r.category + '\t' + r.path;
      if (showExistence) {
        out += '\t';
        out += r.existedBefore.value_or(false) ? yes : no;
        out += '\t';
        out += r.existsAfter.value_or(false) ? yes : no;
      }
      out += '\t' + r.errorCode + '\t' + r.reason;
    }
    return out;
  };

  auto* copyShortcut = new QShortcut(QKeySequence::Copy, table);
  copyShortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(copyShortcut, &QShortcut::activated, table, [table] {
    const auto txt = tableSelectionToText(table);
    if (!txt.isEmpty()) QGuiApplication::clipboard()->setText(txt);
  });
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(table, &QWidget::customContextMenuRequested, table, [table, allRowsToText](const QPoint& pos) {
    QMenu menu;
    auto* copySel = menu.addAction(I18n::tr("Copy selected rows"));
    auto* copyAll = menu.addAction(I18n::tr("Copy all"));
    copySel->setEnabled(!table->selectedRanges().isEmpty());
    QObject::connect(copySel, &QAction::triggered, [table] { QGuiApplication::clipboard()->setText(tableSelectionToText(table)); });
    QObject::connect(copyAll, &QAction::triggered, [allRowsToText] { QGuiApplication::clipboard()->setText(allRowsToText()); });
    menu.exec(table->viewport()->mapToGlobal(pos));
  });

  auto* btns = new QDialogButtonBox;
  const auto* copyAllBtn = btns->addButton(I18n::tr("Copy all"), QDialogButtonBox::ActionRole);
  QObject::connect(copyAllBtn, &QPushButton::clicked, &dlg, [allRowsToText] { QGuiApplication::clipboard()->setText(allRowsToText()); });
  const auto* closeBtn = btns->addButton(I18n::tr("Close"), QDialogButtonBox::AcceptRole);
  QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  lay->addWidget(btns);

  dlg.exec();
}

// 盘符逻辑统一在 uwf::drive（见 src/util/DriveLetter.h）。下面两个 MainWindow
// 内多处复用的函数只是 QString ↔ std::string 的边界适配，不含任何盘符逻辑。
// extractDriveLetter 把 fromPath 区分出的"卷 GUID 路径解析失败"写进日志——
// 调用方只关心拿没拿到盘符，但失败原因值得留痕。
QString extractDriveLetter(const QString& path) {
  std::string err;
  const std::string dl = drive::fromPath(path.toStdString(), &err);
  if (dl.empty() && !err.empty()) UWF_LOG_W("ui") << "extractDriveLetter: " << err;
  return QString::fromStdString(dl);
}
QString systemDriveLetter() { return QString::fromStdString(drive::systemLetter()); }

// RtlGetVersion 是唯一一个在 Windows 8.1+ 上仍返回真实版本号（而不是被
// 应用兼容性"撒谎"成 Windows 8）的接口。动态加载避免对 ntdll 的直接 link。
// 版本型号则从 CurrentVersion\ProductName / EditionID 里取——Windows 11 的
// ProductName 字段至今仍写作 "Windows 10 Xxx"，所以家族名得靠 build 号自己判，
// 型号部分再从 ProductName 去掉前缀拿出来。LTSC 变体 ProductName 里不一定写
// "LTSC" 字样，需要再看 EditionID 末尾有没有 "S"。
QString windowsVersionText() {
  using Fn = LONG(WINAPI*)(OSVERSIONINFOW*);
  auto fn = reinterpret_cast<Fn>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion")));
  OSVERSIONINFOW v{};
  v.dwOSVersionInfoSize = sizeof(v);
  if (!fn || fn(&v) != 0) return QStringLiteral("Windows");

  constexpr std::string_view kCur = config::kRegPathWindowsCurrentVersion;
  const auto ubr = regkey::readDword(kCur, "UBR");
  const QString productName = QString::fromStdString(regkey::readString(kCur, "ProductName")).trimmed();
  const QString editionId = QString::fromStdString(regkey::readString(kCur, "EditionID")).trimmed();

  // 家族名（Windows 10 / 11 共享 Major=10，靠 build ≥ 22000 区分）。
  QString family = QStringLiteral("Windows");
  if (v.dwMajorVersion == 10) {
    family = v.dwBuildNumber >= static_cast<DWORD>(config::kWindows11MinBuildNumber) ? QStringLiteral("Windows 11") : QStringLiteral("Windows 10");
  }

  // 从 ProductName 去掉"Windows 10/11 "前缀拿型号（Pro / Enterprise / Home /
  // ...）。
  QString edition = productName;
  for (const QString& p : {QStringLiteral("Windows 11 "), QStringLiteral("Windows 10 "), QStringLiteral("Windows ")}) {
    if (edition.startsWith(p, Qt::CaseInsensitive)) {
      edition = edition.mid(p.size()).trimmed();
      break;
    }
  }

  // LTSC / LTSB 变体：EditionID = EnterpriseS / EnterpriseSN / IoTEnterpriseS …
  // ProductName 并不总是把 "LTSC" 写出来，需要自己补上。
  const QString ed = editionId.toLower();
  const bool isLtsc =
      std::ranges::any_of(config::kLtscEditionIds, [&ed](std::string_view id) { return ed == QLatin1String(id.data(), static_cast<qsizetype>(id.size())); });
  if (isLtsc && !edition.contains("LTSC", Qt::CaseInsensitive) && !edition.contains("LTSB", Qt::CaseInsensitive)) {
    edition = edition.isEmpty() ? QStringLiteral("LTSC") : (edition + QStringLiteral(" LTSC"));
  }

  QString head = edition.isEmpty() ? family : (family + ' ' + edition);
  return QString("%1 · %2.%3.%4.%5").arg(head).arg(v.dwMajorVersion).arg(v.dwMinorVersion).arg(v.dwBuildNumber).arg(ubr);
}

QString cpuModelText() {
  const QString name =
      QString::fromStdString(regkey::readString(R"(HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0)", "ProcessorNameString"));
  // BIOS 厂商常常在名字里塞大量尾随空格，去一下。
  return name.trimmed().simplified();
}

QString totalRamText() {
  MEMORYSTATUSEX m{};
  m.dwLength = sizeof(m);
  if (!GlobalMemoryStatusEx(&m)) return {};
  // 实际物理内存常常略少于 N GB（给 GPU、硬件保留）。用就近取整的 GB，
  // 失败再退回 MB。例：16777216 KB ≈ 16 GB。
  const auto gb = static_cast<uint64_t>((m.ullTotalPhys + (512ULL << 20)) / (1ULL << 30));
  if (gb >= 1) return QString("%1 GB").arg(gb);
  return QString("%1 MB").arg(m.ullTotalPhys / (1ULL << 20));
}

QString gpuModelText() {
  // EnumDisplayDevices 的 DeviceString 就是设备厂商/型号字符串。
  // 优先选"挂在桌面上"的主适配器，避免拿到 Remote / Mirror 这类假设备。
  DISPLAY_DEVICEW dev{};
  dev.cb = sizeof(dev);
  for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dev, 0); ++i) {
    if (dev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
      return QString::fromWCharArray(dev.DeviceString).trimmed();
    }
    dev = {};
    dev.cb = sizeof(dev);
  }
  dev = {};
  dev.cb = sizeof(dev);
  if (EnumDisplayDevicesW(nullptr, 0, &dev, 0)) {
    return QString::fromWCharArray(dev.DeviceString).trimmed();
  }
  return {};
}

// 紧凑排版：一行一个条目；系统/CPU/显卡本身名字就够识别（"Windows 11"、
// "Intel ..."、"NVIDIA ..."），不再加"系统："之类的 key 标签。
// 只有"XX GB"这种纯数字单位需要 RAM 前缀才知道是内存总量。
QString systemInfoHtml() {
  const QString ver = windowsVersionText();
  const QString cpu = cpuModelText();
  const QString ram = totalRamText();
  const QString gpu = gpuModelText();
  QStringList rows;
  auto addPlain = [&](const QString& v) {
    if (!v.isEmpty()) rows << v.toHtmlEscaped();
  };
  const QString muted = ThemeManager::instance().color(Sem::FgMuted).name();
  auto addWithKey = [&](const QString& k, const QString& v) {
    if (v.isEmpty()) return;
    rows << QString("<span style='color:%1'>%2</span>&nbsp;%3").arg(muted, k.toHtmlEscaped(), v.toHtmlEscaped());
  };
  addPlain(ver);
  addPlain(cpu);
  addWithKey("RAM", ram);
  addPlain(gpu);
  return rows.join("<br>");
}

}  // namespace

MainWindow::MainWindow(bool compatibilityMode, const QString& osProductName, const QString& osEditionId, QWidget* parent)
    : QMainWindow(parent), m_compatibilityMode(compatibilityMode), m_osProductName(osProductName), m_osEditionId(osEditionId) {
  // 构造期摆好窗口外壳（标题 / 图标 / 尺寸），并把窗口设为全透明：窗口会以
  // 透明状态 show 出来——showEvent 照常触发，首屏 rebuildUi() 在 shown 状态下
  // 建好全部内容、拉完数据后才把不透明度恢复成 1 一次性揭幕。整个 buildUi +
  // refresh 期间窗口不可见，用户不会看到空窗 / 改尺寸 / 白屏等中间态。
  setWindowTitle(I18n::tr("Unified Write Filter (UWF) Manager"));
  setWindowIcon(QIcon(":/icons/app.svg"));
  resize(1380, 760);
  setWindowOpacity(0.0);

  // 写会话提前连接一次；读快照时会另起一个独立会话。
  std::string err;
  m_writeSession.connect(config::kWmiNamespaceEmbedded, &err);
  // 内容控件与首屏数据统一交给 showEvent 调度的 rebuildUi()——它一次 buildUi()
  // + refresh() 建好。构造期不再 buildUi()/refresh()：那份产出会被 rebuildUi
  // 整个销毁重建，等于白建一遍 UI、白连一次 WMI、白读一份快照。

  // 系统托盘（图标 + 右键菜单）——独立组件，由本窗口编排：接它的"激活窗口"信号。
  m_tray = new TrayController(m_writeSession, this);
  connect(m_tray, &TrayController::activateWindowRequested, this, &MainWindow::raiseToFront);

  // 每 5s 周期刷新 Usage 数据（占用条）——只读 UWF_Overlay，不做整体 refresh。
  m_usageTimer = new QTimer(this);
  m_usageTimer->setInterval(5000);
  connect(m_usageTimer, &QTimer::timeout, this, &MainWindow::refreshUsage);
  m_usageTimer->start();
  // 首屏 rebuildUi 不在 ctor 里同步触发——widget 此时还没 show，Qt 一些 polish
  // / 几何计算在 widget 真正进入 shown 状态前结果不稳定，会跟后续"切主题 /
  // 切语言时已 shown 状态下的 rebuildUi"产生差异。改放到 showEvent 第一次
  // 触发后用 singleShot 调度，确保首次 rebuild 也在 shown 状态下跑。
}

void MainWindow::raiseToFront() {
  // 最小化时先恢复；否则确保可见。再 raise + activate 抢前台——配合启动方
  // 进程调用的 AllowSetForegroundWindow，能真正前置而非只闪任务栏。
  if (isMinimized())
    showNormal();
  else
    show();
  raise();
  activateWindow();
}

void MainWindow::refreshUsage() {
  // 周期更新只动 Usage 数据：主窗口可见时刷新主面板占用条；托盘那半段交给
  // TrayController（它内部判断右键菜单是否正在显示）。
  if (isVisible() && m_global) {
    if (const auto overlay = m_overlay.read()) {
      core::OverlayRuntime rt;
      rt.currentConsumptionMb = overlay->overlayConsumption;
      rt.availableSpaceMb = overlay->availableSpace;
      m_global->updateUsage(rt);
    }
  }
  if (m_tray) m_tray->refreshUsage();
}

void MainWindow::buildUi() {
  // 标题随语言切换重译，故每次 buildUi（含 rebuildUi 路径）都重设一次；
  // 图标与初始尺寸是一次性窗口外壳设置，已在构造函数里完成，这里不再重复。
  setWindowTitle(I18n::tr("Unified Write Filter (UWF) Manager"));

  // QSS 的 `padding` 在 QToolBar 上不可靠（rebuildUi 后 polish 时机问题），
  // 用 spacer widget 给水平方向兜底。垂直方向不再人为加 margin——
  // setContentsMargins 在 QToolBarLayout 上行为不一致（top margin 部分被
  // QToolBarLayout 默认的 AlignTop 吞掉，bottom 算进总高度但 items 不下沉，
  // 导致底部出现空白条）。直接让 toolbar 高度 = button 高度，靠 button 自己
  // 的 QSS padding (`padding: 7px 16px 9px 16px` + `min-height: 22px`) 提供
  // 内部 breathing room，toolbar 自然紧贴。
  auto* tb = new QToolBar(I18n::tr("Main toolbar"), this);
  tb->setObjectName("mainToolbar");
  tb->setMovable(false);
  tb->setIconSize({16, 16});
  tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  addToolBar(tb);

  // 左侧 padding：固定 12px 宽 spacer widget。Qt 要把 widget 加到 QToolBar
  // 必须走 addWidget——addAction 只接 QAction。
  {
    auto* leftPad = new QWidget(tb);
    leftPad->setFixedWidth(12);
    tb->addWidget(leftPad);
  }

  m_actImport = tb->addAction(I18n::tr("Import"));
  m_actImport->setToolTip(
      I18n::tr("Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you "
               "click \"Review and apply\"."));
  connect(m_actImport, &QAction::triggered, this, &MainWindow::showImport);

  m_actRefresh = tb->addAction(I18n::tr("Refresh"));
  m_actRefresh->setShortcut(QKeySequence::Refresh);
  m_actRefresh->setToolTip(I18n::tr("Re-read the current session state and next-session configuration of UWF."));
  connect(m_actRefresh, &QAction::triggered, this, &MainWindow::refresh);

  m_actPlan = tb->addAction(I18n::tr("Review and apply"));
  m_actPlan->setToolTip(I18n::tr("Review all pending changes and apply them in one batch. Most changes take effect after the next reboot."));
  connect(m_actPlan, &QAction::triggered, this, &MainWindow::showPlan);

  tb->addSeparator();

  m_actShutdown = tb->addAction(I18n::tr("Safe shutdown"));
  m_actShutdown->setToolTip(I18n::tr("Shut down safely, even when the UWF overlay is full."));
  connect(m_actShutdown, &QAction::triggered, this, [this]() { safeShutdown(); });

  m_actRestart = tb->addAction(I18n::tr("Safe restart"));
  m_actRestart->setToolTip(I18n::tr("Restart safely, even when the UWF overlay is full."));
  connect(m_actRestart, &QAction::triggered, this, [this]() { safeRestart(); });

  tb->addSeparator();

  m_actLog = tb->addAction(I18n::tr("Log"));
  m_actLog->setToolTip(I18n::tr("View the internal log accumulated during this session, for troubleshooting."));
  connect(m_actLog, &QAction::triggered, this, &MainWindow::showLogs);

  m_actAbout = tb->addAction(I18n::tr("About"));
  m_actAbout->setToolTip(I18n::tr("About this program."));
  connect(m_actAbout, &QAction::triggered, this, &MainWindow::showAbout);

  // QToolBar 没有原生"右对齐"分组——通过塞一个横向 Expanding 的 spacer
  // QWidget 把后续 action 推到最右。
  {
    auto* spacer = new QWidget(tb);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);
  }

  // 语言切换按钮（IconOnly，下拉菜单展示支持的语言）。
  m_actLang = tb->addAction("");
  m_actLang->setToolTip(I18n::tr("Switch display language"));
  if (auto* btn = qobject_cast<QToolButton*>(tb->widgetForAction(m_actLang))) {
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setPopupMode(QToolButton::InstantPopup);
    // 去掉 InstantPopup 默认的小下拉箭头——纯 icon 按钮更干净。
    btn->setStyleSheet("QToolButton::menu-indicator{image:none;width:0}");
    // 把 menu 的 parent 设成 btn——btn 是 toolbar 的子项，toolbar 在
    // rebuildUi 时被 deleteLater，menu 跟着一起释放，避免每次重建留下
    // 一个孤立 QMenu 挂在 MainWindow 下。
    auto* menu = new QMenu(btn);
    // QActionGroup 让两个语言项互斥单选；语言名一律用各自语言的本地写法
    // (English / 简体中文)，不参与翻译。
    auto* langGroup = new QActionGroup(menu);
    langGroup->setExclusive(true);

    // 切换语言后用 QTimer::singleShot(0, rebuildUi) 异步触发整体重建：
    // 不能在 action 的 triggered 回调里直接 rebuildUi——rebuildUi 会删掉
    // 当前 toolbar 及其上的 QAction，正在执行的回调随即变成悬空指针。
    // 单次延迟到事件循环下一轮再做，让回调先安全返回。
    auto switchTo = [this](const I18n::Lang target) {
      if (I18n::instance().lang() == target) return;
      // 抑制 paint 直到 rebuildUi 结束，避免用户看到 tear-down 中间空白
      // 一帧。配对的 setUpdatesEnabled(true) 在 rebuildUi 末尾。
      setUpdatesEnabled(false);
      I18n::instance().setLang(target);
      QTimer::singleShot(0, this, &MainWindow::rebuildUi);
    };

    auto* enAct = menu->addAction("English");
    enAct->setCheckable(true);
    enAct->setActionGroup(langGroup);
    enAct->setChecked(I18n::instance().lang() == I18n::Lang::En);
    connect(enAct, &QAction::triggered, this, [switchTo]() { switchTo(I18n::Lang::En); });

    auto* zhAct = menu->addAction("简体中文");
    zhAct->setCheckable(true);
    zhAct->setActionGroup(langGroup);
    zhAct->setChecked(I18n::instance().lang() == I18n::Lang::Zh_CN);
    connect(zhAct, &QAction::triggered, this, [switchTo]() { switchTo(I18n::Lang::Zh_CN); });

    btn->setMenu(menu);
  }

  m_actTheme = tb->addAction("");
  m_actTheme->setToolTip(I18n::tr("Toggle light / dark theme. Follows the system setting on startup."));
  // 在 toggle() 之前禁 paint：toggle 内部会 setPalette + setStyleSheet("") +
  // setStyleSheet(sheet)，中间 unstyled 那一帧会闪。先压 paint 再切，再让
  // 后续 themeChanged → singleShot → rebuildUi → setUpdatesEnabled(true)
  // 一次性把最终态画出来。
  connect(m_actTheme, &QAction::triggered, this, [this]() {
    setUpdatesEnabled(false);
    ThemeManager::instance().toggle();
  });
  // 主题按钮只显示 icon、不显示文字（与其他 toolbar action 不同），
  // 单独把它的 button style 改成 IconOnly。
  if (auto* btn = qobject_cast<QToolButton*>(tb->widgetForAction(m_actTheme))) {
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
  }

  // 右侧 padding：跟左边对称，固定 12px。放在最后让它真的贴最右——之前
  // 的 Expanding spacer 只把 lang/theme 推到右半部分，这个 padding 才是
  // 让最末按钮和窗口右边沿之间留 12px 空白。
  {
    auto* rightPad = new QWidget(tb);
    rightPad->setFixedWidth(12);
    tb->addWidget(rightPad);
  }

  // 给工具栏溢出扩展按钮装上雪佛龙绘制器（见 ToolbarExtIcon 注释）。过滤器对象
  // parent 设为按钮本身——rebuildUi 重建 toolbar 时随按钮一并回收。
  if (auto* ext = tb->findChild<QToolButton*>(QStringLiteral("qt_toolbar_ext_button"))) {
    ext->installEventFilter(new ToolbarExtIcon(ext));
  }

  // 主题切换走和语言切换完全相同的入口：rebuildUi 整体重建 toolbar + 中央
  // widget。两套刷新走同一条路径，避免之前"主题切换只刷 icon、语言切换全
  // 重建"两套机制各自的几何跳变。代价跟语言切换一致——会丢 widget 状态里
  // 的 pending changes，所以切主题前应已 apply 过待应用变更。
  // QTimer::singleShot(0, ...) 把 rebuild 推到下一轮事件循环，让发出
  // themeChanged 信号的 ThemeManager::apply 先安全返回。
  // setUpdatesEnabled(false) 已在主题按钮 click handler 里调过（在 toggle()
  // 之前），这里不再重复。配对的 setUpdatesEnabled(true) 在 rebuildUi 末尾。
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { QTimer::singleShot(0, this, &MainWindow::rebuildUi); });

  auto* central = new QWidget(this);
  auto* centralLayout = new QVBoxLayout(central);
  centralLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->setSpacing(0);

  auto* mainRow = new QHBoxLayout();
  mainRow->setContentsMargins(0, 0, 0, 0);
  mainRow->setSpacing(0);

  m_tabs = new QTabWidget(this);
  m_tabs->setObjectName("mainTabs");
  m_tabs->setDocumentMode(true);
  m_tabs->setTabPosition(QTabWidget::North);
  m_tabs->setMinimumWidth(220);
  m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mainRow->addWidget(m_tabs, 1);

  auto* globalWrap = new QWidget(this);
  // 纯 QWidget 默认不会应用 QSS 的 background；必须打开 WA_StyledBackground，
  // 否则 QWidget#globalWrap
  // 规则里的背景色根本不生效，看上去就和主窗口底色有差。
  globalWrap->setAttribute(Qt::WA_StyledBackground, true);
  auto* globalLayout = new QVBoxLayout(globalWrap);
  globalLayout->setContentsMargins(14, 12, 14, 12);
  globalLayout->setSpacing(10);
  m_global = new GlobalStatusPanel(this);
  // 系统版本未通过校验时，把兼容模式提示常驻在面板信息框里。提示文案在此
  // 现翻译——切语言会重跑 buildUi，文案随之跟着切；rebuildUi 重建 m_global
  // 后也连带重新灌入。
  if (m_compatibilityMode) {
    m_global->setCompatibilityNotice(
        I18n::tr("The current system \"%1\" (%2) is not a recognized supported edition. UWF Manager is running in compatibility mode "
                 "and some features may be unavailable.")
            .arg(m_osProductName, m_osEditionId));
  }
  // 顶部全局设置拿走所有可拉伸空间，里面的 QScrollArea 会在高度不足时
  // 自己滚动；tips 区用固定高度贴底。
  globalLayout->addWidget(m_global, 1);
  m_hoverHint = new QLabel(this);
  m_hoverHint->setObjectName("hoverHintBox");
  m_hoverHint->setWordWrap(true);
  m_hoverHint->setTextInteractionFlags(Qt::NoTextInteraction);
  m_hoverHint->setFixedHeight(110);
  m_hoverHint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_hoverHint->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  // 默认文案换成机器基本信息（OS / CPU / RAM / GPU），悬停事件会临时覆盖。
  // 留 AutoText：默认文案里有 HTML 标签会按 RichText 渲染；
  // 普通 tooltip 是纯文本则按 PlainText 渲染，不怕里面的 & / < 被解析走样。
  m_hoverHintDefault = systemInfoHtml();
  m_hoverHint->setText(m_hoverHintDefault);
  globalLayout->addWidget(m_hoverHint, 0);
  globalWrap->setObjectName("globalWrap");
  globalWrap->setFixedWidth(420);
  // 右侧面板整体最小高度：GlobalStatusPanel 自身需要约 360 来容纳筛选器
  // 和覆盖层两张卡片；加上 tips 固定 110 + 内边距。这样即使把主窗口拖到
  // 很矮，这两块也都能完整看见。
  globalWrap->setMinimumHeight(360 + 110 + 24);
  mainRow->addWidget(globalWrap, 0);

  // 让 MainWindow 也跟随到这个最小高度（加上工具栏和状态栏的大致高度）。
  setMinimumHeight(globalWrap->minimumHeight() + 80);

  qApp->installEventFilter(this);

  centralLayout->addLayout(mainRow, 1);
  setCentralWidget(central);

  connect(m_global, &GlobalStatusPanel::pendingChanged, this, &MainWindow::updatePendingSummary);

  m_statusText = new QLabel(this);
  m_statusText->setObjectName("statusBarLabel");
  statusBar()->addPermanentWidget(m_statusText, 1);

  // 临时提示通过一个 singleShot QTimer 覆盖 m_statusText，
  // 到期后恢复 baseline（来自 refresh / updatePendingSummary 的常驻文案）。
  // 必须走这条路径，因为 statusBar()->showMessage() 被 stretch=1 的
  // permanent widget 挤没空间，显示不出来。
  m_hintTimer = new QTimer(this);
  m_hintTimer->setSingleShot(true);
  connect(m_hintTimer, &QTimer::timeout, this, [this]() { m_statusText->setText(m_statusBaseline); });

  // 鼠标离开控件后的"短延迟回到默认提示"定时器 —— 延迟是为了避免光标在
  // 相邻控件之间移动时文字先变空再变回去造成的闪烁。
  m_hoverClearTimer = new QTimer(this);
  m_hoverClearTimer->setSingleShot(true);
  m_hoverClearTimer->setInterval(120);
  connect(m_hoverClearTimer, &QTimer::timeout, this, [this]() {
    if (m_hoverHint) m_hoverHint->setText(m_hoverHintDefault);
  });

  // buildUi 末尾统一应用一次主题：toolbar 图标、disk tab 图标、hover hint
  // 默认文案都按当前主题色生成（构造期间 connect 时 m_hoverHint 还是 null，
  // 这里补一次初始化）。
  refreshThemedUi();
}

void MainWindow::rebuildUi() {
  // 切换语言 / 切换主题统一入口：把 buildUi 创建的 toolbar / central widget /
  // 状态栏 permanent widget / 两个 timer 全部清掉，重置成员指针，再
  // buildUi + refresh 重新拉数据。WMI 会话和快照保留——它们和 UI 主题 / 语言
  // 都无关。
  //
  // pending changes 会随 m_global / m_diskTabs 一起销毁；切换主题 / 语言都是
  // 低频操作，不值得为了保留 pending state 而维护两套刷新机制。

  // ThemeManager 是单例，buildUi 每次都新增一个 themeChanged 连接，重建前
  // 必须先把上一轮的连接拆掉，否则刷主题时回调会被多次触发。
  disconnect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, nullptr);

  if (auto* tb = findChild<QToolBar*>("mainToolbar")) {
    removeToolBar(tb);
    // 同步 delete 而不是 deleteLater——否则旧 toolbar 还挂在 MainWindow 子
    // 对象列表里到下一个事件循环 tick 才回收，期间如果 buildUi 又通过
    // findChild<QToolBar*>("mainToolbar") 找一次，会拿到旧实例。我们走的是
    // QTimer::singleShot(0) 路径调进来，已经脱离 action 触发的回调，同步
    // delete 安全。
    delete tb;
  }
  if (auto* central = takeCentralWidget()) {
    central->deleteLater();
  }
  if (m_statusText) {
    statusBar()->removeWidget(m_statusText);
    m_statusText->deleteLater();
  }
  if (m_hintTimer) m_hintTimer->deleteLater();
  if (m_hoverClearTimer) m_hoverClearTimer->deleteLater();

  // 重置所有指针成员；buildUi 会重新填充。
  m_actRefresh = m_actImport = m_actPlan = m_actShutdown = m_actRestart = nullptr;
  m_actLog = m_actAbout = m_actLang = m_actTheme = nullptr;
  m_tabs = nullptr;
  m_global = nullptr;
  m_hoverHint = nullptr;
  m_statusText = nullptr;
  m_hintTimer = m_hoverClearTimer = nullptr;
  m_diskTabs.clear();
  m_statusBaseline.clear();
  m_hoverHintDefault.clear();

  buildUi();
  refresh();

  // 配对的 setUpdatesEnabled(true)。切主题 / 切语言两条入口在调度 rebuildUi
  // 前会先 setUpdatesEnabled(false)，把中间过渡态（unstyled / 空白 / 重新
  // layout）的 paint 攒起来不画，这里画完最终态再统一放出来——只有一帧旧到
  // 新的硬切。首屏 showEvent 不压帧（本就 enabled），这里是 no-op，安全。
  setUpdatesEnabled(true);

  // 揭幕前先把挂起的布局请求与绘制同步跑完——确保恢复不透明的那一刻窗口
  // 已是完整绘制好的最终形态，不会闪过一帧尚未绘制完的空窗。
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  repaint();

  // 揭幕：首屏时窗口自构造起就是全透明的（opacity 0），此刻内容 / 数据 / 绘制
  // 均已就位，恢复成不透明 —— 窗口一次性以完整形态出现，没有任何中间态。
  // 切主题 / 切语言时窗口本就不透明，这里是 no-op。
  setWindowOpacity(1.0);
}

void MainWindow::showTransientHint(const QString& text, const int msec) const {
  m_statusText->setText(text);
  m_hintTimer->start(msec);
}

void MainWindow::showEvent(QShowEvent* ev) {
  QMainWindow::showEvent(ev);
  if (!m_firstShowDone) {
    m_firstShowDone = true;
    // 首次 show（此时窗口是全透明的）后立刻调度一次 rebuildUi——和"切主题 /
    // 切语言"走完全相同的重建路径，首屏最终形态在 shown 状态下 polish。
    // rebuildUi 建完内容、拉完数据后会把不透明度恢复成 1 揭幕。
    QTimer::singleShot(0, this, &MainWindow::rebuildUi);
  }
}

void MainWindow::refreshThemedUi() {
  auto& tm = ThemeManager::instance();
  if (m_actImport) m_actImport->setIcon(tm.icon(":/icons/add.svg"));
  if (m_actRefresh) m_actRefresh->setIcon(tm.icon(":/icons/refresh.svg"));
  if (m_actPlan) m_actPlan->setIcon(tm.icon(":/icons/apply.svg"));
  if (m_actShutdown) m_actShutdown->setIcon(tm.icon(":/icons/shutdown.svg"));
  if (m_actRestart) m_actRestart->setIcon(tm.icon(":/icons/restart.svg"));
  if (m_actLog) m_actLog->setIcon(tm.icon(":/icons/log.svg"));
  if (m_actAbout) m_actAbout->setIcon(tm.icon(":/icons/info.svg"));
  if (m_actLang) m_actLang->setIcon(tm.icon(":/icons/language.svg"));
  if (m_actTheme) {
    // 当前 dark → 显示太阳图标（点了切到 light）；当前 light → 显示月亮。
    const bool isDark = tm.current() == Theme::Dark;
    m_actTheme->setIcon(tm.icon(isDark ? ":/icons/theme_sun.svg" : ":/icons/theme_moon.svg"));
  }
  // 顺手刷一遍磁盘 TAB 上的 icon —— DiskTab 自己会处理内部的 commit / TAB icon。
  if (m_tabs) {
    const QString sysDl = systemDriveLetter();
    for (auto& t : m_diskTabs) {
      if (!t) continue;
      const int idx = m_tabs->indexOf(t);
      if (idx < 0) continue;
      const bool ok = t->supported();
      const bool isSys = t->driveLetter().toUpper() == sysDl;
      const QIcon ic = !ok ? tm.icon(":/icons/disk_off.svg") : isSys ? tm.icon(":/icons/disk_system.svg") : tm.icon(":/icons/disk.svg");
      m_tabs->setTabIcon(idx, ic);
    }
  }
  // hoverHint 默认 HTML 含 inline color，主题切换后重新生成。
  m_hoverHintDefault = systemInfoHtml();
  if (m_hoverHint && (!m_hintTimer || !m_hintTimer->isActive())) {
    m_hoverHint->setText(m_hoverHintDefault);
  }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev) {
  // hover 到任意带 hoverHint 属性（或 toolTip）的控件，就把说明塞到右侧面板的
  // 提示框里；离开时清空。走 qApp 级事件过滤器才能捕获所有子控件。
  if (!m_hoverHint) return QMainWindow::eventFilter(obj, ev);
  const auto type = ev->type();
  // 屏蔽原生 tooltip 气泡：截停 ToolTip 事件，说明文字只在右下角面板里。
  if (type == QEvent::ToolTip) return true;

  // QMenu 上的 QAction 不是 QWidget——下面的 parent-toolTip 链拿不到它；落到
  // QMenu 自身上又只会走出菜单按钮的 toolTip（不是当前悬停的那一项）。这里专门
  // 拦菜单的悬停事件：用 menu->activeAction() 反查当前选中项，把它显式设置过的
  // toolTip 推到提示框。判定"显式设置"= toolTip() != text()——Qt 默认把未设置的
  // toolTip 回落到 text()，相同时把同一句话再推一次没有价值。
  if (auto* menu = qobject_cast<QMenu*>(obj)) {
    if (type == QEvent::MouseMove || type == QEvent::HoverMove || type == QEvent::Enter || type == QEvent::HoverEnter) {
      const QAction* act = menu->activeAction();
      const QString tip = act ? act->toolTip() : QString();
      if (act && !tip.isEmpty() && tip != act->text()) {
        if (m_hoverClearTimer) m_hoverClearTimer->stop();
        m_hoverHint->setText(tip);
      } else if (m_hoverClearTimer) {
        // 当前项没有专门 toolTip：让面板回到默认（用 clear timer 抗闪烁）。
        m_hoverClearTimer->start();
      }
    } else if (type == QEvent::Leave || type == QEvent::HoverLeave || type == QEvent::Hide) {
      if (m_hoverClearTimer) m_hoverClearTimer->start();
    }
    return QMainWindow::eventFilter(obj, ev);
  }

  if (type == QEvent::Enter || type == QEvent::HoverEnter || type == QEvent::MouseMove || type == QEvent::HoverMove) {
    auto* w = qobject_cast<QWidget*>(obj);
    if (!w) return QMainWindow::eventFilter(obj, ev);
    // QTabBar 需要按坐标查出当前悬停在哪个 tab 上，再取对应的 tooltip。
    if (auto* bar = qobject_cast<QTabBar*>(w)) {
      QPoint pos;
      if (auto* me = dynamic_cast<QMouseEvent*>(ev))
        pos = me->pos();
      else if (auto* he = dynamic_cast<QHoverEvent*>(ev))
        pos = he->position().toPoint();
      else
        pos = bar->mapFromGlobal(QCursor::pos());
      const int idx = bar->tabAt(pos);
      if (idx >= 0) {
        const QString tip = bar->tabToolTip(idx);
        if (!tip.isEmpty()) {
          if (m_hoverClearTimer) m_hoverClearTimer->stop();
          m_hoverHint->setText(tip);
          return QMainWindow::eventFilter(obj, ev);
        }
      }
    }
    QWidget* cur = w;
    while (cur && cur->toolTip().isEmpty() && cur != this) cur = cur->parentWidget();
    if (cur && !cur->toolTip().isEmpty()) {
      if (m_hoverClearTimer) m_hoverClearTimer->stop();
      m_hoverHint->setText(cur->toolTip());
    }
  } else if (type == QEvent::Leave || type == QEvent::HoverLeave) {
    // 离开时延迟恢复默认说明；如果马上移到另一个带 tooltip 的控件上，
    // 下一次 Enter 会 stop() 这个定时器，避免文字反复闪一下。
    if (m_hoverClearTimer) m_hoverClearTimer->start();
  }
  return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::updatePendingSummary() {
  qsizetype pending = 0;
  if (m_global->pendingFilterEnabled()) ++pending;
  {
    const auto [type, maximumSizeMb, warningThresholdMb, criticalThresholdMb] = m_global->pendingOverlay();
    if (type) ++pending;
    if (maximumSizeMb) ++pending;
    if (warningThresholdMb) ++pending;
    if (criticalThresholdMb) ++pending;
  }
  for (const auto& t : m_diskTabs) {
    if (!t) continue;
    if (t->pendingVolumeProtected()) ++pending;
    if (t->pendingBindByVolumeName()) ++pending;
    pending += t->pendingFileAdded().size() + t->pendingFileRemoved().size() + t->pendingRegAdded().size() + t->pendingRegRemoved().size() +
               (t->pendingPersistDomainSecretKey().has_value() ? 1 : 0) + (t->pendingPersistTSCAL().has_value() ? 1 : 0);
  }
  const QString msg = pending > 0 ? I18n::tr("%1 pending change(s) (not yet written to the system)").arg(pending) : I18n::tr("No pending changes");
  m_statusBaseline = msg;
  if (!m_hintTimer->isActive()) m_statusText->setText(m_statusBaseline);
}

void MainWindow::rebuildTabs(const std::vector<core::DiskInfo>& disks) {
  // 重建前按盘符记下两件事，重建后尽量还原——避免 refresh 把用户的"上下文"
  // 都跳走：
  //   1) 当前选中的卷（外层 TAB 的盘符）；
  //   2) 每个 DiskTab 内层"文件 / 注册表排除"TAB 的索引（仅系统盘有 1=注册表）。
  // 内层索引用 int 而不是 tabText，因为 text 受 i18n 影响（切语言后不稳定）；
  // 索引在所有语言下都稳定。
  const QString prevDriveLetter = m_tabs->currentIndex() >= 0 ? m_tabs->tabText(m_tabs->currentIndex()) : QString();
  QMap<QString, int> prevInfoTab;
  for (const auto& t : m_diskTabs)
    if (t) prevInfoTab.insert(t->driveLetter(), t->activeInfoTabIndex());

  // QTabWidget::clear() 只摘掉标签页、不销毁页面控件——上一轮的 DiskTab 会继续
  // 作为 m_tabs 的子对象存活，每次 refresh 泄漏一组（要到下次 rebuildUi 删掉
  // m_tabs 才被连带回收）。这里先显式 deleteLater 回收旧的一组。用 deleteLater
  // 而非 delete：rebuildTabs 可能经由某个 DiskTab 自己的信号回调间接调进来，
  // 同步 delete 会销毁正在执行回调的对象。
  for (const auto& t : m_diskTabs)
    if (t) t->deleteLater();
  m_tabs->clear();
  m_diskTabs.clear();
  const QString sysDl = systemDriveLetter();
  for (const auto& d : disks) {
    auto* tab = new DiskTab(d, this);
    const QString label = QString::fromStdString(d.driveLetter);
    const bool ok = d.support == core::DiskSupport::Supported;
    const bool isSys = QString::fromStdString(d.driveLetter).toUpper() == sysDl;
    auto& tm = ThemeManager::instance();
    const QIcon icon = !ok ? tm.icon(":/icons/disk_off.svg") : isSys ? tm.icon(":/icons/disk_system.svg") : tm.icon(":/icons/disk.svg");
    const int idx = m_tabs->addTab(tab, icon, label);
    if (!ok) {
      const std::string reason = diskSupportText(d.support, d.fileSystem);
      m_tabs->setTabToolTip(idx, QString::fromStdString(reason));
    } else {
      const QString sysExtra = isSys ? I18n::tr(" (System drive: also manages the global registry exclusion list here.)") : QString();
      m_tabs->setTabToolTip(idx, I18n::tr("Switch to protection settings and file exclusions for volume %1.%2").arg(label, sysExtra));
    }
    // 还原本卷内层 TAB 的选中索引。原本不在（磁盘新插入）→ 保持默认 0。
    if (const auto it = prevInfoTab.constFind(label); it != prevInfoTab.constEnd()) {
      tab->setActiveInfoTabIndex(it.value());
    }
    m_diskTabs.push_back(tab);
    connect(tab, &DiskTab::pendingChanged, this, &MainWindow::updatePendingSummary);
    connect(tab, &DiskTab::statusHint, this, &MainWindow::showTransientHint);
    connect(tab, &DiskTab::commitFileRequested, this, &MainWindow::commitFilePath);
    connect(tab, &DiskTab::commitFileDeletionRequested, this, &MainWindow::commitFileDeletionPath);
    connect(tab, &DiskTab::commitRegistryRequested, this, &MainWindow::commitRegistryKey);
    connect(tab, &DiskTab::commitRegistryDeletionRequested, this, &MainWindow::commitRegistryDeletionKey);
  }

  // 切回刷新前选中的卷（按盘符匹配）。该卷若已不在（磁盘被移除）则保持默认第 0 个。
  if (!prevDriveLetter.isEmpty()) {
    for (int i = 0; i < m_tabs->count(); ++i) {
      if (m_tabs->tabText(i) == prevDriveLetter) {
        m_tabs->setCurrentIndex(i);
        break;
      }
    }
  }
}

void MainWindow::refresh() {
  UWF_LOG_I("ui") << "refresh start";
  const auto t0 = std::chrono::steady_clock::now();
  std::string err;
  auto disks = uwf::enumerateDisks(&err);
  if (!err.empty()) {
    UWF_LOG_W("ui") << "enumerateDisks error: " << err;
    warning(this, I18n::tr("Failed to read volume information"), QString::fromStdString(err));
  }
  m_snapshot = uwf::readSnapshot(&err);
  if (!m_snapshot.uwfAvailable) {
    // 不弹模态框——GlobalStatusPanel::setUnavailable 的横幅已常驻展示这条
    // 错误，模态框只是重复打扰（且每点一次刷新就再弹一次）。
    UWF_LOG_E("ui") << "readSnapshot failed: uwfAvailable=false err=" << err;
  }

  // uwfAvailable（命名空间可读）与 elevated（进程已提权）外观相近但用途不同，
  // 各处按需分别判断，不合并成单一标志。
  const bool uwfAvailable = m_snapshot.uwfAvailable;
  const bool elevated = m_snapshot.elevated;

  // 工具栏：UWF 不可用时除"日志 / 关于 / 主题 / 语言"外整体禁用。其中"刷新"
  // 只是重新读取、不写入 UWF，故未提权也允许点——只要 UWF 可读就放开；
  // 导入 / 预览并应用 / 安全关机 / 安全重启会写入，需同时已提权。
  if (m_actRefresh) m_actRefresh->setEnabled(uwfAvailable);
  for (QAction* a : {m_actImport, m_actPlan, m_actShutdown, m_actRestart})
    if (a) a->setEnabled(uwfAvailable && elevated);

  // Usage 定时器只跟随 UWF 可读性——未提权不影响读取占用，故不停表；
  // 仅在 UWF 不可用时停掉，避免每 5s 一次徒劳的 UWF_Overlay 读取。
  if (m_usageTimer) {
    if (uwfAvailable)
      m_usageTimer->start();
    else
      m_usageTimer->stop();
  }

  rebuildTabs(disks);
  if (uwfAvailable) {
    m_global->setData(m_snapshot.current, m_snapshot.next, m_snapshot.runtime);
    // UWF 可读但未提权：补一条红色"需要管理员权限"横幅。UWF 不可用时不补——
    // 那条不可用横幅优先级更高，已由下面的 setUnavailable 占据同一横幅。
    if (!elevated) m_global->showElevationRequired();
  } else {
    m_global->setUnavailable(err.empty() ? I18n::tr("UWF namespace is not available") : QString::fromStdString(err));
  }
  // setData 会把滚动区控件全部恢复 enabled——未提权时随即再统一置灰一次。
  m_global->setControlsEnabled(uwfAvailable && elevated);
  for (auto& t : m_diskTabs)
    if (t) t->applySnapshot(m_snapshot);
  m_statusBaseline = I18n::tr("Refreshed · %1 volumes").arg(disks.size());
  if (!m_hintTimer->isActive()) m_statusText->setText(m_statusBaseline);
  const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
  UWF_LOG_I("ui") << std::format("refresh done: disks={} uwfAvailable={} currentVolumes={} nextVolumes={} elapsedMs={}", disks.size(), m_snapshot.uwfAvailable,
                                 m_snapshot.current.volumes.size(), m_snapshot.next.volumes.size(), elapsedMs);
}

void MainWindow::showPlan() {
  // 收集待应用变更、渲染命令预览、二次确认后写入 WMI 都在 ApplyPlanDialog
  // 里完成；这里只负责把变更来源（GlobalStatusPanel + 各 DiskTab）和写会话
  // 交给它。applied() 用 QueuedConnection 接：等对话框这一轮事件循环回落
  // 再 refresh，避免在回调里递归进 refresh 的弹窗 / WMI 读。
  ApplyPlanDialog dlg(m_global, m_diskTabs, m_snapshot, m_writeSession, this);
  connect(&dlg, &ApplyPlanDialog::applied, this, &MainWindow::refresh, Qt::QueuedConnection);
  dlg.exec();
}

void MainWindow::showImport() {
  ImportDialog dlg(this);
  // applier：把每条 UwfmgrCommand 转成对应的 UI 操作（驱动 m_global 或对应
  // 盘符的 DiskTab）。报告里的"重复"分两种：
  //   1) within-batch：同一次 Import 文本里出现两条等价命令，第二条标 Duplicate；
  //   2) state no-op：命令应用后控件值未变（比如 filter 已经处于目标 enable
  //      状态）；importXxx 返回 false 时归到这里。
  // 都归并到 ImportReportRow::Status::Duplicate。
  dlg.setApplier([this](const QList<api::UwfmgrCommand>& cmds) -> QList<ImportReportRow> {
    QList<ImportReportRow> out;
    out.reserve(cmds.size());

    // within-batch 去重的 canonical key：kind + 大小写无关化的 arg0。
    QSet<QString> seen;
    auto canon = [](const api::UwfmgrCommand& c) {
      const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]).toLower();
      return QString::number(static_cast<int>(c.kind)) + QChar('|') + a0;
    };

    auto findTab = [this](const QString& dl) -> DiskTab* {
      for (auto& t : m_diskTabs) {
        if (t && t->driveLetter().compare(dl, Qt::CaseInsensitive) == 0) return t.data();
      }
      return nullptr;
    };

    auto outcomeToRow = [](const api::UwfmgrCommand& c, ExclusionListWidget::ImportOutcome o, const QString& kindLabel) {
      ImportReportRow r;
      r.lineNo = c.sourceLineNo;
      r.lineText = QString::fromStdString(c.rawLine).trimmed();
      switch (o) {
        case ExclusionListWidget::ImportOutcome::Applied:
          r.status = ImportReportRow::Status::Success;
          r.detail = I18n::tr("Queued as a pending %1 change").arg(kindLabel);
          break;
        case ExclusionListWidget::ImportOutcome::NoOp:
          r.status = ImportReportRow::Status::Duplicate;
          r.detail = I18n::tr("Already in the target state — no-op");
          break;
        case ExclusionListWidget::ImportOutcome::RejectedNotOnVolume:
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)");
          break;
        case ExclusionListWidget::ImportOutcome::RejectedForbidden:
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)");
          break;
      }
      return r;
    };

    for (const auto& c : cmds) {
      ImportReportRow r;
      r.lineNo = c.sourceLineNo;
      r.lineText = QString::fromStdString(c.rawLine).trimmed();

      // 解析阶段失败的命令直接打包：
      // - Unsupported = 整段没识别 → Status::Unsupported；
      // - 其它非 None / Comment = cat/verb 已认出但参数非法 → Status::Failed。
      // parseErrorMessage 把 enum 翻译成中文（来自 ImportDialog.cpp 的 helper）。
      if (c.parseError != api::ParseError::None && c.parseError != api::ParseError::Comment) {
        r.status = c.parseError == api::ParseError::Unsupported ? ImportReportRow::Status::Unsupported : ImportReportRow::Status::Failed;
        r.detail = parseErrorMessage(c.parseError, QString::fromStdString(c.parseErrorContext));
        out.append(r);
        continue;
      }

      // within-batch dedup：第二条等价命令标 Duplicate，跳过 apply。
      const QString key = canon(c);
      if (seen.contains(key)) {
        r.status = ImportReportRow::Status::Duplicate;
        r.detail = I18n::tr("Same command was already issued earlier in this batch");
        out.append(r);
        continue;
      }
      seen.insert(key);

      // 把 args[0] 提到 QString 一次，下面分支统一用。args[0] 永远存在，因为
      // parser 只有在 args 完整时才把 parseError 设回 None；上面的 parseError 检查
      // 已经把缺参数的全部过滤掉了。
      const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]);

      switch (c.kind) {
        case api::UwfmgrKind::FilterEnable:
        case api::UwfmgrKind::FilterDisable: {
          const bool want = c.kind == api::UwfmgrKind::FilterEnable;
          const bool changed = m_global ? m_global->importFilterEnabled(want) : false;
          r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
          r.detail =
              changed ? I18n::tr("Pending filter %1").arg(want ? I18n::tr("enable") : I18n::tr("disable")) : I18n::tr("Filter is already in the target state");
          break;
        }
        case api::UwfmgrKind::OverlaySetType: {
          const auto t = a0 == QStringLiteral("Disk") ? core::OverlayType::Disk : core::OverlayType::RAM;
          const bool changed = m_global ? m_global->importOverlayType(t) : false;
          r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
          r.detail = changed ? I18n::tr("Pending overlay type → %1").arg(a0) : I18n::tr("Overlay type already %1").arg(a0);
          break;
        }
        case api::UwfmgrKind::OverlaySetSize:
        case api::UwfmgrKind::OverlaySetWarningThreshold:
        case api::UwfmgrKind::OverlaySetCriticalThreshold: {
          bool ok = false;
          const auto mb = a0.toUInt(&ok);
          if (!ok) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("Invalid size value: %1").arg(a0);
            break;
          }
          bool changed = false;
          QString label;
          if (c.kind == api::UwfmgrKind::OverlaySetSize) {
            label = I18n::tr("maximum size");
            changed = m_global ? m_global->importOverlayMaxMb(mb) : false;
          } else if (c.kind == api::UwfmgrKind::OverlaySetWarningThreshold) {
            label = I18n::tr("warning threshold");
            changed = m_global ? m_global->importOverlayWarnMb(mb) : false;
          } else {
            label = I18n::tr("critical threshold");
            changed = m_global ? m_global->importOverlayCritMb(mb) : false;
          }
          r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
          r.detail = changed ? I18n::tr("Pending overlay %1 → %2 MB").arg(label).arg(mb) : I18n::tr("Overlay %1 already %2 MB").arg(label).arg(mb);
          break;
        }
        case api::UwfmgrKind::VolumeProtect:
        case api::UwfmgrKind::VolumeUnprotect: {
          auto* tab = findTab(a0);
          if (!tab) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("Unknown volume %1 (no UWF-eligible disk with that drive letter)").arg(a0);
            break;
          }
          const bool want = c.kind == api::UwfmgrKind::VolumeProtect;
          const bool changed = tab->importProtect(want);
          r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
          r.detail = changed ? I18n::tr("Pending volume %1 protection %2").arg(a0, want ? I18n::tr("enable") : I18n::tr("disable"))
                             : I18n::tr("Volume %1 is already in the target protection state").arg(a0);
          break;
        }
        case api::UwfmgrKind::FileAddExclusion:
        case api::UwfmgrKind::FileRemoveExclusion: {
          const QString native = QDir::toNativeSeparators(a0);
          // 路径需要 "<盘符>:" 前缀来路由到对应 DiskTab；缺前缀 → 没办法定位。
          const QString dl = extractDriveLetter(native);
          if (dl.isEmpty()) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("Path %1 has no drive letter; cannot route to a volume tab").arg(native);
            break;
          }
          auto* tab = findTab(dl);
          if (!tab) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("No UWF-eligible disk for drive letter %1").arg(dl);
            break;
          }
          const auto outcome = c.kind == api::UwfmgrKind::FileAddExclusion ? tab->importAddFileExclusion(native) : tab->importRemoveFileExclusion(native);
          r = outcomeToRow(c, outcome, I18n::tr("file exclusion"));
          break;
        }
        case api::UwfmgrKind::RegistryAddExclusion:
        case api::UwfmgrKind::RegistryRemoveExclusion: {
          // 注册表排除是全局的，只挂在系统盘 TAB 上。其它 TAB 的 import* 在
          // m_regs == null 时直接返回 RejectedNotOnVolume，所以这里依次尝试
          // 每个 TAB——第一个非 RejectedNotOnVolume 的结果即视作系统盘 TAB
          // 的处理结果。所有 TAB 都拒说明压根没有系统盘 TAB。
          bool dispatched = false;
          for (auto& t : m_diskTabs) {
            if (!t) continue;
            const auto outcome = c.kind == api::UwfmgrKind::RegistryAddExclusion ? t->importAddRegistryExclusion(a0) : t->importRemoveRegistryExclusion(a0);
            if (outcome != ExclusionListWidget::ImportOutcome::RejectedNotOnVolume) {
              r = outcomeToRow(c, outcome, I18n::tr("registry exclusion"));
              dispatched = true;
              break;
            }
          }
          if (!dispatched) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("Registry exclusions are only available on the system drive tab, which is not present");
          }
          break;
        }
        case api::UwfmgrKind::Unknown:
          // 解析阶段 Unknown 已在前面分支处理了；落到这里说明 parseError
          // 是 None 而 kind 又是 Unknown，理论上不会发生，安全兜底。
          r.status = ImportReportRow::Status::Unsupported;
          r.detail = I18n::tr("Unsupported command");
          break;
      }
      out.append(r);
    }

    // 导入命令逐条 setValue 写入，不触发 spinbox 的 editingFinished，约束链
    // （warn ≤ crit ≤ max）不会自动收紧、range 也停在导入时放宽的状态。批量
    // 导入结束补一次收紧，让面板回到自洽——否则之后任意一次无关交互触发
    // reconfigureRanges 时会静默改写导入值。
    if (m_global) m_global->finishImport();
    return out;
  });

  dlg.exec();
}

void MainWindow::showAbout() {
  // 改用普通 QDialog 而非 QMessageBox：QMessageBox 内部 label 走另一条
  // 字体路径，全局 app.setFont() 设置的 hinting / styleStrategy 不会传播过去，
  // 中文渲染会"糊"。QDialog + QLabel 跟其它对话框一样能继承 app font。
  QDialog dlg(this);
  dlg.setWindowTitle(I18n::tr("About UWF Manager"));
  dlg.setMinimumWidth(520);

  auto* layout = new QVBoxLayout(&dlg);
  layout->setContentsMargins(20, 16, 20, 12);
  layout->setSpacing(10);

  // 头部：左侧软件 logo，右侧标题 + 版本号竖排。
  auto* header = new QHBoxLayout();
  header->setSpacing(14);

  // app.svg 是矢量 logo（同时用作窗口 / 托盘图标），渲染成 64×64 放在左侧。
  auto* logo = new QLabel(&dlg);
  logo->setPixmap(QIcon(QStringLiteral(":/icons/app.svg")).pixmap(64, 64));
  logo->setFixedSize(64, 64);
  header->addWidget(logo);

  auto* titleBox = new QVBoxLayout();
  titleBox->setSpacing(2);

  // 标题：手动用 QLabel + 大字号 + bold（YaHei 真实字重 700）替代 <h3>，
  // 避免 QTextDocument 的 <h3> 默认合成粗体（同样的 hinting 问题）。
  auto* title = new QLabel(I18n::tr("Unified Write Filter (UWF) Manager"), &dlg);
  QFont titleFont = title->font();
  titleFont.setBold(true);
  titleFont.setPointSizeF(titleFont.pointSizeF() + 3);
  title->setFont(titleFont);
  title->setTextInteractionFlags(Qt::TextSelectableByMouse);
  titleBox->addWidget(title);

  // 版本号紧贴标题下方、弱化显示。UWF_VER_STRING 由 cmake/GitVersion.cmake
  // 在构建期注入 git 短哈希（无 git 仓库时回退为 "1.0.0.0"）。用内联 color
  // 走富文本，理由同 body：避开 QSS / palette 对 QLabel 文字色的干扰。
  auto* version = new QLabel(&dlg);
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

  auto* body = new QLabel(&dlg);
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
  auto* uwfNote = new QLabel(&dlg);
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

  auto* btns = new QDialogButtonBox(&dlg);
  auto* closeBtn = btns->addButton(I18n::tr("Close"), QDialogButtonBox::AcceptRole);
  connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  layout->addWidget(btns);

  dlg.exec();
}

void MainWindow::showLogs() {
  LogViewerDialog dlg(this);
  dlg.exec();
}

void MainWindow::safeShutdown() {
  if (!confirm(this, I18n::tr("Safe shutdown"), I18n::tr("The system will shut down safely.\nUncommitted changes in this session will be lost.\n\nContinue?")))
    return;

  std::string err;
  auto row = m_filter.read(&err);
  if (!row) {
    warning(this, I18n::tr("Safe shutdown failed"), I18n::tr("Failed to read filter state: %1").arg(QString::fromStdString(err)));
    return;
  }
  if (const auto r = m_filter.shutdownSystem(*row); !r.ok) {
    warning(this, I18n::tr("Safe shutdown failed"), I18n::tr("Shutdown failed: %1").arg(QString::fromStdString(r.detail)));
  }
}

void MainWindow::safeRestart() {
  if (!confirm(this, I18n::tr("Safe restart"), I18n::tr("The system will restart safely.\nUncommitted changes in this session will be lost.\n\nContinue?")))
    return;

  std::string err;
  auto row = m_filter.read(&err);
  if (!row) {
    warning(this, I18n::tr("Safe restart failed"), I18n::tr("Failed to read filter state: %1").arg(QString::fromStdString(err)));
    return;
  }
  if (const auto r = m_filter.restartSystem(*row); !r.ok) {
    warning(this, I18n::tr("Safe restart failed"), I18n::tr("Restart failed: %1").arg(QString::fromStdString(r.detail)));
  }
}

template <typename Target, typename DisplayFn, typename CommitFn, typename ExistsFn>
void MainWindow::runCommitBatch(const QString& progressTitle, const QList<Target>& targets, DisplayFn displayOf, CommitFn commitOne, ExistsFn existsFn) {
  const int total = static_cast<int>(targets.size());

  // 进度条只在多目标时弹；单目标一两次 WMI 调用，弹窗反因 show 计时 / autoClose
  // 的时序问题残留在屏上。setValue 内部 processEvents——必须 WindowModal，否则
  // commit 半途用户点别处会嵌套触发同一份 m_writeSession（WMI 不可重入）。
  std::unique_ptr<QProgressDialog> progress;
  if (total > 1) {
    progress = std::make_unique<QProgressDialog>(I18n::tr("Committing…"), I18n::tr("Cancel"), 0, total, this);
    progress->setWindowTitle(progressTitle);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(500);
    progress->setAutoClose(true);
    progress->setAutoReset(false);
    Q_ASSERT(progress->windowModality() == Qt::WindowModal);
  }

  // existsFn 为 decltype(nullptr) 即调用方（提交操作）未传——整段 constexpr 跳过；
  // 删除操作传入它，在 commit 前后各探一次目标是否存在。
  constexpr bool kHasExists = !std::is_same_v<ExistsFn, decltype(nullptr)>;

  bool canceled = false;
  QList<CommitReportRow> allRows;
  for (int i = 0; i < total; ++i) {
    if (progress) {
      progress->setValue(i);
      if (progress->wasCanceled()) {
        canceled = true;
        break;
      }
      const QString d = displayOf(targets[i]);
      progress->setLabelText(QString("[%1/%2] %3").arg(i + 1).arg(total).arg(d.size() > 80 ? ("…" + d.right(79)) : d));
    }
    std::optional<bool> existedBefore, existsAfter;
    if constexpr (kHasExists) existedBefore = existsFn(targets[i]);
    const auto res = commitOne(targets[i]);
    if constexpr (kHasExists) existsAfter = existsFn(targets[i]);

    CommitReportRow row;
    row.path = displayOf(targets[i]);
    row.existedBefore = existedBefore;
    row.existsAfter = existsAfter;
    // res 是统一的 WmiResult；commit 类的 Skipped / Failed 三档由 commitOutcome
    // 派生（WBEM_E_NOT_FOUND → Skipped；其它非 ok → Failed）。
    const auto outcome = commitOutcome(res);
    if (outcome == CommitOutcome::Ok) {
      row.category = I18n::tr("Succeeded");
      row.errorCode = QStringLiteral("-");
      row.reason = QStringLiteral("-");
    } else {
      row.category = outcome == CommitOutcome::Skipped ? I18n::tr("Skipped") : I18n::tr("Failed");
      row.errorCode = formatErrorCode(res.hresult, res.returnValue);
      // kHasExists（删除操作传了 existsFn）== 操作类型是 Deletion——HRESULT 含义
      // 在 commit / deletion 间不同，文案要分。
      row.reason = explainCommitFailure(res.hresult, res.returnValue, kHasExists);
    }
    allRows.append(std::move(row));
  }
  if (progress) {
    progress->setValue(total);
    progress->close();
  }
  const int untouched = canceled ? (total - static_cast<int>(allRows.size())) : 0;
  showCommitReport(this, allRows, untouched);
}

void MainWindow::commitFilePath(const QString& path) {
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
    warning(this, I18n::tr("Commit failed"), I18n::tr("The path has no drive letter; cannot identify the target volume."));
    return;
  }

  std::string err;
  auto volumes = m_volume.readAll(&err);
  if (!err.empty()) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("Failed to read volume information: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto dlStd = dl.toStdString();
  const auto* row = api::findBySession(volumes, /*wantCurrent=*/true, [&](const api::VolumeRow& v) { return v.driveLetter == dlStd; });
  if (!row) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("No current-session record found for volume %1.").arg(dl));
    return;
  }

  // 排除列表用 volumeName (Win32_Volume.DeviceID) 作键，按当前会话的运行态判断。
  if (auto it = m_snapshot.current.fileExclusions.find(row->volumeName); it != m_snapshot.current.fileExclusions.end()) {
    const std::string hit = findCoveringExclusion(it->second, path.toStdString());
    if (!hit.empty()) {
      confirmCommit(this, title, heading, path, I18n::tr("This path is in the file exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)),
                    /*allowContinue=*/false);
      return;
    }
  }

  // UWF_Volume.CommitFile 只认单个文件条目；给目录会返回 WBEM_E_NOT_FOUND。
  // 所以目录提交 = 递归遍历目录下所有文件挨个 commit。
  QStringList targets;
  if (isDir) {
    QDirIterator it(path, QDir::Files | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) targets << QDir::toNativeSeparators(it.next());
  } else {
    targets << path;
  }

  if (isDir && targets.isEmpty()) {
    confirmCommit(this, title, heading, path, I18n::tr("No files were found under %1.").arg(path), /*allowContinue=*/false);
    return;
  }

  const QString detail = isDir ? I18n::tr("%1 files in this folder and all its subfolders will be committed.").arg(targets.size()) : QString();
  if (!confirmCommit(this, title, heading, path, detail)) return;

  runCommitBatch(
      I18n::tr("Commit to disk"), targets, [](const QString& f) { return f; },
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

void MainWindow::commitFileDeletionPath(const QString& path) {
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
    warning(this, I18n::tr("Commit file deletion failed"), I18n::tr("The path has no drive letter; cannot identify the target volume."));
    return;
  }

  // 核心校验：CommitFileDeletion 由方法自身执行删除，目标（文件或目录）必须**仍
  // 存在**。不存在就没有可删的东西——走灰按钮对话框告知。
  if (!fi.exists()) {
    confirmCommit(this, title, heading, path, I18n::tr("This path does not exist, so there is nothing to delete."), /*allowContinue=*/false);
    return;
  }

  std::string err;
  const auto volumes = m_volume.readAll(&err);
  if (!err.empty()) {
    warning(this, I18n::tr("Commit file deletion failed"), I18n::tr("Failed to read volume information: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto dlStd = dl.toStdString();
  const auto* row = api::findBySession(volumes, /*wantCurrent=*/true, [&](const api::VolumeRow& v) { return v.driveLetter == dlStd; });
  if (!row) {
    warning(this, I18n::tr("Commit file deletion failed"), I18n::tr("No current-session record found for volume %1.").arg(dl));
    return;
  }

  // 落在文件排除列表里的路径，UWF 不在覆盖层维护，提交删除无意义。
  if (auto it = m_snapshot.current.fileExclusions.find(row->volumeName); it != m_snapshot.current.fileExclusions.end()) {
    const std::string hit = findCoveringExclusion(it->second, path.toStdString());
    if (!hit.empty()) {
      confirmCommit(this, title, heading, path, I18n::tr("This path is in the file exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)),
                    /*allowContinue=*/false);
      return;
    }
  }

  // CommitFileDeletion 不接受非空目录、也不递归——目录删除 = 把子文件、子目录、
  // 目录本身按"最深的先删"收齐，逐个调用；删到某目录时其内容已清空。
  QStringList targets;
  int fileCount = 0;
  int subdirCount = 0;
  if (isDir) {
    QDirIterator fileIt(path, QDir::Files | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (fileIt.hasNext()) targets << QDir::toNativeSeparators(fileIt.next());
    fileCount = static_cast<int>(targets.size());
    QDirIterator dirIt(path, QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (dirIt.hasNext()) targets << QDir::toNativeSeparators(dirIt.next());
    subdirCount = static_cast<int>(targets.size()) - fileCount;
    targets << QDir::toNativeSeparators(path);  // 目录本身——排序后最浅、最后删
    std::sort(targets.begin(), targets.end(), [](const QString& a, const QString& b) { return a.count('\\') > b.count('\\'); });
  } else {
    targets << path;
  }

  const QString detail = isDir ? I18n::tr("%1 files and %2 subfolders will be deleted.").arg(fileCount).arg(subdirCount) : QString();
  if (!confirmCommit(this, title, heading, path, detail)) return;

  runCommitBatch(
      title, targets, [](const QString& f) { return f; },
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

void MainWindow::commitRegistryKey(const QString& key, const QString& valueName) {
  if (key.isEmpty()) return;

  // 多目标提交会弹 QProgressDialog（setValue 内部 processEvents）——暂停占用刷新
  // 定时器，防止半途重入 m_writeSession。
  const ScopedTimerPause usagePause(m_usageTimer);

  // 注册表键先归一成长写 hive（HKLM\… → HKEY_LOCAL_MACHINE\…）：UWF 覆盖层、排除
  // 列表都按长写键路径索引，跳过归一会匹配不到。
  const std::string normKey = regkey::normalize(key.toStdString());
  if (normKey.empty()) return;
  const QString keyText = QString::fromStdString(normKey);

  // 标题 / heading / target 文本提前算——失败和成功路径共用。
  const bool wholeKey = valueName.isEmpty();
  const QString title = I18n::tr("Commit to disk");
  const QString heading = wholeKey ? I18n::tr("Commit this registry key and its whole subtree to disk") : I18n::tr("Commit this registry value to disk");
  const QString target = wholeKey ? keyText : (keyText + " : " + valueName);

  // 注册表排除是全局的，比对当前运行会话即可。覆盖 = 键相等或为其祖先。
  const std::string hit = findCoveringExclusion(m_snapshot.current.registryExclusions, normKey);
  if (!hit.empty()) {
    confirmCommit(this, title, heading, target, I18n::tr("This key is in the registry exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)),
                  /*allowContinue=*/false);
    return;
  }

  // 目标清单：值名给定 = 只提交那一个值；值名留空 = 递归整棵键子树的每一个值
  // （CommitRegistry 只能逐值提交，"提交整键"由这里展开成逐值调用）。
  QList<RegCommitTarget> targets;
  if (!valueName.isEmpty()) {
    if (!regkey::valueExists(normKey, valueName.toStdString())) {
      confirmCommit(this, title, heading, target, I18n::tr("This registry value does not exist, so there is nothing to commit."), /*allowContinue=*/false);
      return;
    }
    targets.append({normKey, valueName.toStdString(), target});
  } else {
    if (!regkey::keyExists(normKey)) {
      confirmCommit(this, title, heading, target, I18n::tr("This registry key does not exist, so there is nothing to commit."), /*allowContinue=*/false);
      return;
    }
    // UWF 的 CommitRegistry 是逐值提交——ValueName="" 提交的是键的 (Default)
    // 值，默认值不存在则返回 NOT_FOUND；UWF 没有"提交键本身（不带任何值）"的
    // 能力。这里递归收集每一个键，并保证**每个键都至少 emit 一次** (k, "")
    // 尝试，让 UWF 在结果表里对该键诚实表态——OK 或 NOT_FOUND——而不是 UI 凭
    // valueNames 是否为空就跳过。命名值另外逐个 emit；若 (Default) 已在
    // valueNames 中（即已被设过），则跳过额外的 (k, "") 避免重复行。
    for (const auto& k : regkey::collectKeyTree(normKey)) {
      const QString kText = QString::fromStdString(k);
      const auto vns = regkey::valueNames(k);
      const bool defaultAlreadyInVns = std::ranges::any_of(vns, [](const std::string& s) { return s.empty(); });
      if (!defaultAlreadyInVns) {
        targets.append({k, std::string{}, kText + " : (Default)"});
      }
      for (const auto& vn : vns) {
        targets.append({k, vn, vn.empty() ? (kText + " : (Default)") : (kText + " : " + QString::fromStdString(vn))});
      }
    }
  }
  const int total = static_cast<int>(targets.size());

  const QString detail = wholeKey ? I18n::tr("%1 values in this key and all its subkeys will be committed.").arg(total) : QString();
  if (!confirmCommit(this, title, heading, target, detail)) return;

  std::string err;
  auto filters = m_registry.readAll(&err);
  if (!err.empty()) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("Failed to read registry filter: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto* row = api::findBySession(filters, /*wantCurrent=*/true);
  if (!row) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("No current-session registry filter record found."));
    return;
  }

  runCommitBatch(
      I18n::tr("Commit to disk"), targets, [](const RegCommitTarget& t) { return t.display; },
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

void MainWindow::commitRegistryDeletionKey(const QString& key, const QString& valueName) {
  if (key.isEmpty()) return;

  const ScopedTimerPause usagePause(m_usageTimer);

  // 同 commitRegistryKey：先归一成长写 hive。
  const std::string normKey = regkey::normalize(key.toStdString());
  if (normKey.empty()) return;
  const QString keyText = QString::fromStdString(normKey);

  // 标题 / heading / target 提前算——失败和成功路径共用同一个 confirmCommit 版式。
  const bool wholeKey = valueName.isEmpty();
  const QString title = I18n::tr("Delete and commit");
  const QString heading = wholeKey ? I18n::tr("Delete this registry key and its whole subtree, and commit the deletions to disk")
                                   : I18n::tr("Delete this registry value, and commit the deletion to disk");
  const QString target = wholeKey ? keyText : (keyText + " : " + valueName);

  const std::string hit = findCoveringExclusion(m_snapshot.current.registryExclusions, normKey);
  if (!hit.empty()) {
    confirmCommit(this, title, heading, target, I18n::tr("This key is in the registry exclusion list.\nExclusion: %1").arg(QString::fromStdString(hit)),
                  /*allowContinue=*/false);
    return;
  }

  // 目标清单：值名给定 = 只删那一个值；值名留空 = 递归整棵键子树。
  // CommitRegistryDeletion 由方法自身执行删除，目标必须仍存在；它不递归——
  // CommitRegistryDeletion(key,"") 只能删叶子键，故按 collectKeyTree 的后序
  // （最深子键在前）逐个删，删到每个键时它都已是叶子。
  QList<RegCommitTarget> targets;
  if (!valueName.isEmpty()) {
    if (!regkey::valueExists(normKey, valueName.toStdString())) {
      confirmCommit(this, title, heading, target, I18n::tr("This registry value does not exist, so there is nothing to delete."), /*allowContinue=*/false);
      return;
    }
    targets.append({normKey, valueName.toStdString(), target});
  } else {
    if (!regkey::keyExists(normKey)) {
      confirmCommit(this, title, heading, target, I18n::tr("This registry key does not exist, so there is nothing to delete."), /*allowContinue=*/false);
      return;
    }
    for (const auto& k : regkey::collectKeyTree(normKey)) {
      targets.append({k, std::string{}, QString::fromStdString(k)});
    }
  }
  const int total = static_cast<int>(targets.size());

  const QString detail = wholeKey ? I18n::tr("%1 keys, including all their values and subkeys, will be deleted.").arg(total) : QString();
  if (!confirmCommit(this, title, heading, target, detail)) return;

  std::string err;
  auto filters = m_registry.readAll(&err);
  if (!err.empty()) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("Failed to read registry filter: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto* row = api::findBySession(filters, /*wantCurrent=*/true);
  if (!row) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("No current-session registry filter record found."));
    return;
  }

  runCommitBatch(
      I18n::tr("Delete and commit"), targets, [](const RegCommitTarget& t) { return t.display; },
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
