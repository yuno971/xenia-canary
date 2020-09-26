/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_groupbox.h"

namespace xe {
namespace ui {
namespace qt {

double kSubLabelSize = 6.5;

SettingsCheckBox* SettingsGroupBox::CreateCheckBox(const QString& text,
    cvar::ConfigVar<bool>* target) {
  auto checkbox_layout = new QVBoxLayout();
  checkbox_layout->setContentsMargins(0, 0, 0, 0);
  checkbox_layout->setSpacing(0);

  auto widget_label = new QLabel();
  widget_label->setObjectName("subLabel");
  widget_label->setProperty("type", "warning");
  auto font = widget_label->font();
  font.setPointSizeF(kSubLabelSize);
  widget_label->setFont(font);

  auto checkbox = new SettingsCheckBox(text, target, widget_label);
  checkbox_layout->addWidget(checkbox);

  checkbox_layout->addWidget(widget_label);

  layout_->addLayout(checkbox_layout);

  return checkbox;
}

SettingsComboBox<int>* SettingsGroupBox::CreateComboBox(const QString& text,
    cvar::ConfigVar<int>* target) {
  return nullptr;
}

SettingsComboBox<std::string>* SettingsGroupBox::CreateComboBox(
    const QString& text, cvar::ConfigVar<std::string>* target) {
  return nullptr;
}

SettingsRadioButton* SettingsGroupBox::CreateRadioButton(const QString& text,
    cvar::ConfigVar<int>* target) {
  return nullptr;
}
}
}  // namespace ui
}  // namespace xe