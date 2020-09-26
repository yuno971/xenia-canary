/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_RADIOBOX_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_RADIOBOX_H_

#include "settings_widget.h"
#include "xenia/ui/qt/widgets/radio_button.h"

namespace xe {
namespace ui {
namespace qt {

class SettingsRadioButton : SettingsWidget<int, XRadioButton> {};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif