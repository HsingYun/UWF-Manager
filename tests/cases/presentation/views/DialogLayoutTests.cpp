// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <qt_windows.h>

#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QCursor>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFontInfo>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSet>
#include <QSignalSpy>
#include <QStringList>
#include <QTest>
#include <QTextBlock>
#include <QTextEdit>
#include <QTimer>
#include <QWindow>
#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/appearance/WindowAppearance.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/layout/FlexibleColumnLayout.h"
#include "presentation/controls/layout/VerticalScrollArea.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/dialogs/InformationDialog.h"
#include "presentation/dialogs/PanelDialog.h"
#include "presentation/views/diagnostics/AboutDialog.h"
#include "presentation/views/power/PowerActionDialog.h"
#include "support/ui/DialogControls.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/ui/WindowActivation.h"
#include "support/visual/AnimationClock.h"
#include "support/visual/InlineIconTextProbe.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

namespace {

using uwf::test::buttonByRole;

template <typename Base>
class WindowDragProbe final : public Base {
 public:
  int dragRequests = 0;

 protected:
  bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override {
    const auto* native = static_cast<const MSG*>(message);
    if (native->message == WM_NCLBUTTONDOWN && native->wParam == HTCAPTION) {
      ++dragRequests;
      *result = 0;
      return true;
    }
    return Base::nativeEvent(type, message, result);
  }
};

class GuardedPanel : public uwf::ui::PanelDialog {
 public:
  bool allowRejection = false;
  int rejectionRequests = 0;
  bool requestRejection() override {
    ++rejectionRequests;
    return allowRejection;
  }
};

class DialogLayoutTests final : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase() {
    uwf::test::initializeVisualComparison();
    uwf::ui::I18n::instance().setLang(uwf::ui::I18n::Lang::En);
  }
  void messageFormattingIsExplicit_data();
  void messageFormattingIsExplicit();
  void shortMessagesFitContentAndPreserveConfirmationResults_data();
  void shortMessagesFitContentAndPreserveConfirmationResults();
  void dialogActionsRemainReadableAcrossStatesAndThemeChanges_data();
  void dialogActionsRemainReadableAcrossStatesAndThemeChanges();
  void aboutKeepsCompleteInformationWhenResized_data();
  void aboutKeepsCompleteInformationWhenResized();
  void aboutExpandedDisclosuresKeepTheirGeometryWhilePainting();
  void dialogBlankAreasDragWithoutStealingControlInput();
  void dialogsCenterOnTheirOwnerAfterMeasuringContent();
  void contentDialogsMeasureVisibleTextWithoutReservingBlankSpace_data();
  void contentDialogsMeasureVisibleTextWithoutReservingBlankSpace();
  void powerFailureDetailsExpandUntilTheScreenLimit_data();
  void powerFailureDetailsExpandUntilTheScreenLimit();
  void readOnlyReportsUseTheScreenBeforeScrolling_data();
  void readOnlyReportsUseTheScreenBeforeScrolling();
  void dialogHeaderStaysVisibleAndUsesTheCloseAction_data();
  void dialogHeaderStaysVisibleAndUsesTheCloseAction();
  void headerMetricsRemainAuthoritativeAfterFontChanges_data();
  void headerMetricsRemainAuthoritativeAfterFontChanges();
  void dialogButtonsHaveOneKeyboardStop_data();
  void dialogButtonsHaveOneKeyboardStop();
  void informationActionsKeepKeyboardActivation_data();
  void informationActionsKeepKeyboardActivation();
  void informationTitleDragsWithoutStealingCloseOrSelection();
  void panelEditorsKeepEnterLocalAndPreserveRejectionGuards();
  void panelWindowPreservesInputAndRejectionRules();
  void panelContentReceivesInitialKeyboardFocus();
  void panelSystemMenuReflectsAvailableWindowActions();
  void readOnlyDetailsKeepLineSpacingAndExposeLongContent();
  void scrollContentKeepsItsLayoutSynchronized_data();
  void scrollContentKeepsItsLayoutSynchronized();
  void dialogActionsDetermineTheMinimumWidth_data();
  void dialogActionsDetermineTheMinimumWidth();
  void dialogGraphicsShareTheirVisibleTextCenter_data();
  void dialogGraphicsShareTheirVisibleTextCenter();
  void windowMenuIconsShareTheirVisibleTextCenter();
};

void DialogLayoutTests::dialogGraphicsShareTheirVisibleTextCenter_data() {
  QTest::addColumn<int>("language");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<int>("width");
  for (const auto language : {uwf::ui::I18n::Lang::En, uwf::ui::I18n::Lang::Zh_CN, uwf::ui::I18n::Lang::Ar})
    for (const bool dark : {false, true})
      for (const int width : {360, 720})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(static_cast<int>(language)).arg(dark ? "dark" : "light").arg(width)))
            << static_cast<int>(language) << dark << width;
}

void DialogLayoutTests::dialogGraphicsShareTheirVisibleTextCenter() {
  QFETCH(int, language);
  QFETCH(bool, dark);
  QFETCH(int, width);
  auto& i18n = uwf::ui::I18n::instance();
  auto& theme = uwf::ui::ThemeManager::instance();
  const auto originalLanguage = i18n.lang();
  const auto originalTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    i18n.setLang(originalLanguage);
    theme.apply(originalTheme);
  });
  i18n.setLang(static_cast<uwf::ui::I18n::Lang>(language));
  theme.apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  const auto check = [&](const QRectF& first, const QRectF& second, const qreal scale, const QString& name) {
    QVERIFY2(!first.isEmpty() && !second.isEmpty(), qUtf8Printable(name));
    QVERIFY2(qRound(std::abs(first.center().y() - second.center().y()) * scale * 2) <= 2,
             qUtf8Printable(QStringLiteral("%1: centers %2 / %3, physical difference %4")
                                .arg(name)
                                .arg(first.center().y())
                                .arg(second.center().y())
                                .arg(std::abs(first.center().y() - second.center().y()) * scale)));
  };
  for (const auto style :
       {uwf::ui::DialogWindow::HeaderStyle::Standard, uwf::ui::DialogWindow::HeaderStyle::Power, uwf::ui::DialogWindow::HeaderStyle::Message}) {
    uwf::ui::PanelDialog dialog;
    dialog.setHeaderStyle(style);
    dialog.setWindowTitle(uwf::ui::I18n::tr("System information"));
    dialog.setHeaderIcon(theme.icon(QStringLiteral(":/icons/info.svg")));
    auto* frame = new uwf::ui::DialogLayout(&dialog);
    frame->body()->addWidget(new uwf::ui::TextLabel(uwf::ui::I18n::tr("Ready"), uwf::ui::TextStyle::Body, &dialog));
    dialog.resize(width, 240);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    auto* title = dialog.findChild<uwf::ui::TextLabel*>(QStringLiteral("dialogTitle"));
    auto* iconSurface = dialog.findChild<QWidget*>(QStringLiteral("dialogHeaderIcon"));
    QVERIFY(title && iconSurface);
    uwf::ui::IconView* glyph = nullptr;
    for (auto widgets = iconSurface->findChildren<QWidget*>(); auto* widget : std::as_const(widgets))
      if (auto* candidate = dynamic_cast<uwf::ui::IconView*>(widget)) glyph = candidate;
    QVERIFY(glyph);
    const auto pixels = dialog.grab().toImage();
    const auto titleInk = uwf::test::renderedInkBounds(*title).translated(title->mapTo(&dialog, QPoint()));
    const auto iconInk = uwf::test::renderedInkBounds(*glyph).translated(glyph->mapTo(&dialog, QPoint()));
    QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("dialog-header-%1").arg(static_cast<int>(style))));
    check(titleInk, iconInk, pixels.devicePixelRatio(), QStringLiteral("dialog header %1").arg(static_cast<int>(style)));
  }
  for (const bool technical : {false, true}) {
    QWidget host;
    host.setAutoFillBackground(true);
    auto* column = new QVBoxLayout(&host);
    column->addStretch();
    const auto hero = uwf::ui::createDialogHero(&host, theme.icon(QStringLiteral(":/icons/file_staging.svg")));
    uwf::ui::updateDialogHeroText(hero, uwf::ui::I18n::tr("File staging"), technical ? QStringLiteral("File staging / Commit") : QString{});
    column->addWidget(hero.widget);
    column->addStretch();
    host.resize(width, 200);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    const auto pixels = host.grab().toImage();
    QRectF textInk;
    for (auto* text : {hero.heading, hero.technical}) {
      if (!text->isVisible()) continue;
      textInk = textInk.united(uwf::test::renderedInkBounds(*text).translated(text->mapTo(&host, QPoint())));
    }
    const auto iconInk = uwf::test::renderedInkBounds(*hero.glyph).translated(hero.glyph->mapTo(&host, QPoint()));
    QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("dialog-hero-%1").arg(technical)));
    check(textInk, iconInk, pixels.devicePixelRatio(), QStringLiteral("dialog hero %1").arg(technical));
  }
}

void DialogLayoutTests::windowMenuIconsShareTheirVisibleTextCenter() {
  uwf::ui::PanelDialog dialog;
  dialog.setWindowTitle(QStringLiteral("Window menu"));
  dialog.resize(480, 240);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  bool captured = false;
  QTimer::singleShot(0, &dialog, [&] {
    auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    QVERIFY(menu);
    const auto errors = uwf::test::probeInlineIconText(*menu, QStringLiteral("window-system-menu"), QStringLiteral("normal"));
    QVERIFY2(errors.isEmpty(), qUtf8Printable(errors));
    QVERIFY(uwf::test::saveScreenshot(*menu, QStringLiteral("window-system-menu")));
    captured = true;
    menu->close();
  });
  uwf::ui::showWindowMenu(&dialog, dialog.mapToGlobal(QPoint(20, 20)));
  QVERIFY(captured);
}

void DialogLayoutTests::headerMetricsRemainAuthoritativeAfterFontChanges_data() {
  QTest::addColumn<uwf::ui::DialogWindow::HeaderStyle>("style");
  QTest::newRow("standard") << uwf::ui::DialogWindow::HeaderStyle::Standard;
  QTest::newRow("message") << uwf::ui::DialogWindow::HeaderStyle::Message;
}

void DialogLayoutTests::headerMetricsRemainAuthoritativeAfterFontChanges() {
  QFETCH(uwf::ui::DialogWindow::HeaderStyle, style);
  uwf::ui::PanelDialog dialog;
  dialog.setHeaderStyle(style);
  auto* frame = new uwf::ui::DialogLayout(&dialog);
  frame->body()->addWidget(new QLabel(QStringLiteral("Body"), &dialog));
  auto* header = dialog.findChild<QWidget*>(QStringLiteral("dialogHeader"));
  auto* title = dialog.findChild<uwf::ui::TextLabel*>(QStringLiteral("dialogTitle"));
  QVERIFY(header && title);
  const QMargins customMargins(18, 23, 18, 16);
  dialog.setHeaderMetrics({customMargins, 20, 1.7});
  auto font = dialog.font();
  font.setPixelSize(font.pixelSize() + 2);
  dialog.setFont(font);
  QCOMPARE(header->layout()->contentsMargins(), customMargins);
  QCOMPARE(title->font().pixelSize(), 20);
  QCOMPARE(title->firstLineBoxBounds().height(), 34.0);
  const QMargins compactMargins(16, 20, 16, 14);
  dialog.setHeaderMetrics({compactMargins, 19, 1.5});
  font.setPixelSize(font.pixelSize() + 1);
  dialog.setFont(font);
  QCOMPARE(header->layout()->contentsMargins(), compactMargins);
  QCOMPARE(title->font().pixelSize(), 19);
  QCOMPARE(title->firstLineBoxBounds().height(), 28.5);
}

void DialogLayoutTests::dialogActionsDetermineTheMinimumWidth_data() {
  QTest::addColumn<bool>("rtl");
  QTest::addColumn<int>("count");
  for (const bool rtl : {false, true})
    for (const int count : {2, 3}) QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-actions").arg(rtl ? "rtl" : "ltr").arg(count))) << rtl << count;
}

void DialogLayoutTests::dialogActionsDetermineTheMinimumWidth() {
  QFETCH(bool, rtl);
  QFETCH(int, count);
  uwf::ui::PanelDialog dialog;
  dialog.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
  auto* frame = new uwf::ui::DialogLayout(&dialog, uwf::ui::DialogContent::Scrollable, QSize(340, 0));
  frame->body()->addWidget(new QLabel(QStringLiteral("Configuration"), &dialog));
  auto* actions = new QDialogButtonBox(&dialog);
  for (int i = 0; i < count; ++i) actions->addButton(QStringLiteral("Action %1").arg(i), QDialogButtonBox::ActionRole);
  frame->setActions(actions);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* footer = actions->parentWidget();
  for (const QString& caption : {QStringLiteral("Action %1"), QStringLiteral("An action with a complete descriptive label %1"), QStringLiteral("Action %1")}) {
    const auto buttons = actions->buttons();
    for (int i = 0; i < buttons.size(); ++i) buttons[i]->setText(caption.arg(i));
    for (auto* button : buttons) QTRY_VERIFY(button->width() >= button->sizeHint().width());
    QTRY_VERIFY(footer->rect().contains(QRect(actions->mapTo(footer, QPoint()), actions->size())));
    QTRY_VERIFY(dialog.minimumWidth() >= footer->minimumSizeHint().width());
    for (auto* button : buttons) {
      QTRY_VERIFY(actions->rect().contains(QRect(button->mapTo(actions, QPoint()), button->size())));
    }
  }
}

void DialogLayoutTests::scrollContentKeepsItsLayoutSynchronized_data() {
  QTest::addColumn<bool>("customText");
  QTest::addColumn<int>("controls");
  QTest::newRow("label") << false << 0;
  QTest::newRow("label-form") << false << 6;
  QTest::newRow("text-label") << true << 0;
  QTest::newRow("text-label-form") << true << 6;
}

void DialogLayoutTests::scrollContentKeepsItsLayoutSynchronized() {
  QFETCH(bool, customText);
  QFETCH(int, controls);
  uwf::ui::VerticalScrollArea scroll;
  auto* content = new QWidget;
  auto* column = new uwf::ui::FlexibleColumnLayout(content);
  column->setContentsMargins(32, 0, 32, 26);
  column->setSpacing(20);
  auto* text = customText ? new uwf::ui::TextLabel({}, uwf::ui::TextStyle::Body, content) : new QLabel(content);
  text->setWordWrap(true);
  column->addWidget(text);
  for (int i = 0; i < controls; ++i) column->addWidget(new QCheckBox(QStringLiteral("Configuration option %1").arg(i), content));
  scroll.setWidget(content);
  scroll.resize(578, 400);
  scroll.show();
  QVERIFY(QTest::qWaitForWindowExposed(&scroll));
  for (const int repetitions : {1, 8, 1}) {
    text->setText(QStringLiteral("A complete wrapped description must remain visible. ").repeated(repetitions));
    for (const QSize size : {QSize(578, 400), QSize(420, 250), QSize(650, 500)}) {
      scroll.resize(size);
      QTRY_COMPARE(column->geometry(), content->contentsRect());
      QTRY_VERIFY(text->height() >= text->heightForWidth(text->width()));
      QTRY_COMPARE(scroll.horizontalScrollBar()->maximum(), 0);
    }
  }
}

void DialogLayoutTests::readOnlyDetailsKeepLineSpacingAndExposeLongContent() {
  uwf::ui::VerticalScrollArea scroll;
  auto* contentHost = new QWidget;
  auto* column = new uwf::ui::FlexibleColumnLayout(contentHost);
  auto& text = *new uwf::ui::ContentHintBox(contentHost);
  column->addWidget(&text);
  scroll.setWidget(contentHost);
  auto font = text.font();
  font.setPixelSize(13);
  text.setFont(font);
  text.setLineHeight(23.4);
  text.setPlainText(QStringLiteral("First line\nSecond line\nThird line"));
  scroll.resize(440, 140);
  scroll.show();
  QVERIFY(QTest::qWaitForWindowExposed(&scroll));
  QTRY_COMPARE(text.verticalScrollBar()->maximum(), 0);
  const auto* layout = text.document()->documentLayout();
  const auto first = text.document()->firstBlock();
  QVERIFY(qAbs(layout->blockBoundingRect(first.next()).top() - layout->blockBoundingRect(first).top() - 23.4) < .1);
  const QString content = QStringLiteral("Complete diagnostic line with a long path and Unicode 文本\n").repeated(100);
  text.setPlainText(content);
  QTRY_VERIFY(text.height() >= text.heightForWidth(text.width()));
  QTRY_COMPARE(text.verticalScrollBar()->maximum(), 0);
  QTRY_VERIFY(scroll.verticalScrollBar()->maximum() > 0);
  scroll.verticalScrollBar()->setValue(scroll.verticalScrollBar()->maximum());
  QVERIFY(scroll.viewport()->rect().contains(text.mapTo(scroll.viewport(), text.rect().bottomLeft())));
  QCOMPARE(text.toPlainText(), content);
}

void DialogLayoutTests::messageFormattingIsExplicit_data() {
  QTest::addColumn<int>("purpose");
  QTest::addColumn<bool>("rich");
  QTest::addColumn<bool>("dark");
  for (const bool dark : {false, true}) {
    const QByteArray theme = dark ? "dark-" : "light-";
    QTest::newRow((theme + "warning-literal").constData()) << 0 << false << dark;
    QTest::newRow((theme + "information-literal").constData()) << 1 << false << dark;
    QTest::newRow((theme + "confirmation-literal").constData()) << 2 << false << dark;
    QTest::newRow((theme + "confirmation-rich").constData()) << 2 << true << dark;
  }
}

void DialogLayoutTests::messageFormattingIsExplicit() {
  QFETCH(int, purpose);
  QFETCH(bool, rich);
  QFETCH(bool, dark);
  const auto previousTheme = uwf::ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] { uwf::ui::ThemeManager::instance().apply(previousTheme); });
  uwf::ui::ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  const auto message =
      rich ? QStringLiteral("<b>Continue</b><br>Apply changes?") : QStringLiteral("Registry <b>HKLM\\Software\\A&amp;B</b>\nDetails &amp; retry");
  const auto expected = rich ? QStringLiteral("Continue\nApply changes?") : message;
  bool inspected = false;
  using namespace uwf::ui;
  QVERIFY(uwf::test::driveModalDialog(
      *this,
      [&] {
        if (purpose == 0)
          dialogs::warning(nullptr, QStringLiteral("Diagnostic"), message);
        else if (purpose == 1)
          dialogs::information(nullptr, QStringLiteral("Diagnostic"), message);
        else if (rich)
          QVERIFY(!dialogs::confirm(nullptr, QStringLiteral("Confirm"), message, Qt::RichText));
        else
          QVERIFY(!dialogs::confirm(nullptr, QStringLiteral("Confirm"), message));
      },
      [&](QDialog& active) {
        auto* dialog = &active;
        auto* body = dialog->findChild<uwf::ui::ContentHintBox*>();
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), expected);
        const auto clipping = uwf::test::waitForUnclippedText(*dialog);
        QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
        QVERIFY(uwf::test::saveScreenshot(*dialog, QStringLiteral("dialog-message-format")));
        inspected = true;
      }));
  QVERIFY(inspected);
}

void DialogLayoutTests::shortMessagesFitContentAndPreserveConfirmationResults_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("language");
  QTest::addColumn<bool>("dark");
  for (const auto& language : uwf::ui::I18n::languages())
    for (const bool dark : {false, true}) {
      const QByteArray name = QStringLiteral("%1-%2").arg(QString(language.code), dark ? "dark" : "light").toLatin1();
      QTest::newRow(name.constData()) << language.id << dark;
    }
}

void DialogLayoutTests::shortMessagesFitContentAndPreserveConfirmationResults() {
  using namespace uwf::ui;
  QFETCH(I18n::Lang, language);
  QFETCH(bool, dark);
  const auto previousLanguage = I18n::instance().lang();
  const auto previousTheme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(previousLanguage);
    ThemeManager::instance().apply(previousTheme);
  });
  I18n::instance().setLang(language);
  ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  const QString message = I18n::tr("%1 change(s) have not been applied.\n\nDiscard them and continue?").arg(1);
  for (int input = 0; input < 5; ++input) {
    bool inspected = false;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(dialog);
      QVERIFY(dialog->windowFlags().testFlag(Qt::FramelessWindowHint));
      const auto nativeStyle = GetWindowLongPtrW(reinterpret_cast<HWND>(dialog->winId()), GWL_STYLE);
      QCOMPARE(nativeStyle & WS_CAPTION, static_cast<LONG_PTR>(0));
      auto close = qScopeGuard([dialog] { dialog->reject(); });
      auto* cancel = buttonByRole(dialog, QDialogButtonBox::RejectRole);
      auto* confirm = buttonByRole(dialog, QDialogButtonBox::AcceptRole);
      QVERIFY(cancel && confirm);
      QVERIFY(cancel->isDefault());
      QVERIFY(!confirm->isDefault());
      auto* body = dialog->findChild<ContentHintBox*>();
      QVERIFY(body);
      body->setFocus(Qt::TabFocusReason);
      uwf::test::moveMouse(body->viewport());
      QCOMPARE(body->viewport()->cursor().shape(), Qt::ArrowCursor);
      QTest::keyClicks(body, "edit");
      QCOMPARE(body->toPlainText(), message);
      cancel->setFocus(Qt::TabFocusReason);
      auto* scroll = dialog->findChild<QScrollArea*>();
      QVERIFY(scroll);
      const auto* bodyLayout = body->parentWidget()->layout();
      QVERIFY(bodyLayout);
      QTRY_VERIFY(scroll->viewport()->height() - body->geometry().bottom() - 1 <= bodyLayout->contentsMargins().bottom());
      QTRY_VERIFY(dialog->minimumSize() != dialog->maximumSize());
      for (auto areas = dialog->findChildren<QAbstractScrollArea*>(); const auto* area : std::as_const(areas)) {
        QTRY_COMPARE(area->verticalScrollBar()->maximum(), 0);
        QTRY_COMPARE(area->horizontalScrollBar()->maximum(), 0);
      }
      const QSize fitted = dialog->size();
      dialog->resize(fitted + QSize(180, 120));
      QCOMPARE(dialog->size(), fitted + QSize(180, 120));
      QVERIFY(dialog->screen()->availableGeometry().size().height() >= dialog->frameGeometry().height());
      QVERIFY(dialog->rect().contains(QRect(cancel->mapTo(dialog, QPoint()), cancel->size())));
      QVERIFY(dialog->rect().contains(QRect(confirm->mapTo(dialog, QPoint()), confirm->size())));
      if (input == 0) {
        QVERIFY(uwf::test::saveScreenshot(*dialog, QStringLiteral("compact-confirmation")));
        if (const QString directory = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR"); !directory.isEmpty()) {
          const QString name = QStringLiteral("native-confirmation-%1.png").arg(QString::fromLatin1(QTest::currentDataTag()));
          QVERIFY(uwf::test::windowScreenshot(*dialog).save(QDir(directory).filePath(name)));
        }
      }
      inspected = true;
      if (input == 0)
        QTest::keyClick(dialog, Qt::Key_Return);
      else if (input == 1)
        QTest::keyClick(dialog, Qt::Key_Escape);
      else if (input == 2)
        uwf::test::clickMouse(confirm, Qt::LeftButton);
      else if (input == 3)
        SendMessageW(reinterpret_cast<HWND>(dialog->winId()), WM_CLOSE, 0, 0);
      else {
        auto* information = qobject_cast<InformationDialog*>(dialog);
        QVERIFY(information && information->closeButton());
        uwf::test::clickMouse(information->closeButton(), Qt::LeftButton);
      }
      close.dismiss();
    });
    QCOMPARE(dialogs::confirm(nullptr, I18n::tr("Discard pending changes?"), message), input == 2);
    QVERIFY(inspected);
  }
}

void DialogLayoutTests::dialogActionsRemainReadableAcrossStatesAndThemeChanges_data() {
  QTest::addColumn<bool>("dark");
  QTest::newRow("light") << false;
  QTest::newRow("dark") << true;
}

void DialogLayoutTests::dialogActionsRemainReadableAcrossStatesAndThemeChanges() {
  using namespace uwf::ui;
  QFETCH(bool, dark);
  const auto previousTheme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] { ThemeManager::instance().apply(previousTheme); });
  ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  InformationDialog dialog;
  dialog.setWindowTitle(QStringLiteral("Dialog action states"));
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(600, 220));
  frame->body()->addWidget(new QLabel(QStringLiteral("Keyboard focus and action contrast"), &dialog));
  auto* buttons = new QDialogButtonBox(&dialog);
  auto* primary = buttons->addButton(QStringLiteral("Safe action"), QDialogButtonBox::AcceptRole);
  auto* destructive = buttons->addButton(QStringLiteral("Destructive action"), QDialogButtonBox::DestructiveRole);
  auto* direct = buttons->addButton(QStringLiteral("Direct action"), QDialogButtonBox::ActionRole);
  auto* cancel = buttons->addButton(QStringLiteral("Cancel"), QDialogButtonBox::RejectRole);
  setDialogAction(primary, DialogAction::Primary);
  setDialogAction(destructive, DialogAction::Destructive);
  setDialogAction(direct, DialogAction::DestructiveSecondary);
  primary->setAutoDefault(false);
  destructive->setAutoDefault(false);
  direct->setAutoDefault(false);
  cancel->setDefault(true);
  cancel->setFocus();
  frame->setActions(buttons);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  uwf::test::AnimationClock animation;
  dialog.raise();
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  const auto movePointer = [&](const QPoint& position) { uwf::test::moveMouse(dialog.windowHandle(), position); };
  const auto surface = [](QPushButton* button) {
    const auto image = button->grab().toImage();
    return uwf::test::regionColor(image, QRectF(button->width() * .25, button->height() * .1, button->width() * .5, button->height() * .15));
  };
  const auto luminance = [](const QColor& color) {
    const auto linear = [](const double value) { return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4); };
    return 0.2126 * linear(color.redF()) + 0.7152 * linear(color.greenF()) + 0.0722 * linear(color.blueF());
  };
  const auto pointerState = [&](QPushButton* button) {
    auto* under = QApplication::widgetAt(QCursor::pos());
    QString details;
    QDebug(&details) << button->text() << "active" << dialog.isActiveWindow() << "cursor" << QCursor::pos() << "button"
                     << QRect(button->mapToGlobal(QPoint()), button->size()) << "under" << under << "grabber" << QWidget::mouseGrabber() << "buttons"
                     << QApplication::mouseButtons() << "modal" << QApplication::activeModalWidget();
    return details;
  };
  for (int themePass = 0; themePass < 2; ++themePass) {
    dialog.raise();
    dialog.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&dialog));
    for (auto* button : {primary, destructive, direct, cancel, dialog.closeButton()}) {
      const auto verifyContrast = [&] {
        const auto image = button->grab().toImage();
        const double background = luminance(surface(button));
        const auto logical = uwf::test::interiorRegion(button->rect());
        const auto dpr = image.devicePixelRatio();
        const auto interior =
            QRectF(logical.x() * dpr, logical.y() * dpr, logical.width() * dpr, logical.height() * dpr).toAlignedRect().intersected(image.rect());
        int readablePixels = 0;
        for (int y = interior.top(); y <= interior.bottom(); ++y)
          for (int x = interior.left(); x <= interior.right(); ++x) {
            const double ink = luminance(image.pixelColor(x, y));
            const double contrast = (std::max(ink, background) + .05) / (std::min(ink, background) + .05);
            if (contrast >= 4.5) ++readablePixels;
          }
        if (readablePixels == 0 && !qEnvironmentVariable("UWF_UI_MOTION_ROOT").isEmpty()) {
          QDir output(qEnvironmentVariable("UWF_UI_MOTION_ROOT") + QStringLiteral("/dialog-actions"));
          QVERIFY(output.mkpath(QStringLiteral(".")));
          QVERIFY(dialog.grab().save(output.filePath(QStringLiteral("%1-%2.png").arg(QString::fromLatin1(QTest::currentDataTag()), button->text()))));
        }
        QVERIFY2(!interior.isEmpty() && static_cast<qreal>(readablePixels) / (static_cast<qreal>(interior.width()) * interior.height()) > 0,
                 qUtf8Printable(QStringLiteral("%1 visible=%2 geometry=%3,%4,%5,%6")
                                    .arg(button->text())
                                    .arg(button->isVisible())
                                    .arg(button->x())
                                    .arg(button->y())
                                    .arg(button->width())
                                    .arg(button->height())));
      };
      // Keep the resting pointer in the client body. The caption can be
      // handled as non-client space by Windows and is not a control hover.
      const QPoint restingPointer(12, dialog.height() / 2);
      movePointer(restingPointer);
      QTRY_VERIFY2(!button->underMouse(), qUtf8Printable(pointerState(button)));
      (button == primary ? cancel : primary)->setFocus(Qt::TabFocusReason);
      QTRY_VERIFY(!button->hasFocus());
      animation.advanceBy(200);
      verifyContrast();
      const QRect focusBounds(button->mapTo(&dialog, QPoint()) - QPoint(4, 4), button->size() + QSize(8, 8));
      const auto normal = dialog.grab(focusBounds).toImage();
      QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("dialog-normal-%1-%2").arg(themePass).arg(button->text())));
      QTest::keyClick(&dialog, Qt::Key_Tab);
      button->setFocus(Qt::TabFocusReason);
      QTRY_VERIFY(button->hasFocus());
      QVERIFY(uwf::test::changedCoverage(normal, dialog.grab(focusBounds).toImage(), QRectF(QPointF(), normal.deviceIndependentSize())) > 0);
      QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("dialog-focus-%1-%2").arg(themePass).arg(button->text())));
      uwf::test::moveMouse(button);
      QTRY_VERIFY2(button->underMouse(), qUtf8Printable(pointerState(button)));
      animation.advanceBy(200);
      verifyContrast();
      QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("dialog-hover-%1-%2").arg(themePass).arg(button->text()), true));
      uwf::test::pressMouse(button, Qt::LeftButton);
      animation.advanceBy(160);
      verifyContrast();
      QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("dialog-pressed-%1-%2").arg(themePass).arg(button->text()), true));
      uwf::test::releaseMouse(button, Qt::LeftButton, Qt::NoModifier, QPoint(-5, -5));
      button->setEnabled(false);
      QSignalSpy activated(button, &QPushButton::clicked);
      uwf::test::clickMouse(button, Qt::LeftButton);
      QCOMPARE(activated.count(), 0);
      QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("dialog-disabled-%1-%2").arg(themePass).arg(button->text())));
      button->setEnabled(true);
    }
    QVERIFY(surface(primary) != surface(cancel));
    QVERIFY(surface(destructive) != surface(cancel));
    ThemeManager::instance().toggle();
  }
}

void DialogLayoutTests::dialogHeaderStaysVisibleAndUsesTheCloseAction_data() {
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("longContent");
  QTest::addColumn<bool>("information");
  for (const bool dark : {false, true})
    for (const bool longContent : {false, true})
      for (const bool information : {false, true})
        QTest::newRow((QByteArray(dark ? "dark-" : "light-") + (longContent ? "long-" : "short-") + (information ? "information" : "panel")).constData())
            << dark << longContent << information;
}

void DialogLayoutTests::dialogHeaderStaysVisibleAndUsesTheCloseAction() {
  using namespace uwf::ui;
  QFETCH(bool, dark);
  QFETCH(bool, longContent);
  QFETCH(bool, information);
  const auto previousTheme = ThemeManager::instance().current();
  const auto originalPointer = QCursor::pos();
  // Native cursor warps must not interleave with QTest window mouse events.
  QCursor::setPos(0, 0);
  QGuiApplication::sync();
  const auto restore = qScopeGuard([&] {
    ThemeManager::instance().apply(previousTheme);
    QCursor::setPos(originalPointer);
    QGuiApplication::sync();
  });
  ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  std::unique_ptr<DialogWindow> window = information ? std::unique_ptr<DialogWindow>(new InformationDialog) : std::unique_ptr<DialogWindow>(new PanelDialog);
  auto& dialog = *window;
  dialog.setWindowTitle(QStringLiteral("Information title"));
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(560, 0));
  auto* text = new ContentHintBox(&dialog);
  text->setPlainText(longContent ? QStringLiteral("Complete diagnostic content.\n").repeated(200) : QStringLiteral("A short informational message."));
  frame->body()->addWidget(text);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* title = dialog.findChild<QLabel*>(QStringLiteral("dialogTitle"));
  auto* close = dialog.closeButton();
  QVERIFY(title && close && !close->icon().isNull());
  QVERIFY(dialog.windowFlags().testFlag(Qt::FramelessWindowHint));
  QCOMPARE(close->width(), close->height());
  QVERIFY(close->width() >= close->iconSize().width() + 12);
  QVERIFY(QFontInfo(title->font()).pixelSize() > QFontInfo(text->font()).pixelSize());
  QVERIFY(close->toolTip().isEmpty());
  QCOMPARE(close->accessibleName(), I18n::tr("Close"));
  QVERIFY(close->text().isEmpty());
  dialog.setWindowTitle(QStringLiteral("Updated information title"));
  QCOMPARE(title->text(), dialog.windowTitle());
  if (longContent) {
    QTRY_VERIFY(dialog.minimumSize() != dialog.maximumSize());
    QTRY_VERIFY(frame->scrollArea()->verticalScrollBar()->maximum() > 0);
    QTRY_VERIFY(frame->scrollArea()->focusPolicy() & Qt::TabFocus);
    const auto position = close->mapTo(&dialog, QPoint());
    frame->scrollArea()->verticalScrollBar()->setValue(frame->scrollArea()->verticalScrollBar()->maximum());
    QCOMPARE(close->mapTo(&dialog, QPoint()), position);
    QVERIFY(!close->visibleRegion().isEmpty());
    RECT bounds{};
    const auto handle = reinterpret_cast<HWND>(dialog.winId());
    QVERIFY(GetWindowRect(handle, &bounds));
    const auto hit = SendMessageW(handle, WM_NCHITTEST, 0, MAKELPARAM(bounds.left + 1, bounds.top + 1));
    QCOMPARE(hit, static_cast<LRESULT>(HTTOPLEFT));
  } else {
    QTRY_VERIFY(dialog.minimumSize() != dialog.maximumSize());
    QTRY_COMPARE(frame->scrollArea()->verticalScrollBar()->maximum(), 0);
    QCOMPARE(frame->scrollArea()->focusPolicy(), Qt::NoFocus);
  }
  const auto clipping = uwf::test::waitForUnclippedText(dialog, 3000);
  const auto layoutDetails = [&] {
    QString result;
    QDebug(&result) << clipping << "title" << title->size() << title->sizeHint() << title->minimumSizeHint() << "header"
                    << title->parentWidget()->size() << title->parentWidget()->sizeHint()
                    << title->parentWidget()->heightForWidth(title->parentWidget()->width()) << "window" << dialog.size() << dialog.minimumSize();
    return result;
  };
  QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping.isEmpty() ? QString{} : layoutDetails()));
  QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("dialog-header")));
  const auto verifyHover = [&](const QString& phase) {
    dialog.raise();
    dialog.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&dialog));
    const auto movePointer = [&](const QPoint& position) { uwf::test::moveMouse(dialog.windowHandle(), position); };
    // Consecutive windows can share geometry. Start away from the preceding
    // release position so Qt does not discard this move as an unchanged point.
    movePointer(QPoint(8, 24));
    QTRY_VERIFY(!close->underMouse());
    close->setFocus(Qt::TabFocusReason);
    const auto surface = [close] {
      const auto image = close->grab().toImage();
      return uwf::test::regionColor(image, QRectF(close->width() * .25, close->height() * .1, close->width() * .5, close->height() * .15));
    };
    const auto normal = surface();
    uwf::test::moveMouse(close);
    QTRY_VERIFY2(close->underMouse(), qUtf8Printable(phase));
    QTRY_VERIFY2(surface() != normal, qUtf8Printable(QStringLiteral("%1 normal=%2 hovered=%3 focus=%4 position=%5,%6")
                                                         .arg(phase, normal.name(), surface().name())
                                                         .arg(close->hasFocus())
                                                         .arg(QCursor::pos().x())
                                                         .arg(QCursor::pos().y())));
    const auto hovered = surface();
    QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("dialog-close-hover-%1").arg(phase), true));
    uwf::test::pressMouse(close, Qt::LeftButton);
    {
      const auto release = qScopeGuard([&] { uwf::test::releaseMouse(dialog.windowHandle(), Qt::LeftButton, Qt::NoModifier, QPoint(8, 8)); });
      QTRY_VERIFY(close->isDown());
      QTRY_VERIFY(surface() != hovered);
    }
    QTRY_VERIFY(!close->isDown());
    QTRY_COMPARE(QApplication::mouseButtons(), Qt::NoButton);
    movePointer(QPoint(8, 8));
    QVERIFY(dialog.isVisible());
  };
  verifyHover(QStringLiteral("initial"));
  if (QTest::currentTestFailed()) return;
  InformationDialog nested(&dialog);
  nested.setWindowTitle(QStringLiteral("Nested information"));
  auto* nestedLayout = new DialogLayout(&nested, DialogContent::Scrollable, QSize(400, 0));
  nestedLayout->body()->addWidget(new QLabel(QStringLiteral("Return to the original dialog."), &nested));
  QTimer::singleShot(0, &nested, [&] {
    const auto closeNested = qScopeGuard([&] { nested.reject(); });
    QVERIFY(QTest::qWaitForWindowActive(&nested));
  });
  nested.exec();
  verifyHover(QStringLiteral("after-nested"));
  if (QTest::currentTestFailed()) return;
  QSignalSpy finished(&dialog, &QDialog::finished);
  uwf::test::clickMouse(close, Qt::LeftButton);
  QCOMPARE(finished.count(), 1);
  QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
}

void DialogLayoutTests::panelEditorsKeepEnterLocalAndPreserveRejectionGuards() {
  using namespace uwf::ui;
  GuardedPanel dialog;
  auto* frame = new DialogLayout(&dialog, DialogContent::Form, QSize(440, 240));
  auto* input = new QLineEdit(QStringLiteral("Editable path"), &dialog);
  frame->body()->addWidget(input);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
  auto* cancel = buttons->button(QDialogButtonBox::Cancel);
  cancel->setDefault(true);
  connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
  frame->setActions(buttons);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  input->setFocus(Qt::OtherFocusReason);
  QTRY_VERIFY(input->hasFocus());
  QVERIFY(cancel->isDefault());
  QSignalSpy submitted(input, &QLineEdit::returnPressed);
  QSignalSpy cancelled(cancel, &QPushButton::clicked);
  QSignalSpy finished(&dialog, &QDialog::finished);
  int submissions = 0;
  for (const auto key : {Qt::Key_Return, Qt::Key_Enter}) {
    QTest::keyClick(input, key);
    ++submissions;
    QCOMPARE(submitted.count(), submissions);
    QCOMPARE(cancelled.count(), 0);
    QCOMPARE(dialog.rejectionRequests, 0);
    QCOMPARE(finished.count(), 0);
    QVERIFY(dialog.isVisible());
  }
  cancel->setFocus(Qt::TabFocusReason);
  QTRY_VERIFY(cancel->hasFocus());
  QTest::keyClick(cancel, Qt::Key_Return);
  QCOMPARE(cancelled.count(), 1);
  QCOMPARE(dialog.rejectionRequests, 1);
  QCOMPARE(finished.count(), 0);
  input->setFocus(Qt::OtherFocusReason);
  QTRY_VERIFY(input->hasFocus());
  QTest::keyClick(input, Qt::Key_Escape);
  QCOMPARE(dialog.rejectionRequests, 2);
  QCOMPARE(finished.count(), 0);
  QTest::mouseClick(dialog.closeButton(), Qt::LeftButton);
  QCOMPARE(dialog.rejectionRequests, 3);
  QCOMPARE(finished.count(), 0);
  dialog.allowRejection = true;
  cancel->setFocus(Qt::TabFocusReason);
  QTRY_VERIFY(cancel->hasFocus());
  QTest::keyClick(cancel, Qt::Key_Return);
  QCOMPARE(cancelled.count(), 2);
  QCOMPARE(dialog.rejectionRequests, 4);
  QCOMPARE(finished.count(), 1);
  QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
}

void DialogLayoutTests::panelWindowPreservesInputAndRejectionRules() {
  using namespace uwf::ui;
  WindowDragProbe<GuardedPanel> dialog;
  dialog.setWindowTitle(QStringLiteral("Panel workflow"));
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(640, 320), QSize(440, 240));
  auto* input = new QLineEdit(&dialog);
  input->setText(QStringLiteral("Editable path"));
  frame->body()->addWidget(input);
  frame->body()->addStretch();
  auto* buttons = new QDialogButtonBox(&dialog);
  auto* proceed = buttons->addButton(QStringLiteral("Continue"), QDialogButtonBox::AcceptRole);
  proceed->setAutoDefault(false);
  auto* cancel = buttons->addButton(QStringLiteral("Cancel"), QDialogButtonBox::RejectRole);
  cancel->setDefault(true);
  connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
  frame->setActions(buttons);
  cancel->setFocus();
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  QSignalSpy finished(&dialog, &QDialog::finished);
  const auto handle = reinterpret_cast<HWND>(dialog.winId());
  QCOMPARE(GetWindowLongPtrW(handle, GWL_STYLE) & WS_CAPTION, static_cast<LONG_PTR>(0));
  QVERIFY(!dialog.closeButton()->isDefault());
  QVERIFY(cancel->isDefault());
  QVERIFY(cancel->hasFocus());
  auto* title = dialog.findChild<QLabel*>(QStringLiteral("dialogTitle"));
  QVERIFY(title);
  QTest::mouseClick(title, Qt::LeftButton);
  QCOMPARE(dialog.dragRequests, 1);
  QTest::mouseClick(input, Qt::LeftButton);
  QTest::keyClick(input, Qt::Key_A, Qt::ControlModifier);
  QTest::keyClicks(input, "New path");
  QCOMPARE(input->text(), QStringLiteral("New path"));
  QCOMPARE(dialog.dragRequests, 1);
  QTest::keyClick(input, Qt::Key_Return);
  QCOMPARE(dialog.rejectionRequests, 0);
  QCOMPARE(finished.count(), 0);
  cancel->setFocus(Qt::TabFocusReason);
  QTest::keyClick(cancel, Qt::Key_Return);
  QCOMPARE(dialog.rejectionRequests, 1);
  QCOMPARE(finished.count(), 0);
  QTest::mouseClick(dialog.closeButton(), Qt::LeftButton);
  QCOMPARE(dialog.rejectionRequests, 2);
  QCOMPARE(finished.count(), 0);
  QTest::keyClick(input, Qt::Key_Escape);
  QCOMPARE(dialog.rejectionRequests, 3);
  SendMessageW(handle, WM_CLOSE, 0, 0);
  QTRY_COMPARE(dialog.rejectionRequests, 4);
  QCOMPARE(finished.count(), 0);
  const QSize before = dialog.size();
  dialog.resize(before + QSize(40, 40));
  QCOMPARE(dialog.size(), before + QSize(40, 40));
  dialog.setFixedSize(dialog.size());
  RECT bounds{};
  QVERIFY(GetWindowRect(handle, &bounds));
  QCOMPARE(SendMessageW(handle, WM_NCHITTEST, 0, MAKELPARAM(bounds.left + 1, bounds.top + 1)), static_cast<LRESULT>(HTCLIENT));
  dialog.allowRejection = true;
  dialog.closeButton()->setFocus(Qt::TabFocusReason);
  QTest::keyClick(dialog.closeButton(), Qt::Key_Return);
  QCOMPARE(finished.count(), 1);
  QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
}

void DialogLayoutTests::panelContentReceivesInitialKeyboardFocus() {
  uwf::ui::PanelDialog dialog;
  auto* frame = new uwf::ui::DialogLayout(&dialog);
  auto* input = new QLineEdit(&dialog);
  frame->body()->addWidget(input);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  QTRY_VERIFY(input->hasFocus());
  QTest::keyClick(input, Qt::Key_Tab);
  QCOMPARE(QApplication::focusWidget(), dialog.closeButton());
  QTest::keyClick(dialog.closeButton(), Qt::Key_Tab);
  QCOMPARE(QApplication::focusWidget(), input);
}

void DialogLayoutTests::panelSystemMenuReflectsAvailableWindowActions() {
  uwf::ui::PanelDialog dialog;
  dialog.setWindowTitle(QStringLiteral("Window actions"));
  auto* frame = new uwf::ui::DialogLayout(&dialog, uwf::ui::DialogContent::Scrollable, QSize(560, 300), QSize());
  auto* input = new QLineEdit(&dialog);
  frame->body()->addWidget(input);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QVERIFY(GetSystemMenu(reinterpret_cast<HWND>(dialog.winId()), FALSE));
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  auto* title = dialog.findChild<QLabel*>(QStringLiteral("dialogTitle"));
  QVERIFY(title);
  int menusOpened = 0;
  const auto inspectMenu = [&](bool enabled) {
    QTimer::singleShot(50, &dialog, [&, enabled] {
      auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      QVERIFY(menu);
      const auto close = qScopeGuard([menu] { menu->close(); });
      ++menusOpened;
      const auto action = [&](const QString& text) -> QAction* {
        for (auto actions = menu->actions(); auto* entry : std::as_const(actions))
          if (entry->text() == text) return entry;
        return nullptr;
      };
      auto* size = action(uwf::ui::I18n::tr("Size"));
      auto* closeAction = action(uwf::ui::I18n::tr("Close"));
      auto* minimize = action(uwf::ui::I18n::tr("Minimize"));
      QVERIFY(size && closeAction && minimize);
      QCOMPARE(size->isEnabled(), enabled);
      QCOMPARE(closeAction->isEnabled(), enabled);
      QVERIFY(!minimize->isEnabled());
    });
  };
  inspectMenu(true);
  const auto position = title->rect().center();
  QContextMenuEvent context(QContextMenuEvent::Mouse, position, title->mapToGlobal(position));
  QApplication::sendEvent(title, &context);
  QCOMPARE(menusOpened, 1);
  dialog.setFixedSize(dialog.size());
  dialog.setRejectionEnabled(false);
  inspectMenu(false);
  QTest::keyClick(input, Qt::Key_Space, Qt::AltModifier);
  QTRY_COMPARE(menusOpened, 2);
  QVERIFY(dialog.isVisible());
}

void DialogLayoutTests::aboutKeepsCompleteInformationWhenResized_data() { shortMessagesFitContentAndPreserveConfirmationResults_data(); }

void DialogLayoutTests::informationActionsKeepKeyboardActivation_data() { dialogActionsRemainReadableAcrossStatesAndThemeChanges_data(); }

void DialogLayoutTests::informationActionsKeepKeyboardActivation() {
  using namespace uwf::ui;
  QFETCH(bool, dark);
  const auto previousTheme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] { ThemeManager::instance().apply(previousTheme); });
  ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  const QString report = QStringLiteral("Mock diagnostic report\nNo machine data.\n");
  int collections = 0;
  AboutDialog dialog([&] {
    ++collections;
    return uwf::presentation::DiagnosticReportViewModel(report);
  });
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* open = dialog.findChild<QPushButton*>(QStringLiteral("aboutSystemInformationButton"));
  QVERIFY(open);
  bool inspected = false;
  open->setFocus(Qt::TabFocusReason);
  QVERIFY(uwf::test::driveModalDialog(
      dialog, [&] { QTest::keyClick(open, Qt::Key_Return); },
      [&](QDialog& active) {
        auto* information = qobject_cast<InformationDialog*>(&active);
        QVERIFY(information);
        auto* copy = information->findChild<QPushButton*>(QStringLiteral("systemInformationCopyButton"));
        QVERIFY(copy);
        copy->setFocus(Qt::TabFocusReason);
        QSignalSpy activated(copy, &QPushButton::clicked);
        QTest::keyClick(copy, Qt::Key_Return);
        QCOMPARE(activated.count(), 1);
        QCOMPARE(QApplication::clipboard()->text(), report);
        QVERIFY(information->isVisible());
        inspected = true;
      }));
  QCOMPARE(collections, 1);
  QVERIFY(inspected);
  QVERIFY(dialog.isVisible());
  dialog.closeButton()->setFocus(Qt::TabFocusReason);
  QTest::keyClick(dialog.closeButton(), Qt::Key_Return);
  QVERIFY(!dialog.isVisible());
}

void DialogLayoutTests::informationTitleDragsWithoutStealingCloseOrSelection() {
  WindowDragProbe<uwf::ui::InformationDialog> dialog;
  dialog.setWindowTitle(QStringLiteral("Draggable information title"));
  auto* frame = new uwf::ui::DialogLayout(&dialog, uwf::ui::DialogContent::Scrollable, QSize(560, 0));
  auto* body = new uwf::ui::ContentHintBox(&dialog);
  body->setPlainText(QStringLiteral("Selectable diagnostic details"));
  body->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  frame->body()->addWidget(body);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* title = dialog.findChild<QLabel*>(QStringLiteral("dialogTitle"));
  QVERIFY(title);
  uwf::test::clickMouse(title, Qt::LeftButton);
  QCOMPARE(dialog.dragRequests, 1);
  uwf::test::clickMouse(title->parentWidget(), Qt::LeftButton, Qt::NoModifier, QPoint(8, 8));
  QCOMPARE(dialog.dragRequests, 2);
  uwf::test::pressMouse(body->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(2, body->fontMetrics().height() / 2));
  uwf::test::moveMouse(body->viewport(), QPoint(body->viewport()->width() - 2, body->fontMetrics().height() / 2));
  uwf::test::releaseMouse(body->viewport(), Qt::LeftButton);
  QVERIFY(!body->textCursor().selectedText().isEmpty());
  QCOMPARE(dialog.dragRequests, 2);
  uwf::test::clickMouse(dialog.closeButton(), Qt::LeftButton);
  QCOMPARE(dialog.dragRequests, 2);
  QVERIFY(!dialog.isVisible());
}

void DialogLayoutTests::dialogButtonsHaveOneKeyboardStop_data() {
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("information");
  for (const bool dark : {false, true})
    for (const bool information : {false, true})
      QTest::newRow((QByteArray(dark ? "dark-" : "light-") + (information ? "information" : "panel")).constData()) << dark << information;
}

void DialogLayoutTests::dialogButtonsHaveOneKeyboardStop() {
  using namespace uwf::ui;
  QFETCH(bool, dark);
  QFETCH(bool, information);
  const auto previousTheme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] { ThemeManager::instance().apply(previousTheme); });
  ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  std::unique_ptr<QDialog> dialog = information ? std::unique_ptr<QDialog>(new InformationDialog) : std::unique_ptr<QDialog>(new PanelDialog);
  dialog->setWindowTitle(QStringLiteral("Keyboard navigation"));
  auto* frame = new DialogLayout(dialog.get(), DialogContent::Scrollable, QSize(560, 0));
  frame->body()->addWidget(new QLabel(QStringLiteral("Each action has one keyboard focus stop."), dialog.get()));
  auto* buttons = new QDialogButtonBox(dialog.get());
  auto* primary = buttons->addButton(QStringLiteral("Continue"), QDialogButtonBox::AcceptRole);
  setDialogAction(primary, DialogAction::Primary);
  primary->setAutoDefault(false);
  auto* secondary = buttons->addButton(QStringLiteral("Copy"), QDialogButtonBox::ActionRole);
  auto* cancel = buttons->addButton(QStringLiteral("Cancel"), QDialogButtonBox::RejectRole);
  cancel->setDefault(true);
  auto* auxiliary = new QDialogButtonBox(dialog.get());
  auto* details = auxiliary->addButton(QStringLiteral("Details"), QDialogButtonBox::ActionRole);
  details->setAutoDefault(false);
  frame->setActions(buttons, auxiliary);
  QSet<QWidget*> expected{primary, secondary, cancel, details};
  expected.insert(qobject_cast<DialogWindow*>(dialog.get())->closeButton());
  dialog->show();
  QVERIFY(QTest::qWaitForWindowExposed(dialog.get()));
  QVERIFY(uwf::test::activateWindowForInput(*dialog));
  cancel->setFocus(Qt::TabFocusReason);
  QSet<QWidget*> visited;
  QList<QWidget*> order;
  for (qsizetype stop = 0; stop < expected.size(); ++stop) {
    auto* focused = QApplication::focusWidget();
    QVERIFY(focused);
    QVERIFY2(expected.contains(focused), focused->metaObject()->className());
    QVERIFY2(!visited.contains(focused), qUtf8Printable(QStringLiteral("Repeated stop %1: %2").arg(stop).arg(qobject_cast<QPushButton*>(focused)->text())));
    visited.insert(focused);
    order.append(focused);
    QVERIFY(uwf::test::saveScreenshot(*dialog, QStringLiteral("dialog-tab-%1").arg(stop)));
    QTest::keyClick(focused, Qt::Key_Tab);
  }
  QCOMPARE(visited, expected);
  QCOMPARE(QApplication::focusWidget(), cancel);
  for (qsizetype stop = order.size(); stop > 0; --stop) {
    QTest::keyClick(QApplication::focusWidget(), Qt::Key_Tab, Qt::ShiftModifier);
    QCOMPARE(QApplication::focusWidget(), order[stop - 1]);
  }
  for (int change = 0; change < 3; ++change) {
    if (change == 0) {
      secondary->setEnabled(false);
      expected.remove(secondary);
    } else if (change == 1) {
      ThemeManager::instance().toggle();
    } else {
      details->hide();
      expected.remove(details);
    }
    QTest::qWait(1);
    cancel->setFocus(Qt::TabFocusReason);
    visited.clear();
    for (qsizetype stop = 0; stop < expected.size(); ++stop) {
      auto* focused = QApplication::focusWidget();
      QVERIFY(expected.contains(focused));
      QVERIFY2(!visited.contains(focused),
               qUtf8Printable(QStringLiteral("Repeated stop %1 after change %2: %3").arg(stop).arg(change).arg(qobject_cast<QPushButton*>(focused)->text())));
      visited.insert(focused);
      QTest::keyClick(focused, Qt::Key_Tab);
    }
    QCOMPARE(visited, expected);
  }
  primary->setFocus(Qt::TabFocusReason);
  QSignalSpy clicked(primary, &QPushButton::clicked);
  QTest::keyClick(primary, Qt::Key_Space);
  QCOMPARE(clicked.count(), 1);
}

void DialogLayoutTests::aboutKeepsCompleteInformationWhenResized() {
  using namespace uwf::ui;
  QFETCH(I18n::Lang, language);
  QFETCH(bool, dark);
  const auto previousLanguage = I18n::instance().lang();
  const auto previousTheme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(previousLanguage);
    ThemeManager::instance().apply(previousTheme);
  });
  I18n::instance().setLang(language);
  ThemeManager::instance().apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  AboutDialog dialog([] { return uwf::presentation::DiagnosticReportViewModel(QStringLiteral("Mock report")); }, nullptr);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* license = dialog.findChild<Disclosure*>(QStringLiteral("aboutLicense"));
  QVERIFY(license);
  auto* description = license->findChild<ContentHintBox*>();
  QVERIFY(description && description->toHtml().contains(QStringLiteral("gnu.org")));
  QVERIFY(dialog.minimumSize() != dialog.maximumSize());
  for (int size = 0; size < 2; ++size) {
    const auto clipping = uwf::test::waitForUnclippedText(dialog);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    auto* close = dialog.closeButton();
    QVERIFY(close);
    QVERIFY(!close->visibleRegion().isEmpty());
    QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("about-natural-spacing-%1").arg(size)));
    const QSize requested = (dialog.size() + QSize(120, 100)).boundedTo(dialog.screen()->availableGeometry().size() - QSize(48, 80));
    dialog.resize(requested);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCOMPARE(dialog.size(), requested);
  }
  auto* scroll = dialog.findChild<QScrollArea*>();
  QVERIFY(scroll);
  QCOMPARE(scroll->verticalScrollBar()->maximum(), 0);
  QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QTest::mouseClick(license->toggleButton(), Qt::LeftButton);
  QVERIFY(license->isOpen());
  QTRY_VERIFY(description->isVisible());
  const auto clipping = uwf::test::waitForUnclippedText(dialog);
  QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
  QCOMPARE(description->verticalScrollBar()->maximum(), 0);
  QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());
  QTest::keyClick(&dialog, Qt::Key_Escape);
  QVERIFY(!dialog.isVisible());
}

void DialogLayoutTests::aboutExpandedDisclosuresKeepTheirGeometryWhilePainting() {
  using namespace uwf::ui;
  const auto previousLanguage = I18n::instance().lang();
  const auto previousTheme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(previousLanguage);
    ThemeManager::instance().apply(previousTheme);
  });
  I18n::instance().setLang(I18n::Lang::Zh_CN);
  ThemeManager::instance().apply(uwf::app::Theme::Dark);
  AboutDialog dialog([] { return uwf::presentation::DiagnosticReportViewModel(QStringLiteral("Mock report")); }, nullptr);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* impact = dialog.findChild<Disclosure*>(QStringLiteral("aboutUwfImpact"));
  auto* license = dialog.findChild<Disclosure*>(QStringLiteral("aboutLicense"));
  auto* scroll = dialog.findChild<QScrollArea*>();
  QVERIFY(impact && license && scroll && scroll->widget());
  auto* impactText = impact->findChild<ContentHintBox*>();
  QVERIFY(impactText);
  impact->setOpen(true);
  QTRY_VERIFY(impactText->isVisible());
  QTRY_VERIFY(impactText->height() >= impactText->heightForWidth(impactText->width()));
  QTRY_COMPARE(impact->height(), impact->heightForWidth(impact->width()));
  QTRY_COMPARE(license->height(), license->hasHeightForWidth() ? license->heightForWidth(license->width()) : license->sizeHint().height());

  class DisclosurePaintProbe final : public QObject {
   public:
    DisclosurePaintProbe(QScrollArea& scroll, Disclosure& expanded, ContentHintBox& text, Disclosure& changing)
        : scroll(scroll),
          expanded(expanded),
          text(text),
          changing(changing),
          expandedTitleY(expanded.toggleButton()->y()),
          changingTitleY(changing.toggleButton()->y()) {}
    QScrollArea& scroll;
    Disclosure& expanded;
    ContentHintBox& text;
    Disclosure& changing;
    const int expandedTitleY;
    const int changingTitleY;
    int paints = 0;
    QStringList violations;

   protected:
    bool eventFilter(QObject*, QEvent* event) override {
      if (event->type() != QEvent::Paint) return false;
      ++paints;
      const auto* content = scroll.widget();
      const int expandedY = expanded.mapTo(content, QPoint()).y();
      const int textY = text.mapTo(content, QPoint()).y();
      const int changingY = changing.mapTo(content, QPoint()).y();
      const int requiredTextHeight = text.heightForWidth(text.width());
      const int requiredExpandedHeight = expanded.heightForWidth(expanded.width());
      if (!expanded.isOpen() || !text.isVisible() || text.height() < requiredTextHeight || expanded.height() < requiredExpandedHeight ||
          changingY < expandedY + expanded.height() || changingY < textY + requiredTextHeight || expanded.toggleButton()->y() != expandedTitleY ||
          changing.toggleButton()->y() != changingTitleY) {
        violations.append(QStringLiteral("paint %1: expanded=%2/%3 text=%4/%5 followingY=%6 expandedBottom=%7 textBottom=%8 "
                                         "titleOffsets=%9,%10 expected=%11,%12 scroll=%13 contentHeight=%14")
                              .arg(paints)
                              .arg(expanded.height())
                              .arg(requiredExpandedHeight)
                              .arg(text.height())
                              .arg(requiredTextHeight)
                              .arg(changingY)
                              .arg(expandedY + expanded.height())
                              .arg(textY + requiredTextHeight)
                              .arg(expanded.toggleButton()->y())
                              .arg(changing.toggleButton()->y())
                              .arg(expandedTitleY)
                              .arg(changingTitleY)
                              .arg(scroll.verticalScrollBar()->value())
                              .arg(content->height()));
      }
      return false;
    }
  } probe(*scroll, *impact, *impactText, *license);
  license->toggleButton()->installEventFilter(&probe);
  for (int toggle = 0; toggle < 8; ++toggle) {
    const int previousPaints = probe.paints;
    QTest::mouseClick(license->toggleButton(), Qt::LeftButton);
    // Observe natural paint delivery. Activating layouts or grabbing a widget here
    // can correct an invalid intermediate layout before it reaches the screen.
    QTRY_VERIFY(probe.paints > previousPaints);
    QTRY_VERIFY(impactText->height() >= impactText->heightForWidth(impactText->width()));
    QTRY_COMPARE(impact->height(), impact->heightForWidth(impact->width()));
    QTRY_COMPARE(license->height(), license->hasHeightForWidth() ? license->heightForWidth(license->width()) : license->sizeHint().height());
    QTRY_COMPARE(license->toggleButton()->y(), probe.changingTitleY);
  }
  QVERIFY2(probe.violations.isEmpty(), qUtf8Printable(probe.violations.join(u'\n')));
}

void DialogLayoutTests::dialogBlankAreasDragWithoutStealingControlInput() {
  WindowDragProbe<QDialog> dialog;
  auto* frame = new uwf::ui::DialogLayout(&dialog, uwf::ui::DialogContent::Scrollable, QSize(600, 400));
  auto* text = new QLineEdit(QStringLiteral("Editable text"), &dialog);
  auto* details = new QPlainTextEdit(QStringLiteral("Selectable diagnostic details"), &dialog);
  details->setReadOnly(true);
  frame->body()->addWidget(text);
  frame->body()->addWidget(details);
  auto* buttons = new QDialogButtonBox(&dialog);
  auto* action = buttons->addButton(QStringLiteral("Action"), QDialogButtonBox::ActionRole);
  frame->setActions(buttons);
  QSignalSpy clicked(action, &QPushButton::clicked);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QTest::mouseClick(&dialog, Qt::LeftButton, Qt::NoModifier, QPoint(8, 8));
  QCOMPARE(dialog.dragRequests, 1);
  QTest::mouseClick(frame->body()->parentWidget(), Qt::LeftButton, Qt::NoModifier, QPoint(4, 4));
  QCOMPARE(dialog.dragRequests, 2);
  QTest::mouseClick(action, Qt::LeftButton);
  QCOMPARE(clicked.count(), 1);
  action->setEnabled(false);
  QTest::mouseClick(action, Qt::LeftButton);
  QCOMPARE(clicked.count(), 1);
  QCOMPARE(dialog.dragRequests, 2);
  action->setEnabled(true);
  QTest::mouseClick(text, Qt::LeftButton);
  QTest::keyClick(text, Qt::Key_A, Qt::ControlModifier);
  QCOMPARE(text->selectedText(), text->text());
  QTest::mouseClick(details->viewport(), Qt::LeftButton);
  QTest::keyClick(details, Qt::Key_A, Qt::ControlModifier);
  QCOMPARE(details->textCursor().selectedText(), details->toPlainText());
  QCOMPARE(dialog.dragRequests, 2);
  dialog.setFixedSize(dialog.size());
  QTest::mouseClick(&dialog, Qt::LeftButton, Qt::NoModifier, QPoint(8, 8));
  QCOMPARE(dialog.dragRequests, 3);
}

void DialogLayoutTests::dialogsCenterOnTheirOwnerAfterMeasuringContent() {
  QWidget owner;
  const auto requested = uwf::test::boundedWindowSize(owner, QSize(1000, 800));
  QVERIFY(!requested.isEmpty());
  owner.resize(requested);
  owner.show();
  QVERIFY(QTest::qWaitForWindowExposed(&owner));
  owner.move(owner.screen()->availableGeometry().center() - owner.rect().center());
  uwf::ui::AboutDialog about([] { return uwf::presentation::DiagnosticReportViewModel(QStringLiteral("Mock report")); }, &owner);
  about.show();
  QVERIFY(QTest::qWaitForWindowExposed(&about));
  QTRY_VERIFY(uwf::test::centersAligned(about.frameGeometry(), owner.frameGeometry(), Qt::Horizontal));
  QTRY_VERIFY(uwf::test::centersAligned(about.frameGeometry(), owner.frameGeometry()));
  QDialog child(&about);
  auto* frame = new uwf::ui::DialogLayout(&child, uwf::ui::DialogContent::Scrollable, QSize(400, 0));
  auto* message = new QLabel(QStringLiteral("A child dialog is centered on the dialog that opened it."), &child);
  message->setWordWrap(true);
  frame->body()->addWidget(message);
  child.show();
  QVERIFY(QTest::qWaitForWindowExposed(&child));
  QTRY_VERIFY(uwf::test::centersAligned(child.frameGeometry(), about.frameGeometry(), Qt::Horizontal));
  QTRY_VERIFY(uwf::test::centersAligned(child.frameGeometry(), about.frameGeometry()));
}

void DialogLayoutTests::contentDialogsMeasureVisibleTextWithoutReservingBlankSpace_data() {
  QTest::addColumn<int>("pointSize");
  QTest::newRow("normal-font") << 9;
  QTest::newRow("large-font") << 12;
  QTest::newRow("larger-font") << 16;
}

void DialogLayoutTests::contentDialogsMeasureVisibleTextWithoutReservingBlankSpace() {
  QFETCH(int, pointSize);
  QDialog dialog;
  auto font = dialog.font();
  font.setPointSize(pointSize);
  dialog.setFont(font);
  auto* frame = new uwf::ui::DialogLayout(&dialog, uwf::ui::DialogContent::Scrollable, QSize(650, 0));
  auto* text = new uwf::ui::ContentHintBox(&dialog);
  const QString message = QStringLiteral("A read-only message must be measured at its final width. ").repeated(8);
  text->setPlainText(message);
  frame->body()->addWidget(text);
  auto* futureDetails = new QPlainTextEdit(&dialog);
  futureDetails->setMaximumHeight(190);
  futureDetails->hide();
  frame->body()->addWidget(futureDetails);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  frame->setActions(buttons);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* scroll = dialog.findChild<QScrollArea*>();
  auto* footer = dialog.findChild<QFrame*>(QStringLiteral("dialogFooter"));
  QVERIFY(scroll && footer);
  const auto verifyFit = [&] {
    QTRY_VERIFY(dialog.minimumSize() != dialog.maximumSize());
    QTRY_VERIFY2(scroll->verticalScrollBar()->maximum() == 0,
                 qUtf8Printable(QStringLiteral("range %1 content %2x%3 minimum %4x%5 text %6x%7 hfw %8 minimum %9x%10")
                                    .arg(scroll->verticalScrollBar()->maximum())
                                    .arg(scroll->widget()->width())
                                    .arg(scroll->widget()->height())
                                    .arg(scroll->widget()->minimumWidth())
                                    .arg(scroll->widget()->minimumHeight())
                                    .arg(text->width())
                                    .arg(text->height())
                                    .arg(text->heightForWidth(text->width()))
                                    .arg(text->minimumWidth())
                                    .arg(text->minimumHeight())));
    QTRY_COMPARE(text->verticalScrollBar()->maximum(), 0);
    const auto clipping = uwf::test::waitForUnclippedText(dialog);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    QTRY_VERIFY(scroll->viewport()->height() - text->geometry().bottom() - 1 <= frame->body()->contentsMargins().bottom());
  };
  verifyFit();
  const int initialHeight = dialog.height();
  dialog.move(dialog.pos() + QPoint(80, 40));
  const QRect movedFrame = dialog.frameGeometry();
  text->setPlainText(text->toPlainText() + QStringLiteral("\nAdditional diagnostic information.\n").repeated(3));
  QTRY_COMPARE(dialog.height(), initialHeight);
  QTRY_VERIFY(scroll->verticalScrollBar()->maximum() > 0);
  QCOMPARE(text->verticalScrollBar()->maximum(), 0);
  QTRY_VERIFY(uwf::test::centersAligned(dialog.frameGeometry(), movedFrame, Qt::Horizontal));
  QTRY_VERIFY(uwf::test::centersAligned(dialog.frameGeometry(), movedFrame));
  text->setPlainText(message);
  QTRY_COMPARE(dialog.height(), initialHeight);
  verifyFit();
  QTRY_VERIFY(uwf::test::centersAligned(dialog.frameGeometry(), movedFrame, Qt::Horizontal));
  QTRY_VERIFY(uwf::test::centersAligned(dialog.frameGeometry(), movedFrame));
  text->setPlainText(QStringLiteral("Long diagnostic content.\n").repeated(200));
  QTRY_VERIFY(dialog.minimumSize() != dialog.maximumSize());
  QTRY_VERIFY(scroll->verticalScrollBar()->maximum() > 0);
  QVERIFY(dialog.screen()->availableGeometry().contains(dialog.frameGeometry()));
  QVERIFY(dialog.rect().contains(QRect(buttons->mapTo(&dialog, QPoint()), buttons->size())));
}

void DialogLayoutTests::powerFailureDetailsExpandUntilTheScreenLimit_data() {
  QTest::addColumn<int>("lines");
  QTest::newRow("one-line") << 1;
  QTest::newRow("twenty-lines") << 20;
  QTest::newRow("beyond-screen") << 200;
}

void DialogLayoutTests::powerFailureDetailsExpandUntilTheScreenLimit() {
  QFETCH(int, lines);
  QString message;
  for (int line = 1; line <= lines; ++line) message += QStringLiteral("File %1: access denied.\n").arg(line);
  bool inspected = false;
  auto outcome = uwf::app::PowerActionDialogOutcome::Canceled;
  QVERIFY(uwf::test::driveModalDialog(
      *this,
      [&] {
        outcome = uwf::ui::runPowerActionDialog(nullptr, {uwf::app::PowerAction::Restart, [message]() -> std::optional<uwf::app::PowerStagingPreparation> {
                                                            return uwf::app::PowerStagingFailure{message};
                                                          }});
      },
      [&](QDialog& active) {
        auto* dialog = &active;
        auto* details = dialog->findChild<uwf::ui::ContentHintBox*>(QStringLiteral("powerStagingFailureDetails"));
        auto* outer = dialog->findChild<QScrollArea*>(QStringLiteral("powerConfirmationScroll"));
        auto* cancel = buttonByRole(dialog, QDialogButtonBox::RejectRole);
        QVERIFY(details && outer && cancel);
        auto* disclosure = dialog->findChild<uwf::ui::Disclosure*>(QStringLiteral("powerFailureDisclosure"));
        QVERIFY(disclosure);
        QTRY_VERIFY_WITH_TIMEOUT(disclosure->isVisibleTo(dialog), 5000);
        QCOMPARE(details->toPlainText(), message);
        const QSize initial = dialog->size();
        disclosure->setOpen(true);
        QTRY_COMPARE(details->verticalScrollBar()->maximum(), 0);
        QTRY_COMPARE(dialog->size(), initial);
        QTRY_COMPARE(outer->verticalScrollBar()->maximum() > 0, outer->widget()->height() > outer->viewport()->height());
        const int expandedRange = outer->verticalScrollBar()->maximum();
        outer->verticalScrollBar()->setValue(expandedRange);
        QTRY_VERIFY(dialog->screen()->availableGeometry().contains(dialog->frameGeometry()));
        QVERIFY(cancel->isDefault());
        QTRY_VERIFY(dialog->rect().contains(QRect(cancel->mapTo(dialog, QPoint()), cancel->size())));
        QVERIFY(uwf::test::saveScreenshot(*dialog, QStringLiteral("power-failure-natural")));
        disclosure->setOpen(false);
        QTRY_COMPARE(dialog->size(), initial);
        if (expandedRange > 0) QTRY_VERIFY(outer->verticalScrollBar()->maximum() < expandedRange);
        inspected = true;
      }));
  QCOMPARE(outcome, uwf::app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
  QVERIFY(inspected);
}

void DialogLayoutTests::readOnlyReportsUseTheScreenBeforeScrolling_data() {
  QTest::addColumn<bool>("plain");
  QTest::addColumn<bool>("rich");
  QTest::addColumn<int>("lines");
  struct Editors {
    const char* name;
    bool plain;
    bool rich;
  };
  for (const auto& editors : {Editors{"plain", true, false}, Editors{"rich", false, true}, Editors{"multiple", true, true}})
    for (const int lines : {20, 200}) {
      const QByteArray name = QStringLiteral("%1-lines-%2").arg(QLatin1StringView(editors.name)).arg(lines).toLatin1();
      QTest::newRow(name.constData()) << editors.plain << editors.rich << lines;
    }
}

void DialogLayoutTests::readOnlyReportsUseTheScreenBeforeScrolling() {
  QFETCH(bool, plain);
  QFETCH(bool, rich);
  QFETCH(int, lines);
  QDialog dialog;
  auto* frame = new uwf::ui::DialogLayout(&dialog, uwf::ui::DialogContent::Scrollable, QSize(650, 300));
  const QString message = QStringLiteral("Read-only diagnostic information.\n").repeated(lines);
  QList<QAbstractScrollArea*> editors;
  if (plain) {
    auto* editor = new QPlainTextEdit(message, &dialog);
    editor->setReadOnly(true);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    frame->body()->addWidget(editor, 1);
    editors.append(editor);
  }
  if (rich) {
    auto* editor = new QTextEdit(&dialog);
    editor->setReadOnly(true);
    editor->setPlainText(message);
    frame->body()->addWidget(editor, 1);
    editors.append(editor);
  }
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  frame->setActions(buttons);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QVERIFY(dialog.height() <= uwf::test::availableClientSize(dialog).height());
  for (const auto* editor : std::as_const(editors)) QTRY_VERIFY(editor->verticalScrollBar()->maximum() > 0);
  QVERIFY(dialog.screen()->availableGeometry().contains(dialog.frameGeometry()));
  QVERIFY(dialog.rect().contains(QRect(buttons->mapTo(&dialog, QPoint()), buttons->size())));
}

}  // namespace

QTEST_MAIN(DialogLayoutTests)

#include "DialogLayoutTests.moc"
