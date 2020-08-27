#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_COMBOBOX_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_COMBOBOX_H_

#include "settings_widget.h"
#include "xenia/ui/qt/widgets/combobox.h"

namespace xe {
namespace ui {
namespace qt {

template <typename T>
class SettingsComboBox : SettingsWidget<T, XComboBox> {
  static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, int>,
                "Settings TextEdit must use std::string or int");
};

}  // namespace qt
}  // namespace ui
}  // namespace xe