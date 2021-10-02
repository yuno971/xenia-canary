/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_slider.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

SettingsSlider::SettingsSlider(RangeInputSettingsItem& item)
    : XSlider(), item_(item) {
  assert_true(Initialize(), "Could not initialize SettingsSlider");
}

SettingsSlider::~SettingsSlider() { item_.cvar()->RemoveListener(this); }

bool SettingsSlider::Initialize() {
  int min = number_value_to_int(item_.min());
  int max = number_value_to_int(item_.max());

  this->setMinimum(min);
  this->setMaximum(max);

  int current_value = number_value_to_int(item_.current_value());
  setValue(current_value);

  XSlider::connect(this, &XSlider::valueChanged, [&](int value) {
    // TODO: handle UpdateValue returning false for value out of range
    item_.UpdateValue(value);
  });

  return true;
}

void SettingsSlider::OnValueUpdated(const ICommandVar& var) {
  if (!is_value_updating_) {
    int current_value = number_value_to_int(item_.current_value());
    // update value on UI thread
    QMetaObject::invokeMethod(this, "setValue", Qt::QueuedConnection,
                              Q_ARG(int, current_value));
  }
}

}  // namespace qt
}  // namespace ui
}  // namespace xe