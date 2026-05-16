#include "TableText.h"

#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>

namespace uwf::ui {

QString tableSelectionToText(const QTableWidget* t) {
  const auto ranges = t->selectedRanges();
  if (ranges.isEmpty()) return {};
  QString txt;
  for (const auto& range : ranges) {
    for (int r = range.topRow(); r <= range.bottomRow(); ++r) {
      QStringList cells;
      for (int c = range.leftColumn(); c <= range.rightColumn(); ++c) {
        const auto* it = t->item(r, c);
        cells << (it ? it->text() : QString());
      }
      txt += cells.join('\t') + '\n';
    }
  }
  return txt;
}

QString tableAllToText(const QTableWidget* t) {
  QString txt;
  QStringList header;
  for (int c = 0; c < t->columnCount(); ++c) {
    // horizontalHeaderItem 对未设过 header 的列返回 nullptr；调用方虽然都
    // 设了表头，这里加保险防止以后扩展时悄悄 crash。
    const auto* it = t->horizontalHeaderItem(c);
    header << (it ? it->text() : QString());
  }
  txt += header.join('\t') + '\n';
  for (int r = 0; r < t->rowCount(); ++r) {
    QStringList cells;
    for (int c = 0; c < t->columnCount(); ++c) {
      const auto* it = t->item(r, c);
      cells << (it ? it->text() : QString());
    }
    txt += cells.join('\t') + '\n';
  }
  return txt;
}

}  // namespace uwf::ui
