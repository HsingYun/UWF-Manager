// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "VolumeStatusPanel.h"

#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>

#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/widgets/ComboBoxSelection.h"
#include "ui/widgets/FlowLayout.h"
#include "ui/widgets/Notice.h"
#include "ui/widgets/SelectBox.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/SwitchButton.h"
#include "ui/widgets/TextLabel.h"

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
  // 会话状态、绑定方式和操作按钮分别保持完整，组之间按实际内容宽度换行。
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
    target->setToolTip(I18n::tr("Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot."));
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

  // itemData 的 true 表示按卷 ID 绑定。
  m_bindNext = new SelectBox();
  m_bindNext->setObjectName(QStringLiteral("volumeBindingNext"));
  m_bindNext->addItem(I18n::tr("Drive letter"), false);
  m_bindNext->addItem(I18n::tr("Volume ID"), true);
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
  connect(m_bindNext, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { m_viewModel->editBinding(m_bindNext->currentData().toBool()); });
  connect(m_viewModel.get(), &presentation::VolumeStatusViewModel::availabilityChanged, this, &VolumeStatusPanel::renderAvailability);
  connect(m_viewModel.get(), &presentation::VolumeStatusViewModel::stateChanged, this, &VolumeStatusPanel::renderEditor);
  I18n::bind(this, [this] {
    m_bindNext->setItemText(0, I18n::tr("Drive letter"));
    m_bindNext->setItemText(1, I18n::tr("Volume ID"));
    m_protectCur->setText(m_viewModel->currentProtectionText());
  });
  renderEditor();
  renderAvailability();
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
  // 常驻提示不改变控件可用性，刷新时继续保留。
  m_hasStickyNotice = true;
  m_banner->setText(text);
  m_banner->setTone(Notice::Tone::Warning);
  m_banner->show();
}

void VolumeStatusPanel::renderAvailability() const {
  m_protectNext->setEnabled(m_viewModel->controlsEnabled());
  m_bindNext->setEnabled(m_viewModel->controlsEnabled());
}

void VolumeStatusPanel::renderEditor() {
  if (!m_hasStickyNotice) m_banner->hide();
  m_protectCur->setText(m_viewModel->currentProtectionText());
  static_cast<TextLabel*>(m_protectCur)->setColorRole(m_viewModel->currentProtected() ? design::Color::Success : design::Color::Muted);
  const QSignalBlocker protectionSignals(m_protectNext);
  const QSignalBlocker bindingSignals(m_bindNext);
  m_protectNext->setChecked(m_viewModel->draft().protectedVolume);
  setComboValue(m_bindNext, m_viewModel->draft().bindByVolumeName);
}

}  // namespace uwf::ui

#include "moc_VolumeStatusPanel.cpp"
