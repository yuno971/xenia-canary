/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings.h"
#include "settings_loader.h"
#include "xenia/base/cvar.h"

namespace xe {
namespace cvars {

void LoadSettings(xe::app::settings::SettingsLoader& loader) {
#define ADD_BOOLEAN_SETTING(title, description, cvar_name, group, set) \
  extern xe::cvar::ConfigVar<bool>* cv_##cvar_name;                    \
  loader.AddBooleanInputSetting(title, description, cv_##cvar_name, group, set)

#define ADD_TEXT_INPUT_SETTING(title, description, cvar_name, group, set) \
  extern xe::cvar::ConfigVar<std::string>* cv_##cvar_name;                \
  loader.AddTextInputSetting(title, description, cv_##cvar_name, group, set)

#define ADD_PATH_INPUT_SETTING(title, description, cvar_name, group, set) \
  extern xe::cvar::ConfigVar<std::filesystem::path>* cv_##cvar_name;      \
  loader.AddFilePathInputSetting(title, description, cv_##cvar_name, group, set)

  loader.AddBooleanInputSetting("Discord",
                                "Enable support for Discord rich presence",
                                "discord", "General Settings", "General");

  // #include "settings.inc"
}

}  // namespace cvars

namespace app {
namespace settings {

void ActionSettingsItem::Trigger() { on_triggered(); }

void SettingsGroup::AddItem(std::unique_ptr<ISettingsItem>&& item) {
  items.emplace_back(std::move(item));
}

SettingsGroup& SettingsSet::FindOrCreateSettingsGroup(
    const std::string& title) {
  const auto& it = std::find_if(
      groups.begin(), groups.end(),
      [&title](const SettingsGroup& group) { return group.title == title; });

  if (it == groups.end()) {
    groups.push_back(SettingsGroup{title});
    return groups.back();
  }
  return *it;
}

SettingsSet& Settings::FindOrCreateSettingsSet(const std::string& title) {
  const auto& it = std::find_if(
      settings_.begin(), settings_.end(),
      [&title](const SettingsSet& set) { return set.title == title; });

  if (it == settings_.end()) {
    settings_.push_back(SettingsSet{title});
    return settings_.back();
  }
  return *it;
}

Settings& Settings::Instance() {
  static Settings settings;
  return settings;
}

void Settings::LoadSettingsItems() {
  SettingsLoader loader(*this);

  // xe::cvars::LoadSettings(loader);
  loader.LoadSettingsFromEmbeddedXml();
}

}  // namespace settings
}  // namespace app
}  // namespace xe
