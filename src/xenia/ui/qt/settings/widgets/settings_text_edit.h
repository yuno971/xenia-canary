#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_TEXT_EDIT_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_TEXT_EDIT_H_

#include <filesystem>
#include <string>
#include "settings_widget.h"
#include "xenia/ui/qt/widgets/text_edit.h"

namespace xe {
namespace ui {
namespace qt {

template <typename T>
class SettingsTextEdit : SettingsWidget<T, XRadioBox> {
  static_assert(
      std::is_same_v<T, std::string> ||
          std::is_same_v<T, std::filesystem::path>,
      "Settings TextEdit must use std::string or std::filesystem::path");
};

}  // namespace qt
}  // namespace ui
}  // namespace xe