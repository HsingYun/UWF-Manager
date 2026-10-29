// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "GlobalStatusPanel.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <algorithm>
#include <format>
#include <limits>
#include <utility>

#include "core/UwfConstraints.h"
#include "shared/ByteFormat.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/overlay/OverlayUsageBar.h"
#include "ui/widgets/ComboBoxSelection.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/Notice.h"
#include "ui/widgets/NumberBox.h"
#include "ui/widgets/SelectBox.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/SwitchButton.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {

using core::OverlayType;

namespace {

constexpr int kOverlaySpinMaxMb = std::numeric_limits<int>::max();

// 将纯数字越界输入保留为 Intermediate，允许继续编辑，并在提交编辑时修正到边界。
class ClampingSpinBox : public NumberBox {
 public:
  using NumberBox::NumberBox;

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

  // 显式将越界值限制到有效范围，避免默认 fixup 恢复为上一次值。
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
  auto* c = new SelectBox();
  c->addItem("RAM", static_cast<int>(OverlayType::RAM));
  c->addItem("Disk", static_cast<int>(OverlayType::Disk));
  return c;
}

QLabel* makeKey(const char* source) {
  auto* l = I18n::text(new TextLabel({}, TextStyle::Caption), [source] { return I18n::tr(source); });
  l->setProperty("designTone", "muted");
  l->setWordWrap(false);
  l->setObjectName("statusKey");
  return l;
}

QLabel* makeLockedHint(const QString& tooltip) {
  auto* l = new QLabel("?");
  l->setObjectName("lockedHint");
  l->setAlignment(Qt::AlignCenter);
  l->setToolTip(tooltip);
  l->setCursor(Qt::WhatsThisCursor);
  l->hide();
  return l;
}

struct SettingsLayoutMetrics {
  int rowHeight;
  int rowGap;
  int side;
  int headingTop;
  int headingBottom;
  std::optional<int> formBottom;
  int usageTop;
  int usageBottom;
  int captionGap;
  int usageGap;
  int sectionGap;
};
SettingsLayoutMetrics settingsLayoutMetrics(const QSize& size) {
  if (size.height() <= design::smallWindowHeight) return {design::smallSettingsRowHeight, design::smallSettingsRowGap, 11, 6, 4, 8, 7, 8, 0, 6, 8};
  if (size.height() <= design::shortWindowHeight) return {design::compactSettingsRowHeight, design::compactSettingsRowGap, 12, 8, 6, 10, 9, 10, 1, 8, 10};
  return {design::settingsRowHeight, design::settingsRowGap, 14, 13, 10, std::nullopt, 14, 15, 5, 8, 18};
}
}  // namespace

QWidget* GlobalStatusPanel::makeSection(const char* title, QGridLayout* inner, QWidget* hint, QWidget* footer) {
  auto* card = new Surface();
  card->setObjectName(QStringLiteral("statusCard"));
  card->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  card->setClipContent(true);
  card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  auto* stack = new QVBoxLayout(card);
  stack->setContentsMargins(1, 1, 1, 1);
  stack->setSpacing(0);
  auto* heading = new QWidget(card);
  auto* headingRow = new QHBoxLayout(heading);
  headingRow->setContentsMargins(14, 13, 14, 10);
  auto* caption = I18n::text(new TextLabel({}, TextStyle::Section, heading), [title] { return I18n::tr(title); });
  auto type = caption->font();
  type.setPixelSize(12);
  caption->setFont(type);
  caption->setFixedHeight(19);
  headingRow->addWidget(caption, 1);
  if (hint) headingRow->addWidget(hint, 0, Qt::AlignVCenter);
  stack->addWidget(heading);
  auto* form = new QWidget(card);
  form->setLayout(inner);
  inner->setContentsMargins(14, 0, 14, footer ? 15 : 14);
  m_sections.push_back({inner, headingRow, footer ? 15 : 14});
  stack->addWidget(form);
  if (footer) stack->addWidget(footer);
  return card;
}

GlobalStatusPanel::GlobalStatusPanel(std::shared_ptr<presentation::GlobalConfigurationViewModel> model, QWidget* parent)
    : QWidget(parent), m_viewModel(std::move(model)) {
  setObjectName("globalStatusPanel");
  auto* outer = new QVBoxLayout(this);
  m_outerLayout = outer;
  outer->setContentsMargins(0, 0, 0, 0);
  outer->setSpacing(8);

  auto* title = new TextLabel(QString{}, TextStyle::Section, this);
  m_title = title;
  I18n::bind(title, [target = title] { target->setText(I18n::tr("Global settings")); });
  title->setObjectName(QStringLiteral("panelTitle"));
  auto headingFont = title->font();
  headingFont.setPixelSize(15);
  title->setFont(headingFont);
  title->setContentsMargins(7, 0, 0, 0);
  outer->setSpacing(18);
  outer->addWidget(title);
  m_controls = new QWidget(this);
  auto* body = new QVBoxLayout(m_controls);
  m_sectionsLayout = body;
  body->setContentsMargins(0, 0, 0, 0);
  body->setSpacing(18);
  auto* filterGrid = new QGridLayout();
  filterGrid->setHorizontalSpacing(8);
  filterGrid->setVerticalSpacing(design::settingsRowGap);
  filterGrid->setRowMinimumHeight(0, design::settingsRowHeight);
  filterGrid->setRowMinimumHeight(1, design::settingsRowHeight);
  m_filterCur = new TextLabel(QStringLiteral("—"), TextStyle::Caption, this);
  m_filterCur->setObjectName(QStringLiteral("statusCurLabel"));
  m_filterCur->setTextFormat(Qt::PlainText);
  I18n::bind(m_filterCur, [target = m_filterCur] { target->setToolTip(I18n::tr("UWF filter state in the current session (read-only).")); });
  m_filterNext = new SwitchButton(this);
  I18n::bind(m_filterNext, [target = m_filterNext] {
    target->setToolTip(
        I18n::tr("Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot."));
  });
  auto* nextRow = new QHBoxLayout();
  nextRow->setContentsMargins(0, 0, 0, 0);
  nextRow->setSpacing(10);
  m_filterNextText = new TextLabel({}, TextStyle::Eyebrow, this);
  m_filterNextText->setProperty("designTone", "muted");
  nextRow->addWidget(m_filterNextText);
  nextRow->addWidget(m_filterNext);
  filterGrid->addWidget(makeKey(QT_TRANSLATE_NOOP("uwf", "Current session")), 0, 0);
  auto* currentState = new QWidget(this);
  auto* currentStateLayout = new QHBoxLayout(currentState);
  currentStateLayout->setContentsMargins(0, 0, 0, 0);
  currentStateLayout->setSpacing(5);
  m_filterCurrentIcon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/status_warning.svg")), 13, design::Color::Muted, currentState);
  m_filterCurrentIcon->setObjectName(QStringLiteral("currentSessionStateIcon"));
  currentStateLayout->addWidget(m_filterCurrentIcon);
  currentStateLayout->addWidget(m_filterCur);
  filterGrid->addWidget(currentState, 0, 1, Qt::AlignTrailing | Qt::AlignVCenter);
  filterGrid->addWidget(makeKey(QT_TRANSLATE_NOOP("uwf", "Next session")), 1, 0);
  filterGrid->addLayout(nextRow, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
  body->addWidget(makeSection(QT_TRANSLATE_NOOP("uwf", "UWF filter"), filterGrid));

  auto* overlayGrid = new QGridLayout();
  overlayGrid->setHorizontalSpacing(8);
  overlayGrid->setVerticalSpacing(design::settingsRowGap);
  auto makeNumSpin = [this] {
    auto* spin = new ClampingSpinBox(this);
    spin->setRange(0, kOverlaySpinMaxMb);
    spin->setUnit(QStringLiteral("MB"));
    spin->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    spin->setKeyboardTracking(false);
    spin->setFixedSize(111, design::settingsRowHeight);
    auto type = spin->font();
    type.setPixelSize(12);
    spin->setFont(type);
    return spin;
  };
  m_overlayTypeNext = makeOverlayTypeCombo();
  auto overlayTypeFont = m_overlayTypeNext->font();
  overlayTypeFont.setPixelSize(12);
  m_overlayTypeNext->setFont(overlayTypeFont);
  m_overlayTypeNext->setObjectName(QStringLiteral("overlayTypeNext"));
  m_overlayTypeNext->setFixedSize(111, design::settingsRowHeight);
  I18n::bind(m_overlayTypeNext, [target = m_overlayTypeNext] {
    target->setToolTip(I18n::tr(
        "Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot."));
  });
  overlayGrid->addWidget(makeKey(QT_TRANSLATE_NOOP("uwf", "Type")), 0, 0);
  overlayGrid->addWidget(m_overlayTypeNext, 0, 1, Qt::AlignRight);
  m_maxNext = makeNumSpin();
  m_warnNext = makeNumSpin();
  m_critNext = makeNumSpin();
  m_maxNext->setObjectName(QStringLiteral("overlayMaximumNext"));
  m_warnNext->setObjectName(QStringLiteral("overlayWarningNext"));
  m_critNext->setObjectName(QStringLiteral("overlayCriticalNext"));
  m_maxLabel = makeKey(QT_TRANSLATE_NOOP("uwf", "Maximum size"));
  overlayGrid->addWidget(m_maxLabel, 1, 0);
  overlayGrid->addWidget(m_maxNext, 1, 1, Qt::AlignRight);
  overlayGrid->addWidget(makeKey(QT_TRANSLATE_NOOP("uwf", "Warning threshold")), 2, 0);
  overlayGrid->addWidget(m_warnNext, 2, 1, Qt::AlignRight);
  overlayGrid->addWidget(makeKey(QT_TRANSLATE_NOOP("uwf", "Critical threshold")), 3, 0);
  overlayGrid->addWidget(m_critNext, 3, 1, Qt::AlignRight);
  for (int row = 0; row < 4; ++row) overlayGrid->setRowMinimumHeight(row, design::settingsRowHeight);
  m_totalRamMb = m_viewModel->totalRamMb();
  I18n::bind(m_maxNext, [target = m_maxNext] {
    target->setToolTip(
        I18n::tr("Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the "
                 "system volume."));
  });
  I18n::bind(m_warnNext, [target = m_warnNext] {
    target->setToolTip(I18n::tr(
        "Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower "
        "than the critical threshold. Set to 0 to disable only the warning event."));
  });
  I18n::bind(m_critNext, [target = m_critNext] {
    target->setToolTip(
        I18n::tr("Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be "
                 "higher than the warning threshold. Set to 0 to disable only the critical event."));
  });
  m_typeLockedHint = makeLockedHint({});
  I18n::bind(m_typeLockedHint, [target = m_typeLockedHint] {
    target->setToolTip(
        I18n::tr("Overlay type and maximum size can only be changed while the filter is disabled:\n1. Disable the filter using the switch above.\n2. Reboot "
                 "(the filter will be off after reboot).\n3. Change this setting."));
  });
  m_typeLockedHint->setText(QString());
  m_typeLockedHint->setFixedSize(20, 20);
  m_typeLockedHint->setPixmap(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")).pixmap(QSize(13, 13), devicePixelRatioF()));
  auto* usage = new Surface(design::Color::Surface, this);
  usage->setObjectName(QStringLiteral("settingsUsage"));
  usage->setBorder(Qt::TopEdge);
  auto* usageLayout = new QVBoxLayout(usage);
  m_usageLayout = usageLayout;
  usageLayout->setContentsMargins(14, 14, 14, 15);
  usageLayout->setSpacing(0);
  auto* caption = new TextLabel(QString{}, TextStyle::Eyebrow, usage);
  I18n::bind(caption, [target = caption] { target->setText(I18n::tr("Current usage")); });
  caption->setProperty("designTone", "muted");
  usageLayout->addWidget(caption);
  m_usageCaptionGap = new QSpacerItem(0, 5, QSizePolicy::Minimum, QSizePolicy::Fixed);
  usageLayout->addSpacerItem(m_usageCaptionGap);
  auto* values = new QHBoxLayout();
  m_usagePercent = new TextLabel(QStringLiteral("—"), TextStyle::Section, usage);
  m_usagePercent->setLayoutDirection(Qt::LeftToRight);
  auto percentFont = m_usagePercent->font();
  percentFont.setPixelSize(19);
  m_usagePercent->setFont(percentFont);
  m_usedLabel = new TextLabel(QStringLiteral("—"), TextStyle::Eyebrow, usage);
  m_usedLabel->setObjectName(QStringLiteral("statusCurrent"));
  m_usedLabel->setProperty("designTone", "muted");
  values->addWidget(m_usagePercent);
  values->addStretch();
  values->addWidget(m_usedLabel, 0, Qt::AlignBottom);
  usageLayout->addLayout(values);
  m_usageBar = new OverlayUsageBar(usage);
  m_usageBarGap = new QSpacerItem(0, 8, QSizePolicy::Minimum, QSizePolicy::Fixed);
  usageLayout->addSpacerItem(m_usageBarGap);
  usageLayout->addWidget(m_usageBar);
  m_usageLegendGap = new QSpacerItem(0, 8, QSizePolicy::Minimum, QSizePolicy::Fixed);
  usageLayout->addSpacerItem(m_usageLegendGap);
  m_overlayLegend = new TextLabel({}, TextStyle::Eyebrow, usage);
  m_overlayLegend->setTextFormat(Qt::PlainText);
  m_overlayLegend->setObjectName(QStringLiteral("usageLegend"));
  m_overlayLegend->setWordWrap(true);
  m_criticalLegend = new TextLabel({}, TextStyle::Eyebrow, usage);
  auto* legend = new QHBoxLayout();
  legend->setContentsMargins(0, 0, 0, 0);
  legend->setSpacing(5);
  legend->addWidget(m_overlayLegend, 1);
  legend->addWidget(m_criticalLegend);
  usageLayout->addLayout(legend);
  body->addWidget(makeSection(QT_TRANSLATE_NOOP("uwf", "Overlay"), overlayGrid, m_typeLockedHint, usage));
  outer->addWidget(m_controls);

  connect(m_filterNext, &QAbstractButton::toggled, this, [this](bool value) { m_viewModel->editFilterEnabled(value); });
  connect(m_overlayTypeNext, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this] { m_viewModel->editOverlayType(static_cast<OverlayType>(m_overlayTypeNext->currentData().toInt())); });
  connect(m_maxNext, QOverload<int>::of(&QSpinBox::valueChanged), m_viewModel.get(), &presentation::GlobalConfigurationViewModel::editMaximumSize);
  connect(m_warnNext, QOverload<int>::of(&QSpinBox::valueChanged), m_viewModel.get(), &presentation::GlobalConfigurationViewModel::editWarningThreshold);
  connect(m_critNext, QOverload<int>::of(&QSpinBox::valueChanged), m_viewModel.get(), &presentation::GlobalConfigurationViewModel::editCriticalThreshold);
  connect(m_viewModel.get(), &presentation::GlobalConfigurationViewModel::availabilityChanged, this, &GlobalStatusPanel::renderAvailability);
  connect(m_viewModel.get(), &presentation::GlobalConfigurationViewModel::usageChanged, this, &GlobalStatusPanel::renderUsage);
  connect(m_viewModel.get(), &presentation::GlobalConfigurationViewModel::stateChanged, this, [this] {
    renderEditor();
    renderAvailability();
    refreshTypeDependentUi();
  });

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { applyTheme(); });
  renderEditor();
  renderAvailability();
  applyTheme();
  I18n::bind(this, [this] {
    m_filterCur->setText(!m_viewModel->available()             ? I18n::tr("Unavailable")
                         : m_viewModel->currentFilterEnabled() ? I18n::tr("Enabled")
                                                               : I18n::tr("Disabled"));
    m_filterNextText->setText(m_viewModel->draft().filterEnabled ? I18n::tr("On") : I18n::tr("Off"));
    refreshTypeDependentUi();
    renderUsage();
  });
}

void GlobalStatusPanel::applyTheme() {
  const auto& theme = ThemeManager::instance();
  if (m_typeLockedHint) m_typeLockedHint->setPixmap(theme.icon(QStringLiteral(":/icons/lock.svg")).pixmap(QSize(13, 13), devicePixelRatioF()));
  for (auto* label : {m_overlayLegend, m_criticalLegend}) {
    if (!label) continue;
    auto type = label->font();
    type.setPixelSize(10);
    label->setFont(type);
  }
  static_cast<TextLabel*>(m_overlayLegend)->setColorRole(design::Color::Faint);
  static_cast<TextLabel*>(m_criticalLegend)->setColorRole(design::Color::Faint);
  renderUsage();
}

void GlobalStatusPanel::setLayoutSize(const QSize& size) {
  const auto metrics = settingsLayoutMetrics(size);
  for (const auto& section : m_sections) {
    section.rows->setVerticalSpacing(metrics.rowGap);
    for (int row = 0; row < section.rows->rowCount(); ++row) section.rows->setRowMinimumHeight(row, metrics.rowHeight);
    section.heading->setContentsMargins(metrics.side, metrics.headingTop, metrics.side, metrics.headingBottom);
    section.rows->setContentsMargins(metrics.side, 0, metrics.side, metrics.formBottom.value_or(section.bottomMargin));
  }
  for (auto* field :
       {static_cast<QWidget*>(m_overlayTypeNext), static_cast<QWidget*>(m_maxNext), static_cast<QWidget*>(m_warnNext), static_cast<QWidget*>(m_critNext)})
    field->setFixedHeight(metrics.rowHeight);
  m_usageLayout->setContentsMargins(metrics.side, metrics.usageTop, metrics.side, metrics.usageBottom);
  m_usageCaptionGap->changeSize(0, metrics.captionGap, QSizePolicy::Minimum, QSizePolicy::Fixed);
  for (auto* gap : {m_usageBarGap, m_usageLegendGap}) gap->changeSize(0, metrics.usageGap, QSizePolicy::Minimum, QSizePolicy::Fixed);
  m_usageLayout->invalidate();
  m_title->setContentsMargins(size.width() <= design::compactWindowWidth ? 6 : 7, 0, 0, 0);
  m_outerLayout->setSpacing(metrics.sectionGap);
  m_sectionsLayout->setSpacing(metrics.sectionGap);
}

void GlobalStatusPanel::renderEditor() const {
  const QSignalBlocker filterSignals(m_filterNext);
  const QSignalBlocker typeSignals(m_overlayTypeNext);
  const QSignalBlocker maxSignals(m_maxNext);
  const QSignalBlocker warningSignals(m_warnNext);
  const QSignalBlocker criticalSignals(m_critNext);
  const auto& draft = m_viewModel->draft();
  m_filterNext->setChecked(draft.filterEnabled);
  m_filterNextText->setText(draft.filterEnabled ? I18n::tr("On") : I18n::tr("Off"));
  setComboValue(m_overlayTypeNext, static_cast<int>(draft.overlay.type));
  const auto maximum = m_viewModel->maximumRange();
  const auto critical = m_viewModel->criticalRange();
  const auto warning = m_viewModel->warningRange();
  const auto spinValue = [](const uint32_t value) { return static_cast<int>(std::min(value, static_cast<uint32_t>(kOverlaySpinMaxMb))); };
  m_maxNext->setRange(maximum.minimum, maximum.maximum);
  m_maxNext->setValue(spinValue(draft.overlay.maximumSizeMb));
  m_critNext->setRange(critical.minimum, critical.maximum);
  m_critNext->setValue(spinValue(draft.overlay.criticalThresholdMb));
  m_warnNext->setRange(warning.minimum, warning.maximum);
  m_warnNext->setValue(spinValue(draft.overlay.warningThresholdMb));
}

void GlobalStatusPanel::refreshTypeDependentUi() {
  const bool isRam = static_cast<OverlayType>(m_overlayTypeNext->currentData().toInt()) == OverlayType::RAM;

  m_maxLabel->setText(I18n::tr("Maximum size"));
  if (isRam && m_totalRamMb > 0)
    m_maxLabel->setToolTip(I18n::tr("Maximum size · RAM %1").arg(QString::fromStdString(formatMb(m_totalRamMb))));
  else
    m_maxLabel->setToolTip(QString());
  applyTheme();
}

void GlobalStatusPanel::renderAvailability() const {
  const bool available = m_viewModel->available();
  const bool enabled = available && m_viewModel->currentFilterEnabled();
  m_filterCur->setText(!available ? I18n::tr("Unavailable") : enabled ? I18n::tr("Enabled") : I18n::tr("Disabled"));
  const auto icon = !available ? QStringLiteral(":/icons/status_warning.svg")
                    : enabled  ? QStringLiteral(":/icons/check.svg")
                               : QStringLiteral(":/icons/remove.svg");
  m_filterCurrentIcon->setIcon(ThemeManager::instance().icon(icon));
  m_filterCurrentIcon->setColorRole(enabled ? design::Color::Success : design::Color::Muted);
  static_cast<TextLabel*>(m_filterCur)->setColorRole(enabled ? design::Color::Success : design::Color::Muted);
  m_controls->setEnabled(m_viewModel->controlsEnabled());
  const bool locked = m_viewModel->overlayConfigLocked();
  m_overlayTypeNext->setEnabled(!locked);
  m_maxNext->setEnabled(!locked);
  m_overlayTypeNext->setCursor(locked ? Qt::ForbiddenCursor : Qt::ArrowCursor);
  m_maxNext->setCursor(locked ? Qt::ForbiddenCursor : Qt::IBeamCursor);
  m_typeLockedHint->setVisible(m_viewModel->available() && locked);
}

void GlobalStatusPanel::renderUsage() {
  const auto usage = m_viewModel->usage();
  m_usedLabel->setLayoutDirection(usage.enabled ? Qt::LeftToRight : layoutDirection());
  m_usedLabel->setText(usage.valueText);
  m_usagePercent->setText(usage.percentageText);
  using Level = core::OverlayUsageSeverity;
  static_cast<TextLabel*>(m_usagePercent)
      ->setColorRole(!usage.enabled                      ? design::Color::Faint
                     : usage.severity == Level::Critical ? design::Color::Danger
                     : usage.severity == Level::Warning  ? design::Color::Warning
                                                         : design::Color::Text);
  m_overlayLegend->setText(usage.warningLegend);
  m_criticalLegend->setText(usage.criticalLegend);
  m_criticalLegend->setVisible(usage.enabled);
  m_usageBar->setData(usage.currentMb, usage.warningMb, usage.criticalMb, usage.maximumMb);
  if (m_usageWarning != usage.warningText) {
    m_usageWarning = usage.warningText;
    emit usageWarningChanged(m_usageWarning);
  }
}

}  // namespace uwf::ui

#include "moc_GlobalStatusPanel.cpp"
