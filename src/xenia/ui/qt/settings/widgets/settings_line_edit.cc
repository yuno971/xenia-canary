/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_line_edit.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

const int kLineEditMaxWidth = 480;

SettingsLineEdit::SettingsLineEdit(TextInputSettingsItem& item)
    : XLineEdit(), item_(item), type_(Type::Text) {
  assert_true(Initialize(), "Could not initialize SettingsLineEdit");
}

SettingsLineEdit::SettingsLineEdit(FilePathInputSettingsItem& item)
    : XLineEdit(), item_(item), type_(Type::Path) {
  assert_true(Initialize(), "Could not initialize SettingsLineEdit");
}

SettingsLineEdit::~SettingsLineEdit() {
  if (type_ == Type::Path) {
    auto& item = dynamic_cast<FilePathInputSettingsItem&>(item_);
    item.cvar()->RemoveListener(this);
  } else if (type_ == Type::Text) {
    auto& item = dynamic_cast<TextInputSettingsItem&>(item_);
    item.cvar()->RemoveListener(this);
  }
}

bool SettingsLineEdit::Initialize() {
  if (type_ == Type::Path) {
    auto item = dynamic_cast<FilePathInputSettingsItem*>(&item_);
    if (!item) return false;

    this->setPlaceholderText(item->description().c_str());
    this->setMaximumWidth(kLineEditMaxWidth);

    const auto& current_path = *item->cvar()->current_value();
    std::string current_path_str = std::string(current_path.u8string());
    this->setText(QString(current_path_str.c_str()));

    XLineEdit::connect(this, &XLineEdit::textChanged,
                       [=](const QString& text) {
                         auto path = std::string(text.toUtf8());
                         item->UpdateValue(std::filesystem::path(path));
                       });

    return true;
  } else if (type_ == Type::Text) {
    auto item = dynamic_cast<TextInputSettingsItem*>(&item_);
    if (!item) return false;

    this->setPlaceholderText(item->description().c_str());
    this->setMaximumWidth(kLineEditMaxWidth);

    const auto& current_text = *item->cvar()->current_value();
    this->setText(QString(current_text.c_str()));

    XLineEdit::connect(this, &XLineEdit::textChanged,
                       [=](const QString& text) {
                         item->UpdateValue(std::string(text.toUtf8()));
                       });

    return true;
  }

  return false;
}

void SettingsLineEdit::OnValueUpdated(const ICommandVar& var) {
  if (!is_value_updating_) {
    QString text = this->text();
    if (type_ == Type::Path) {
      auto& item = dynamic_cast<FilePathInputSettingsItem&>(item_);
      text = item.cvar()->current_value()->string().c_str();
    } else if (type_ == Type::Text) {
      auto& item = dynamic_cast<TextInputSettingsItem&>(item_);
      text = item.cvar()->current_value()->c_str();
    }
    QMetaObject::invokeMethod(this, "setText", Qt::QueuedConnection,
                              Q_ARG(QString, text));
  }
}

}  // namespace qt
}  // namespace ui
}  // namespace xe