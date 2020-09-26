/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_SETTINGS_SETTINGS_GROUPBOX_H_
#define XENIA_UI_QT_SETTINGS_SETTINGS_GROUPBOX_H_

#include "settings_checkbox.h"
#include "settings_combobox.h"
#include "settings_radio_button.h"
#include "xenia/ui/qt/widgets/groupbox.h"

namespace xe {
namespace ui {
namespace qt {

class SettingsGroupBox : public XGroupBox {
 public:
  SettingsGroupBox(const QString& title, QWidget* parent = nullptr)
      : XGroupBox(title, parent), layout_(nullptr) {
    layout_ = new QVBoxLayout();
    this->setLayout(layout_);
  }

  SettingsCheckBox* CreateCheckBox(const QString& text,
                                   cvar::ConfigVar<bool>* target = nullptr);
  SettingsComboBox<int>* CreateComboBox(const QString& text,
                                        cvar::ConfigVar<int>* target = nullptr);
  SettingsComboBox<std::string>* CreateComboBox(
      const QString& text, cvar::ConfigVar<std::string>* target = nullptr);
  SettingsRadioButton* CreateRadioButton(
      const QString& text, cvar::ConfigVar<int>* target = nullptr);

  template <typename T>
  void AddSettingsWidget(SettingsWidget<T>* widget) {
    layout_->addWidget(widget);
  }

 private:
  QVBoxLayout* layout_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif