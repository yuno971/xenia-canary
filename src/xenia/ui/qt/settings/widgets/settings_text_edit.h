/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_TEXT_EDIT_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_TEXT_EDIT_H_

#include <filesystem>
#include <string>
#include "settings_widget.h"
#include "xenia/ui/qt/widgets/text_edit.h"

namespace xe {
namespace ui {
namespace qt {

class SettingsTextEdit : SettingsWidget<std::string, XTextEdit> {};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif
