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

#include <QLabel>
#include <QVBoxLayout>
#include <functional>
#include "xenia/base/cvar.h"
#include "xenia/config.h"
#include "xenia/ui/qt/themeable_widget.h"

namespace xe {
namespace ui {
namespace qt {

template <typename T, typename Widget = QWidget>
class SettingsWidget : public Widget {
  static_assert(std::is_base_of_v<QWidget, Widget>,
                "SettingsWidget base must be a derivative of QWidget");

 public:
  template <typename... Args>
  SettingsWidget(cvar::IConfigVar* config_var, QLabel* label = nullptr,
                 Args... args)
      : Widget(args...), cvar_(config_var), label_(label) {
    // default config update function
    update_config_fn_ = [](T val, cvar::ConfigVar<T>& cvar) {
      cvar.set_config_value(val);
    };
  }

  void UpdateLabel(const QString& text) {
    if (label_) {
      label_->setText(text);
      label_->setVisible(true);
    } else {
      label_->setVisible(false);
    }
  }

  cvar::IConfigVar* config_var() const { return cvar_; }
  void set_config_var(cvar::IConfigVar* cvar) { cvar_ = cvar; }

  void set_update_config_fn(
      const std::function<void(T, cvar::ConfigVar<T>&)>& fn) {
    update_config_fn_ = fn;
  }

  void UpdateValue(T val) {
    if (cvar_) {
      auto cvar = cvar_->as<T>();
      if (cvar) {
        update_config_fn_(val, *cvar);
        SaveToConfig();
      }
    }
  }

  void SaveToConfig() { Config::Instance().SaveConfig(); }

 protected:
  cvar::IConfigVar* cvar_;
  std::function<void(T, cvar::ConfigVar<T>&)> update_config_fn_;
  QLabel* label_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif
