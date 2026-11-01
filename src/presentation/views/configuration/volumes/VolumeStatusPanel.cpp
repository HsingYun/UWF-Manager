// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/volumes/VolumeStatusPanel.h"

#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/inputs/ComboBoxSelection.h"
#include "presentation/controls/inputs/SelectBox.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/layout/FlowLayout.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/dialogs/DialogAppearance.h"

namespace uwf::ui {

namespace {

QLabel* makeKey(const char* source) {
  auto* l = I18n::text(new TextLabel({}, TextStyle::Caption), [source] { return I18n::tr(source); });
  l->setProperty("designTone", "muted");
  l->setObjectName("inlineKey");
  return l;
}

QWidget* makeBareCard(QLayout* inner) {
  auto* f = new Surface();
  f->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  f->setObjectName("statusCard");
  f->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto* v = new QVBoxLayout(f);
  v->setContentsMargins(16, 14, 16, 14);
  v->setSpacing(10);
  v->addLayout(inner);
  return f;
}

}  // namespace

VolumeStatusPanel::VolumeStatusPanel(std::shared_ptr<presentation::VolumeStatusViewModel> model, QWidget* parent)
    : QWidget(parent), m_viewModel(std::move(model)) {
  auto* outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  outer->setSpacing(8);

  m_banner = new Notice(this);
  m_banner->setObjectName("statusBanner");
  m_banner->hide();
  outer->addWidget(m_banner);

  auto* flow = new FlowLayout();
  // Status, binding and action groups wrap without splitting their contents.
  auto* protection = new QWidget();
  auto* row = new QHBoxLayout(protection);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(6);

  m_protectCur = new TextLabel(QStringLiteral("—"), TextStyle::Caption);
  m_protectCur->setObjectName("statusCurLabel");
  m_protectCur->setTextFormat(Qt::PlainText);
  I18n::bind(m_protectCur, [target = m_protectCur] { target->setToolTip(I18n::tr("Protection state of this volume in the current session (read-only).")); });
  m_protectNext = new SwitchButton();
  m_protectNext->setObjectName(QStringLiteral("volumeProtectionNext"));
  I18n::bind(m_protectNext, [target = m_protectNext] {
    target->setToolTip(
        I18n::tr("Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the "
                 "overlay settings."));
  });

  row->setSpacing(8);
  row->addWidget(makeKey(QT_TRANSLATE_NOOP("uwf", "Current session")), 0, Qt::AlignVCenter);
  row->addWidget(m_protectCur, 0, Qt::AlignVCenter);
  row->addSpacing(12);
  row->addWidget(makeKey(QT_TRANSLATE_NOOP("uwf", "Next session")), 0, Qt::AlignVCenter);
  row->addWidget(m_protectNext, 0, Qt::AlignVCenter);

  flow->addWidget(protection);
  auto* binding = new QWidget();
  auto* bindRow = new QHBoxLayout(binding);
  bindRow->setContentsMargins(0, 0, 0, 0);
  bindRow->setSpacing(6);

  m_bindNext = new SelectBox();
  m_bindNext->setObjectName(QStringLiteral("volumeBindingNext"));
  m_bindNext->addItem(I18n::tr("Drive letter"), static_cast<int>(core::VolumeBinding::DriveLetter));
  m_bindNext->addItem(I18n::tr("Volume ID"), static_cast<int>(core::VolumeBinding::VolumeId));
  m_bindNext->setFixedHeight(31);
  m_bindNext->setMinimumWidth(84);
  auto bindingFont = m_bindNext->font();
  bindingFont.setPixelSize(12);
  m_bindNext->setFont(bindingFont);
  I18n::bind(m_bindNext, [target = m_bindNext] {
    target->setToolTip(
        I18n::tr("How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing "
                 "other disks). Volume ID stays stable across drive letter changes."));
  });
  bindRow->addWidget(makeKey(QT_TRANSLATE_NOOP("uwf", "Binding")), 0, Qt::AlignVCenter);
  bindRow->addWidget(m_bindNext, 0, Qt::AlignVCenter);
  flow->addWidget(binding);
  flow->setAlignment(binding, Qt::AlignRight);
  auto* card = makeBareCard(flow);
  m_cardLayout = card->layout();
  outer->addWidget(card);

  connect(m_protectNext, &QAbstractButton::toggled, m_viewModel.get(), &presentation::VolumeStatusViewModel::editProtection);
  connect(m_bindNext, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this] { m_viewModel->editBinding(static_cast<core::VolumeBinding>(m_bindNext->currentData().toInt())); });
  connect(m_viewModel.get(), &presentation::VolumeStatusViewModel::availabilityChanged, this, &VolumeStatusPanel::renderState);
  connect(m_viewModel.get(), &presentation::VolumeStatusViewModel::stateChanged, this, [this] {
    renderEditor();
    renderState();
  });
  I18n::bind(this, [this] {
    m_bindNext->setItemText(0, I18n::tr("Drive letter"));
    m_bindNext->setItemText(1, I18n::tr("Volume ID"));
    renderState();
  });
  renderEditor();
  renderState();
}

void VolumeStatusPanel::setLayoutSize(const QSize& size) {
  const bool compact = size.width() <= design::compactWindowWidth;
  const bool shortWindow = size.height() <= design::shortWindowHeight;
  if (m_cardLayout)
    m_cardLayout->setContentsMargins(compact ? 13 : 16, shortWindow ? 9 : compact ? 13 : 14, compact ? 13 : 16, shortWindow ? 9 : compact ? 13 : 14);
}

void VolumeStatusPanel::setUnsupported(const QString& reason) {
  m_banner->setText(I18n::tr("This volume is not supported by UWF: ") + reason);
  m_banner->setTone(Notice::Tone::Warning);
  m_banner->show();
}

void VolumeStatusPanel::setNotice(const QString& text) {
  m_hasStickyNotice = true;
  m_banner->setText(text);
  m_banner->setTone(Notice::Tone::Warning);
  m_banner->show();
}

void VolumeStatusPanel::renderState() const {
  m_protectNext->setEnabled(m_viewModel->controlsEnabled());
  m_bindNext->setEnabled(m_viewModel->bindingEnabled());
  m_protectCur->setText(m_viewModel->currentProtectionText());
  static_cast<TextLabel*>(m_protectCur)->setColorRole(m_viewModel->currentProtected() ? design::Color::Success : design::Color::Muted);
}

void VolumeStatusPanel::renderEditor() {
  if (!m_hasStickyNotice) m_banner->hide();
  const QSignalBlocker protectionSignals(m_protectNext);
  const QSignalBlocker bindingSignals(m_bindNext);
  m_protectNext->setChecked(m_viewModel->draft().protectedVolume);
  setComboValue(m_bindNext, static_cast<int>(m_viewModel->draft().binding));
}

}  // namespace uwf::ui

#include "moc_VolumeStatusPanel.cpp"
