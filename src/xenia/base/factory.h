/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_BASE_FACTORY_H_
#define XENIA_BASE_FACTORY_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace xe {

template <typename T, typename... Args>
class Factory {
 private:
  struct Creator {
    std::string name;
    std::function<bool()> is_available;
    std::function<std::unique_ptr<T>(Args...)> instantiate;
  };

  std::vector<Creator> creators_;

 public:
  void Add(const std::string_view name, std::function<bool()> is_available,
           std::function<std::unique_ptr<T>(Args...)> instantiate) {
    creators_.push_back({std::string(name), is_available, instantiate});
  }

  void Add(const std::string_view name,
           std::function<std::unique_ptr<T>(Args...)> instantiate) {
    auto always_available = []() { return true; };
    Add(name, always_available, instantiate);
  }

  template <typename DT>
  void Add(const std::string_view name) {
    Add(name, DT::IsAvailable, [](Args... args) {
      return std::make_unique<DT>(std::forward<Args>(args)...);
    });
  }

  std::unique_ptr<T> Create(const std::string_view name, Args... args) {
    if (!name.empty() && name != "any") {
      auto it = std::find_if(
          creators_.cbegin(), creators_.cend(),
          [&name](const auto& f) { return name.compare(f.name) == 0; });
      if (it != creators_.cend() && (*it).is_available()) {
        return (*it).instantiate(std::forward<Args>(args)...);
      }
      return nullptr;
    } else {
      for (const auto& creator : creators_) {
        if (!creator.is_available()) continue;
        auto instance = creator.instantiate(std::forward<Args>(args)...);
        if (!instance) continue;
        return instance;
      }
      return nullptr;
    }
  }

  std::vector<std::unique_ptr<T>> CreateAll(const std::string_view name,
                                            Args... args) {
    std::vector<std::unique_ptr<T>> instances;
    if (!name.empty() && name != "any") {
      auto it = std::find_if(
          creators_.cbegin(), creators_.cend(),
          [&name](const auto& f) { return name.compare(f.name) == 0; });
      if (it != creators_.cend() && (*it).is_available()) {
        auto instance = (*it).instantiate(std::forward<Args>(args)...);
        if (instance) {
          instances.emplace_back(std::move(instance));
        }
      }
    } else {
      for (const auto& creator : creators_) {
        if (!creator.is_available()) continue;
        auto instance = creator.instantiate(std::forward<Args>(args)...);
        if (instance) {
          instances.emplace_back(std::move(instance));
        }
      }
    }
    return instances;
  }
};

}  // namespace xe

#endif