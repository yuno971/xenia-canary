/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_checkbox.h"

namespace xe {
namespace ui {
namespace qt {
  
using namespace xe::app::settings;
using namespace xe::cvar;

SettingsCheckBox::SettingsCheckBox(BooleanSettingsItem& item)
    : XCheckBox(item.title().c_str()), item_(item) {
  assert_true(Initialize(), "Could not initialize SettingsCheckBox");
}

SettingsCheckBox::~SettingsCheckBox() { item_.cvar()->RemoveListener(this); }

bool SettingsCheckBox::Initialize() {
  item_.cvar()->RegisterListener(this);

  setCheckState(*item_.cvar()->current_value() ? Qt::Checked : Qt::Unchecked);

  // update cvar when checkbox state changes
  XCheckBox::connect(this, &XCheckBox::stateChanged, [&](int state) {
    is_value_updating_ = true;
    if (state == Qt::Checked) {
      item_.UpdateValue(true);
    } else if (state == Qt::Unchecked) {
      item_.UpdateValue(false);
    } else {
      XELOGW("PartiallyChecked state not supported for SettingsCheckBox");
    }
    is_value_updating_ = false;
  });

  return true;
}

void SettingsCheckBox::OnValueUpdated(const ICommandVar& var) {
  const auto& new_value = item_.cvar()->current_value();
  if (!this->is_value_updating_) {
    // emit state change on ui thread
    QMetaObject::invokeMethod(this, "setChecked", Qt::QueuedConnection,
                              Q_ARG(bool, *new_value));
  }
}

}  // namespace qt
}  // namespace ui
}  // namespace xe