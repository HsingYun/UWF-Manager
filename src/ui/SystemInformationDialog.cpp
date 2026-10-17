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
#include "SystemInformationDialog.h"

#include <QClipboard>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>

#include "I18n.h"
#include "ThemeManager.h"

namespace uwf::ui {

namespace {

bool isSectionLine(const QString& text) { return text.size() > 2 && text.startsWith(QChar('[')) && text.indexOf(QChar(']')) == text.size() - 1; }

class DiagnosticReportHighlighter final : public QSyntaxHighlighter {
 public:
  explicit DiagnosticReportHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) {}

 protected:
  void highlightBlock(const QString& text) override {
    const auto& theme = ThemeManager::instance();
    const int blockLength = currentBlock().length() - 1;

    if (currentBlock().blockNumber() == 0) {
      QTextCharFormat title;
      title.setForeground(theme.color(Sem::Fg));
      title.setFontWeight(QFont::DemiBold);
      setFormat(0, blockLength, title);
      return;
    }

    if (isSectionLine(text)) {
      QTextCharFormat section;
      section.setForeground(theme.color(Sem::Accent));
      section.setFontWeight(QFont::DemiBold);
      setFormat(0, blockLength, section);
      return;
    }

    if (text.startsWith(QStringLiteral("--- "))) {
      QTextCharFormat marker;
      marker.setForeground(theme.color(Sem::FgMuted));
      marker.setFontItalic(true);
      setFormat(0, blockLength, marker);
      return;
    }

    const qsizetype separatorPosition = text.indexOf(QStringLiteral(" = "));
    if (separatorPosition > 0) {
      const int separator = static_cast<int>(separatorPosition);
      QTextCharFormat key;
      key.setForeground(theme.color(Sem::FgMuted));
      setFormat(0, separator, key);

      QTextCharFormat equals;
      equals.setForeground(theme.color(Sem::Fg));
      setFormat(separator, 3, equals);

      if (text.left(separator).endsWith(QStringLiteral(".error"))) {
        QTextCharFormat error;
        error.setForeground(theme.color(Sem::Danger));
        setFormat(separator + 3, blockLength - separator - 3, error);
      }
      return;
    }

    // 日志格式固定为 "[HH:mm:ss.zzz L category] message"。只突出级别，
    // 保持消息正文为普通前景色，长日志仍然容易连续扫读。
    if (text.startsWith(QChar('['))) {
      const qsizetype firstSpacePosition = text.indexOf(QChar(' '));
      const qsizetype headerEndPosition = text.indexOf(QChar(']'));
      if (firstSpacePosition > 1 && headerEndPosition > firstSpacePosition + 2) {
        const int firstSpace = static_cast<int>(firstSpacePosition);
        const int headerEnd = static_cast<int>(headerEndPosition);
        QTextCharFormat metadata;
        metadata.setForeground(theme.color(Sem::FgMuted));
        setFormat(0, headerEnd + 1, metadata);

        const QChar level = text.at(firstSpace + 1);
        QTextCharFormat severity;
        severity.setFontWeight(QFont::DemiBold);
        severity.setForeground(level == QChar('W') ? theme.color(Sem::Warn) : level == QChar('E') ? theme.color(Sem::Danger) : theme.color(Sem::Accent));
        setFormat(firstSpace + 1, 1, severity);
      }
    }

    const qsizetype truncatedPosition = text.lastIndexOf(QStringLiteral("... <truncated>"));
    if (truncatedPosition >= 0) {
      const int truncated = static_cast<int>(truncatedPosition);
      QTextCharFormat warning;
      warning.setForeground(theme.color(Sem::Warn));
      warning.setFontItalic(true);
      setFormat(truncated, blockLength - truncated, warning);
    }
  }
};

void formatReportBlocks(QTextDocument& document) {
  for (QTextBlock block = document.begin(); block.isValid(); block = block.next()) {
    QTextCursor cursor(block);
    QTextBlockFormat format = cursor.blockFormat();
    format.setLineHeight(122.0, QTextBlockFormat::ProportionalHeight);
    if (block.blockNumber() == 0) format.setBottomMargin(8.0);
    const QString text = block.text();
    if (isSectionLine(text)) {
      format.setTopMargin(10.0);
      format.setBottomMargin(3.0);
    }
    cursor.setBlockFormat(format);
  }
}

}  // namespace

SystemInformationDialog::SystemInformationDialog(QString report, QWidget* parent) : QDialog(parent) {
  setObjectName(QStringLiteral("systemInformationDialog"));
  setWindowTitle(I18n::tr("System information"));
  resize(920, 700);
  setMinimumSize(680, 480);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(20, 18, 20, 14);
  layout->setSpacing(12);

  auto* heading = new QWidget(this);
  auto* headingLayout = new QHBoxLayout(heading);
  headingLayout->setContentsMargins(0, 0, 0, 0);
  headingLayout->setSpacing(12);

  auto* headingIcon = new QLabel(heading);
  headingIcon->setFixedSize(32, 32);
  headingIcon->setAlignment(Qt::AlignCenter);
  headingLayout->addWidget(headingIcon, 0, Qt::AlignTop);

  auto* headingText = new QWidget(heading);
  auto* headingTextLayout = new QVBoxLayout(headingText);
  headingTextLayout->setContentsMargins(0, 0, 0, 0);
  headingTextLayout->setSpacing(2);
  auto* title = new QLabel(I18n::tr("Diagnostic snapshot"), headingText);
  QFont titleFont = title->font();
  titleFont.setPointSizeF(titleFont.pointSizeF() + 2.0);
  titleFont.setWeight(QFont::Medium);
  title->setFont(titleFont);
  headingTextLayout->addWidget(title);
  auto* subtitle = new QLabel(I18n::tr("System, security, UWF, enhanced mode, and recent application logs."), headingText);
  subtitle->setWordWrap(true);
  headingTextLayout->addWidget(subtitle);
  headingLayout->addWidget(headingText, 1);

  auto* readOnlyBadge = new QLabel(I18n::tr("Read only"), heading);
  readOnlyBadge->setObjectName(QStringLiteral("systemInformationReadOnlyBadge"));
  readOnlyBadge->setAlignment(Qt::AlignCenter);
  headingLayout->addWidget(readOnlyBadge, 0, Qt::AlignTop);
  layout->addWidget(heading);

  auto* noticeCard = new QFrame(this);
  noticeCard->setObjectName(QStringLiteral("systemInformationPrivacyNotice"));
  auto* noticeLayout = new QHBoxLayout(noticeCard);
  noticeLayout->setContentsMargins(12, 9, 12, 9);
  noticeLayout->setSpacing(9);
  auto* noticeIcon = new QLabel(noticeCard);
  noticeIcon->setFixedSize(18, 18);
  noticeIcon->setAlignment(Qt::AlignCenter);
  noticeLayout->addWidget(noticeIcon, 0, Qt::AlignTop);
  auto* notice = new QLabel(
      I18n::tr("This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing."), this);
  notice->setWordWrap(true);
  noticeLayout->addWidget(notice, 1);
  layout->addWidget(noticeCard);

  const qsizetype lineCount = report.isEmpty() ? 0 : report.count(QChar('\n')) + 1;
  auto* reportCard = new QFrame(this);
  reportCard->setObjectName(QStringLiteral("systemInformationReportCard"));
  auto* reportLayout = new QVBoxLayout(reportCard);
  reportLayout->setContentsMargins(0, 0, 0, 0);
  reportLayout->setSpacing(0);

  auto* reportToolbar = new QWidget(reportCard);
  auto* reportToolbarLayout = new QHBoxLayout(reportToolbar);
  reportToolbarLayout->setContentsMargins(14, 9, 14, 9);
  reportToolbarLayout->setSpacing(8);
  auto* reportTitle = new QLabel(I18n::tr("Diagnostic report"), reportToolbar);
  QFont reportTitleFont = reportTitle->font();
  reportTitleFont.setWeight(QFont::Medium);
  reportTitle->setFont(reportTitleFont);
  reportToolbarLayout->addWidget(reportTitle);
  reportToolbarLayout->addStretch();
  auto* reportSize = new QLabel(I18n::tr("%1 lines").arg(lineCount), reportToolbar);
  reportToolbarLayout->addWidget(reportSize);
  reportLayout->addWidget(reportToolbar);

  auto* divider = new QFrame(reportCard);
  divider->setObjectName(QStringLiteral("systemInformationReportDivider"));
  divider->setFixedHeight(1);
  reportLayout->addWidget(divider);

  auto* editor = new QPlainTextEdit(reportCard);
  editor->setObjectName(QStringLiteral("systemInformationReport"));
  editor->setReadOnly(true);
  editor->setLineWrapMode(QPlainTextEdit::NoWrap);
  QFont reportFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  if (reportFont.pointSizeF() > 0.0) reportFont.setPointSizeF(reportFont.pointSizeF() + 0.5);
  editor->setFont(reportFont);
  editor->setTabStopDistance(QFontMetricsF(reportFont).horizontalAdvance(QChar(' ')) * 4.0);
  editor->document()->setDocumentMargin(15.0);
  editor->setPlainText(std::move(report));
  formatReportBlocks(*editor->document());
  auto* highlighter = new DiagnosticReportHighlighter(editor->document());
  editor->moveCursor(QTextCursor::Start);
  reportLayout->addWidget(editor, 1);
  layout->addWidget(reportCard, 1);

  auto* buttons = new QDialogButtonBox(this);
  auto* copy = buttons->addButton(I18n::tr("Copy all"), QDialogButtonBox::ActionRole);
  copy->setObjectName(QStringLiteral("systemInformationCopyButton"));
  copy->setAutoDefault(false);
  const QString copyLabel = copy->text();
  connect(copy, &QPushButton::clicked, this, [copy, copyLabel, editor] {
    QGuiApplication::clipboard()->setText(editor->toPlainText());
    copy->setText(I18n::tr("Copied"));
    copy->setEnabled(false);
    QTimer::singleShot(1200, copy, [copy, copyLabel] {
      copy->setText(copyLabel);
      copy->setEnabled(true);
    });
  });
  auto* close = buttons->addButton(I18n::tr("Close"), QDialogButtonBox::AcceptRole);
  close->setDefault(true);
  connect(close, &QPushButton::clicked, this, &QDialog::accept);
  layout->addWidget(buttons);

  const auto applyVisualStyle = [headingIcon, noticeCard, noticeIcon, notice, subtitle, readOnlyBadge, reportCard, divider, editor, reportSize] {
    const auto& theme = ThemeManager::instance();
    headingIcon->setPixmap(theme.icon(QStringLiteral(":/icons/info.svg"), Qt::AlignCenter).pixmap(28, 28));
    noticeIcon->setPixmap(theme.icon(QStringLiteral(":/icons/info.svg"), Qt::AlignCenter).pixmap(16, 16));
    subtitle->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
    notice->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
    readOnlyBadge->setStyleSheet(
        QStringLiteral("QLabel#systemInformationReadOnlyBadge { color: %1; background: %2; border: 1px solid %3; border-radius: 10px; padding: 3px 9px; }")
            .arg(theme.color(Sem::Accent).name(), theme.color(Sem::Surface).name(), theme.color(Sem::Border).name()));
    noticeCard->setStyleSheet(QStringLiteral("QFrame#systemInformationPrivacyNotice { background: %1; border: 1px solid %2; border-radius: 8px; }")
                                  .arg(theme.color(Sem::Bg).name(), theme.color(Sem::Border).name()));
    reportCard->setStyleSheet(
        QStringLiteral(
            "QFrame#systemInformationReportCard { background: %1; border: 1px solid %2; border-radius: 10px; }"
            " QFrame#systemInformationReportCard QPlainTextEdit#systemInformationReport { background: %1; border: none; border-radius: 0; padding: 0; }")
            .arg(theme.color(Sem::Surface).name(), theme.color(Sem::Border).name()));
    divider->setStyleSheet(QStringLiteral("background: %1; border: 0;").arg(theme.color(Sem::Border).name()));
    reportSize->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
    editor->viewport()->update();
  };
  applyVisualStyle();
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [applyVisualStyle, highlighter](Theme) {
    applyVisualStyle();
    highlighter->rehighlight();
  });
}

}  // namespace uwf::ui
