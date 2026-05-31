#include "StatusPanel.h"

#include <QComboBox>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>

#include "I18n.h"
#include "SwitchButton.h"
#include "ThemeManager.h"
#include "UiUtil.h"

namespace uwf::ui {

namespace {

QLabel* makeKey(const QString& text) {
  auto* l = new QLabel(text);
  l->setObjectName("inlineKey");
  return l;
}

QFrame* makeBareCard(QLayout* inner) {
  auto* f = new QFrame();
  f->setObjectName("statusCard");
  f->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto* v = new QVBoxLayout(f);
  v->setContentsMargins(16, 12, 16, 14);
  v->setSpacing(10);
  v->addLayout(inner);
  return f;
}

}  // namespace

StatusPanel::StatusPanel(QWidget* parent) : QWidget(parent) {
  auto* outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  outer->setSpacing(8);

  m_banner = new QLabel(this);
  m_banner->setObjectName("statusBanner");
  m_banner->setWordWrap(true);
  m_banner->hide();
  outer->addWidget(m_banner);

  // 保护状态 + 绑定方式 放同一行；每组的形式是：
  //   标签：[当前值] →[下次会话编辑控件]
  // 这样用户可以同时看到本次会话的生效值和将要写入的值。
  auto* row = new QHBoxLayout();
  row->setSpacing(6);
  m_row = row;

  m_protectCur = new QLabel("—");
  m_protectCur->setObjectName("statusCurLabel");
  m_protectCur->setToolTip(I18n::tr("Protection state of this volume in the current session (read-only)."));
  m_protectNext = new SwitchButton();
  m_protectNext->setToolTip(I18n::tr("Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot."));
  row->addWidget(makeKey(I18n::tr("Protection:")));
  row->addWidget(m_protectCur, 0, Qt::AlignVCenter);
  {
    m_arrow = new QLabel();
    m_arrow->setObjectName("statusArrow");
    m_arrow->setPixmap(ThemeManager::instance().icon(":/icons/arrow_right.svg").pixmap(14, 14));
    m_arrow->setFixedSize(18, 18);
    m_arrow->setAlignment(Qt::AlignCenter);
    row->addWidget(m_arrow, 0, Qt::AlignVCenter);
  }
  row->addWidget(m_protectNext, 0, Qt::AlignVCenter);

  row->addSpacing(28);

  // 绑定方式不是 on/off，保留为选择框：按盘符 / 按卷 ID。
  // data=true → bBindByVolumeName=true → 按卷 ID
  m_bindNext = new QComboBox();
  m_bindNext->addItem(I18n::tr("By drive letter"), false);
  m_bindNext->addItem(I18n::tr("By volume ID"), true);
  m_bindNext->setToolTip(
      I18n::tr("How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing "
               "other disks). Volume ID stays stable across drive letter changes."));
  row->addWidget(makeKey(I18n::tr("Bind by:")));
  row->addWidget(m_bindNext);

  row->addStretch(1);

  outer->addWidget(makeBareCard(row));

  connect(m_protectNext, &QAbstractButton::toggled, this, &StatusPanel::emitIfChanged);
  connect(m_bindNext, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StatusPanel::emitIfChanged);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) {
    if (m_arrow) {
      m_arrow->setPixmap(ThemeManager::instance().icon(":/icons/arrow_right.svg").pixmap(14, 14));
    }
  });
}

void StatusPanel::addTrailingAction(QWidget* w) const {
  if (!m_row || !w) return;
  // 直接 append 到 row 末尾；由于 row 末尾已经有一个 addStretch，
  // 这里新增的 widget 会被 stretch 推到行尾，实现右对齐。
  // 同时把 widget reparent 到 card 上，让它随 setUnsupported 的遍历一起被
  // 置灰（调用方可在其上再次 setEnabled(true) 来覆盖）。
  m_row->addWidget(w);
}

void StatusPanel::setUnsupported(const QString& reason) {
  m_supported = false;
  m_hasVolume = false;
  m_banner->setText("⚠ " + I18n::tr("This volume is not supported by UWF: ") + reason);
  m_banner->setProperty("level", "warn");
  m_banner->show();
  const auto children = findChildren<QWidget*>();
  for (auto* w : children)
    if (w != m_banner) w->setEnabled(false);
  m_banner->setEnabled(true);
}

void StatusPanel::setNotice(const QString& text) {
  // 不动 m_supported / m_hasVolume，控件保持原本的 enabled/disabled 状态。
  // sticky 标记让后续 setData 不把 banner 隐藏掉。
  m_hasStickyNotice = true;
  m_banner->setText("⚠ " + text);
  m_banner->setProperty("level", "warn");
  m_banner->show();
}

void StatusPanel::setData(const core::VolumeRecord* cvol, const core::VolumeRecord* nvol) {
  m_supported = true;
  // sticky notice（如 FS 受限提示、调试用 banner）保留显示；只在没有 notice
  // 时把上一轮 setUnsupported 的横幅清掉。
  if (!m_hasStickyNotice) m_banner->hide();
  const auto children = findChildren<QWidget*>();
  for (auto* w : children) w->setEnabled(true);

  m_hasVolume = (cvol != nullptr || nvol != nullptr);
  m_baselineProtect = nvol ? nvol->isProtected : (cvol ? cvol->isProtected : false);
  const auto* bindSrc = nvol ? nvol : cvol;
  m_baselineBindByVolumeName = bindSrc ? !bindSrc->bindByDriveLetter : false;

  const QString curProtectText = cvol ? (cvol->isProtected ? I18n::tr("Enabled") : I18n::tr("Disabled")) : QStringLiteral("—");
  m_protectCur->setText(curProtectText);

  if (m_hasVolume) {
    m_protectNext->blockSignals(true);
    m_protectNext->setChecked(m_baselineProtect);
    m_protectNext->blockSignals(false);
    m_protectNext->setEnabled(true);

    m_bindNext->blockSignals(true);
    setComboValue(m_bindNext, m_baselineBindByVolumeName);
    m_bindNext->blockSignals(false);
    m_bindNext->setEnabled(true);
  } else {
    m_protectNext->setEnabled(false);
    m_bindNext->setEnabled(false);
  }

  // 上面只按"有无卷数据"点亮控件；可写性（提权 + UWF 可用）由调用方紧接着
  // 用 setControlsEnabled 叠加收口。
  updateDirtyStyle();
}

void StatusPanel::setControlsEnabled(const bool enabled) const {
  // 与 m_hasVolume 取与：无卷数据时保护开关 / 绑定方式本就该禁用，可写也不点亮。
  m_protectNext->setEnabled(enabled && m_hasVolume);
  m_bindNext->setEnabled(enabled && m_hasVolume);
}

void StatusPanel::updateDirtyStyle() {
  if (!m_hasVolume) return;
  markDirty(m_protectNext, m_protectNext->isChecked() != m_baselineProtect);
  markDirty(m_bindNext, m_bindNext->currentData().toBool() != m_baselineBindByVolumeName);
}

void StatusPanel::emitIfChanged() {
  updateDirtyStyle();
  emit pendingChanged();
}

std::optional<bool> StatusPanel::pendingVolumeProtected() const {
  if (!m_supported || !m_hasVolume) return std::nullopt;
  const bool v = m_protectNext->isChecked();
  return v == m_baselineProtect ? std::nullopt : std::optional<bool>(v);
}

bool StatusPanel::importProtect(bool v) {
  if (!m_supported || !m_hasVolume) return false;
  if (m_protectNext->isChecked() == v) return false;
  m_protectNext->setChecked(v);  // 触发 toggled → emitIfChanged
  return true;
}

bool StatusPanel::importBindByVolumeName(bool v) {
  if (!m_supported || !m_hasVolume) return false;
  if (m_bindNext->currentData().toBool() == v) return false;
  setComboValue(m_bindNext, v);  // 触发 currentIndexChanged → emitIfChanged
  return true;
}

std::optional<bool> StatusPanel::pendingBindByVolumeName() const {
  if (!m_supported || !m_hasVolume) return std::nullopt;
  const bool v = m_bindNext->currentData().toBool();
  return v == m_baselineBindByVolumeName ? std::nullopt : std::optional<bool>(v);
}

}  // namespace uwf::ui
