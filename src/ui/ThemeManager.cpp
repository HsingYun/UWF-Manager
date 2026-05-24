#include "ThemeManager.h"

#include <windows.h>

#include <QApplication>
#include <QByteArray>
#include <QFile>
#include <QIconEngine>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QStyleHints>
#include <QSvgRenderer>

#include "../util/RegistryKey.h"

namespace uwf::ui {

namespace {

// 直接吃染色后的 svg 字节流，每次 paint() / pixmap() 用 QSvgRenderer 按目标
// 尺寸矢量渲染——HiDPI 下零位图缩放损失，比预先栅格化成 QPixmap 再交给
// QIcon 干净。Qt 在 HiDPI 路径上会调 pixmap(size×dpr) 取物理像素，svg 渲染
// 直接吃这个尺寸出锐利图。
class ThemedSvgIconEngine : public QIconEngine {
 public:
  explicit ThemedSvgIconEngine(QByteArray svgBytes) : m_bytes(std::move(svgBytes)) {}

  void paint(QPainter* painter, const QRect& rect, QIcon::Mode, QIcon::State) override {
    QSvgRenderer r(m_bytes);
    r.render(painter, rect);
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

  [[nodiscard]] QIconEngine* clone() const override { return new ThemedSvgIconEngine(m_bytes); }

  [[nodiscard]] QString key() const override { return QStringLiteral("themed-svg"); }

 private:
  QByteArray m_bytes;
};

}  // namespace

ThemeManager& ThemeManager::instance() {
  static ThemeManager mgr;
  return mgr;
}

ThemeManager::ThemeManager() = default;

void ThemeManager::apply(Theme t) {
  m_theme = t;

  // ── 强制 Qt native style 的 color scheme ───────────
  // Windows 11 上 Qt 默认用 QWindows11Style，它会自己读系统主题决定渲染（一些
  // 子 widget 比如 QStackedWidget 不走 QSS / QPalette）。结果"系统暗 + app 切到
  // 亮"的组合下，style 内部还按 dark 画，露出和我们 QSS / palette 不一致的暗
  // 底（用户报告的"白模式下背景消失"）。
  // Qt 6.8+ 加的 QStyleHints::setColorScheme 是官方解法：直接告诉 native style
  // "用这个色调"，不再去问系统。本程序 Qt 6.11，可以用。
  QGuiApplication::styleHints()->setColorScheme(t == Theme::Light ? Qt::ColorScheme::Light : Qt::ColorScheme::Dark);

  // ── QPalette 同步 ──────────────────────────────────
  // Qt 内部一部分 widget 渲染（QStackedWidget 的页面底色 / QListView viewport
  // 的 base 色 / QTabWidget 默认 pane / 原生 QPushButton 默认色等）走 QPalette
  // 而不是 QSS。光设 stylesheet 不动 palette，palette 就一直停在"系统默认值"
  // ——系统亮主题下是亮色 palette，系统暗主题下是暗色 palette。
  //
  // 如果用户系统是亮色但在我们 app 里切到暗主题，我们的 QSS 把外圈染成了暗
  // 色，但 palette 仍是亮色，于是没被 QSS 覆盖到的内层 widget 就露出亮色底
  // ——这就是"切到夜间模式 tab 还是白色"的根因。反过来同理。
  //
  // 用 ThemeManager 的语义色重写关键 palette role，强制覆盖系统默认。
  {
    QPalette p = qApp->palette();
    const QColor bg = color(Sem::Bg);
    const QColor surface = color(Sem::Surface);
    const QColor fg = color(Sem::Fg);
    const QColor muted = color(Sem::FgMuted);
    const QColor accent = color(Sem::Accent);
    const QColor border = color(Sem::Border);

    // Window = 主背景；Base = 输入类 widget 底色（QLineEdit / QListView
    // viewport），按惯例略亮一档对应到 Surface。
    p.setColor(QPalette::Window, bg);
    p.setColor(QPalette::Base, surface);
    p.setColor(QPalette::AlternateBase, surface);
    p.setColor(QPalette::WindowText, fg);
    p.setColor(QPalette::Text, fg);
    p.setColor(QPalette::Button, surface);
    p.setColor(QPalette::ButtonText, fg);
    p.setColor(QPalette::BrightText, fg);
    p.setColor(QPalette::ToolTipBase, surface);
    p.setColor(QPalette::ToolTipText, fg);
    p.setColor(QPalette::PlaceholderText, muted);
    p.setColor(QPalette::Highlight, accent);
    p.setColor(QPalette::HighlightedText, QColor(0xFFFFFF));
    p.setColor(QPalette::Light, surface);
    p.setColor(QPalette::Midlight, surface);
    p.setColor(QPalette::Mid, border);
    p.setColor(QPalette::Dark, border);
    p.setColor(QPalette::Shadow, border);
    // disabled 组：颜色稍暗一档，让控件不可用时视觉上能区分。
    p.setColor(QPalette::Disabled, QPalette::WindowText, muted);
    p.setColor(QPalette::Disabled, QPalette::Text, muted);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, muted);
    qApp->setPalette(p);
  }

  // ── QSS 加载 ──────────────────────────────────────
  const QString path = (t == Theme::Light) ? ":/style_light.qss" : ":/style_dark.qss";
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    emit themeChanged(t);
    return;
  }
  const QString sheet = QString::fromUtf8(f.readAll());
  // 先清空再设：避免 Qt 把"两套 QSS 解析后的 sizeHint"做差量合并，
  // 导致 toolbar 等容器高度在切换时一次性叠高（再切回也回不来）。
  // 经过空字符串 polish 后，所有 widget 的 cached layout 状态被重置，
  // 再加载新 QSS 重新计算就能得到稳定的几何尺寸。
  qApp->setStyleSheet(QString());
  qApp->setStyleSheet(sheet);
  emit themeChanged(t);
}

void ThemeManager::toggle() { apply(m_theme == Theme::Dark ? Theme::Light : Theme::Dark); }

Theme ThemeManager::detectSystemTheme() {
  // AppsUseLightTheme：0 = 深色应用主题，非 0 / 缺失 = 浅色。读不到该值
  // （注册表项不存在等）时 readDword 返回 0——与"未配置时回退深色"一致。
  return regkey::readDword(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", "AppsUseLightTheme") == 0 ? Theme::Dark
                                                                                                                                          : Theme::Light;
}

QColor ThemeManager::color(Sem s) const {
  // 颜色对照表 ——（dark, light）。HTML/RichText 用时取 .name()。
  // 用值结构而不是 lambda：调用频繁，避免每次构造 vector / map。
  switch (s) {
    case Sem::Fg:
      return isLight() ? QColor(0x1F1F1F) : QColor(0xE8EAED);
    case Sem::FgMuted:
      return isLight() ? QColor(0x5F6368) : QColor(0x9AA0A6);
    case Sem::Accent:
      return isLight() ? QColor(0x1A73E8) : QColor(0x4C8BF5);
    case Sem::Warn:
      return isLight() ? QColor(0xF29900) : QColor(0xE4A465);
    case Sem::Danger:
      return isLight() ? QColor(0xD93025) : QColor(0xC94D58);
    case Sem::AddOk:
      return isLight() ? QColor(0x137333) : QColor(0x28A745);
    case Sem::Surface:
      return isLight() ? QColor(0xFFFFFF) : QColor(0x22252A);
    case Sem::Bg:
      return isLight() ? QColor(0xF8F9FA) : QColor(0x1B1D20);
    case Sem::Border:
      return isLight() ? QColor(0xDADCE0) : QColor(0x2E323A);
    case Sem::TrackOff:
      return isLight() ? QColor(0xDADCE0) : QColor(0x4A505C);
    case Sem::BarBg:
      return isLight() ? QColor(0xE8EAED) : QColor(0x2E323A);
    case Sem::BarGrid:
      return isLight() ? QColor::fromRgba(0x1C000000) : QColor::fromRgba(0x24FFFFFF);
    case Sem::Thumb:
      return isLight() ? QColor(0xFFFFFF) : QColor(0xFFFFFF);
  }
  return {0xFF00FF};  // 不可达，露出来更好定位漏改的分支。
}

QIcon ThemeManager::icon(const QString& resourcePath) const {
  // 现有 svg 都把前景色硬编码成 #E8EAED（dark 默认前景）；按当前主题
  // 替换为对应的前景色。
  const QColor fg = (m_theme == Theme::Light) ? QColor(0x1F1F1F) : QColor(0xE8EAED);
  return iconWithColor(resourcePath, fg);
}

QIcon ThemeManager::iconWithColor(const QString& resourcePath, const QColor& fg) {
  QFile f(resourcePath);
  if (!f.open(QIODevice::ReadOnly)) return {};
  QString text = QString::fromUtf8(f.readAll());
  text.replace("#E8EAED", fg.name(QColor::HexRgb), Qt::CaseInsensitive);

  // 把染色后的 svg 字节流交给自定义 IconEngine，每次绘制时按真实目标尺寸
  // 矢量渲染，HiDPI 下零位图缩放损失。
  return QIcon(new ThemedSvgIconEngine(text.toUtf8()));
}

}  // namespace uwf::ui
