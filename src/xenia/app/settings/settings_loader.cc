/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_loader.h"
#include <array>
#include "build/settings.h"
#include "third_party/pugixml/src/pugixml.hpp"
#include "xenia/base/logging.h"
#include "xenia/config.h"

namespace xe {
namespace app {
namespace settings {

// All possible settings node types
static std::string_view valid_node_types[] = {
    "BooleanSetting",     "TextInputSetting",  "PathInputSetting",
    "NumberInputSetting", "RangeInputSetting", "MultiChoiceSetting",
    "ActionSetting",      "CustomSetting"};

static std::string_view valid_multi_choice_types[] = {
    "int", "int32", "int64", "uint", "uint32", "uint64", "double", "string"};

template <>
SettingsLoader::Option<int32_t> SettingsLoader::LoadMultiChoiceOption<int32_t>(
    const pugi::xml_node& node) {
  std::string title = node.child("title").text().as_string();
  int32_t value = node.child("value").text().as_int();
  return Option<int32_t>{title, value};
}

template <>
SettingsLoader::Option<int64_t> SettingsLoader::LoadMultiChoiceOption<int64_t>(
    const pugi::xml_node& node) {
  std::string title = node.child("title").text().as_string();
  int64_t value = node.child("value").text().as_llong();
  return Option<int64_t>{title, value};
}

template <>
SettingsLoader::Option<uint32_t>
SettingsLoader::LoadMultiChoiceOption<uint32_t>(const pugi::xml_node& node) {
  std::string title = node.child("title").text().as_string();
  uint32_t value = node.child("value").text().as_uint();
  return Option<uint32_t>{title, value};
}

template <>
SettingsLoader::Option<uint64_t>
SettingsLoader::LoadMultiChoiceOption<uint64_t>(const pugi::xml_node& node) {
  std::string title = node.child("title").text().as_string();
  uint64_t value = node.child("value").text().as_ullong();
  return Option<uint64_t>{title, value};
}

template <>
SettingsLoader::Option<double> SettingsLoader::LoadMultiChoiceOption<double>(
    const pugi::xml_node& node) {
  std::string title = node.child("title").text().as_string();
  double value = node.child("value").text().as_double();
  return Option<double>{title, value};
}

template <>
SettingsLoader::Option<std::string>
SettingsLoader::LoadMultiChoiceOption<std::string>(const pugi::xml_node& node) {
  std::string title = node.child("title").text().as_string();
  std::string value = node.child("value").text().as_string();
  return Option<std::string>{title, value};
}

void SettingsLoader::AddSetting(std::unique_ptr<ISettingsItem>&& item,
                                const std::string& group,
                                const std::string& set) {
  auto& settings_set = settings_.FindOrCreateSettingsSet(set);
  auto& settings_group = settings_set.FindOrCreateSettingsGroup(group);
  settings_group.AddItem(std::move(item));
}

bool SettingsLoader::LoadSettingsFromEmbeddedXml() {
  constexpr size_t size = sizeof(xe::embedded::settings_data);
  std::array<uint8_t, size> settings_buf;

  // copy embedded data to buffer
  std::copy(std::begin(xe::embedded::settings_data),
            std::end(xe::embedded::settings_data), std::begin(settings_buf));

  pugi::xml_document doc;

  doc.load_buffer_inplace(settings_buf.data(), settings_buf.size());

  const auto& sets_node = doc.first_child();
  if (std::string_view(sets_node.name()) != "Sets") {
    XELOGE("Settings key 'Sets' not found");
    return false;
  }

  for (const auto& set_node : sets_node.children()) {
    XELOGD("Loading settings set with name {}", set_node.name());
    for (const auto& group_node : set_node.children()) {
      // load group and set names
      std::string group = group_node.attribute("name").value();
      std::string set = set_node.attribute("name").value();

      for (const auto& settings_item_node : group_node.children()) {
        auto settings_item = LoadSettingsItemFromXmlNode(settings_item_node);
        if (settings_item) {
          AddSetting(std::move(settings_item), group, set);
        }
      }
    }
  }

  return true;
}

std::unique_ptr<ISettingsItem> SettingsLoader::LoadSettingsItemFromXmlNode(
    const pugi::xml_node& node) {
  std::string_view node_name = node.name();
  auto node_found = std::find(std::begin(valid_node_types),
                              std::end(valid_node_types), node_name);
  // if node is a valid settings node
  if (node_found != std::end(valid_node_types)) {
    std::string_view node_type = *node_found;

    // TODO: handle exceptions for child not found
    std::string title = node.child_value("title");
    std::string description = node.child_value("description");

    // if cvar specified, load it
    IConfigVar* cvar = nullptr;
    if (node.child("cvar")) {
      std::string cvar_name = node.child_value("cvar");
      cvar = Config::Instance().FindConfigVarByName(cvar_name);
      if (!cvar) {
        XELOGE("Failed to find cvar with name '{}'", cvar_name);
        return nullptr;
      }
    }

    if (node_type == "BooleanSetting") {
      return std::make_unique<BooleanSettingsItem>(
          title, description, dynamic_cast<ConfigVar<bool>*>(cvar));
    } else if (node_type == "TextInputSetting") {
      return std::make_unique<TextInputSettingsItem>(
          title, description, dynamic_cast<ConfigVar<std::string>*>(cvar));
    } else if (node_type == "PathInputSetting") {
      return std::make_unique<FilePathInputSettingsItem>(
          title, description,
          dynamic_cast<ConfigVar<std::filesystem::path>*>(cvar));
    } else if (node_type == "MultiChoiceSetting") {
      // TODO: load type, load options and add options to array of type `type`
      std::string_view type_name = node.child_value("type");
      if (type_name.empty()) {
        XELOGE("'type' node not found for MultiChoiceSetting");
        return nullptr;
      }

      auto node_found =
          std::find(std::begin(valid_multi_choice_types),
                    std::end(valid_multi_choice_types), type_name);
      if (node_found == std::end(valid_multi_choice_types)) {
        XELOGE("Invalid type '{}' provided for MultiChoiceSetting type",
               type_name);
        return nullptr;
      }

      std::string_view multi_choice_type = *node_found;

      if (multi_choice_type == "int" || multi_choice_type == "int32") {
        std::vector<Option<int32_t>> values;
        for (const auto& option_node : node.child("options").children()) {
          values.push_back(LoadMultiChoiceOption<int32_t>(option_node));
        }
      }
      if (multi_choice_type == "int64") {
        std::vector<Option<int64_t>> values;
        for (const auto& option_node : node.child("options").children()) {
          values.push_back(LoadMultiChoiceOption<int64_t>(option_node));
        }
      }
    }
  } else {
    XELOGE("Unknown settings node type {}", node.name());
  }

  return nullptr;
}

void SettingsLoader::AddBooleanInputSetting(std::string title,
                                            std::string description,
                                            cvar::ConfigVar<bool>* cvar,
                                            std::string group,
                                            std::string set) {
  if (cvar) {
    auto setting_item =
        std::make_unique<BooleanSettingsItem>(title, description, cvar);
    AddSetting(std::move(setting_item), group, set);
  } else {
    XELOGE("Could not find cvar for setting with title {}", title);
  }
}

void SettingsLoader::AddBooleanInputSetting(std::string title,
                                            std::string description,
                                            std::string cvar_name,
                                            std::string group,
                                            std::string set) {
  auto cvar = Config::Instance().FindConfigVarByName(cvar_name);
  return AddBooleanInputSetting(
      title, description, dynamic_cast<ConfigVar<bool>*>(cvar), group, set);
}

void SettingsLoader::AddTextInputSetting(std::string title,
                                         std::string description,
                                         cvar::ConfigVar<std::string>* cvar,
                                         std::string group, std::string set) {
  if (cvar) {
    auto setting_item =
        std::make_unique<TextInputSettingsItem>(title, description, cvar);
    AddSetting(std::move(setting_item), group, set);
  } else {
    XELOGE("Could not find cvar for setting with title {}", title);
  }
}

void SettingsLoader::AddTextInputSetting(std::string title,
                                         std::string description,
                                         std::string cvar_name,
                                         std::string group, std::string set) {
  auto cvar = Config::Instance().FindConfigVarByName(cvar_name);
  return AddTextInputSetting(title, description,
                             dynamic_cast<ConfigVar<std::string>*>(cvar), group,
                             set);
}

void SettingsLoader::AddFilePathInputSetting(
    std::string title, std::string description,
    cvar::ConfigVar<std::filesystem::path>* cvar, std::string group,
    std::string set) {
  if (cvar) {
    auto setting_item =
        std::make_unique<FilePathInputSettingsItem>(title, description, cvar);
    AddSetting(std::move(setting_item), group, set);
  } else {
    XELOGE("Could not find cvar for setting with title {}", title);
  }
}

void SettingsLoader::AddFilePathInputSetting(std::string title,
                                             std::string description,
                                             std::string cvar_name,
                                             std::string group,
                                             std::string set) {
  auto cvar = Config::Instance().FindConfigVarByName(cvar_name);
  return AddFilePathInputSetting(
      title, description, dynamic_cast<ConfigVar<std::filesystem::path>*>(cvar),
      group, set);
}

void SettingsLoader::AddNumberInputInputSetting(
    std::string title, std::string description, cvar::IConfigVar* cvar,
    std::string group, std::string set, ValueType value_type) {
  if (cvar) {
    auto setting_item = std::make_unique<NumberInputSettingsItem>(
        value_type, title, description, cvar);
    AddSetting(std::move(setting_item), group, set);
  } else {
    XELOGE("Could not find cvar for setting with title {}", title);
  }
}

void SettingsLoader::AddNumberInputInputSetting(
    std::string title, std::string description, std::string cvar_name,
    std::string group, std::string set, ValueType value_type) {
  auto cvar = Config::Instance().FindConfigVarByName(cvar_name);
  return AddNumberInputInputSetting(title, description, cvar, group, set,
                                    value_type);
}

void SettingsLoader::AddRangeInputSetting(std::string title,
                                          std::string description,
                                          cvar::IConfigVar* cvar,
                                          std::string group, std::string set,
                                          ValueType value_type, NumberValue min,
                                          NumberValue max) {
  if (cvar) {
    auto setting_item = std::make_unique<RangeInputSettingsItem>(
        value_type, title, min, max, description, cvar);
    AddSetting(std::move(setting_item), group, set);
  } else {
    XELOGE("Could not find cvar for setting with title {}", title);
  }
}

void SettingsLoader::AddRangeInputSetting(std::string title,
                                          std::string description,
                                          std::string cvar_name,
                                          std::string group, std::string set,
                                          ValueType value_type, NumberValue min,
                                          NumberValue max) {
  auto cvar = Config::Instance().FindConfigVarByName(cvar_name);
  return AddRangeInputSetting(title, description, cvar, group, set, value_type,
                              min, max);
}

}  // namespace settings
}  // namespace app
}  // namespace xe