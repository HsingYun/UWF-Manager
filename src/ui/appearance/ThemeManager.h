// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractNativeEventFilter>
#include <QIcon>
#include <QObject>
#include <QSize>
#include <QString>
#include <optional>

#include "app/settings/UiPreferences.h"

namespace uwf::ui {

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

// 全局外观管理：初始化共享字体/样式、探测系统主题、加载对应 QSS、为 SVG 图标做按主题染色，
// 并向自绘控件 / RichText 提供语义色。单例（按需懒加载）。
// Qt-only 类型，在 GUI 线程使用。
class ThemeManager : public QObject, private QAbstractNativeEventFilter {
  Q_OBJECT
 public:
  static ThemeManager& instance();

  [[nodiscard]] app::Theme current() const { return m_theme; }
  [[nodiscard]] bool isLight() const { return m_theme == app::Theme::Light; }
  [[nodiscard]] app::Theme systemTheme() const { return m_systemTheme; }

  // 加载并应用对应 QSS 到 qApp，发射 themeChanged 信号。
  void apply(app::Theme t);
  // 启动偏好仅在初始化时使用；未指定时读取系统应用主题。后续手动切换不
  // 改写启动偏好，也不影响独立跟随 Windows 外壳的任务栏主题。
  void applyStartupTheme(std::optional<app::Theme> overrideTheme);
  void toggle();
  void requestToggle();

  // 读取 Windows 注册表 AppsUseLightTheme；非 Windows 或读失败时返回 Dark。
  static app::Theme detectSystemTheme();
  // Windows 外壳（任务栏、开始菜单）主题使用独立的 SystemUsesLightTheme。
  static app::Theme detectSystemShellTheme();

  // 替换 SVG 的约定前景色，并按最终尺寸矢量渲染。
  // 非正方形图标槽可配置水平对齐；额外宽度用于图文间距。
  [[nodiscard]] QIcon icon(const QString& resourcePath, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter) const;

  // 固定染色用于彩色填充按钮，保持各主题下的图标对比度。
  static QIcon iconWithColor(const QString& resourcePath, const QColor& fg, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter);

  // 取语义色。HTML/RichText 中用 .name()（"#RRGGBB"）即可。
  [[nodiscard]] QColor color(Sem s) const;
  [[nodiscard]] QString styleSheet(const QString& resourcePath) const;

 signals:
  // 信号参数使用枚举定义处的完全限定名，保证 MOC 与排队连接识别同一类型。
  void themeChanged(uwf::app::Theme t);
  void systemThemeChanged(uwf::app::Theme t);

 private:
  ThemeManager();
  bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
  void refreshSystemTheme();

  app::Theme m_theme = app::Theme::Dark;
  app::Theme m_systemTheme = app::Theme::Dark;
  std::optional<app::Theme> m_requestedTheme;
  bool m_applied = false;
};

}  // namespace uwf::ui
