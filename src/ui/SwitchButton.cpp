#include "SwitchButton.h"

#include <QPainter>
#include <QPainterPath>

#include "ThemeManager.h"

namespace uwf::ui {

namespace {
constexpr int kTrackH = 20;
constexpr int kTrackW = 42;
constexpr int kThumbMargin = 3;
// 左右各留 2px 呼吸位，避免相邻 "→" 箭头标签或布局切得太紧
// 把开关轨道的抗锯齿左边缘吃掉。
constexpr int kSidePad = 2;

// 把颜色按比例 t∈[0,1] 朝中灰 (128,128,128) 混合，得到"褪色"版本——
// 既降饱和又压对比，适合禁用态。t=0 原样返回；t=1 直接是中灰。
QColor fadeToGray(const QColor& c, const double t) {
  return {static_cast<int>(c.red() * (1 - t) + 128 * t), static_cast<int>(c.green() * (1 - t) + 128 * t), static_cast<int>(c.blue() * (1 - t) + 128 * t)};
}
}  // namespace

SwitchButton::SwitchButton(QWidget* parent) : QAbstractButton(parent) {
  setCheckable(true);
  setCursor(Qt::PointingHandCursor);
  setFocusPolicy(Qt::StrongFocus);
  setFixedSize(kTrackW + kSidePad * 2, kTrackH + 4);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { update(); });
}

QSize SwitchButton::sizeHint() const { return {kTrackW + kSidePad * 2, kTrackH + 4}; }

void SwitchButton::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  const bool on = isChecked();
  const bool dirty = property("dirty").toBool();
  const bool dis = !isEnabled();

  const auto& tm = ThemeManager::instance();
  const QColor accent = tm.color(Sem::Accent);
  const QColor offTrack = tm.color(Sem::TrackOff);
  const QColor trackOn = dis ? fadeToGray(accent, 0.45) : accent;
  const QColor trackOff = dis ? fadeToGray(offTrack, 0.35) : offTrack;
  // 关闭态 thumb 要与 track 反色对比：浅色主题下 track 是浅灰，thumb 必须
  // 用深灰才看得见；深色主题下 track 是暗灰，thumb 仍用白。开启态 track
  // 是品牌蓝，两种主题下 thumb 都用白配蓝最经典。
  const QColor onThumb = QColor(0xFFFFFF);
  const QColor offThumb = tm.isLight() ? QColor(0x5F6368) : QColor(0xFFFFFF);
  const QColor thumbBase = on ? onThumb : offThumb;
  const QColor thumb = dis ? fadeToGray(thumbBase, 0.25) : thumbBase;

  const int cy = (height() - kTrackH) / 2;
  QRect track(kSidePad, cy, kTrackW, kTrackH);

  QPainterPath path;
  path.addRoundedRect(track, kTrackH / 2.0, kTrackH / 2.0);
  p.fillPath(path, on ? trackOn : trackOff);
  if (dirty) {
    p.setPen(QPen(tm.color(Sem::Warn), 1.5));
    p.drawPath(path);
  }

  constexpr int thumbD = kTrackH - kThumbMargin * 2;
  const int thumbX = on ? track.right() - kThumbMargin - thumbD + 1 : track.left() + kThumbMargin;
  const int thumbY = cy + kThumbMargin;
  p.setPen(Qt::NoPen);
  p.setBrush(thumb);
  p.drawEllipse(QRect(thumbX, thumbY, thumbD, thumbD));
}

}  // namespace uwf::ui
