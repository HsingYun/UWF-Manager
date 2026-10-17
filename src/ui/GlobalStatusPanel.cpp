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
#include "GlobalStatusPanel.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QSpinBox>
#include <algorithm>
#include <format>
#include <limits>
#include <utility>

#include "../core/Config.h"
#include "../util/ByteFormat.h"
#include "I18n.h"
#include "OverlayUsageBar.h"
#include "StatusBanner.h"
#include "SwitchButton.h"
#include "ThemeManager.h"
#include "UiUtil.h"

namespace uwf::ui {

using core::OverlayType;

namespace {

constexpr int kOverlaySpinMaxMb = std::numeric_limits<int>::max();

int toOverlaySpinValue(const uint32_t valueMb) { return static_cast<int>(std::min<uint32_t>(valueMb, static_cast<uint32_t>(kOverlaySpinMaxMb))); }

int strictThresholdUpper(const int parentValue) { return parentValue <= 1 ? 0 : parentValue - 1; }

// QSpinBox 默认在输入值超过 maximum() 时 validate() 返回 Invalid，字符会被
// 拒录入，CorrectToNearestValue 也不会触发（它只在 Intermediate 状态 clamp）。
// 这里把"全数字但超 max"的 Invalid 降级成 Intermediate，让用户能继续打字，
// 离焦/回车时再由父类的修正机制 clamp 到 max。
class ClampingSpinBox : public QSpinBox {
 public:
  using QSpinBox::QSpinBox;

 protected:
  QValidator::State validate(QString& input, int& pos) const override {
    const auto state = QSpinBox::validate(input, pos);
    if (state != QValidator::Invalid) return state;
    QString body = stripFix(input);
    if (body.isEmpty()) return QValidator::Intermediate;
    for (QChar c : std::as_const(body))
      if (!c.isDigit()) return QValidator::Invalid;
    return QValidator::Intermediate;
  }

  // 失焦/回车时 Qt 会调用 fixup 把 Intermediate 文本"修正"成 Acceptable。
  // 默认实现不会 clamp 到 maximum()，会让超限值落回上一次值；这里显式把
  // 超过 [minimum, maximum] 的数值钳到边界，满足"超过就吸到最大值"。
  void fixup(QString& input) const override {
    const QString body = stripFix(input);
    bool ok = false;
    qlonglong v = body.toLongLong(&ok);
    if (!ok) {
      QSpinBox::fixup(input);
      return;
    }
    if (v > maximum()) v = maximum();
    if (v < minimum()) v = minimum();
    input = prefix() + QString::number(v) + suffix();
  }

 private:
  [[nodiscard]] QString stripFix(const QString& text) const {
    QString body = text;
    if (!prefix().isEmpty() && body.startsWith(prefix())) body = body.mid(prefix().size());
    if (!suffix().isEmpty() && body.endsWith(suffix())) body = body.left(body.size() - suffix().size());
    return body.trimmed();
  }
};

QComboBox* makeOverlayTypeCombo() {
  auto* c = new QComboBox();
  c->addItem("RAM", static_cast<int>(OverlayType::RAM));
  c->addItem("Disk", static_cast<int>(OverlayType::Disk));
  return c;
}

QLabel* makeKey(const QString& text) {
  auto* l = new QLabel(text);
  l->setObjectName("statusKey");
  return l;
}

// "?" 锁定提示徽标。默认隐藏；在 UWF 当前会话启用时由调用方 show() 出来，
// 配合 setEnabled(false) 告诉用户该字段为何不能改。
QLabel* makeLockedHint(const QString& tooltip) {
  auto* l = new QLabel("?");
  l->setObjectName("lockedHint");
  l->setAlignment(Qt::AlignCenter);
  l->setToolTip(tooltip);
  l->setCursor(Qt::WhatsThisCursor);
  l->hide();
  return l;
}

QString formatUsageText(const uint32_t usedMb, const uint32_t totalMb) {
  const double pct = totalMb == 0 ? 0.0 : (static_cast<double>(usedMb) * 100.0 / static_cast<double>(totalMb));
  return QString::fromStdString(std::format("{:.1f}% · {} / {} MB", pct, usedMb, totalMb));
}

// 把控件 + ? 锁定提示打包成 grid 第 1 列里的右对齐布局。
QWidget* wrapWithLockedHint(QWidget* control, QLabel* hint) {
  auto* w = new QWidget();
  auto* h = new QHBoxLayout(w);
  h->setContentsMargins(0, 0, 0, 0);
  h->setSpacing(6);
  h->addStretch(1);
  h->addWidget(control, 0);
  h->addWidget(hint, 0, Qt::AlignVCenter);
  return w;
}

QFrame* makeSection(const QString& title, QLayout* inner) {
  auto* f = new QFrame();
  f->setObjectName("statusCard");
  f->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto* v = new QVBoxLayout(f);
  v->setContentsMargins(16, 12, 16, 14);
  v->setSpacing(10);
  auto* h = new QLabel(title);
  h->setObjectName("statusSection");
  v->addWidget(h);
  v->addLayout(inner);
  return f;
}

}  // namespace

GlobalStatusPanel::GlobalStatusPanel(QWidget* parent) : QWidget(parent) {
  setObjectName("globalStatusPanel");
  auto* outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  outer->setSpacing(8);

  auto* title = new QLabel(I18n::tr("Global settings"));
  title->setObjectName("panelTitle");
  outer->addWidget(title);

  // 兼容模式警告——放在状态横幅之上，一经 setCompatibilityNotice 显示便常驻，
  // 不随 setData / setUnavailable 的刷新清除。
  m_compatBanner = new StatusBanner(this);
  m_compatBanner->setObjectName("statusBanner");
  m_compatBanner->setProperty("level", "warn");
  m_compatBanner->setWordWrap(true);
  m_compatBanner->hide();
  outer->addWidget(m_compatBanner);

  m_banner = new StatusBanner(this);
  m_banner->setObjectName("statusBanner");
  m_banner->setWordWrap(true);
  m_banner->hide();
  outer->addWidget(m_banner);

  m_controls = new QWidget(this);
  auto* body = new QVBoxLayout(m_controls);
  body->setContentsMargins(0, 0, 0, 0);
  body->setSpacing(10);

  // 筛选器 —— 一行："本次 / 下次"两张会话状态 mini 卡片，左对齐。不再单列
  // "Filter state:" 标签：小节标题已叫「筛选器」，重复且其 min-width:120px 会把
  // 整行撑宽，在右侧滚动面板里逼出横向滚动条。
  auto* filterRow = new QHBoxLayout();
  filterRow->setSpacing(6);

  m_filterCur = new QLabel("—");
  m_filterCur->setObjectName("statusCurLabel");
  m_filterCur->setTextFormat(Qt::RichText);  // 启用/停用状态走富文本（绿/红，见 enabledStateLabel）
  m_filterCur->setToolTip(I18n::tr("UWF filter state in the current session (read-only)."));
  m_filterNext = new SwitchButton();
  m_filterNext->setToolTip(
      I18n::tr("Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot."));

  // 本次 / 下次筛选状态各装进一张 mini 卡片，靠卡片边界把"当前生效值"和
  // "重启后才生效的目标值"分隔开，避免两者挨在一起被混淆。
  filterRow->addWidget(makeSessionChip(I18n::tr("Current session"),
                                       I18n::tr("The currently active session (read-only). Changes you make never take effect in this session."), m_filterCur),
                       0, Qt::AlignVCenter);
  filterRow->addWidget(
      makeSessionChip(I18n::tr("Next session"),
                      I18n::tr("The session that takes effect after a reboot. Changes you make take effect after the system restarts."), m_filterNext),
      0, Qt::AlignVCenter);
  filterRow->addStretch(1);

  body->addWidget(makeSection(I18n::tr("Filter"), filterRow));

  // 覆盖层 —— 单行：标签 + 输入控件。
  auto* overlayGrid = new QGridLayout();
  overlayGrid->setHorizontalSpacing(10);
  overlayGrid->setVerticalSpacing(10);
  int r = 0;

  auto makeNumSpin = [](const QString& suffix) {
    auto* s = new ClampingSpinBox();
    s->setRange(0, kOverlaySpinMaxMb);
    s->setSuffix(" " + suffix);
    // 用户输入超过 max 时，钳到 max（而不是 Qt 默认的"恢复到上个值"），
    // 因为我们的范围是动态计算的，用户的意图是"尽量接近上限"。
    s->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    s->setKeyboardTracking(false);
    // 宽度交给字体、数值范围及原生样式共同测量；同一网格列保持等宽。
    // 固定像素宽度在放大字号或不同 Windows 样式下可能裁掉数值。
    s->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    return s;
  };
  auto addNumRow = [&](const QString& label, QSpinBox*& next, const QString& suffix, QLabel** outLabel = nullptr) {
    next = makeNumSpin(suffix);
    auto* keyLbl = makeKey(label);
    if (outLabel) *outLabel = keyLbl;
    overlayGrid->addWidget(keyLbl, r, 0);
    overlayGrid->addWidget(next, r, 1);
    ++r;
  };

  // 类型 / 最大大小 都是 UWF_OverlayConfig 的 Set* 调用，要求当前会话 UWF
  // 已禁用；这里给两个控件各挂一个隐藏的 "?" 徽标，setData 时根据 filter
  // 的 CurrentEnabled 决定是否禁用控件并显示徽标。
  const QString lockTip = I18n::tr(
      "Overlay type and maximum size can only be changed while the filter is disabled:\n1. Disable the filter using the switch above.\n2. Reboot (the filter "
      "will be off after reboot).\n3. Change this setting.");

  m_overlayTypeNext = makeOverlayTypeCombo();
  m_overlayTypeNext->setToolTip(I18n::tr(
      "Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot."));
  m_typeLockedHint = makeLockedHint(lockTip);
  overlayGrid->addWidget(makeKey(I18n::tr("Type")), r, 0);
  overlayGrid->addWidget(wrapWithLockedHint(m_overlayTypeNext, m_typeLockedHint), r, 1, Qt::AlignRight);
  ++r;

  m_maxNext = makeNumSpin("MB");
  m_maxNext->setToolTip(
      I18n::tr("Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free "
               "space on the system volume."));
  m_maxLabel = makeKey(I18n::tr("Maximum size"));
  m_maxLockedHint = makeLockedHint(lockTip);
  overlayGrid->addWidget(m_maxLabel, r, 0);
  overlayGrid->addWidget(wrapWithLockedHint(m_maxNext, m_maxLockedHint), r, 1, Qt::AlignRight);
  ++r;

  addNumRow(I18n::tr("Warning threshold"), m_warnNext, "MB");
  m_warnNext->setToolTip(
      I18n::tr("Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical "
               "threshold. Set to 0 to disable this event."));
  addNumRow(I18n::tr("Critical threshold"), m_critNext, "MB");
  m_critNext->setToolTip(
      I18n::tr("Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning "
               "threshold. Set to 0 to disable this event."));

  m_totalRamMb = systemTotalRamMb();

  m_usedLabel = new QLabel("—");
  m_usedLabel->setObjectName("statusCurrent");
  m_usedLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  // 这行只有 QLabel，自然高度比 spinbox 行矮一截；显式对齐到 spinbox 的
  // sizeHint 高度，否则 QGridLayout 会按"最高控件"收缩此行，视觉上比前几行
  // 更贴近上一行。AlignRight|AlignVCenter 让文字在这个高度里居中。
  m_usedLabel->setMinimumHeight(m_maxNext->sizeHint().height());
  auto* usedKey = makeKey(I18n::tr("Used / total"));
  usedKey->setMinimumHeight(m_maxNext->sizeHint().height());
  overlayGrid->addWidget(usedKey, r, 0);
  overlayGrid->addWidget(m_usedLabel, r, 1, Qt::AlignRight | Qt::AlignVCenter);
  ++r;

  m_usageBar = new OverlayUsageBar();
  overlayGrid->addWidget(m_usageBar, r, 0, 1, 2);
  ++r;

  m_overlayLegend = new QLabel();
  m_overlayLegend->setTextFormat(Qt::RichText);
  m_overlayLegend->setObjectName("usageLegend");
  overlayGrid->addWidget(m_overlayLegend, r, 0, 1, 2);
  overlayGrid->setColumnStretch(1, 1);

  body->addWidget(makeSection(I18n::tr("Overlay"), overlayGrid));
  outer->addWidget(m_controls);

  connect(m_filterNext, &QAbstractButton::toggled, this, &GlobalStatusPanel::emitIfChanged);
  connect(m_overlayTypeNext, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
    refreshTypeDependentUi();
    reconfigureRanges();
    emitIfChanged();
  });
  for (auto* s : {m_maxNext, m_warnNext, m_critNext}) {
    // valueChanged：值每变一次都实时重绘横条（已占用/阈值预览）；
    // 不要在这里强推 chain，否则用户打字中途 setValue 会把光标踢飞。
    connect(s, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() {
      refreshTypeDependentUi();
      emitIfChanged();
    });
    // editingFinished：光标离开或回车时，按最新值重算各 spinbox 的依赖上限，
    // Qt 会自动把超限的当前值钳回新 range（max 变小时 crit 自动跌落，
    // crit 变小时 warn 自动跌落），实现"约束向下传递"。
    connect(s, &QSpinBox::editingFinished, this, [this]() {
      reconfigureRanges();
      refreshTypeDependentUi();
      emitIfChanged();
    });
  }

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { applyTheme(); });
  applyTheme();
}

void GlobalStatusPanel::applyTheme() const {
  const auto& tm = ThemeManager::instance();
  if (m_overlayLegend) {
    const QString accent = tm.color(Sem::Accent).name();
    const QString warn = tm.color(Sem::Warn).name();
    const QString danger = tm.color(Sem::Danger).name();
    m_overlayLegend->setText(
        I18n::tr("<span style='color:%1'>■</span> Used &nbsp; <span style='color:%2'>■</span> Warning &nbsp; <span style='color:%3'>■</span> Critical")
            .arg(accent, warn, danger));
  }
}

void GlobalStatusPanel::reconfigureRanges() const {
  // 约束向下传递 —— 每个 spinbox 的区间取决于覆盖层类型与"更粗一级"的当前值：
  //   - max  ∈ [maxFloor, maxCeiling]
  //     * RAM 模式  : maxFloor = 0；maxCeiling = 系统总内存 MB
  //     * Disk 模式 : maxFloor = config::kDiskOverlayMinSizeMb（基于磁盘的覆盖层
  //       UWF 要求至少 1024 MB）；maxCeiling = spinbox 最大值（物理上限来自目标卷
  //       容量，不在这里卡）
  //   - crit ∈ [0, max 当前值]
  //   - warn ∈ [0, crit 当前值]
  // crit / warn 的下限一律是 0：当前已占用是动态值，不能当下限——否则用户输入
  // 偏小值时会被吸到 currentConsumption，视觉上像"系统擅自填入了无意义的数字"。
  // setRange 会把超出新区间的 value 自动钳回边界：切到 Disk 时低于 1024 的
  // max 会被抬到 1024；max 下调时 crit 联动下压、crit 下调时 warn 跟着下压。
  // 超过上限的字符由 ClampingSpinBox::fixup() 在失焦/回车时吸到 maximum()。
  const bool isRam = static_cast<OverlayType>(m_overlayTypeNext->currentData().toInt()) == OverlayType::RAM;
  const int maxCeiling = (isRam && m_totalRamMb > 0) ? toOverlaySpinValue(m_totalRamMb) : kOverlaySpinMaxMb;
  const int maxFloor = isRam ? 0 : static_cast<int>(config::kDiskOverlayMinSizeMb);

  m_maxNext->setRange(maxFloor, maxCeiling);
  m_critNext->setRange(0, strictThresholdUpper(m_maxNext->value()));
  m_warnNext->setRange(0, strictThresholdUpper(m_critNext->value()));
}

void GlobalStatusPanel::refreshTypeDependentUi() {
  const bool isRam = static_cast<OverlayType>(m_overlayTypeNext->currentData().toInt()) == OverlayType::RAM;

  if (m_maxLabel) {
    if (isRam && m_totalRamMb > 0) {
      m_maxLabel->setText(I18n::tr("Maximum size · RAM %1").arg(QString::fromStdString(formatMb(m_totalRamMb))));
    } else {
      m_maxLabel->setText(I18n::tr("Maximum size"));
    }
  }

  if (m_usageBar) {
    m_usageBar->setOverlayData(m_currentConsumptionMb, static_cast<uint32_t>(m_warnNext->value()), static_cast<uint32_t>(m_critNext->value()),
                               static_cast<uint32_t>(m_maxNext->value()), isRam);
  }
}

void GlobalStatusPanel::setCompatibilityNotice(const QString& text) {
  m_compatBanner->setText("⚠ " + text);
  m_compatBanner->show();
}

void GlobalStatusPanel::setUnavailable(const QString& reason) {
  m_available = false;
  m_banner->setProperty("level", "");
  m_banner->setText("⚠ " + I18n::tr("UWF status unavailable: ") + reason);
  // 不设 level=warn —— UWF 不可用是硬错误，用默认的 statusBanner 红色错误样式。
  m_banner->show();
  // 不禁用整个面板，警告文本仍可阅读；宿主窗口负责滚动。
  m_controls->setEnabled(false);
  // UWF 不可用时 ? 徽标无意义，强制隐藏。
  if (m_typeLockedHint) m_typeLockedHint->hide();
  if (m_maxLockedHint) m_maxLockedHint->hide();
}

void GlobalStatusPanel::showVolumeInfoWarning(const QString& reason) const {
  m_banner->setProperty("level", "warn");
  m_banner->setText("⚠ " + I18n::tr("Failed to read volume information: %1").arg(reason));
  m_banner->show();
}

void GlobalStatusPanel::showElevationRequired() const {
  // 复用同一条红色状态横幅（statusBanner 默认即红色错误样式）。数据已由
  // setData 正常填好，这里只补一条横幅说明为何控件全灰、不可改。
  m_banner->setProperty("level", "");
  m_banner->setText("⚠ " + I18n::tr("Administrator privileges are required to change UWF settings. "
                                    "Restart the program via right-click → \"Run as administrator\"."));
  m_banner->show();
}

void GlobalStatusPanel::setControlsEnabled(const bool enabled) const { m_controls->setEnabled(enabled); }

void GlobalStatusPanel::setData(const core::SessionSnapshot& cur, const core::SessionSnapshot& nxt, const core::OverlayRuntime& rt) {
  m_available = true;
  m_banner->hide();
  const auto children = findChildren<QWidget*>();
  for (auto* w : children) w->setEnabled(true);

  // 当前会话已启用 UWF 时，类型 / 最大大小不允许改（WMI 会拒），禁用并亮起
  // ? 徽标提示用户操作步骤。Qt 默认 disabled 控件不会改 cursor，再显式设
  // ForbiddenCursor 让"不能点"的暗示更直观。
  const bool overlayConfigLocked = cur.filter.enabled;
  m_overlayTypeNext->setEnabled(!overlayConfigLocked);
  m_maxNext->setEnabled(!overlayConfigLocked);
  m_overlayTypeNext->setCursor(overlayConfigLocked ? Qt::ForbiddenCursor : Qt::ArrowCursor);
  m_maxNext->setCursor(overlayConfigLocked ? Qt::ForbiddenCursor : Qt::IBeamCursor);
  m_typeLockedHint->setVisible(overlayConfigLocked);
  m_maxLockedHint->setVisible(overlayConfigLocked);

  m_filterCur->setText(enabledStateLabel(cur.filter.enabled));

  m_baselineFilter = nxt.filter;
  m_baselineOverlay = nxt.overlay;
  m_currentConsumptionMb = rt.currentConsumptionMb;

  m_filterNext->blockSignals(true);
  m_filterNext->setChecked(nxt.filter.enabled);
  m_filterNext->blockSignals(false);

  // 暂时放宽 spinbox 量程，让 baseline 值能被写入；
  // 随后由 reconfigureRanges 收紧到满足约束链的区间。
  for (auto* s : {m_maxNext, m_warnNext, m_critNext}) {
    s->blockSignals(true);
    s->setRange(0, kOverlaySpinMaxMb);
    s->blockSignals(false);
  }

  m_overlayTypeNext->blockSignals(true);
  setComboValue(m_overlayTypeNext, static_cast<int>(nxt.overlay.type));
  m_overlayTypeNext->blockSignals(false);

  auto setNum = [](QSpinBox* ns, uint32_t nxtV) {
    ns->blockSignals(true);
    ns->setValue(toOverlaySpinValue(nxtV));
    ns->blockSignals(false);
  };
  setNum(m_maxNext, nxt.overlay.maximumSizeMb);
  setNum(m_warnNext, nxt.overlay.warningThresholdMb);
  setNum(m_critNext, nxt.overlay.criticalThresholdMb);

  const uint32_t totalMb = rt.availableSpaceMb + rt.currentConsumptionMb;
  m_usedLabel->setText(formatUsageText(rt.currentConsumptionMb, totalMb));

  refreshTypeDependentUi();
  reconfigureRanges();
  updateDirtyStyle();
}

void GlobalStatusPanel::updateUsage(const core::OverlayRuntime& rt) {
  if (!m_available) return;  // 面板不可用（无 UWF）时没有占用可显示
  m_currentConsumptionMb = rt.currentConsumptionMb;
  const uint32_t totalMb = rt.availableSpaceMb + rt.currentConsumptionMb;
  m_usedLabel->setText(formatUsageText(rt.currentConsumptionMb, totalMb));
  refreshTypeDependentUi();  // 用新的 m_currentConsumptionMb 重新喂占用条
}

void GlobalStatusPanel::updateDirtyStyle() {
  markDirty(m_filterNext, m_filterNext->isChecked() != m_baselineFilter.enabled);
  markDirty(m_overlayTypeNext, static_cast<OverlayType>(m_overlayTypeNext->currentData().toInt()) != m_baselineOverlay.type);
  markDirty(m_maxNext, static_cast<uint32_t>(m_maxNext->value()) != m_baselineOverlay.maximumSizeMb);
  markDirty(m_warnNext, static_cast<uint32_t>(m_warnNext->value()) != m_baselineOverlay.warningThresholdMb);
  markDirty(m_critNext, static_cast<uint32_t>(m_critNext->value()) != m_baselineOverlay.criticalThresholdMb);
}

void GlobalStatusPanel::emitIfChanged() {
  updateDirtyStyle();
  emit pendingChanged();
}

std::optional<bool> GlobalStatusPanel::pendingFilterEnabled() const {
  if (!m_available) return std::nullopt;
  const bool v = m_filterNext->isChecked();
  return v == m_baselineFilter.enabled ? std::nullopt : std::optional<bool>(v);
}

bool GlobalStatusPanel::importFilterEnabled(bool v) {
  if (!m_available) return false;
  if (m_filterNext->isChecked() == v) return false;
  m_filterNext->setChecked(v);
  return true;
}

bool GlobalStatusPanel::importOverlayType(core::OverlayType t) {
  if (!m_available) return false;
  if (static_cast<OverlayType>(m_overlayTypeNext->currentData().toInt()) == t) return false;
  setComboValue(m_overlayTypeNext, static_cast<int>(t));
  return true;
}

bool GlobalStatusPanel::importOverlayMaxMb(uint32_t mb) {
  if (!m_available) return false;
  // 临时放宽 range 让任意 MB 都能写入，不然导入比当前 max 大的 size 会被
  // setValue 静默 clamp 到旧上限，看起来"导入成功了但值不对"。约束链不在这里
  // 收紧——import* 用 setValue 只触发 valueChanged、不触发 editingFinished；
  // 由 caller 在整批导入结束后调 finishImport() 统一收紧。
  if (static_cast<uint32_t>(m_maxNext->value()) == mb) return false;
  m_maxNext->setRange(0, kOverlaySpinMaxMb);
  m_maxNext->setValue(toOverlaySpinValue(mb));
  return true;
}

bool GlobalStatusPanel::importOverlayWarnMb(uint32_t mb) {
  if (!m_available) return false;
  if (static_cast<uint32_t>(m_warnNext->value()) == mb) return false;
  m_warnNext->setRange(0, kOverlaySpinMaxMb);
  m_warnNext->setValue(toOverlaySpinValue(mb));
  return true;
}

bool GlobalStatusPanel::importOverlayCritMb(uint32_t mb) {
  if (!m_available) return false;
  if (static_cast<uint32_t>(m_critNext->value()) == mb) return false;
  m_critNext->setRange(0, kOverlaySpinMaxMb);
  m_critNext->setValue(toOverlaySpinValue(mb));
  return true;
}

void GlobalStatusPanel::finishImport() const {
  if (!m_available) return;
  // import* 用 setValue 写入，只触发 valueChanged、不触发 editingFinished，故
  // 约束链不会自动收紧、range 仍停在导入时放宽的状态。这里收紧 range 并重建
  // warn ≤ crit ≤ max，超限的现值由 setRange 当场钳回（钳值经 valueChanged
  // 正常发出 pendingChanged）。
  reconfigureRanges();
}

core::OverlayConfigDelta GlobalStatusPanel::pendingOverlay() const {
  core::OverlayConfigDelta d;
  if (!m_available) return d;
  const auto& b = m_baselineOverlay;
  const auto t = static_cast<OverlayType>(m_overlayTypeNext->currentData().toInt());
  if (t != b.type) d.type = t;
  if (static_cast<uint32_t>(m_maxNext->value()) != b.maximumSizeMb) d.maximumSizeMb = static_cast<uint32_t>(m_maxNext->value());
  if (static_cast<uint32_t>(m_warnNext->value()) != b.warningThresholdMb) d.warningThresholdMb = static_cast<uint32_t>(m_warnNext->value());
  if (static_cast<uint32_t>(m_critNext->value()) != b.criticalThresholdMb) d.criticalThresholdMb = static_cast<uint32_t>(m_critNext->value());
  return d;
}

}  // namespace uwf::ui
