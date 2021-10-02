/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_widget_factory.h"
#include <QHBoxLayout>
#include "xenia/ui/qt/settings/widgets/settings_checkbox.h"
#include "xenia/ui/qt/settings/widgets/settings_combobox.h"
#include "xenia/ui/qt/settings/widgets/settings_line_edit.h"
#include "xenia/ui/qt/settings/widgets/settings_slider.h"
#include "xenia/ui/qt/widgets/groupbox.h"
#include "xenia/ui/qt/widgets/push_button.h"
#include "xenia/ui/qt/widgets/scroll_area.h"

namespace xe {
namespace ui {
namespace qt {

const double kSubLabelSize = 6.5;
const int kLineEditMaxWidth = 420;

QLabel* create_title_label(const std::string& title) {
  auto label = new QLabel(title.c_str());
  label->setObjectName("titleLabel");
  return label;
}

QWidget* SettingsWidgetFactory::BuildSettingsWidget(
    const std::string& set_name) {
  const auto& sets = settings_.settings();
  auto set = std::find_if(
      sets.begin(), sets.end(),
      [&](const settings::SettingsSet& s) { return set_name == s.title; });

  if (set == sets.end()) {
    return nullptr;
  }

  QWidget* base_widget = new QWidget();

  QVBoxLayout* layout = new QVBoxLayout();
  base_widget->setLayout(layout);

  for (const auto& group : set->groups) {
    XGroupBox* group_box = new XGroupBox(group.title.c_str());
    QVBoxLayout* gb_layout = new QVBoxLayout();
    group_box->setLayout(gb_layout);

    for (auto& item : group.items) {
      QWidget* settings_widget = CreateWidgetForSettingsItem(*item);
      if (settings_widget) {
        gb_layout->addWidget(settings_widget);
      }
    }
    layout->addWidget(group_box);
  }

  base_widget->setObjectName("settingsContainer");

  return base_widget;
}

QWidget* SettingsWidgetFactory::CreateWidgetForSettingsItem(
    settings::ISettingsItem& item) {
  try {
    switch (item.type()) {
      case settings::SettingsType::Boolean: {
        return CreateCheckBoxWidget(
            dynamic_cast<settings::BooleanSettingsItem&>(item));
        break;
      }
      case settings::SettingsType::TextInput: {
        return CreateTextInputWidget(
            dynamic_cast<settings::TextInputSettingsItem&>(item));
        break;
      }
      case settings::SettingsType::PathInput: {
        return CreatePathInputWidget(
            dynamic_cast<settings::FilePathInputSettingsItem&>(item));
        break;
      }
      case settings::SettingsType::NumberInput: {
        return CreateNumberInputWidget(
            dynamic_cast<settings::NumberInputSettingsItem&>(item));
        break;
      }
      case settings::SettingsType::MultiChoice: {
        return CreateMultiChoiceWidget(
            dynamic_cast<settings::IMultiChoiceSettingsItem&>(item));
        break;
      }
      case settings::SettingsType::Range: {
        return CreateRangeInputWidget(
            dynamic_cast<settings::RangeInputSettingsItem&>(item));
        break;
      }
      case settings::SettingsType::Action: {
        return CreateActionWidget(
            dynamic_cast<settings::ActionSettingsItem&>(item));
        break;
      }
      case settings::SettingsType::Custom: {
        return nullptr;
        // TODO: when/if custom widgets are required, build them here
        break;
      }
    }
  } catch (const std::bad_cast&) {
    XELOGE("SettingsItem \"{}\" had wrong type value", item.title());
  }

  return nullptr;
}

QWidget* SettingsWidgetFactory::CreateCheckBoxWidget(
    settings::BooleanSettingsItem& item) {
  SettingsCheckBox* checkbox = new SettingsCheckBox(item);

  return CreateWidgetContainer(checkbox);
}

QWidget* SettingsWidgetFactory::CreateTextInputWidget(
    settings::TextInputSettingsItem& item) {
  QWidget* ctr = new QWidget();
  QVBoxLayout* ctr_layout = new QVBoxLayout();
  ctr->setLayout(ctr_layout);

  QLabel* title_label = create_title_label(item.title());

  SettingsLineEdit* line_edit = new SettingsLineEdit(item);

  ctr_layout->addWidget(title_label);
  ctr_layout->addWidget(line_edit);

  return CreateWidgetContainer(ctr);
}

QWidget* SettingsWidgetFactory::CreatePathInputWidget(
    settings::FilePathInputSettingsItem& item) {
  QWidget* ctr = new QWidget();
  QVBoxLayout* ctr_layout = new QVBoxLayout();
  ctr->setLayout(ctr_layout);
  ctr_layout->setContentsMargins(0, 0, 0, 0);
  ctr_layout->setSpacing(8);

  QLabel* title_label = create_title_label(item.title());

  QHBoxLayout* control_layout = new QHBoxLayout();
  control_layout->setSpacing(8);

  SettingsLineEdit* line_edit = new SettingsLineEdit(item);
  XPushButton* browse_btn = new XPushButton();
  browse_btn->SetIconFromGlyph(0xE838);
  // TODO: Setup browse button logic

  control_layout->addWidget(line_edit);
  control_layout->addWidget(browse_btn);
  control_layout->addStretch();

  ctr_layout->addWidget(title_label);
  ctr_layout->addLayout(control_layout);

  return CreateWidgetContainer(ctr);
}

QWidget* SettingsWidgetFactory::CreateNumberInputWidget(
    settings::NumberInputSettingsItem& item) {
  // TODO: use XSpinBox (styled QSpinBox)
  return nullptr;
}

QWidget* SettingsWidgetFactory::CreateRangeInputWidget(
    settings::RangeInputSettingsItem& item) {
  using xe::app::settings::ValueType;

  QWidget* ctr = new QWidget();
  QHBoxLayout* ctr_layout = new QHBoxLayout();
  ctr_layout->setContentsMargins(0, 0, 0, 0);
  ctr_layout->setSpacing(20);
  ctr->setLayout(ctr_layout);

  QLabel* title_label = create_title_label(item.title());

  SettingsSlider* slider = new SettingsSlider(item);
  int min = xe::app::settings::number_value_to_int(item.min());
  int max = xe::app::settings::number_value_to_int(item.max());

  SettingsSlider::connect(slider, &SettingsSlider::valueChanged,
                          [&](int value) { item.UpdateValue(value); });

  ctr_layout->addWidget(title_label);
  ctr_layout->addWidget(slider);
  ctr_layout->addStretch();

  return CreateWidgetContainer(ctr);
}

QWidget* SettingsWidgetFactory::CreateMultiChoiceWidget(
    settings::IMultiChoiceSettingsItem& item) {
  QWidget* ctr = new QWidget();
  QHBoxLayout* ctr_layout = new QHBoxLayout();
  ctr_layout->setContentsMargins(0, 0, 0, 0);
  ctr_layout->setSpacing(20);
  ctr->setLayout(ctr_layout);

  QLabel* title_label = create_title_label(item.title());

  SettingsComboBox* combobox = new SettingsComboBox(item);

  ctr_layout->addWidget(title_label);
  ctr_layout->addWidget(combobox);
  ctr_layout->addStretch();

  return CreateWidgetContainer(ctr);
}

QWidget* SettingsWidgetFactory::CreateActionWidget(
    settings::ActionSettingsItem& item) {
  return nullptr;
}

QWidget* SettingsWidgetFactory::CreateWidgetContainer(QWidget* target_widget) {
  QWidget* container_widget = new QWidget();
  QVBoxLayout* widget_layout = new QVBoxLayout();
  widget_layout->setContentsMargins(0, 0, 0, 0);
  widget_layout->setSpacing(0);
  container_widget->setLayout(widget_layout);

  // label used to show warnings
  QLabel* widget_label = new QLabel();
  widget_label->setObjectName("subLabel");
  widget_label->setProperty("type", "warning");
  auto font = widget_label->font();
  font.setPointSizeF(kSubLabelSize);
  widget_label->setFont(font);
  widget_label->setVisible(false);

  widget_layout->addWidget(target_widget);
  widget_layout->addWidget(widget_label);

  return container_widget;
}

}  // namespace qt
}  // namespace ui
}  // namespace xe