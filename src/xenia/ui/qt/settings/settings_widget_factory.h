/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_SETTINGS_WIDGET_FACTORY_H_
#define XENIA_SETTINGS_WIDGET_FACTORY_H_

#include "xenia/app/settings/settings.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

namespace xe {
namespace ui {
namespace qt {

namespace settings = xe::app::settings;

class SettingsWidgetFactory {
 public:
  explicit SettingsWidgetFactory(
      settings::Settings& settings = settings::Settings::Instance())
      : settings_(settings) {}

  QWidget* BuildSettingsWidget(const std::string& set_name);
  QWidget* CreateWidgetForSettingsItem(settings::ISettingsItem& item);
 private:
  QWidget* CreateCheckBoxWidget(settings::BooleanSettingsItem& item);
  QWidget* CreateTextInputWidget(settings::TextInputSettingsItem& item);
  QWidget* CreatePathInputWidget(settings::FilePathInputSettingsItem& item);
  QWidget* CreateNumberInputWidget(settings::NumberInputSettingsItem& item);
  QWidget* CreateRangeInputWidget(settings::RangeInputSettingsItem& item);
  //QWidget* CreateMultiChoiceWidget(settings::MultiChoiceSettingsItem<>& item);
  QWidget* CreateActionWidget(settings::ActionSettingsItem& item);

  QWidget* CreateWidgetContainer(QWidget* target);

  settings::Settings& settings_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif
