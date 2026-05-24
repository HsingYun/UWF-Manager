#pragma once

#include <QIcon>
#include <QObject>
#include <QSize>
#include <QString>

namespace uwf::ui {

enum class Theme { Dark, Light };

// 自绘控件 / RichText 用的"语义色"。每个枚举对应一种用途，避免在控件里硬编码
// 十六进制色，也让两套主题切换时只在这里维护一次对照表。
enum class Sem {
  Fg,        // 主文字
  FgMuted,   // 次文字（label key、说明文字等）
  Accent,    // 主蓝（链接、active tab、选中文字）
  Warn,      // 琥珀（警告徽标 / 阈值警告色）
  Danger,    // 红（错误状态 / 移除徽标）
  AddOk,     // 绿（添加徽标 / 成功）
  Surface,   // 卡片背景
  Bg,        // 主背景
  Border,    // 边框
  TrackOff,  // SwitchButton 关闭态轨道
  BarBg,     // OverlayUsageBar 底色
  BarGrid,   // OverlayUsageBar 网格线
  Thumb,     // SwitchButton 滑块色
};

// 全局主题管理器：探测系统主题、加载对应 QSS、为 SVG 图标做按主题染色，
// 并向自绘控件 / RichText 提供语义色。单例（按需懒加载）。
// Qt-only 类型，仅供 src/ui 使用。
class ThemeManager : public QObject {
  Q_OBJECT
 public:
  static ThemeManager& instance();

  [[nodiscard]] Theme current() const { return m_theme; }
  [[nodiscard]] bool isLight() const { return m_theme == Theme::Light; }

  // 加载并应用对应 QSS 到 qApp，发射 themeChanged 信号。
  void apply(Theme t);
  void toggle();

  // 读取 Windows 注册表 AppsUseLightTheme；非 Windows 或读失败时返回 Dark。
  static Theme detectSystemTheme();

  // 把 svg 资源里的 dark 前景 #E8EAED 替换成当前主题的前景色，包装成 QIcon。
  // 真实渲染尺寸由 caller 决定（toolbar iconSize / pixmap(...)），ThemedSvgIconEngine
  // 每次按目标尺寸矢量绘制——HiDPI 下零位图缩放损失。
  [[nodiscard]] QIcon icon(const QString& resourcePath) const;

  // 用指定的颜色染色 svg，不跟随主题。适合用在带强烈背景色的 fill 按钮
  // （primary / danger）上：背景永远是品牌色，icon 颜色也得固定才好看。
  static QIcon iconWithColor(const QString& resourcePath, const QColor& fg);

  // 取语义色。HTML/RichText 中用 .name()（"#RRGGBB"）即可。
  [[nodiscard]] QColor color(Sem s) const;

 signals:
  // 信号参数必须全限定——MOC 按字面解析 header，跨 TU 的 queued connection
  // 会按字符串匹配类型名，写成 `Theme` 而非 `uwf::ui::Theme` 时匹配会失败。
  void themeChanged(uwf::ui::Theme t);

 private:
  ThemeManager();
  Theme m_theme = Theme::Dark;
};

}  // namespace uwf::ui
