// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ThemeManager.h"

#include <windows.h>

#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QByteArray>
#include <QElapsedTimer>
#include <QEvent>
#include <QFile>
#include <QFontDatabase>
#include <QHash>
#include <QIconEngine>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPointer>
#include <QStyleHints>
#include <QSvgRenderer>
#include <QWidget>
#include <exception>
#include <string_view>
#include <utility>

#include "AppearanceTransition.h"
#include "app/ports/RegistryKey.h"
#include "shared/Log.h"
#include "ui/design/DesignStyle.h"
#include "ui/design/DesignSystem.h"

namespace uwf::ui {

namespace {

app::Theme readSystemTheme(std::string_view valueName) noexcept {
  try {
    return regkey::readDword(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", valueName).value_or(0) == 0
               ? app::Theme::Dark
               : app::Theme::Light;
  } catch (const std::exception& error) {
    UWF_LOG_W("theme") << "system theme unavailable: fallback=dark error=" << error.what();
  } catch (...) {
    UWF_LOG_W("theme") << "system theme unavailable: fallback=dark error=non-standard-exception";
  }
  return app::Theme::Dark;
}

// 按目标物理尺寸渲染 SVG，避免预先栅格化后缩放造成模糊。
class ThemedSvgIconEngine : public QIconEngine {
 public:
  explicit ThemedSvgIconEngine(QByteArray svgBytes, Qt::Alignment align) : m_bytes(std::move(svgBytes)), m_renderer(m_bytes), m_align(align) {}

  void paint(QPainter* painter, const QRect& rect, const QIcon::Mode mode, QIcon::State) override {
    painter->save();
    if (mode == QIcon::Disabled) painter->setOpacity(painter->opacity() * 0.38);
    // 字形按短边保持正方形；额外宽度作为透明留白，按 m_align 对齐。
    const int side = qMin(rect.width(), rect.height());
    int x = rect.x();
    if (m_align & Qt::AlignHCenter)
      x = rect.x() + (rect.width() - side) / 2;
    else if (m_align & Qt::AlignRight)
      x = rect.right() - side + 1;
    const int y = rect.y() + (rect.height() - side) / 2;
    m_renderer.render(painter, QRectF(x, y, side, side));
    painter->restore();
  }

  QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override {
    QPixmap pm(size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    paint(&p, QRect(QPoint(0, 0), size), mode, state);
    return pm;
  }

  [[nodiscard]] QIconEngine* clone() const override { return new ThemedSvgIconEngine(m_bytes, m_align); }

  [[nodiscard]] QString key() const override { return QStringLiteral("themed-svg"); }

 private:
  QByteArray m_bytes;
  QSvgRenderer m_renderer;
  Qt::Alignment m_align;
};

}  // namespace

ThemeManager& ThemeManager::instance() {
  static ThemeManager mgr;
  return mgr;
}

ThemeManager::ThemeManager() : m_systemTheme(detectSystemShellTheme()) {
  // Structural QSS must not freeze inherited widget palettes. Let Qt propagate
  // the application palette while retaining explicit dialog stylesheet colors.
  QCoreApplication::setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);
  QApplication::setStyle(new DesignStyle());
  QApplication::setFont(DesignSystem::defaultFont());
  if (auto* dispatcher = QAbstractEventDispatcher::instance()) dispatcher->installNativeEventFilter(this);
}

void ThemeManager::apply(app::Theme t) {
  if (m_applied && m_theme == t) return;
  AppearanceTransition::run(
      [&] {
        m_theme = t;
        m_applied = true;
        QElapsedTimer elapsed;
        elapsed.start();

        // 同步原生样式的颜色方案，避免系统主题与应用主题不一致。
        QGuiApplication::styleHints()->setColorScheme(t == app::Theme::Light ? Qt::ColorScheme::Light : Qt::ColorScheme::Dark);
        const auto schemeMs = elapsed.elapsed();

        auto& designSystem = DesignSystem::instance();
        designSystem.setLight(t == app::Theme::Light);
        qApp->setPalette(designSystem.palette());
        const auto paletteMs = elapsed.elapsed();
        const auto style = styleSheet(QStringLiteral(":/styles/surfaces.qss"));
        if (qApp->styleSheet() != style) qApp->setStyleSheet(style);
        const auto styleMs = elapsed.elapsed();
        // Qt queues palette changes to QWindow and resets frameless HWNDs to light.
        // Deliver them before consumers restore the application's native appearance.
        QCoreApplication::sendPostedEvents(nullptr, QEvent::ApplicationPaletteChange);
        const auto deliveryMs = elapsed.elapsed();
        emit themeChanged(t);
        UWF_LOG_D("appearance") << "theme updated: schemeMs=" << schemeMs << " paletteMs=" << paletteMs - schemeMs << " styleMs=" << styleMs - paletteMs
                                << " deliveryMs=" << deliveryMs - styleMs << " consumersMs=" << elapsed.elapsed() - deliveryMs;
      },
      false);
}

void ThemeManager::applyStartupTheme(const std::optional<app::Theme> overrideTheme) { apply(overrideTheme ? *overrideTheme : detectSystemTheme()); }

void ThemeManager::toggle() { apply(m_theme == app::Theme::Dark ? app::Theme::Light : app::Theme::Dark); }

void ThemeManager::requestToggle() {
  m_requestedTheme = m_requestedTheme.value_or(m_theme) == app::Theme::Dark ? app::Theme::Light : app::Theme::Dark;
  AppearanceTransition::request(
      this,
      [this] {
        const auto theme = std::exchange(m_requestedTheme, std::nullopt);
        if (theme) apply(*theme);
      },
      [this] { return m_requestedTheme && *m_requestedTheme != m_theme; });
}

app::Theme ThemeManager::detectSystemTheme() {
  // AppsUseLightTheme：0 = 深色应用主题，非 0 = 浅色。未配置时回退深色。
  return readSystemTheme("AppsUseLightTheme");
}

app::Theme ThemeManager::detectSystemShellTheme() { return readSystemTheme("SystemUsesLightTheme"); }

bool ThemeManager::nativeEventFilter(const QByteArray& eventType, void* message, qintptr*) {
  if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") return false;

  const auto* msg = static_cast<const MSG*>(message);
  if (msg && (msg->message == WM_SETTINGCHANGE || msg->message == WM_THEMECHANGED)) refreshSystemTheme();
  return false;
}

void ThemeManager::refreshSystemTheme() {
  const app::Theme next = detectSystemShellTheme();
  if (next == m_systemTheme) return;
  m_systemTheme = next;
  emit systemThemeChanged(next);
}

QColor ThemeManager::color(Sem s) const {
  using enum design::Color;
  design::Color role = Text;
  switch (s) {
    case Sem::Fg:
      role = Text;
      break;
    case Sem::FgMuted:
      role = Muted;
      break;
    case Sem::Accent:
      role = Accent;
      break;
    case Sem::Warn:
      role = Warning;
      break;
    case Sem::Danger:
      role = Danger;
      break;
    case Sem::AddOk:
      role = Success;
      break;
    case Sem::Surface:
      role = Surface;
      break;
    case Sem::Bg:
      role = Window;
      break;
    case Sem::Border:
      role = Line;
      break;
    case Sem::TrackOff:
      role = Faint;
      break;
    case Sem::BarBg:
      role = Line;
      break;
    case Sem::BarGrid:
      role = LineSoft;
      break;
    case Sem::Thumb:
      role = AccentText;
      break;
  }
  return DesignSystem::instance().color(role);
}

QIcon ThemeManager::icon(const QString& resourcePath, Qt::Alignment align) const {
  // 资源前景使用 #E8EAED，渲染前替换为目标语义色。
  const QColor fg = color(Sem::Fg);
  return iconWithColor(resourcePath, fg, align);
}

QString ThemeManager::styleSheet(const QString& resourcePath) const {
  static QHash<QString, QString> styles;
  const auto& design = DesignSystem::instance();
  const auto key = resourcePath + u'|' + QString::number(design.isLight());
  if (const auto cached = styles.constFind(key); cached != styles.cend()) return *cached;
  QFile file(resourcePath);
  if (!file.open(QIODevice::ReadOnly)) return {};
  const auto resolved = design.resolveStyle(QString::fromUtf8(file.readAll()));
  styles.insert(key, resolved);
  return resolved;
}

QIcon ThemeManager::iconWithColor(const QString& resourcePath, const QColor& fg, Qt::Alignment align) {
  static QHash<QString, QByteArray> sources;
  static QHash<QString, QIcon> icons;
  const auto key = resourcePath + u'|' + fg.name(QColor::HexArgb) + u'|' + QString::number(align.toInt());
  if (const auto cached = icons.constFind(key); cached != icons.cend()) return *cached;
  if (!sources.contains(resourcePath)) {
    QFile f(resourcePath);
    if (!f.open(QIODevice::ReadOnly)) return {};
    sources.insert(resourcePath, f.readAll());
  }
  QString text = QString::fromUtf8(sources.value(resourcePath));
  text.replace("#E8EAED", fg.name(QColor::HexRgb), Qt::CaseInsensitive);

  const QIcon icon(new ThemedSvgIconEngine(text.toUtf8(), align));
  if (icons.size() >= 512) icons.clear();
  icons.insert(key, icon);
  return icon;
}

}  // namespace uwf::ui

#include "moc_ThemeManager.cpp"
