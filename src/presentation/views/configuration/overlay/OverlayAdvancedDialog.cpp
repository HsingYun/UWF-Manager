// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/overlay/OverlayAdvancedDialog.h"

#include <QBoxLayout>
#include <QGridLayout>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/layout/AlignedGraphic.h"
#include "presentation/controls/layout/FlexibleColumnLayout.h"
#include "presentation/controls/layout/FlowLayout.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/controls/ControlAnimation.h"
#include "presentation/design/controls/ControlShape.h"
#include "presentation/design/painting/IconPainting.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDetailsDialog.h"
#include "presentation/views/configuration/overlay/OverlayMaintenanceDialogs.h"

namespace uwf::ui {
namespace {
using Model = presentation::OverlayAdvancedViewModel;
using Field = Model::Field;
using Color = design::Color;

namespace metrics {
constexpr int width = 660, minimumWidth = 360, narrowWidth = 560, compactWidth = 390;
constexpr int padding = 28, narrowPadding = 20, compactPadding = 16;
constexpr int columnWidth = 104, narrowColumnWidth = 78, compactCurrentWidth = 64, compactNextWidth = 70;
constexpr int rowHeight = 72, columnGap = 12, nameGap = 7, indicatorSize = 5;
constexpr int actionSize = 26, narrowActionWidth = 22, actionIconSize = 15, controlWidth = 39, controlHeight = 21;
constexpr int footerHeight = 72, footerPadding = 16, footerGap = 18;
}  // namespace metrics
TextLabel* text(QWidget* owner, const int pixels, const Color color = Color::Muted, const QFont::Weight weight = QFont::Normal) {
  auto* label = new TextLabel({}, TextStyle::Caption, owner);
  auto font = label->font();
  font.setPixelSize(pixels);
  font.setWeight(weight);
  label->setFont(font);
  label->setColorRole(color);
  return label;
}
IconView* dot(QWidget* parent, const Color color) {
  auto* indicator = new IconView(QIcon(QStringLiteral(":/icons/dot.svg")), metrics::indicatorSize, color, parent);
  // Keep one logical pixel around the circular ink for fractional alignment.
  indicator->setFixedSize(metrics::indicatorSize + 2, metrics::indicatorSize + 2);
  return indicator;
}
QWidget* separator(QWidget* parent) {
  auto* line = new Surface(Color::LineSection, parent);
  line->setFixedHeight(1);
  return line;
}
class FeatureActionButton final : public QPushButton {
 public:
  enum class Tone { Information, Restriction };
  FeatureActionButton(const Tone tone, QWidget* parent) : QPushButton(parent), m_tone(tone) {
    setFixedSize(metrics::actionSize, metrics::actionSize);
    setIconSize({metrics::actionIconSize, metrics::actionIconSize});
    setAutoDefault(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    ControlAnimation::attach(this);
    connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
  }

 protected:
  void paintEvent(QPaintEvent*) override {
    const auto& colors = DesignSystem::instance();
    QPainter painter(this);
    const bool warning = m_tone == Tone::Restriction;
    const qreal emphasis = isEnabled() ? std::max(ControlAnimation::hover(this, underMouse()), ControlAnimation::press(this, isDown())) : 0.0;
    auto fill = colors.color(warning ? Color::WarningBg : Color::Selected);
    fill.setAlpha(qRound(fill.alpha() * emphasis));
    auto border = colors.color(Color::Warning);
    constexpr qreal warningBorderOpacity = .25;
    border.setAlpha(warning ? qRound(border.alpha() * warningBorderOpacity * emphasis) : 0);
    constexpr int hoverRadius = 4;
    paintControlShape(painter, rect(), hoverRadius, fill, border);
    const auto ink = warning ? colors.color(Color::Warning) : DesignSystem::mix(colors.color(Color::Faint), colors.color(Color::Accent), emphasis);
    if (!isEnabled()) painter.setOpacity(.42);
    const auto iconBounds =
        QRectF((width() - iconSize().width()) / 2.0, (height() - iconSize().height()) / 2.0 + property("designVerticalAlignmentOffset").toReal(),
               iconSize().width(), iconSize().height());
    paintIcon(painter, icon(), iconBounds, ink);
  }

 private:
  const Tone m_tone;
};
class OverlayControlCell final : public QWidget {
 public:
  explicit OverlayControlCell(QWidget* parent)
      : QWidget(parent), control(new SwitchButton(this)), reason(new FeatureActionButton(FeatureActionButton::Tone::Restriction, this)) {
    control->setFixedSize(metrics::controlWidth, metrics::controlHeight);
  }
  [[nodiscard]] QSize sizeHint() const override { return {metrics::columnWidth, metrics::actionSize}; }
  [[nodiscard]] QSize minimumSizeHint() const override { return {metrics::controlWidth + reason->width() * 2, metrics::actionSize}; }
  SwitchButton* const control;
  QPushButton* const reason;

 protected:
  void resizeEvent(QResizeEvent* event) override {
    QWidget::resizeEvent(event);
    control->move((width() - control->width()) / 2, (height() - control->height()) / 2);
    reason->move(width() - reason->width(), (height() - reason->height()) / 2);
  }
};
QPushButton* link(QWidget* parent, const QString& name) {
  auto* button = new QPushButton(parent);
  button->setObjectName(name);
  button->setProperty("designRole", "link");
  auto font = button->font();
  font.setPixelSize(11);
  button->setFont(font);
  button->setAutoDefault(true);
  return button;
}
Color foreground(const Model::ValueState state) {
  return state == Model::ValueState::On ? Color::Success : state == Model::ValueState::Off ? Color::Accent : Color::Warning;
}
Color background(const Model::ValueState state) {
  return state == Model::ValueState::On ? Color::SuccessBg : state == Model::ValueState::Off ? Color::Selected : Color::WarningBg;
}
bool distinctAlias(const QString& title, const QString& alias) {
  return !alias.trimmed().isEmpty() && title.trimmed().toCaseFolded() != alias.trimmed().toCaseFolded();
}
}  // namespace

OverlayAdvancedDialog::OverlayAdvancedDialog(std::shared_ptr<Model> model, QWidget* parent) : OverlayAdvancedDialog(std::move(model), {}, parent) {}
OverlayAdvancedDialog::OverlayAdvancedDialog(std::shared_ptr<Model> model, std::shared_ptr<presentation::OverlayMaintenanceViewModel> maintenance,
                                             QWidget* parent)
    : PanelDialog(parent),
      m_model(std::move(model)),
      m_session(std::make_shared<presentation::OverlaySettingsEditViewModel>(m_model, std::move(maintenance))),
      m_maintenance(m_session->maintenance()) {
  setObjectName(QStringLiteral("overlayAdvancedDialog"));
  setHeaderStyle(HeaderStyle::Message);
  setHeaderMetrics({QMargins(metrics::padding, 26, metrics::padding, 20), 21, 1.5});
  m_frame = new DialogLayout(this, DialogContent::Scrollable, QSize(metrics::width, 0), QSize(metrics::minimumWidth, 240));
  setMaximumHeight(std::max(240, std::min(720, screen()->availableGeometry().height() - 48)));
  m_frame->setAutoSizeToContent(true);
  m_frame->setActionMetrics(metrics::padding, metrics::footerPadding, metrics::footerHeight);
  m_frame->setDividerColor(Color::LineSection);
  m_frame->body()->setContentsMargins(metrics::padding, 0, metrics::padding, 10);
  m_frame->body()->setSpacing(0);

  m_configurationPage = new QWidget(this);
  m_configurationPage->setObjectName(QStringLiteral("overlayConfigurationPage"));
  auto* configurationBody = new QVBoxLayout(m_configurationPage);
  configurationBody->setContentsMargins(0, 0, 0, 0);
  configurationBody->setSpacing(0);
  if (m_maintenance) {
    m_tabs = new TabView(this);
    m_tabs->setObjectName(QStringLiteral("overlayAdvancedTabs"));
    m_tabs->setKind(TabView::Kind::Pages);
    m_tabs->setMeasureCurrentPage(true);
    m_tabs->addTab(m_configurationPage, QString{});
    configurationBody->setContentsMargins(0, 20, 0, 0);
  }

  auto* context = new Surface(Color::Surface, this);
  context->setObjectName(QStringLiteral("overlayAdvancedContext"));
  context->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  m_contextLayout = new QBoxLayout(QBoxLayout::LeftToRight, context);
  m_contextLayout->setContentsMargins(15, 12, 15, 12);
  m_contextLayout->setSpacing(14);
  auto* filter = new QWidget(context);
  auto* filterRow = new QHBoxLayout(filter);
  filterRow->setContentsMargins(0, 0, 0, 0);
  filterRow->setSpacing(6);
  m_filterIndicator = dot(filter, Color::Accent);
  m_filterIndicator->setObjectName(QStringLiteral("overlayAdvancedFilterIndicator"));
  m_filterText = text(filter, 11, Color::Accent, QFont::Medium);
  m_filterText->setObjectName(QStringLiteral("overlayAdvancedFilterState"));
  filterRow->addWidget(new AlignedGraphic(m_filterIndicator, m_filterText, filter));
  filterRow->addWidget(m_filterText);
  filterRow->addStretch();
  m_contextLayout->addWidget(filter, 1);
  auto* types = new QWidget(context);
  auto* typeFlow = new QHBoxLayout(types);
  m_typeLayout = typeFlow;
  typeFlow->setContentsMargins(0, 0, 0, 0);
  typeFlow->setSpacing(14);
  auto* typeArrow = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/arrow.svg")), 14, Color::Faint, types);
  typeArrow->setObjectName(QStringLiteral("overlayAdvancedTypeArrow"));
  for (std::size_t index = 0; index < m_contextKeys.size(); ++index) {
    auto* item = new QWidget(types);
    auto* pair = new QHBoxLayout(item);
    pair->setContentsMargins(0, 0, 0, 0);
    pair->setSpacing(4);
    m_contextKeys[index] = text(item, 11);
    m_contextKeys[index]->setWordWrap(true);
    m_contextValues[index] = text(item, 11, Color::Text, QFont::DemiBold);
    m_contextValues[index]->setObjectName(index == 0 ? QStringLiteral("overlayAdvancedCurrentType") : QStringLiteral("overlayAdvancedNextType"));
    pair->addWidget(m_contextKeys[index]);
    pair->addWidget(m_contextValues[index]);
    if (index) typeFlow->addWidget(new AlignedGraphic(typeArrow, m_contextValues[index], types));
    typeFlow->addWidget(item);
  }
  typeFlow->addStretch(1);
  m_contextLayout->addWidget(types);
  m_frame->body()->addWidget(context);
  m_contextGap = new QSpacerItem(0, 22, QSizePolicy::Minimum, QSizePolicy::Fixed);
  m_frame->body()->addItem(m_contextGap);
  m_frame->body()->addWidget(m_tabs ? static_cast<QWidget*>(m_tabs) : m_configurationPage);

  auto* grid = new QWidget(this);
  m_grid = new QGridLayout(grid);
  m_grid->setContentsMargins(0, 0, 0, 0);
  m_grid->setHorizontalSpacing(metrics::columnGap);
  m_grid->setVerticalSpacing(0);
  m_grid->setColumnStretch(0, 1);
  constexpr std::array headingNames{"overlayFeatureHeading", "overlayCurrentSessionHeading", "overlayNextSessionHeading"};
  for (std::size_t column = 0; column < m_headings.size(); ++column) {
    auto* heading = text(grid, 11);
    heading->setObjectName(QString::fromLatin1(headingNames[column]));
    heading->setWordWrap(true);
    heading->setAlignment((column ? Qt::AlignHCenter : Qt::AlignLeading) | Qt::AlignVCenter);
    heading->setContentsMargins(0, 0, 0, 12);
    m_headings[column] = heading;
    m_grid->addWidget(heading, 0, static_cast<int>(column));
  }
  m_grid->addWidget(separator(grid), 1, 0, 1, 3);
  constexpr auto& fields = app::overlayPolicyFields;
  constexpr std::array names{"overlayPassthrough", "overlayPersistent", "overlayResetPersistent", "overlayReadOnlyMedia"};
  static_assert(names.size() == fields.size());
  for (std::size_t index = 0; index < fields.size(); ++index) {
    const int gridRow = 2 + static_cast<int>(index) * 2;
    auto& row = m_rows[index];
    row.field = fields[index];
    const auto name = QString::fromLatin1(names[index]);
    auto* feature = new QWidget(grid);
    feature->setMinimumHeight(metrics::rowHeight - (index + 1 < fields.size() ? 1 : 0));
    auto* titleRow = new QHBoxLayout(feature);
    titleRow->setContentsMargins(0, 16, 0, 16);
    titleRow->setSpacing(metrics::nameGap);
    row.name = text(feature, 14, Color::Text, QFont::Medium);
    row.name->setObjectName(name + QStringLiteral("Name"));
    row.name->setWordWrap(true);
    titleRow->addWidget(row.name);
    row.info = new FeatureActionButton(FeatureActionButton::Tone::Information, feature);
    row.info->setObjectName(name + QStringLiteral("Info"));
    titleRow->addWidget(new AlignedGraphic(row.info, row.name, feature));
    row.changed = dot(feature, Color::Accent);
    row.changed->setObjectName(name + QStringLiteral("Changed"));
    titleRow->addWidget(new AlignedGraphic(row.changed, row.name, feature));
    titleRow->addStretch(1);
    m_grid->addWidget(feature, gridRow, 0);

    row.currentBadge = new Surface(Color::Selected, grid);
    row.currentBadge->setObjectName(name + QStringLiteral("CurrentBadge"));
    row.currentBadge->setBorder({}, 4);
    auto* badge = new QHBoxLayout(row.currentBadge);
    badge->setContentsMargins(9, 3, 9, 3);
    badge->setSpacing(6);
    row.currentIndicator = dot(row.currentBadge, Color::Accent);
    row.currentIndicator->setObjectName(name + QStringLiteral("CurrentIndicator"));
    row.current = text(row.currentBadge, 12, Color::Accent, QFont::Medium);
    row.current->setObjectName(name + QStringLiteral("Current"));
    badge->addWidget(new AlignedGraphic(row.currentIndicator, row.current, row.currentBadge));
    badge->addWidget(row.current);
    m_grid->addWidget(row.currentBadge, gridRow, 1, Qt::AlignCenter);

    auto* controlCell = new OverlayControlCell(grid);
    row.controlColumn = controlCell;
    row.control = controlCell->control;
    row.control->setOffThumbColorRole(Color::Faint);
    row.control->setObjectName(name);
    row.control->setFixedSize(metrics::controlWidth, metrics::controlHeight);
    row.reason = controlCell->reason;
    row.reason->setObjectName(name + QStringLiteral("Reason"));
    m_grid->addWidget(row.controlColumn, gridRow, 2);
    row.name->setBuddy(row.control);
    if (index + 1 < fields.size()) m_grid->addWidget(separator(grid), gridRow + 1, 0, 1, 3);
    connect(row.control, &QAbstractButton::clicked, this, [this, field = row.field](const bool checked) { m_session->setPolicyValue(field, checked); });
    connect(row.info, &QPushButton::clicked, this, [this, field = row.field, origin = row.info] { showFeatureDetails(field, false, origin); });
    connect(row.reason, &QPushButton::clicked, this, [this, field = row.field, origin = row.reason] { showFeatureDetails(field, true, origin); });
  }
  configurationBody->addWidget(grid);
  configurationBody->addStretch(1);

  if (m_maintenance) {
    m_maintenancePage = new QWidget(m_tabs);
    m_maintenancePage->setObjectName(QStringLiteral("overlayMaintenancePage"));
    auto* maintenanceBody = new FlexibleColumnLayout(m_maintenancePage);
    maintenanceBody->setContentsMargins(0, 18, 0, 8);
    maintenanceBody->setSpacing(0);
    const auto maintenanceRow = [this, maintenanceBody](const QString& resource, const QString& name, TextLabel*& title, TextLabel*& description,
                                                        TextLabel*& status, QPushButton*& action) {
      auto* row = new QWidget(m_maintenancePage);
      auto* layout = new QGridLayout(row);
      layout->setContentsMargins(0, name == QStringLiteral("overlaySwap") ? 6 : 20, 0, 23);
      layout->setHorizontalSpacing(14);
      layout->setVerticalSpacing(10);
      layout->setColumnStretch(1, 1);
      m_maintenanceRows[name == QStringLiteral("overlaySwap") ? 0 : name == QStringLiteral("overlayHorm") ? 1 : 2] = layout;
      auto* icon = new IconView(ThemeManager::instance().icon(resource), 23, Color::Muted, row);
      icon->setObjectName(name + QStringLiteral("Icon"));
      auto* copyWidget = new QWidget(row);
      auto* copy = new FlexibleColumnLayout(copyWidget);
      copy->setContentsMargins(0, 0, 0, 0);
      copy->setSpacing(4);
      title = text(row, 14, Color::Text, QFont::Medium);
      title->setWordWrap(true);
      title->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
      title->setVerticalReference(TextVerticalReference::GlyphBounds);
      title->setObjectName(name + QStringLiteral("Name"));
      layout->addWidget(new AlignedGraphic(icon, title, row), 0, 0);
      description = text(row, 12, Color::Muted);
      description->setWordWrap(true);
      description->setLineHeight(1.75);
      status = text(row, 11, Color::Muted);
      status->setWordWrap(true);
      status->setObjectName(name + QStringLiteral("Status"));
      auto* titleLine = new QWidget(copyWidget);
      titleLine->setObjectName(name + QStringLiteral("TitleLine"));
      auto* titleFlow = new FlowLayout(titleLine);
      titleFlow->setSpacing(8);
      titleFlow->addWidget(title);
      if (name != QStringLiteral("overlayVolumes")) {
        auto* alias = text(row, 11, Color::Muted);
        alias->setObjectName(name + QStringLiteral("Alias"));
        alias->setWordWrap(true);
        alias->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
        alias->setVerticalReference(TextVerticalReference::GlyphBounds);
        alias->setText(name == QStringLiteral("overlayHorm") ? QStringLiteral("HORM") : QStringLiteral("create swapfile"));
        titleFlow->addWidget(alias);
      }
      copy->addWidget(titleLine);
      copy->addWidget(description);
      copy->addSpacing(4);
      copy->addWidget(status);
      layout->addWidget(copyWidget, 0, 1);
      action = new QPushButton(row);
      action->setObjectName(name + QStringLiteral("Manage"));
      action->setMinimumHeight(34);
      action->setAutoDefault(false);
      action->setMinimumWidth(72);
      layout->addWidget(action, 0, 2, Qt::AlignTop);
      maintenanceBody->addWidget(row);
    };
    maintenanceRow(QStringLiteral(":/icons/disk.svg"), QStringLiteral("overlaySwap"), m_swapName, m_swapDescription, m_swapStatus, m_swapConfigure);
    maintenanceBody->addWidget(separator(m_maintenancePage));
    maintenanceRow(QStringLiteral(":/icons/restart.svg"), QStringLiteral("overlayHorm"), m_hormName, m_hormDescription, m_hormStatus, m_hormManage);
    maintenanceBody->addWidget(separator(m_maintenancePage));
    maintenanceRow(QStringLiteral(":/icons/disk.svg"), QStringLiteral("overlayVolumes"), m_volumesName, m_volumesDescription, m_volumesStatus, m_volumesManage);
    auto* volumeActions = new QMenu(m_volumesManage);
    m_protectAll = volumeActions->addAction(QString{});
    m_protectAll->setObjectName(QStringLiteral("protectAllVolumes"));
    m_unprotectAll = volumeActions->addAction(QString{});
    m_unprotectAll->setObjectName(QStringLiteral("unprotectAllVolumes"));
    m_volumesManage->setMenu(volumeActions);
    connect(m_protectAll, &QAction::triggered, this, [this] { m_maintenance->stageAllVolumesProtection(true); });
    connect(m_unprotectAll, &QAction::triggered, this, [this] { m_maintenance->stageAllVolumesProtection(false); });
    m_maintenanceIssue = new Notice(m_maintenancePage);
    m_maintenanceIssue->setObjectName(QStringLiteral("overlayMaintenanceIssue"));
    maintenanceBody->addSpacing(14);
    maintenanceBody->addWidget(m_maintenanceIssue);
    maintenanceBody->addStretch(1);
    m_tabs->addTab(m_maintenancePage, QString{});
    connect(m_tabs, &QTabWidget::currentChanged, this, [this] { render(); });
    connect(m_swapConfigure, &QPushButton::clicked, this, &OverlayAdvancedDialog::showSwap);
    connect(m_hormManage, &QPushButton::clicked, this, &OverlayAdvancedDialog::showHorm);
    connect(m_maintenance.get(), &presentation::OverlayMaintenanceViewModel::stateChanged, this, &OverlayAdvancedDialog::render);
    connect(m_maintenance.get(), &presentation::OverlayMaintenanceViewModel::pendingChanged, this, &OverlayAdvancedDialog::render);
    if (!m_maintenance->busy() && !m_maintenance->snapshot().volumesReadable)
      QTimer::singleShot(0, m_maintenance.get(), &presentation::OverlayMaintenanceViewModel::refresh);
  }

  auto* footer = new QWidget(this);
  footer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_footerLayout = new QBoxLayout(QBoxLayout::LeftToRight, footer);
  m_footerLayout->setContentsMargins(0, 0, 0, 0);
  m_footerLayout->setSpacing(metrics::footerGap);
  auto* message = new QWidget(footer);
  auto* messageFlow = new FlowLayout(message);
  messageFlow->setContentsMargins(0, 0, 0, 0);
  messageFlow->setSpacing(12);
  auto* hintGroup = new QWidget(message);
  auto* hintRow = new QHBoxLayout(hintGroup);
  hintRow->setContentsMargins(0, 0, 0, 0);
  hintRow->setSpacing(7);
  m_warning = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/status_warning.svg")), 15, Color::Warning, hintGroup);
  m_hint = text(hintGroup, 11);
  m_hint->setObjectName(QStringLiteral("overlayAdvancedHint"));
  m_hint->setWordWrap(true);
  hintRow->addWidget(m_warning);
  hintRow->addWidget(m_hint);
  messageFlow->addWidget(hintGroup);
  m_pendingCount = text(message, 11, Color::Accent, QFont::Medium);
  m_pendingCount->setObjectName(QStringLiteral("overlayAdvancedPendingCount"));
  messageFlow->addWidget(m_pendingCount);
  m_conflictDetails = link(message, QStringLiteral("overlayAdvancedConflictDetails"));
  messageFlow->addWidget(m_conflictDetails);
  m_revert = link(message, QStringLiteral("overlayAdvancedRevert"));
  messageFlow->addWidget(m_revert);
  m_footerLayout->addWidget(message, 1);
  auto* buttons = new QWidget(footer);
  auto* actions = new QHBoxLayout(buttons);
  actions->setContentsMargins(0, 0, 0, 0);
  actions->setSpacing(9);
  m_cancel = new QPushButton(buttons);
  m_cancel->setFixedHeight(36);
  m_confirm = new QPushButton(buttons);
  m_confirm->setFixedHeight(36);
  actions->addWidget(m_cancel);
  actions->addWidget(m_confirm);
  bindRejectionButton(m_cancel);
  m_confirm->setObjectName(QStringLiteral("overlayAdvancedConfirm"));
  setDialogAction(m_confirm, DialogAction::Primary);
  m_confirm->setAutoDefault(true);
  m_cancel->setDefault(true);
  m_footerLayout->addWidget(buttons, 0, Qt::AlignRight | Qt::AlignVCenter);
  m_frame->setActions(static_cast<QWidget*>(nullptr), footer);
  connect(m_cancel, &QPushButton::clicked, this, &QDialog::reject);
  connect(m_confirm, &QPushButton::clicked, this, [this] {
    if (m_session->commit())
      accept();
    else
      render();
  });
  connect(m_revert, &QPushButton::clicked, this, [this] { m_session->revert(); });
  connect(m_conflictDetails, &QPushButton::clicked, this, &OverlayAdvancedDialog::showConflicts);
  connect(m_session.get(), &presentation::OverlaySettingsEditViewModel::stateChanged, this, &OverlayAdvancedDialog::render);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &OverlayAdvancedDialog::render);
  I18n::bind(this, [this] { render(); });
}

void OverlayAdvancedDialog::render() {
  setWindowTitle(I18n::tr("Overlay advanced settings"));
  m_filterText->setText(QStringLiteral("UWF %1").arg(m_model->currentFilterText()));
  const auto filterColor = foreground(m_model->currentFilterState());
  m_filterText->setColorRole(filterColor);
  m_filterIndicator->setColorRole(filterColor);
  m_contextKeys[0]->setText(I18n::tr("Current overlay"));
  m_contextKeys[1]->setText(I18n::tr("Next overlay"));
  m_contextValues[0]->setText(m_model->currentTypeText());
  m_contextValues[1]->setText(m_model->targetTypeText(m_session->policyEdit()));
  m_headings[0]->setText(I18n::tr("Feature"));
  m_headings[1]->setText(I18n::tr("Current session"));
  m_headings[2]->setText(I18n::tr("Next session"));
  const auto& colors = DesignSystem::instance();
  for (auto& row : m_rows) {
    const auto name = Model::fieldName(row.field);
    row.name->setText(name);
    row.info->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")));
    row.info->setToolTip(I18n::tr("Feature details"));
    row.reason->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")));
    row.reason->setToolTip(I18n::tr("View change requirements"));
    const auto state = m_model->currentState(row.field);
    row.currentBadge->setFill(background(state));
    row.currentIndicator->setColorRole(foreground(state));
    row.current->setColorRole(foreground(state));
    row.current->setText(m_model->currentText(row.field));
    row.changed->setVisible(m_model->changed(m_session->policyEdit(), row.field));
    const auto reason = m_model->disabledReason(m_session->policyEdit(), row.field);
    const bool editable = m_model->canEditField(m_session->policyEdit(), row.field);
    row.reason->setVisible(!editable);
    const QSignalBlocker blocked(row.control);
    const auto value = Model::value(m_session->policyEdit(), row.field);
    row.control->setUnknown(!value.has_value());
    row.control->setChecked(value.value_or(false));
    row.control->setEnabled(editable);
    row.control->setCursor(editable ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
    const auto currentDescription = I18n::tr("Current session") + QStringLiteral(": ") + m_model->currentText(row.field);
    row.current->setProperty("accessibleName", currentDescription);
    row.info->setProperty("accessibleName", I18n::tr("Learn about %1").arg(name));
    row.reason->setProperty("accessibleName", I18n::tr("Why %1 cannot be changed").arg(name));
    row.reason->setProperty("accessibleDescription", reason);
    row.changed->setProperty("accessibleName", I18n::tr("Changed"));
    row.control->setProperty("accessibleName", I18n::tr("%1 · Next session").arg(name));
    row.control->setProperty("accessibleDescription", Model::description(row.field) + u'\n' + currentDescription + u'\n' + reason);
  }
  const bool conflict = !m_session->conflicts().isEmpty();
  const auto count = m_session->pendingCount();
  m_warning->setVisible(conflict);
  m_hint->parentWidget()->setVisible(conflict || m_model->readOnly());
  m_hint->setVisible(conflict || m_model->readOnly());
  m_hint->setColorRole(conflict ? Color::Warning : Color::Muted);
  m_hint->setText(conflict ? I18n::tr("Configuration conflict") : m_model->readOnly() ? I18n::tr("View only") : QString{});
  m_pendingCount->setText(I18n::tr("%1 changes").arg(count));
  m_pendingCount->setVisible(count > 0 && !conflict);
  m_conflictDetails->setText(I18n::tr("View details"));
  m_conflictDetails->setProperty("designTextColor", colors.color(Color::Warning));
  m_conflictDetails->setVisible(conflict);
  m_revert->setText(I18n::tr("Revert"));
  m_revert->setProperty("designTextColor", colors.color(conflict ? Color::Warning : Color::Accent));
  m_revert->setVisible(count > 0 && !m_model->readOnly());
  m_confirm->setText(I18n::tr("Save"));
  m_cancel->setText(I18n::tr(m_model->readOnly() ? "Close" : "Cancel"));
  m_confirm->setVisible(!m_model->readOnly());
  m_confirm->setEnabled(m_session->canCommit());
  if (m_tabs) {
    m_tabs->setTabText(0, I18n::tr("Overlay configuration"));
    m_tabs->setTabText(1, I18n::tr("Maintenance and recovery"));
    renderMaintenance();
  }
  updateResponsiveLayout();
}

void OverlayAdvancedDialog::renderMaintenance() {
  if (!m_maintenance) return;
  const auto state = m_maintenance->panelState();
  m_swapName->setText(I18n::tr("Configure storage location"));
  m_swapDescription->setText(I18n::tr("Choose where to store the disk overlay."));
  m_swapStatus->setText(state.storageStatus);
  m_swapConfigure->setText(I18n::tr("Configure…"));
  m_swapConfigure->setEnabled(!state.busy);
  m_hormName->setText(I18n::tr("Hibernate once / resume many"));
  for (const auto& [name, title] : {std::pair{QStringLiteral("overlaySwap"), m_swapName}, std::pair{QStringLiteral("overlayHorm"), m_hormName}})
    if (auto* alias = findChild<TextLabel*>(name + QStringLiteral("Alias"))) alias->setVisible(distinctAlias(title->text(), alias->text()));
  m_hormDescription->setText(I18n::tr("Restore the same desktop and running programs at every startup."));
  m_hormStatus->setText(state.hormStatus);
  m_hormStatus->setColorRole(state.hormActive ? Color::Success : Color::Muted);
  m_hormManage->setText(I18n::tr("Configure…"));
  m_hormManage->setEnabled(!state.busy);
  m_volumesName->setText(I18n::tr("Volume protection"));
  m_volumesDescription->setText(I18n::tr("Configure protection for all volumes in the next session."));
  m_volumesManage->setText(I18n::tr("Configure…"));
  m_protectAll->setText(I18n::tr("Protect all"));
  m_unprotectAll->setText(I18n::tr("Unprotect all"));
  m_protectAll->setEnabled(state.protectEnabled);
  m_unprotectAll->setEnabled(state.unprotectEnabled);
  m_protectAll->setToolTip(state.protectReason);
  m_unprotectAll->setToolTip(state.unprotectReason);
  m_volumesManage->setEnabled(state.manageProtectionEnabled);
  m_volumesManage->setToolTip(state.protectReason);
  m_volumesStatus->setText(state.protectionStatus);
  m_maintenanceIssue->setText(state.readError);
  m_maintenanceIssue->setTone(Notice::Tone::Warning);
  m_maintenanceIssue->setVisible(!state.readError.isEmpty() && !state.busy);
}
void OverlayAdvancedDialog::showSwap() {
  if (!m_maintenance || m_maintenance->busy()) return;
  const QPointer<QPushButton> focus = m_swapConfigure;
  SwapDialog dialog(m_maintenance, this);
  dialog.exec();
  renderMaintenance();
  if (focus) focus->setFocus(Qt::OtherFocusReason);
}
void OverlayAdvancedDialog::showHorm() {
  if (!m_maintenance || m_maintenance->busy()) return;
  const QPointer<QPushButton> focus = m_hormManage;
  HormDialog dialog(m_maintenance, this);
  dialog.exec();
  renderMaintenance();
  if (focus) focus->setFocus(Qt::OtherFocusReason);
}

void OverlayAdvancedDialog::resizeEvent(QResizeEvent* event) {
  PanelDialog::resizeEvent(event);
  if (m_grid) updateResponsiveLayout();
}
void OverlayAdvancedDialog::updateResponsiveLayout() {
  const int profile = width() <= metrics::compactWidth ? 2 : width() <= metrics::narrowWidth ? 1 : 0;
  if (m_layoutProfile == profile) return;
  m_layoutProfile = profile;
  const bool narrow = profile != 0;
  if (m_maintenance) {
    for (std::size_t index = 0; index < m_maintenanceRows.size(); ++index) {
      auto* row = m_maintenanceRows[index];
      auto* action = index == 0 ? m_swapConfigure : index == 1 ? m_hormManage : m_volumesManage;
      if (!row || !action) continue;
      row->removeWidget(action);
      row->setHorizontalSpacing(narrow ? 10 : 14);
      row->addWidget(action, narrow ? 1 : 0, narrow ? 1 : 2, Qt::AlignLeft | Qt::AlignTop);
    }
  }
  const int padding = profile == 2 ? metrics::compactPadding : narrow ? metrics::narrowPadding : metrics::padding;
  setHeaderMetrics({QMargins(padding, narrow ? 23 : 26, padding, narrow ? 16 : 20), narrow ? 20 : 21, 1.5});
  m_frame->body()->setContentsMargins(padding, 0, padding, narrow ? 8 : 10);
  m_frame->setActionMetrics(narrow ? metrics::narrowPadding : metrics::padding, narrow ? 14 : metrics::footerPadding, metrics::footerHeight);
  m_grid->setHorizontalSpacing(narrow ? 6 : metrics::columnGap);
  const int currentWidth = profile == 2 ? metrics::compactCurrentWidth : narrow ? metrics::narrowColumnWidth : metrics::columnWidth;
  const int requestedNextWidth = profile == 2 ? metrics::compactNextWidth : narrow ? metrics::narrowColumnWidth : metrics::columnWidth;
  const int actionWidth = narrow ? metrics::narrowActionWidth : metrics::actionSize;
  const int nextWidth = std::max(requestedNextWidth, metrics::controlWidth + actionWidth * 2);
  m_headings[1]->setMinimumWidth(currentWidth);
  m_headings[2]->setFixedWidth(nextWidth);
  m_grid->setColumnMinimumWidth(1, currentWidth);
  m_grid->setColumnMinimumWidth(2, nextWidth);
  m_contextLayout->setDirection(narrow ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
  m_contextLayout->setSpacing(narrow ? 8 : 14);
  m_typeLayout->setSpacing(narrow ? 8 : 14);
  m_contextLayout->setStretch(0, narrow ? 0 : 1);
  m_contextLayout->setContentsMargins(narrow ? 11 : 15, narrow ? 11 : 12, narrow ? 11 : 15, narrow ? 11 : 12);
  m_contextGap->changeSize(0, narrow ? 18 : 22, QSizePolicy::Minimum, QSizePolicy::Fixed);
  m_frame->body()->invalidate();
  m_footerLayout->setDirection(narrow ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
  m_footerLayout->setSpacing(narrow ? 12 : metrics::footerGap);
  m_footerLayout->setStretch(0, narrow ? 0 : 1);
  for (auto& row : m_rows) {
    row.info->setFixedWidth(actionWidth);
    row.reason->setFixedWidth(actionWidth);
    row.controlColumn->setFixedWidth(nextWidth);
    row.name->parentWidget()->layout()->setSpacing(narrow ? 3 : metrics::nameGap);
    row.currentBadge->layout()->setContentsMargins(narrow ? 6 : 9, 3, narrow ? 6 : 9, 3);
    row.currentBadge->layout()->setSpacing(narrow ? 4 : 6);
    auto font = row.name->font();
    font.setPixelSize(narrow ? 13 : 14);
    row.name->setFont(font);
    font = row.current->font();
    font.setPixelSize(narrow ? 11 : 12);
    row.current->setFont(font);
  }
}

void OverlayAdvancedDialog::showFeatureDetails(const Field field, const bool restriction, QPushButton* origin) {
  const QPointer<QPushButton> focus = origin;
  OverlayAdvancedDetailsDialog dialog(m_model, m_session->policyEdit(),
                                      restriction ? OverlayAdvancedDetailsDialog::Purpose::Restriction : OverlayAdvancedDetailsDialog::Purpose::Feature, field,
                                      this);
  dialog.exec();
  if (focus) focus->setFocus(Qt::OtherFocusReason);
}
void OverlayAdvancedDialog::showConflicts() {
  const QPointer<QPushButton> focus = m_conflictDetails;
  OverlayAdvancedDetailsDialog dialog(m_session, this);
  dialog.exec();
  if (focus) focus->setFocus(Qt::OtherFocusReason);
}
}  // namespace uwf::ui
#include "moc_OverlayAdvancedDialog.cpp"
