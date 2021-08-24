/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_SETTINGS_SETTINGS_LOADER_H_
#define XENIA_APP_SETTINGS_SETTINGS_LOADER_H_

#include <memory>
#include <string>
#include "settings.h"
#include "third_party/pugixml/src/pugixml.hpp"
#include "xenia/base/cvar.h"

namespace xe {
namespace app {
namespace settings {

class SettingsLoader {
 public:
  SettingsLoader(Settings& settings) : settings_(settings) {}

  void AddSetting(std::unique_ptr<ISettingsItem>&& item,
                  const std::string& group, const std::string& set);

  bool LoadSettingsFromEmbeddedXml();

  void AddBooleanInputSetting(std::string title, std::string description,
                              std::string cvar_name, std::string group,
                              std::string set);
  void AddBooleanInputSetting(std::string title, std::string description,
                              cvar::ConfigVar<bool>* cvar, std::string group,
                              std::string set);

  void AddTextInputSetting(std::string title, std::string description,
                           std::string cvar_name, std::string group,
                           std::string set);
  void AddTextInputSetting(std::string title, std::string description,
                           cvar::ConfigVar<std::string>* cvar,
                           std::string group, std::string set);

  void AddFilePathInputSetting(std::string title, std::string description,
                               std::string cvar_name, std::string group,
                               std::string set);
  void AddFilePathInputSetting(std::string title, std::string description,
                               cvar::ConfigVar<std::filesystem::path>* cvar,
                               std::string group, std::string set);

  void AddNumberInputInputSetting(std::string title, std::string description,
                                  std::string cvar_name, std::string group,
                                  std::string set, ValueType value_type);
  void AddNumberInputInputSetting(std::string title, std::string description,
                                  cvar::IConfigVar* cvar, std::string group,
                                  std::string set, ValueType value_type);

  void AddRangeInputSetting(std::string title, std::string description,
                            std::string cvar_name, std::string group,
                            std::string set, ValueType value_type,
                            NumberValue min, NumberValue max);
  void AddRangeInputSetting(std::string title, std::string description,
                            cvar::IConfigVar* cvar, std::string group,
                            std::string set, ValueType value_type,
                            NumberValue min, NumberValue max);

  template <typename T>
  void AddMultiChoiceInputSetting(
      std::string title, std::string description, std::string cvar_name,
      std::string group, std::string set,
      std::initializer_list<typename MultiChoiceSettingsItem<T>::Option>
          options);
  template <typename T>
  void AddMultiChoiceInputSetting(
      std::string title, std::string description, cvar::ConfigVar<T>* cvar,
      std::string group, std::string set,
      std::initializer_list<typename MultiChoiceSettingsItem<T>::Option>
          options);

 private:
  template <typename T>
  using Option = typename MultiChoiceSettingsItem<T>::Option;

  std::unique_ptr<ISettingsItem> LoadSettingsItemFromXmlNode(
      const pugi::xml_node& node);

  template <typename T>
  Option<T> LoadMultiChoiceOption(const pugi::xml_node& option_node);

  Settings& settings_;
};

template <typename T>
void SettingsLoader::AddMultiChoiceInputSetting(
    std::string title, std::string description, std::string cvar_name,
    std::string group, std::string set,
    std::initializer_list<typename MultiChoiceSettingsItem<T>::Option>
        options) {
  auto cvar = Config::Instance().FindConfigVarByName(cvar_name);
  return AddMultiChoiceInputSetting(title, description, cvar, group, set,
                                    options);
}

template <typename T>
void SettingsLoader::AddMultiChoiceInputSetting(
    std::string title, std::string description, cvar::ConfigVar<T>* cvar,
    std::string group, std::string set,
    std::initializer_list<typename MultiChoiceSettingsItem<T>::Option>
        options) {
  if (cvar) {
    auto setting_item = std::make_unique<MultiChoiceSettingsItem<T>>(
        title, description, options, cvar);
    AddSetting(std::move(setting_item), group, set);
  } else {
    XELOGE("Could not find cvar named {}", cvar_name);
  }
}

}  // namespace settings
}  // namespace app
}  // namespace xe

#endif