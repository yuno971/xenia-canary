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
#include "xenia/ui/qt/widgets/checkbox.h"
#include "xenia/ui/qt/widgets/groupbox.h"
#include "xenia/ui/qt/widgets/line_edit.h"
#include "xenia/ui/qt/widgets/push_button.h"
#include "xenia/ui/qt/widgets/scroll_area.h"
#include "xenia/ui/qt/widgets/slider.h"

namespace xe {
namespace ui {
namespace qt {

const double kSubLabelSize = 6.5;
const int kLineEditMaxWidth = 420;

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
        return nullptr;  // TODO:
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
  XCheckBox* checkbox = new XCheckBox();
  checkbox->setText(item.title().c_str());
  checkbox->setCheckState(*item.cvar()->current_value() ? Qt::Checked
                                                        : Qt::Unchecked);

  XCheckBox::connect(checkbox, &XCheckBox::stateChanged, [&](int state) {
    if (state == Qt::Checked) {
      item.UpdateValue(true);
    } else if (state == Qt::Unchecked) {
      item.UpdateValue(false);
    } else {
      XELOGW("PartiallyChecked state not supported for SettingsCheckBox");
    }
  });

  return CreateWidgetContainer(checkbox);
}

QWidget* SettingsWidgetFactory::CreateTextInputWidget(
    settings::TextInputSettingsItem& item) {
  QWidget* ctr = new QWidget();
  QVBoxLayout* ctr_layout = new QVBoxLayout();
  ctr->setLayout(ctr_layout);

  QLabel* title_label = new QLabel(item.title().c_str());

  XLineEdit* line_edit = new XLineEdit();
  line_edit->setPlaceholderText(item.description().c_str());
  line_edit->setMaximumWidth(kLineEditMaxWidth);

  const auto& current_text = *item.cvar()->current_value();
  line_edit->setText(QString(current_text.c_str()));

  XLineEdit::connect(line_edit, &XLineEdit::textChanged,
                     [&](const QString& text) {
                       item.UpdateValue(std::string(text.toUtf8()));
                     });

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

  QLabel* title_label = new QLabel(item.title().c_str());

  QHBoxLayout* control_layout = new QHBoxLayout();
  control_layout->setSpacing(8);

  XLineEdit* line_edit = new XLineEdit();
  line_edit->setPlaceholderText(item.description().c_str());
  line_edit->setMaximumWidth(kLineEditMaxWidth);

  const auto& current_path = *item.cvar()->current_value();
  std::string current_path_str = std::string(current_path.u8string());
  line_edit->setText(QString(current_path_str.c_str()));

  XPushButton* browse_btn = new XPushButton();
  browse_btn->SetIconFromGlyph(0xE838);

  control_layout->addWidget(line_edit);
  control_layout->addWidget(browse_btn);
  control_layout->addStretch();

  XLineEdit::connect(line_edit, &XLineEdit::textChanged,
                     [&](const QString& text) {
                       auto path = std::string(text.toUtf8());
                       item.UpdateValue(std::filesystem::path(path));
                     });

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
  QVBoxLayout* ctr_layout = new QVBoxLayout();
  ctr->setLayout(ctr_layout);

  QLabel* title_label = new QLabel(item.title().c_str());

  XSlider* slider = new XSlider();
  int min = xe::app::settings::number_value_to_int(item.min());
  int max = xe::app::settings::number_value_to_int(item.max());

  ctr_layout->addWidget(title_label);
  ctr_layout->addWidget(slider);

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