#include "OverlayUsageBar.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include "ThemeManager.h"

namespace uwf::ui {

namespace {
constexpr int kBarH = 20;
}  // namespace

OverlayUsageBar::OverlayUsageBar(QWidget* parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  // 主题切换时需要重绘，避免上一主题的色块残留。
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { update(); });
}

void OverlayUsageBar::setData(const uint32_t currentMb, const uint32_t warningMb, const uint32_t criticalMb, const uint32_t maximumMb, const uint32_t scaleMb) {
  m_current = currentMb;
  m_warning = warningMb;
  m_critical = criticalMb;
  m_max = maximumMb;
  m_scale = scaleMb;
  update();
}

QSize OverlayUsageBar::sizeHint() const { return {240, kBarH}; }
QSize OverlayUsageBar::minimumSizeHint() const { return {160, kBarH}; }

void OverlayUsageBar::paintEvent(QPaintEvent*) {
  QPainter p(this);
  // 抗锯齿 + SmoothPixmapTransform：让所有路径 / 子像素坐标的几何能走 AA。
  // 整数坐标的 fillRect 即便开 AA 也是硬边（rasterizer 没插值空间），所以下面
  // 把所有几何都换成 QRectF / QPointF / qreal，让色块右边缘和斜纹端点能落在
  // 子像素位置上、AA 才有发挥余地。
  p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  const auto& tm = ThemeManager::instance();
  const QColor cUsed = tm.color(Sem::Accent);
  const QColor cWarn = tm.color(Sem::Warn);
  const QColor cCrit = tm.color(Sem::Danger);
  const QColor cTotal = tm.color(Sem::BarBg);

  const QRectF bar(0, 0, width(), kBarH);
  QPainterPath barPath;
  barPath.addRoundedRect(bar, kBarH / 2.0, kBarH / 2.0);

  // 1) 底色 = 100%（RAM 类型为总内存；Disk 类型为 overlay 最大容量）
  p.fillPath(barPath, cTotal);

  const uint32_t scale = std::max(m_scale > 0 ? m_scale : m_max, 1u);
  // 返回 qreal——保留分数像素让色块右边缘走 AA 平滑过渡，而不是 round 到整数
  // 像素后出现锯齿边。
  auto widthFor = [&](uint32_t v) -> qreal {
    const double ratio = static_cast<double>(std::min(v, scale)) / scale;
    return ratio * bar.width();
  };

  p.setClipPath(barPath);

  // 2) RAM 模式下，先给整条横条铺满交叉斜纹；后面的 [0..max] 会用底色盖一次
  //    把左侧网格清掉，再用 critical/warning/used 着色；这样只剩右边
  //    [max..scale] 的"不可能被占用"区域保留网格。
  //    触发条件：scale > max（底色代表总内存，远大于 overlay 最大容量）。
  if (m_scale > 0 && m_max > 0 && m_scale > m_max) {
    QPen pen(tm.color(Sem::BarGrid));
    pen.setWidthF(1.0);
    pen.setCapStyle(Qt::FlatCap);
    p.setPen(pen);
    constexpr qreal kStep = 6.0;
    const qreal h = bar.height();
    const qreal minX = bar.left() - h;
    const qreal maxX = bar.right() + h;
    for (qreal x = minX; x < maxX; x += kStep) {
      // 正向 "/"
      p.drawLine(QPointF(x, bar.bottom()), QPointF(x + h, bar.top()));
      // 反向 "\"
      p.drawLine(QPointF(x, bar.top()), QPointF(x + h, bar.bottom()));
    }
    p.setPen(Qt::NoPen);
  }

  // 3) [0..max] 区段：先用底色覆盖网格线，让左半段回到"干净"的底色，
  //    后续 critical/warning/used 依次覆盖即可。
  if (m_max > 0) {
    p.fillRect(QRectF(bar.left(), bar.top(), widthFor(m_max), bar.height()), cTotal);
  }
  // 4) 严重从左填充（红）
  if (m_critical > 0) {
    p.fillRect(QRectF(bar.left(), bar.top(), widthFor(m_critical), bar.height()), cCrit);
  }
  // 5) 警告从左填充（橙），覆盖住 [0..warning] 那段严重
  if (m_warning > 0) {
    p.fillRect(QRectF(bar.left(), bar.top(), widthFor(m_warning), bar.height()), cWarn);
  }
  // 6) 已占用从左填充（蓝）
  if (m_current > 0) {
    p.fillRect(QRectF(bar.left(), bar.top(), widthFor(m_current), bar.height()), cUsed);
  }
}

}  // namespace uwf::ui
