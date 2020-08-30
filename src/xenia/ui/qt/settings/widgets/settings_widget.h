/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_WIDGET_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_WIDGET_H_

#include "xenia/base/cvar.h"
#include "xenia/config.h"
#include "xenia/ui/qt/themeable_widget.h"

namespace xe {
namespace ui {
namespace qt {

template <typename T, typename Widget>
class SettingsWidget : public Widget {
 public:
  template <typename... Args>
  SettingsWidget(const std::string& config_name, Args... args)
      : Widget(args...), cvar_(nullptr) {
    cvar_ = dynamic_cast<cvar::ConfigVar<T>*>(
        Config::Instance().FindConfigVarByName(config_name));
  }

  template <typename... Args>
  SettingsWidget(const T& config_ref, Args... args)
      : Widget(args...), cvar_(nullptr) {
    cvar_ = dynamic_cast<cvar::ConfigVar<T>*>(
        Config::Instance().FindConfigVar(config_ref));
  }

  void UpdateValue(T val) {
    if (cvar_) {
      cvar_->set_config_value(val);
    }
    Config::Instance().SaveConfig();
  }

  void SaveToConfig() { Config::Instance().SaveConfig(); }

 protected:
  cvar::ConfigVar<T>* cvar_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif