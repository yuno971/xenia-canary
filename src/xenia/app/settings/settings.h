/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_SETTINGS_SETTINGS_H_
#define XENIA_APP_SETTINGS_SETTINGS_H_

#include <filesystem>
#include <string>
#include <variant>
#include <vector>
#include "xenia/base/cvar.h"
#include "xenia/base/delegate.h"
#include "xenia/base/logging.h"

namespace xe {
namespace app {
namespace settings {

using cvar::ConfigVar;
using cvar::IConfigVar;

enum class SettingsType {
  Boolean,
  TextInput,
  PathInput,
  NumberInput,
  MultiChoice,
  Range,
  Action,
  Custom
};

class ISettingsItem {
 public:
  virtual ~ISettingsItem() = default;

  ISettingsItem(SettingsType type, const std::string& title,
                const std::string& description)
      : type_(type), title_(title), description_(description) {}

  const SettingsType type() const { return type_; }
  const std::string& title() const { return title_; }
  const std::string& description() const { return description_; }

 private:
  SettingsType type_;
  std::string title_;
  std::string description_;
};

template <typename T, SettingsType Type>
class BasicSettingsItem : public ISettingsItem {
 public:
  BasicSettingsItem(const std::string& title, const std::string& description,
                    ConfigVar<T>* cvar = nullptr)
      : ISettingsItem(Type, title, description), cvar_(cvar) {}

  ConfigVar<T>* cvar() const { return cvar_; }

  virtual bool UpdateValue(T value) {
    if (auto cvar = cvar_->as<T>(); cvar != nullptr) {
      cvar->set_config_value(value);
      Config::Instance().SaveConfig();
      return true;
    }
    return false;
  }

 private:
  ConfigVar<T>* cvar_;
};

using BooleanSettingsItem = BasicSettingsItem<bool, SettingsType::Boolean>;
using TextInputSettingsItem =
    BasicSettingsItem<std::string, SettingsType::TextInput>;
using FilePathInputSettingsItem =
    BasicSettingsItem<std::filesystem::path, SettingsType::PathInput>;

enum class ValueType {
  Int8,
  Int16,
  Int32,
  Int64,
  UInt8,
  UInt16,
  UInt32,
  UInt64,
  Double
};

using NumberValue = std::variant<int8_t, int16_t, int32_t, int64_t, uint8_t,
                                 uint16_t, uint32_t, uint64_t, double>;

inline int number_value_to_int(NumberValue v) {
  struct Visitor {
    int operator()(int8_t value) { return value; }
    int operator()(int16_t value) { return value; }
    int operator()(int32_t value) { return value; }
    int operator()(int64_t value) { return static_cast<int>(value); }
    int operator()(uint8_t value) { return value; }
    int operator()(uint16_t value) { return value; }
    int operator()(uint32_t value) { return value; }
    int operator()(uint64_t value) { return static_cast<int>(value); }
    int operator()(double value) { return static_cast<int>(value); }
  };

  return std::visit(Visitor{}, v);
}

template <SettingsType settings_type>
class NumberSettingsItem : public ISettingsItem {
  struct ValueUpdater {
    ValueUpdater(NumberSettingsItem* item) : item(item) {}

    bool operator()(int8_t value) { return update(ValueType::Int8, value); }
    bool operator()(int16_t value) { return update(ValueType::Int16, value); }
    bool operator()(int32_t value) { return update(ValueType::Int32, value); }
    bool operator()(int64_t value) { return update(ValueType::Int64, value); }
    bool operator()(uint8_t value) { return update(ValueType::UInt8, value); }
    bool operator()(uint16_t value) { return update(ValueType::UInt16, value); }
    bool operator()(uint32_t value) { return update(ValueType::UInt32, value); }
    bool operator()(uint64_t value) { return update(ValueType::UInt64, value); }
    bool operator()(double value) { return update(ValueType::Double, value); }

    template <typename T>
    bool update(ValueType type, T value) {
      if (type == item->value_type()) {
        if (auto cvar = item->cvar()->as<T>(); cvar != nullptr) {
          cvar->set_config_value(value);
          Config::Instance().SaveConfig();
          return true;
        }
      }
      return false;
    }

    NumberSettingsItem* item;
  };

 public:
  NumberSettingsItem(ValueType value_type, const std::string& title,
                     const std::string& description, IConfigVar* cvar = nullptr)
      : ISettingsItem(settings_type, title, description),
        value_type_(value_type),
        cvar_(cvar) {}

  ValueType value_type() const { return value_type_; }

  IConfigVar* cvar() const { return cvar_; }

  NumberValue current_value() const {
    switch (value_type()) {
      case ValueType::Int8: {
        return *cvar()->as<int8_t>()->current_value();
      }
      case ValueType::Int16: {
        return *cvar()->as<int16_t>()->current_value();
      }
      case ValueType::Int32: {
        return *cvar()->as<int32_t>()->current_value();
      }
      case ValueType::Int64: {
        return *cvar()->as<int64_t>()->current_value();
      }
      case ValueType::UInt8: {
        return *cvar()->as<uint8_t>()->current_value();
      }
      case ValueType::UInt16: {
        return *cvar()->as<uint16_t>()->current_value();
      }
      case ValueType::UInt32: {
        return *cvar()->as<uint32_t>()->current_value();
      }
      case ValueType::UInt64: {
        return *cvar()->as<uint64_t>()->current_value();
      }
      case ValueType::Double: {
        return *cvar()->as<double>()->current_value();
      }
      default: {
        assert_always("Invalid number type provided");
        return 0;
      }
    }
  }

  virtual bool UpdateValue(NumberValue value) {
    auto res = std::visit(ValueUpdater(this), value);
    if (!res) {
      XELOGE("Could not update value for {0}. Incorrect value type provided",
             cvar_->name());
    }

    return res;
  }

 private:
  ValueType value_type_;
  IConfigVar* cvar_;
};

class RangeInputSettingsItem : public NumberSettingsItem<SettingsType::Range> {
 public:
  RangeInputSettingsItem(ValueType value_type, std::string title,
                         std::string description, NumberValue min,
                         NumberValue max, IConfigVar* cvar = nullptr)
      : NumberSettingsItem(value_type, title, description, cvar),
        min_(min),
        max_(max) {}

  bool UpdateValue(NumberValue value) override {
    if (value < min_ || value > max_) {
      return false;
    }
    return NumberSettingsItem::UpdateValue(value);
  }

  NumberValue min() const { return min_; }

  NumberValue max() const { return max_; }

 private:
  NumberValue min_;
  NumberValue max_;
};

using NumberInputSettingsItem = NumberSettingsItem<SettingsType::NumberInput>;

class IMultiChoiceSettingsItem : public ISettingsItem {
 public:
  IMultiChoiceSettingsItem(std::string title, std::string description)
      : ISettingsItem(SettingsType::MultiChoice, title, description) {}

  virtual bool UpdateIndex(int index) = 0;
  virtual std::vector<std::string> option_names() const = 0;
  virtual int current_index() const = 0;
  virtual IConfigVar* cvar() const = 0;
};

template <typename T>
class MultiChoiceSettingsItem : public IMultiChoiceSettingsItem {
 public:
  struct Option {
    std::string title;
    T value;
  };

  MultiChoiceSettingsItem(std::string title, std::string description,
                          std::vector<Option> options,
                          ConfigVar<T>* cvar = nullptr)
      : IMultiChoiceSettingsItem(title, description),
        cvar_(cvar),
        options_(options) {}

  MultiChoiceSettingsItem(std::string title, std::string description,
                          std::initializer_list<Option> args,
                          ConfigVar<T>* cvar = nullptr)
      : IMultiChoiceSettingsItem(title, description),
        cvar_(cvar),
        options_(args) {}

  bool UpdateIndex(int index) override {
    if (cvar_) {
      if (index < options_.size()) {
        cvar_->set_config_value(options_.at(index).value);
        Config::Instance().SaveConfig();
        return true;
      } else {
        XELOGE("Out of range index when updating multi choice settings item");
      }
    }
    return false;
  }

  const Option& current_option() const {
    T* value = cvar_->current_value();
    for (const auto& option : options_) {
      if (option.value == *value) {
        return option;
      }
    }

    throw std::runtime_error("Could not find option for current cvar value");
  }

  int current_index() const override {
    try {
      auto it = std::find_if(options_.begin(), options_.end(),
                             [this](const Option& opt) {
                               return opt.title == current_option().title &&
                                      opt.value == current_option().value;
                             });
      return static_cast<int>(std::distance(options_.begin(), it));
    } catch (const std::runtime_error&) {
      return -1;
    }
  }

  std::vector<std::string> option_names() const override {
    std::vector<std::string> names;
    std::transform(options_.begin(), options_.end(), std::back_inserter(names),
                   [](const Option& opt) { return opt.title; });
    return names;
  }

  IConfigVar* cvar() const override { return cvar_; }

 private:
  ConfigVar<T>* cvar_;
  std::vector<Option> options_;
};

class ActionSettingsItem : public ISettingsItem {
 public:
  ActionSettingsItem(std::string title, std::string description = "")
      : ISettingsItem(SettingsType::Action, title, description) {}

  Delegate<> on_triggered;

  virtual void Trigger();
};

struct SettingsGroup {
  std::string title;
  std::vector<std::unique_ptr<ISettingsItem>> items;

  void AddItem(std::unique_ptr<ISettingsItem>&& item);
};

struct SettingsSet {
  std::string title;
  std::vector<SettingsGroup> groups;

  SettingsGroup& FindOrCreateSettingsGroup(const std::string& title);
};

class Settings {
 public:
  static Settings& Instance();

  void LoadSettingsItems();

  const std::vector<SettingsSet>& settings() const { return settings_; }

  SettingsSet& FindOrCreateSettingsSet(const std::string& title);

 private:
  std::vector<SettingsSet> settings_;
};

}  // namespace settings
}  // namespace app
}  // namespace xe

#endif
