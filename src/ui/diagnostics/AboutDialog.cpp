// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "AboutDialog.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSysInfo>
#include <QTextBlock>
#include <QTextCursor>
#include <QUrl>
#include <QVBoxLayout>
#include <exception>
#include <stdexcept>
#include <utility>

#include "SystemInformationDialog.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/TextLabel.h"
#include "uwf_version.h"

namespace uwf::ui {
namespace {
QWidget* disclosureText(const QString& html, QWidget* parent) {
  auto* content = new QWidget(parent);
  auto* layout = new QVBoxLayout(content);
  const bool rtl = content->isRightToLeft();
  layout->setContentsMargins(rtl ? 0 : 23, 12, rtl ? 23 : 0, 2);
  layout->setSpacing(0);
  auto* text = new ContentHintBox(content);
  auto font = text->font();
  font.setPixelSize(12);
  text->setFont(font);
  text->setProperty("dialogTone", "muted");
  text->setStyleSheet(QStringLiteral("QTextBrowser { background: transparent; border: none; padding: 0; }"));
  text->document()->setDocumentMargin(0);
  text->setOpenExternalLinks(true);
  text->setLineHeight(12 * 1.65);
  text->setHtml(html);
  for (auto block = text->document()->firstBlock(); block.isValid(); block = block.next()) {
    auto format = block.blockFormat();
    format.setTopMargin(0);
    format.setBottomMargin(block.next().isValid() ? 10 : 0);
    QTextCursor(block).setBlockFormat(format);
  }
  layout->addWidget(text);
  return content;
}
}  // namespace
AboutDialog::AboutDialog(std::function<presentation::DiagnosticReportViewModel()> systemInformationProvider, QWidget* parent)
    : InformationDialog(parent), m_systemInformationProvider(std::move(systemInformationProvider)) {
  if (!m_systemInformationProvider) throw std::invalid_argument("About dialog requires a diagnostic report provider");
  buildUi();
}
void AboutDialog::buildUi() {
  setWindowTitle(I18n::tr("About"));
  auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(654, 660), QSize(480, 0));
  auto* layout = frame->body();
  auto* hero = new QHBoxLayout();
  hero->setContentsMargins(0, 0, 0, 8);
  hero->setSpacing(18);
  auto* logo = new QLabel(this);
  logo->setPixmap(QIcon(QStringLiteral(":/icons/app.svg")).pixmap(QSize(68, 68), devicePixelRatioF()));
  logo->setFixedSize(68, 68);
  logo->setAlignment(Qt::AlignCenter);
  hero->addWidget(logo);
  auto* identity = new QVBoxLayout();
  identity->setSpacing(5);
  auto* name = new TextLabel(QStringLiteral("UWF Manager"), TextStyle::Section, this);
  auto font = name->font();
  font.setPixelSize(23);
  font.setLetterSpacing(QFont::AbsoluteSpacing, -.5);
  name->setFont(font);
  identity->addWidget(name);
  const QString versionText = I18n::tr("Version %1").arg(QString::fromLatin1(UWF_VER_STRING));
  auto* versionRow = new QHBoxLayout();
  versionRow->setSpacing(10);
  for (const auto& value : {versionText, QStringLiteral("Qt %1").arg(QString::fromLatin1(qVersion()))}) {
    auto* label = new TextLabel(value, TextStyle::Caption, this);
    label->setColorRole(design::Color::Muted);
    versionRow->addWidget(label);
  }
  versionRow->addStretch();
  identity->addLayout(versionRow);
  hero->addLayout(identity, 1);
  layout->addLayout(hero);
  auto* description =
      new TextLabel(I18n::tr("A graphical manager for Windows Unified Write Filter.\nManage volume protection, overlay, exclusions, and staged files."),
                    TextStyle::Description, this);
  description->setWordWrap(true);
  description->setColorRole(design::Color::Muted);
  layout->addWidget(description);
  auto* links = new QHBoxLayout();
  links->setSpacing(10);
  const auto addLink = [&](const QString& text, const QString& resource, const QUrl& url, const bool external) {
    auto* button = new QPushButton(ThemeManager::instance().icon(resource), text, this);
    auto type = button->font();
    type.setPixelSize(12);
    button->setFont(type);
    button->setIconSize({15, 15});
    button->setFixedHeight(34);
    button->setProperty("designPaddingX", 12);
    if (external) button->setProperty("designTrailingIcon", ThemeManager::instance().icon(QStringLiteral(":/icons/external.svg")));
    button->setToolTip(url.toString());
    connect(button, &QPushButton::clicked, this, [url] { QDesktopServices::openUrl(url); });
    links->addWidget(button);
  };
  addLink(I18n::tr("Source code"), QStringLiteral(":/icons/github.svg"), QUrl(QStringLiteral("https://github.com/HsingYun/UWF-Manager")), true);
  addLink(I18n::tr("Contact author"), QStringLiteral(":/icons/mail.svg"), QUrl(QStringLiteral("mailto:iakext@gmail.com")), false);
  links->addStretch();
  layout->addLayout(links);
  auto* note = new QHBoxLayout();
  note->setSpacing(9);
  note->addWidget(new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")), 17, design::Color::Muted, this), 0, Qt::AlignVCenter);
  auto* noteText = new TextLabel(I18n::tr("Windows UWF provides write protection. This application configures and manages it."), TextStyle::Caption, this);
  noteText->setColorRole(design::Color::Muted);
  noteText->setWordWrap(true);
  note->addWidget(noteText, 1);
  layout->addLayout(note);
  auto* impact = new Disclosure(I18n::tr("System changes when UWF is first enabled"), this);
  impact->setObjectName(QStringLiteral("aboutUwfImpact"));
  impact->setContent(disclosureText(I18n::tr("<p>Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast "
                                             "startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.</p><p>You can adjust these settings "
                                             "afterwards. For example, move the paging file to an unprotected volume before enabling paging again.</p>"),
                                    impact));
  layout->addWidget(impact);
  auto* license = new Disclosure(I18n::tr("License and copyright"), this);
  license->setObjectName(QStringLiteral("aboutLicense"));
  license->setContent(disclosureText(
      I18n::tr(
          "<p>Released under the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU General Public License v3.0</a>. Full terms are in the LICENSE file "
          "included with the application.</p><p>You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.</p>"),
      license));
  layout->addWidget(license);
  auto* copyright = new TextLabel(QStringLiteral("Copyright © 2026 HsingYun · iakext@gmail.com"), TextStyle::Eyebrow, this);
  copyright->setColorRole(design::Color::Faint);
  layout->addWidget(copyright);
  layout->addStretch();
  auto* leading = new QDialogButtonBox(this);
  auto* system = leading->addButton(I18n::tr("System information"), QDialogButtonBox::ActionRole);
  system->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/service.svg")));
  system->setObjectName(QStringLiteral("aboutSystemInformationButton"));
  connect(system, &QPushButton::clicked, this, &AboutDialog::openSystemInformation);
  auto* trailing = new QDialogButtonBox(this);
  auto* copy = trailing->addButton(I18n::tr("Copy version"), QDialogButtonBox::ActionRole);
  copy->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
  copy->setProperty("designRole", "quiet");
  connect(copy, &QPushButton::clicked, this, [] {
    const QString architecture = QSysInfo::buildCpuArchitecture() == QLatin1String("x86_64") ? QStringLiteral("x64") : QSysInfo::buildCpuArchitecture();
    QGuiApplication::clipboard()->setText(
        QStringLiteral("UWF Manager %1 · Qt %2 · %3").arg(QString::fromLatin1(UWF_VER_STRING), QString::fromLatin1(qVersion()), architecture));
  });
  m_saveConfigButton = trailing->addButton(I18n::tr("Export configuration"), QDialogButtonBox::ActionRole);
  m_saveConfigButton->setObjectName(QStringLiteral("aboutSaveConfigButton"));
  m_saveConfigButton->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/export.svg")));
  m_saveConfigButton->setProperty("designRole", "quiet");
  m_saveConfigButton->setToolTip(I18n::tr("Save the current theme, language, and HUB visibility to UWF.ini next to this application."));
  m_saveConfigButton->setProperty("toolTipRequired", true);
  m_saveConfigButton->setEnabled(false);
  connect(m_saveConfigButton, &QPushButton::clicked, this, &AboutDialog::saveConfiguration);
  frame->setActions(trailing, leading);
}
void AboutDialog::setConfigurationExporter(std::function<presentation::UserConfigExportViewModel::Result()> exporter) {
  m_configurationExporter = std::move(exporter);
  m_saveConfigButton->setEnabled(static_cast<bool>(m_configurationExporter));
}
void AboutDialog::saveConfiguration() {
  if (!m_configurationExporter) return;
  using Outcome = app::TextFileCreationResult::Outcome;
  presentation::UserConfigExportViewModel::Result result{Outcome::Failed, {}, {}};
  try {
    result = m_configurationExporter();
  } catch (const std::exception& error) {
    result.error = QString::fromUtf8(error.what());
  } catch (...) {
    result.error = I18n::tr("The operation failed with an unknown error.");
  }
  const auto path = result.path.isEmpty() ? QStringLiteral("UWF.ini") : QDir::toNativeSeparators(result.path);
  switch (result.outcome) {
    case Outcome::Created:
      dialogs::information(this, I18n::tr("Configuration saved"), I18n::tr("Saved the current interface settings to:\n%1").arg(path));
      break;
    case Outcome::AlreadyExists:
      dialogs::information(this, I18n::tr("Export configuration"), I18n::tr("UWF.ini already exists. The existing file was not changed:\n%1").arg(path));
      break;
    case Outcome::Failed:
      dialogs::warning(this, I18n::tr("Export failed"),
                       I18n::tr("Could not save UWF.ini:\n%1\n%2")
                           .arg(path, result.error.isEmpty() ? I18n::tr("The operation failed with an unknown error.") : result.error));
      break;
  }
}
void AboutDialog::openSystemInformation() {
  auto report = presentation::DiagnosticReportViewModel(QString{});
  try {
    report = m_systemInformationProvider();
  } catch (const std::exception& error) {
    report = presentation::DiagnosticReportViewModel(
        QStringLiteral("UWF Manager diagnostic report\n\n[Collection failure]\n%1").arg(QString::fromUtf8(error.what())));
  } catch (...) {
    report = presentation::DiagnosticReportViewModel(QStringLiteral("UWF Manager diagnostic report\n\n[Collection failure]\nUnknown error"));
  }
  SystemInformationDialog dialog(std::move(report), this);
  dialog.exec();
}
}  // namespace uwf::ui
#include "moc_AboutDialog.cpp"
