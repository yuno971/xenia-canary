#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_RADIOBOX_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_RADIOBOX_H_

#include "settings_widget.h"
#include "xenia/ui/qt/widgets/radiobox.h"

namespace xe {
namespace ui {
namespace qt {

class SettingsRadioBox : SettingsWidget<int, XRadioBox> {};

}  // namespace qt
}  // namespace ui
}  // namespace xe