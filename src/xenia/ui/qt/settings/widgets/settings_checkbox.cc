/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_checkbox.h"

namespace xe {
namespace ui {
namespace qt {

void SettingsCheckBox::Initialize() {
  if (!cvar_) {
    return;
  }

  auto cvar = cvar_->as<bool>();
  if (!cvar) {
    return;
  }

  setChecked(*cvar->current_value());

  connect(this, &SettingsCheckBox::stateChanged, [this](int state) {
    if (state == Qt::Checked) {
      UpdateValue(true);
    } else if (state == Qt::Unchecked) {
      UpdateValue(false);
    } else {
      XELOGW("PartiallyChecked state not supported for SettingsCheckBox");
    }
  });
}

}  // namespace qt
}  // namespace ui
}  // namespace xe