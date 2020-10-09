/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_COMBOBOX_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_COMBOBOX_H_

#include "settings_widget.h"
#include "xenia/ui/qt/widgets/combobox.h"

namespace xe {
namespace ui {
namespace qt {

template <typename T>
class SettingsComboBox : public SettingsWidget<T, XComboBox> {
  using SettingsCvar = cvar::ConfigVar<T>;

 public:
  SettingsComboBox(SettingsCvar* config_var = nullptr, QLabel* label = nullptr,
                   QWidget* parent = nullptr)
      : SettingsWidget(config_var, label, parent) {}

  void Initialize();
};

template <>
inline void SettingsComboBox<int>::Initialize() {
  if (!cvar_) {
    return;
  }
  if (!cvar_) {
    return;
  }

  auto cvar = cvar_->as<int>();
  if (!cvar) {
    return;
  }

  setCurrentIndex(*cvar->current_value());

  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this](int index) { UpdateValue(index); });
}

template <>
inline void SettingsComboBox<std::string>::Initialize() {
  if (!cvar_) {
    return;
  }
  if (!cvar_) {
    return;
  }

  auto cvar = cvar_->as<std::string>();
  if (!cvar) {
    return;
  }

  setCurrentText(QString(*cvar->current_value()->c_str()));

  connect(this, &QComboBox::currentTextChanged, [this](const QString& text) {
    UpdateValue(std::string(text.toUtf8()));
  });
}

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif