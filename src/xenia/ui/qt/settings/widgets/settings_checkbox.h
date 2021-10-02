/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_SETTINGS_CHECKBOX_H_
#define XENIA_SETTINGS_CHECKBOX_H_

#include <atomic>
#include "xenia/app/settings/settings.h"
#include "xenia/ui/qt/widgets/checkbox.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

class SettingsCheckBox : public XCheckBox, ICommandVarListener {
  Q_OBJECT;

 public:
  explicit SettingsCheckBox(BooleanSettingsItem& item);
  ~SettingsCheckBox();


  bool Initialize();
  void OnValueUpdated(const ICommandVar& var) override;

 private:
  BooleanSettingsItem& item_;
  std::atomic_bool is_value_updating_ = false;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif