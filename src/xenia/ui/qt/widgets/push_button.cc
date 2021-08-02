#include "push_button.h"

namespace xe {
namespace ui {
namespace qt {

XPushButton::XPushButton(QWidget* parent)
    : Themeable<QPushButton>("XPushButton", parent) {
  Build();
}

XPushButton::XPushButton(const QString& text, QWidget* parent)
    : Themeable<QPushButton>("XPushButton", text, parent) {
  Build();
}

XPushButton::XPushButton(const QIcon& icon, const QString& text,
                         QWidget* parent)
    : Themeable<QPushButton>("XPushButton", icon, text, parent) {
  Build();
}

void XPushButton::SetIconFromGlyph(QChar glyph, QColor color, int size) {
  auto glyph_font = QFont("Segoe MDL2 Assets", size);
  // Measure the Glyph
  QFontMetrics measure(glyph_font);
  QRect icon_rect = measure.boundingRect(glyph);
  double max = qMax(icon_rect.width(), icon_rect.height());

  // Create the Pixmap
  // boundingRect can be inaccurate so add a 4px padding to be safe
  QPixmap pixmap(max + 4, max + 4);
  pixmap.fill(Qt::transparent);

  // Paint the Glyph
  QPainter painter(&pixmap);
  painter.setFont(glyph_font);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  painter.setPen(QPen(color));

  painter.drawText(pixmap.rect(), Qt::AlignVCenter, glyph);

  painter.end();

  this->setIcon(pixmap);
}

void XPushButton::Build() {
  setFlat(true);
  setCursor(Qt::PointingHandCursor);
  setFocusPolicy(Qt::TabFocus);
}

}  // namespace qt
}  // namespace ui
}  // namespace xe
