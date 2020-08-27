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

class SettingsCheckBox : public SettingsWidget<bool, XCheckBox> {
 public:
  explicit SettingsCheckBox(const std::string& config_name,
                            QWidget* parent = nullptr)
      : SettingsWidget(config_name, parent) {
    if (!cvar_) {
      return;
    }

    setChecked(*cvar_->current_value());

    connect(this, &SettingsCheckBox::stateChanged, [&](int state) {
      if (state == Qt::Checked) {
        UpdateValue(true);
      } else if (state == Qt::Unchecked) {
        UpdateValue(false);
      } else {
        XELOGW("PartiallyChecked state not supported for SettingsCheckBox");
      }
    });
  }
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif