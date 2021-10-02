/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_SETTINGS_LINE_EDIT_H_
#define XENIA_SETTINGS_LINE_EDIT_H_

#include "xenia/app/settings/settings.h"
#include "xenia/base/cvar.h"
#include "xenia/ui/qt/widgets/line_edit.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

class SettingsLineEdit : public XLineEdit, ICommandVarListener {
  Q_OBJECT;

  enum class Type { Text, Path };

 public:
  explicit SettingsLineEdit(TextInputSettingsItem& item);
  explicit SettingsLineEdit(FilePathInputSettingsItem& item);
  ~SettingsLineEdit();

  bool Initialize();
  void OnValueUpdated(const ICommandVar& var) override;

 private:
  ISettingsItem& item_;
  std::atomic_bool is_value_updating_ = false;
  Type type_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif