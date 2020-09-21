#include "checkbox.h"

#include <QStyleOption>

#include "xenia/ui/qt/theme_manager.h"

namespace xe {
namespace ui {
namespace qt {

XCheckBox::XCheckBox(QWidget* parent)
    : Themeable<QCheckBox>("XCheckBox", parent) {
  setFocusPolicy(Qt::TabFocus);  // disable retaining focus through mouse click
}

void XCheckBox::paintEvent(QPaintEvent* e) {
  QStyleOptionButton option;
  initStyleOption(&option);

  // create rect for indicator box
  // rect must start at 1 as the painter draws either side of start offset so
  // starting at (0,0) would leave 2 sides cut off
  QRectF indicator_rect = style()->proxy()->subElementRect(
      QStyle::SE_CheckBoxIndicator, &option, this);
  QRectF indicator_box =
      QRectF(indicator_rect.x() + 1.0, indicator_rect.y() + 0.0, 16, 16);

  // get original rect for checkbox label
  QRectF label_rect = style()->proxy()->subElementRect(
      QStyle::SE_CheckBoxContents, &option, this);

  QPainter painter(this);
  painter.setClipping(false);
  painter.setRenderHints(QPainter::Antialiasing);

  QPen pen(border_color_);

  if (hasFocus()) {
    pen.setColor(focus_color_);
  }
  pen.setJoinStyle(Qt::MiterJoin);

  painter.setPen(pen);

  painter.drawRect(indicator_box);

  // paint checked inner box if checkbox is checked
  if (isChecked()) {
    // remove pen
    painter.setPen(Qt::NoPen);

    auto checked_brush = QBrush(checked_color_);
    auto checked_rect =
        QRectF(indicator_box.x() + (indicator_box.width() * 0.125),
               indicator_box.y() + (indicator_box.height() * 0.125),
               indicator_box.width() * 0.75, indicator_box.height() * 0.75);
    painter.setBrush(checked_brush);
    painter.drawRect(checked_rect);

    // reset pen
    painter.setPen(pen);
  }

  // Draw checkbox label text
  auto font = this->font();
  painter.setFont(font);

  QFontMetrics metrics(font);
  QRectF font_rect = metrics.boundingRect(text());

  label_rect.setY(indicator_box.center().y() - (font_rect.height() / 2) - 1);
  label_rect.translate(label_indent_, 0);

  painter.drawText(label_rect, text());
}

QSize XCheckBox::sizeHint() const {
  // Increase sizeHint by indent amount to compensate for slightly larger
  // indicator box and translated label.
  // This is not exact, but to get it exact would require using an algorithm
  // with QFontMetrics.
  return QCheckBox::sizeHint() + QSize(int(label_indent_), 0);
}

}  // namespace qt
}  // namespace ui
}  // namespace xe
