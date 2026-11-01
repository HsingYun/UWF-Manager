// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/overlay/OverlayAdvancedDetailsDialog.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>
#include <QVariant>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/layout/AlignedGraphic.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/viewmodels/configuration/OverlaySettingsEditViewModel.h"

namespace uwf::ui {
namespace {
namespace metrics {
constexpr int width = 500, narrowScreenWidth = 560, padding = 30, narrowPadding = 24;
constexpr int noteHeadingGap = 4, conflictIndent = 20, conflictGap = 8;
constexpr qreal warningBorderOpacity = .23;
}  // namespace metrics
TextLabel* text(QWidget* owner, const int pixels, const design::Color color, const QFont::Weight weight = QFont::Normal) {
  auto* label = new TextLabel({}, TextStyle::Caption, owner);
  auto font = label->font();
  font.setPixelSize(pixels);
  font.setWeight(weight);
  label->setFont(font);
  label->setColorRole(color);
  label->setWordWrap(true);
  return label;
}
void notesDivider(QVBoxLayout* layout, QWidget* owner) {
  layout->addSpacing(20);
  auto* line = new Surface(design::Color::Line, owner);
  line->setFixedHeight(1);
  layout->addWidget(line);
  layout->addSpacing(18);
}
}  // namespace

OverlayAdvancedDetailsDialog::OverlayAdvancedDetailsDialog(std::shared_ptr<Model> model, const Edit& edit, const Purpose purpose, const Field field,
                                                           QWidget* parent)
    : InformationDialog(parent), m_model(std::move(model)), m_edit(edit), m_purpose(purpose), m_field(field) {
  setObjectName(QStringLiteral("overlayAdvancedDetailsDialog"));
  setProperty("purpose", QVariant::fromValue(purpose));
  setHeaderStyle(HeaderStyle::Message);
  setWindowModality(Qt::WindowModal);
  setHeaderMetrics({QMargins(metrics::padding, 28, metrics::padding, 20), 20, 1.5});
  m_frame = new DialogLayout(this, DialogContent::Scrollable, QSize(metrics::width, 0), QSize(320, 180));
  m_frame->setAutoSizeToContent(true);
  m_frame->setActionMetrics(28, 15, 68);
  auto* body = m_frame->body();
  body->setContentsMargins(30, 0, 30, 28);
  body->setSpacing(0);

  const bool warning = purpose != Purpose::Feature;
  const auto hero = createDialogHero(this, {}, warning);
  hero.icon->setObjectName(QStringLiteral("overlayDetailIcon"));
  m_icon = hero.glyph;
  m_introduction = hero.heading;
  m_official = hero.technical;
  m_official->setObjectName(QStringLiteral("officialName"));
  body->addWidget(hero.widget);
  body->addSpacing(18);

  if (purpose == Purpose::Conflicts) {
    auto* list = new QWidget(this);
    list->setObjectName(QStringLiteral("overlayConflictList"));
    m_conflicts = new QVBoxLayout(list);
    m_conflicts->setContentsMargins(0, 0, 0, 0);
    m_conflicts->setSpacing(12);
    body->addWidget(list);
  } else if (purpose == Purpose::Restriction) {
    auto* callout = new Surface(design::Color::WarningBg, this);
    callout->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
    callout->setBorderColor(design::Color::Warning, metrics::warningBorderOpacity);
    auto* reasonLayout = new QVBoxLayout(callout);
    reasonLayout->setContentsMargins(16, 14, 16, 14);
    m_reason = text(callout, 13, design::Color::Warning);
    m_reason->setObjectName(QStringLiteral("restrictionReason"));
    m_reason->setLineHeight(1.9);
    reasonLayout->addWidget(m_reason);
    body->addWidget(callout);
    notesDivider(body, this);
    m_featureHeading = text(this, 12, design::Color::Text, QFont::Medium);
    body->addWidget(m_featureHeading);
    body->addSpacing(metrics::noteHeadingGap);
    m_description = text(this, 12, design::Color::Muted);
    m_description->setObjectName(QStringLiteral("detailDescription"));
    m_description->setLineHeight(1.85);
    body->addWidget(m_description);
  } else {
    m_description = text(this, 13, design::Color::Text);
    m_description->setObjectName(QStringLiteral("detailDescription"));
    m_description->setLineHeight(1.9);
    body->addWidget(m_description);
    notesDivider(body, this);
    const auto details = Model::featureDetails(field);
    for (qsizetype index = 0; index < details.conditions.size(); ++index) {
      if (!m_conditions.isEmpty()) body->addSpacing(14);
      auto* conditionTitle = text(this, 12, design::Color::Text, QFont::Medium);
      auto* conditionText = text(this, 12, design::Color::Muted);
      conditionText->setLineHeight(1.85);
      body->addWidget(conditionTitle);
      body->addSpacing(metrics::noteHeadingGap);
      body->addWidget(conditionText);
      m_conditions.append(ConditionLabels{conditionTitle, conditionText});
    }
  }

  m_back = new QPushButton(this);
  m_back->setObjectName(QStringLiteral("overlayDetailsBack"));
  m_back->setFixedHeight(36);
  setDialogAction(m_back, DialogAction::Secondary);
  bindRejectionButton(m_back);
  closeButton()->setDefault(false);
  m_back->setDefault(true);
  m_back->setFocus(Qt::OtherFocusReason);
  m_frame->setActions(m_back, static_cast<QWidget*>(nullptr));
  connect(m_back, &QPushButton::clicked, this, &QDialog::reject);
  connect(m_model.get(), &Model::stateChanged, this, &OverlayAdvancedDetailsDialog::render);
  I18n::bind(this, [this] { render(); });
}

OverlayAdvancedDetailsDialog::OverlayAdvancedDetailsDialog(std::shared_ptr<presentation::OverlaySettingsEditViewModel> session, QWidget* parent)
    : OverlayAdvancedDetailsDialog(session->policyModel(), session->policyEdit(), Purpose::Conflicts, Field::Passthrough, parent) {
  m_session = std::move(session);
  connect(m_session.get(), &presentation::OverlaySettingsEditViewModel::stateChanged, this, &OverlayAdvancedDetailsDialog::render);
  render();
}
void OverlayAdvancedDetailsDialog::render() {
  const auto details = Model::featureDetails(m_field);
  const bool conflict = m_purpose == Purpose::Conflicts;
  const bool restriction = m_purpose == Purpose::Restriction;
  setWindowTitle(conflict ? I18n::tr("Check configuration conflicts") : Model::fieldName(m_field));
  m_introduction->setText(conflict      ? I18n::tr("Resolve these configuration conflicts before keeping the draft.")
                          : restriction ? I18n::tr("This setting cannot be changed right now")
                                        : I18n::tr("Feature details"));
  m_official->setText(details.officialName);
  m_official->setVisible(!conflict && Model::fieldName(m_field).trimmed().toCaseFolded() != details.officialName.trimmed().toCaseFolded());
  const auto icon = conflict      ? QStringLiteral(":/icons/status_warning.svg")
                    : restriction ? QStringLiteral(":/icons/lock.svg")
                                  : QStringLiteral(":/icons/info.svg");
  m_icon->setIcon(ThemeManager::instance().icon(icon));
  if (m_description) m_description->setText(details.description);
  if (m_reason) m_reason->setText(m_model->disabledReason(m_edit, m_field));
  if (m_featureHeading) m_featureHeading->setText(I18n::tr("Feature details"));
  for (qsizetype index = 0; index < m_conditions.size(); ++index) {
    m_conditions[index].title->setText(details.conditions[index].title);
    m_conditions[index].text->setText(details.conditions[index].text);
  }
  if (m_conflicts) {
    const auto conflicts = m_session ? m_session->conflicts() : m_model->conflicts(m_edit);
    auto* list = m_conflicts->parentWidget();
    while (m_conflictRows.size() < conflicts.size()) {
      auto* item = new QWidget(list);
      auto* row = new QHBoxLayout(item);
      row->setContentsMargins(0, 0, 0, 0);
      row->setSpacing(metrics::conflictGap);
      auto* bullet = text(item, 13, design::Color::Muted);
      bullet->setObjectName(QStringLiteral("overlayConflictBullet"));
      bullet->setText(QStringLiteral("•"));
      bullet->setWordWrap(false);
      bullet->setFixedWidth(metrics::conflictIndent - metrics::conflictGap);
      bullet->setAlignment(Qt::AlignCenter);
      bullet->setLineHeight(1);
      auto* reason = text(item, 13, design::Color::Muted);
      reason->setObjectName(QStringLiteral("overlayConflictReason"));
      reason->setLineHeight(1.9);
      row->addWidget(new AlignedGraphic(bullet, reason, item));
      row->addWidget(reason, 1);
      m_conflicts->addWidget(item);
      m_conflictRows.append(ConflictRow{item, reason});
    }
    for (qsizetype index = 0; index < m_conflictRows.size(); ++index) {
      const bool visible = index < conflicts.size();
      if (visible) m_conflictRows[index].text->setText(conflicts[index]);
      m_conflictRows[index].widget->setVisible(visible);
    }
  }
  m_back->setText(I18n::tr("Back to settings"));
  updateResponsiveLayout();
}
void OverlayAdvancedDetailsDialog::resizeEvent(QResizeEvent* event) {
  InformationDialog::resizeEvent(event);
  if (m_frame) updateResponsiveLayout();
}
void OverlayAdvancedDetailsDialog::updateResponsiveLayout() {
  const bool narrow = width() < metrics::width || screen()->availableGeometry().width() <= metrics::narrowScreenWidth;
  const int profile = narrow ? 1 : 0;
  if (m_layoutProfile == profile) return;
  m_layoutProfile = profile;
  const int padding = narrow ? metrics::narrowPadding : metrics::padding;
  setHeaderMetrics({QMargins(padding, narrow ? metrics::narrowPadding : 28, padding, 20), 20, 1.5});
  m_frame->body()->setContentsMargins(padding, 0, padding, narrow ? metrics::narrowPadding : 28);
}
}  // namespace uwf::ui
#include "moc_OverlayAdvancedDetailsDialog.cpp"
