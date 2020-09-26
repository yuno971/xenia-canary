/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_CHECKBOX_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_CHECKBOX_H_

#include "settings_widget.h"
#include "xenia/base/logging.h"
#include "xenia/ui/qt/widgets/checkbox.h"

namespace xe {
namespace ui {
namespace qt {

using SettingsCvar = cvar::ConfigVar<bool>;

class SettingsCheckBox : public SettingsWidget<bool, XCheckBox> {
 public:
  explicit SettingsCheckBox(const QString& text,
                            SettingsCvar* config_var = nullptr, QLabel* label = nullptr,
                            QWidget* parent = nullptr)
      : SettingsWidget(config_var, label, text, parent) {
    Initialize();
  }

  void Initialize();
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif