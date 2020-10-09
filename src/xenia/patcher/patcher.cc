/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2016 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */
#include <regex>

#include "xenia/patcher/patcher.h"

#include "xenia/base/cvar.h"
#include "xenia/base/filesystem.h"
#include "xenia/base/logging.h"

DEFINE_bool(apply_patches, true, "Enables patching functionality", "General");

namespace xe {
namespace patcher {

PatchingSystem::PatchingSystem() {
  isAnyPatchApplied_ = false;
  loadPatches();
}

PatchingSystem::~PatchingSystem() {}

void PatchingSystem::loadPatches() {
  if (!cvars::apply_patches) {
    return;
  }

  std::filesystem::path patches_directory =
      filesystem::GetExecutableFolder() / "patches";

  auto patch_files = filesystem::ListFiles(patches_directory);
  std::regex file_name_regex_match = std::regex("^[A-Fa-f0-9]{8}\\.patch$");

  for (const auto patch_file : patch_files) {
    // Skip files that doesn't have only title_id as name and .patch as
    // extension
    if (!std::regex_match(patch_file.name.u8string(), file_name_regex_match)) {
      XELOGE(
          "PatchingSystem: Skipped loading file {} due to incorrect filename",
          patch_file.name.u8string());
      continue;
    }
    // Load every patch from directory
    auto loaded_file_patches = readPatchFile(patch_file.path / patch_file.name);
    loaded_patch_files.push_back(loaded_file_patches);
  }
  XELOGI("Patching System: Loaded patches for {} titles",
         loaded_patch_files.size());
}

void PatchingSystem::applyPatchesForTitle(Memory* memory,
                                          const uint32_t title_id) {
  for (const auto patchFile : loaded_patch_files) {
    if (patchFile.title_id != title_id) {
      continue;
    }

    auto title_patches = patchFile.patch_info;

    for (const auto patchEntry : title_patches) {
      if (!patchEntry.is_enabled) {
        continue;
      }
      XELOGE("Applying patch for: {}({:08X}) - {}", patchFile.title_name,
             patchFile.title_id, patchEntry.patch_name);
      applyPatch(memory, &patchEntry);
    }
  }
}

void PatchingSystem::applyPatch(Memory* memory, const patchInfoEntry* patch) {
  for (const auto patch_data_entry : patch->patch_data) {
    uint32_t old_address_protect = 0;
    auto address = memory->TranslateVirtual(patch_data_entry.memory_address_);
    auto heap = memory->LookupHeap(patch_data_entry.memory_address_);

    if (!heap) {
      continue;
    }

    heap->QueryProtect(patch_data_entry.memory_address_, &old_address_protect);

    heap->Protect(patch_data_entry.memory_address_,
                  patch_data_entry.alloc_size_,
                  kMemoryProtectRead | kMemoryProtectWrite);

    switch (patch_data_entry.alloc_size_) {
      case 1:
        xe::store_and_swap(address, uint8_t(patch_data_entry.new_value_));
        break;
      case 2:
        xe::store_and_swap(address, uint16_t(patch_data_entry.new_value_));
        break;
      case 4:
        xe::store_and_swap(address, uint32_t(patch_data_entry.new_value_));
        break;
      case 8:
        xe::store_and_swap(address, uint64_t(patch_data_entry.new_value_));
        break;
      default:
        XELOGE("Unsupported patch allocation size");
        break;
    }
    // Restore previous protection
    heap->Protect(patch_data_entry.memory_address_,
                  patch_data_entry.alloc_size_, old_address_protect);

    if (!isAnyPatchApplied_) {
      isAnyPatchApplied_ = true;
    }
  }
}

patchFileEntry PatchingSystem::readPatchFile(
    const std::filesystem::path& file_path) {
  patchFileEntry patchFile;

  std::shared_ptr<cpptoml::table> patch_toml_fields;

  try {
    patch_toml_fields = cpptoml::parse_file(file_path.u8string());
  } catch (...) {
    XELOGE("Cannot load patch file: {}", file_path.u8string());
    return patchFile;
  };

  auto title_name = patch_toml_fields->get_as<std::string>("title_name");
  auto title_id = patch_toml_fields->get_as<std::string>("title_id");

  patchFile.title_id = strtol((*title_id).c_str(), NULL, 16);
  patchFile.title_name = *title_name;

  auto tarr = patch_toml_fields->get_table_array("patch");

  for (auto table : *tarr) {
    patchInfoEntry patch = patchInfoEntry();
    auto patch_name = *table->get_as<std::string>("name");
    auto patch_desc = *table->get_as<std::string>("desc");
    auto patch_author = *table->get_as<std::string>("author");
    auto is_enabled = *table->get_as<bool>("is_enabled");

    patch.id = 0;  // ToDo: Implement id for future GUI stuff
    patch.patch_name = patch_name;
    patch.patch_desc = patch_desc;
    patch.patch_author = patch_author;
    patch.is_enabled = is_enabled;

    const std::string data_types[] = {"be64", "be32", "be16", "be8"};

    // Iterate through all available data sizes
    for (const std::string& type : data_types) {
      auto entries = readPatchData(type, table);

      for (const auto entry : *entries) {
        patch.patch_data.push_back(entry);
      }
    }
    patchFile.patch_info.push_back(patch);
  }
  return patchFile;
}

std::vector<patchDataEntry>* PatchingSystem::readPatchData(
    const std::string size_type,
    const std::shared_ptr<cpptoml::table>& patch_table) {
  std::vector<patchDataEntry>* patch_data = new std::vector<patchDataEntry>();
  auto patch_data_tarr = patch_table->get_table_array(size_type);

  if (!patch_data_tarr) {
    return patch_data;
  }

  for (const auto& patch_data_table : *patch_data_tarr) {
    auto address = patch_data_table->get_as<std::uint32_t>("address");
    // Todo: How to handle different sizes
    auto value = patch_data_table->get_as<std::uint64_t>("value");

    patchDataEntry patchData = {getAllocSize(size_type), *address, *value};
    patch_data->push_back(patchData);
  }
  return patch_data;
}

// TODO(Gliniak): Somehow resolve this mess. Maybe by template?
uint8_t PatchingSystem::getAllocSize(const std::string provided_size) {
  uint8_t alloc_size = sizeof(uint32_t);

  if (provided_size == "be64") {
    alloc_size = sizeof(uint64_t);
  }

  if (provided_size == "be32") {
    alloc_size = sizeof(uint32_t);
  }

  if (provided_size == "be16") {
    alloc_size = sizeof(uint16_t);
  }

  if (provided_size == "be8") {
    alloc_size = sizeof(uint8_t);
  }
  return alloc_size;
}

}  // namespace patcher
}  // namespace xe