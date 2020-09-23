#include "radio_button.h"

#include <QStyleOption>

#include "xenia/ui/qt/theme_manager.h"

namespace xe {
namespace ui {
namespace qt {

XRadioButton::XRadioButton(QWidget* parent)
  : Themeable<QRadioButton>("XRadioButton", parent) {
  setFocusPolicy(Qt::TabFocus); // disable retaining focus through mouse click
}

void XRadioButton::paintEvent(QPaintEvent* e) {
  QStyleOptionButton option;
  initStyleOption(&option);

  // create rect for indicator box
  // rect must start at 1 as the painter draws either side of start offset so
  // starting at (0,0) would leave 2 sides cut off
  QRectF indicator_rect = style()->proxy()->subElementRect(
      QStyle::SE_RadioButtonIndicator, &option,
      this);
  QRectF indicator_box =
      QRectF(indicator_rect.x() + 1.0, indicator_rect.y() + 0.0, 16, 16);

  // get original rect for radio button label
  QRect label_rect = style()->proxy()->subElementRect(
      QStyle::SE_RadioButtonContents, &option,
      this);

  QPainter painter(this);
  painter.setRenderHints(QPainter::Antialiasing);

  QPen pen(border_color_);

  if (hasFocus()) {
    pen.setColor(focus_color_);
  }

  pen.setJoinStyle(Qt::MiterJoin);
  painter.setPen(pen);

  painter.drawEllipse(indicator_box);

  // paint checked inner box if radio button is checked
  if (isChecked()) {
    painter.setPen(Qt::NoPen);
    QBrush checked_brush = QBrush(checked_color_);
    QRectF checked_rect =
        QRectF(indicator_box.x() + (indicator_box.width() * 0.125),
               indicator_box.y() + (indicator_box.height() * 0.125),
               indicator_box.width() * 0.75, indicator_box.height() * 0.75);

    painter.setBrush(checked_brush);
    painter.drawEllipse(checked_rect);

    painter.setPen(pen);
  }

  QFont font = this->font();
  painter.setFont(font);

  QFontMetrics metrics(font);
  QRect font_rect = metrics.boundingRect(text());

  // TODO(Razzile): I can't seem to work out why this -1 is needed. I think the
  // Segoe UI font file misreports height of font
  label_rect.setY(indicator_box.center().y() - (font_rect.height() / 2) - 1);

  label_rect.translate(label_indent_, 0);

  painter.drawText(label_rect, text());
}

QSize XRadioButton::sizeHint() const {
  // Increase sizeHint by indent amount to compensate for slightly larget
  // indicator box and translated label.
  // This is not exact, but to get it exact would require using an algorithm
  // with QFontMetrics.
  return QRadioButton::sizeHint() + QSize(label_indent_, 0);
}

} // namespace qt
} // namespace ui
} // namespace xe
