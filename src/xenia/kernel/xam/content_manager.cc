/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/xam/content_manager.h"

#include <string>

#include "third_party/fmt/include/fmt/format.h"
#include "xenia/base/filesystem.h"
#include "xenia/base/string.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/xam/content_package.h"
#include "xenia/kernel/user_module.h"
#include "xenia/kernel/xobject.h"
#include "xenia/vfs/devices/host_path_device.h"
#include "xenia/vfs/devices/stfs_container_device.h"

namespace xe {
namespace kernel {
namespace xam {

constexpr const wchar_t* const ContentManager::kStfsHeadersExtension;
static const char* kGameUserContentDirName = "profile";

ContentManager::ContentManager(KernelState* kernel_state,
                               const std::filesystem::path& root_path)
    : kernel_state_(kernel_state), root_path_(root_path) {}

ContentManager::~ContentManager() = default;

uint32_t ContentManager::title_id() {
  if (title_id_override_) {
    return title_id_override_;
  }
  if (!kernel_state_->GetExecutableModule()) {
    return -1;
  }
  return kernel_state_->title_id();
}

std::filesystem::path ContentManager::ResolvePackageRoot(
    uint32_t content_type) {
  auto title_id_str = fmt::format("{:08X}", title_id());
  auto content_type_str = fmt::format("{:08X}", content_type);

  // Package root path:
  // content_root/title_id/type_name/
  return root_path_ / title_id_str / content_type_str;
}

std::filesystem::path ContentManager::ResolvePackagePath(
    const XCONTENT_DATA& data) {
  // Content path:
  // content_root/title_id/type_name/data_file_name/
  auto package_root = ResolvePackageRoot(data.content_type);
  auto package_path = package_root / xe::to_path(data.file_name);
  // Add slash to end of path if this is a folder
  // (or package doesn't exist, meaning we're creating a new folder)
  filesystem::FileInfo entry;
  filesystem::GetInfo(package_path, &entry);

  if (!std::filesystem::exists(package_path) ||
      entry.type == filesystem::FileInfo::Type::kDirectory) {
  // Add slash to end of path if this is a folder
  // (or package doesn't exist, meaning we're creating a new folder)
    package_path += xe::kPathSeparator;
  }
  return package_path;
}

std::vector<XCONTENT_DATA> ContentManager::ListContent(uint32_t device_id,
                                                       uint32_t content_type) {
  std::vector<XCONTENT_DATA> result;

  // StfsHeader is a huge class - alloc on heap instead of stack
  vfs::StfsHeader* header = new vfs::StfsHeader();

  // Search path:
  // content_root/title_id/type_name/*
  auto package_root = ResolvePackageRoot(content_type);
  auto file_infos = xe::filesystem::ListFiles(package_root);
  for (const auto& file_info : file_infos) {
    XCONTENT_DATA content_data;
    content_data.device_id = device_id;
    content_data.content_type = content_type;
    content_data.display_name = xe::path_to_utf16(file_info.name);
    content_data.file_name = xe::path_to_utf8(file_info.name);

    auto headers_path = file_info.path / file_info.name;
    if (file_info.type == xe::filesystem::FileInfo::Type::kDirectory) {
      headers_path = headers_path / ContentManager::kStfsHeadersExtension;
    }

    filesystem::FileInfo entry;
    filesystem::GetInfo(headers_path, &entry);

    if (std::filesystem::exists(headers_path)) {
      // File is either package or directory that has .headers file

      if (file_info.type != xe::filesystem::FileInfo::Type::kDirectory) {
        // Not a directory so must be a package, verify size to make sure
        if (file_info.total_size <= vfs::StfsHeader::kHeaderLength) {
          continue;  // Invalid package (maybe .headers.bin)
        }
      }

      auto map = MappedMemory::Open(headers_path, MappedMemory::Mode::kRead, 0,
                                    vfs::StfsHeader::kHeaderLength);
      if (map) {
        if (header->Read(map->data())) {
          content_data.content_type =
              static_cast<uint32_t>(header->content_type);
          content_data.display_name = header->display_names;
          // TODO: select localized display name
          // some games may expect different ones depending on language setting.
        }
        map->Close();
      }
    }
    result.emplace_back(std::move(content_data));
  }

  delete header;

  return result;
}

ContentPackage* ContentManager::ResolvePackage(const XCONTENT_DATA& data) {
  auto package_path = ResolvePackagePath(data);
  if (!std::filesystem::exists(package_path)) {
    return nullptr;
  }

  auto global_lock = global_critical_region_.Acquire();

  for (auto package : open_packages_) {
    if (package->package_path() == package_path) {
      return package;
    }
  }

  std::unique_ptr<ContentPackage> package;

  filesystem::FileInfo entry;
  filesystem::GetInfo(package_path, &entry);
  // Open as FolderContentPackage if the package is a folder or doesn't exist
  if (!std::filesystem::exists(package_path) ||
      entry.type == filesystem::FileInfo::Type::kDirectory) {
    package = std::make_unique<FolderContentPackage>(kernel_state_, data,
                                                     package_path.string());
  } else {
    package = std::make_unique<StfsContentPackage>(kernel_state_, data,
                                                   package_path.string());
  }

  return package.release();
}

bool ContentManager::ContentExists(const XCONTENT_DATA& data) {
  auto path = ResolvePackagePath(data);
  return std::filesystem::exists(path);
}

X_RESULT ContentManager::CreateContent(const std::string_view root_name,
                                       const XCONTENT_DATA& data) {
  auto global_lock = global_critical_region_.Acquire();

  auto package_path = ResolvePackagePath(data);
  if (std::filesystem::exists(package_path)) {
    // Exists, must not!
    return X_ERROR_ALREADY_EXISTS;
  }

  for (auto package : open_packages_) {
    if (package->package_path() == package_path) {
      return X_ERROR_ALREADY_EXISTS;
    }
  }

  if (!xe::filesystem::CreateParentFolder(package_path)) {
    return X_ERROR_ACCESS_DENIED;
  }

  auto package = ResolvePackage(data);
  if (!package) {
    return X_ERROR_FUNCTION_FAILED;  // Failed to create directory?
  }

  if (!package->Mount(root_name.data())) {
    return X_ERROR_DEVICE_NOT_CONNECTED;
  }

  open_packages_.push_back(package);
  return X_ERROR_SUCCESS;
}

X_RESULT ContentManager::OpenContent(const std::string_view root_name,
                                     const XCONTENT_DATA& data) {
  auto global_lock = global_critical_region_.Acquire();

  auto package = ResolvePackage(data);
  if (!package) {
    return X_ERROR_FILE_NOT_FOUND;
  }

  if (!package->Mount(root_name.data())) {
    return X_ERROR_DEVICE_NOT_CONNECTED;
  }

  open_packages_.push_back(package);
  return X_ERROR_SUCCESS;
}

X_RESULT ContentManager::CloseContent(const std::string_view root_name) {
  auto global_lock = global_critical_region_.Acquire();

  for (auto it = open_packages_.begin(); it != open_packages_.end(); ++it) {
    auto& root_names = (*it)->root_names();
    auto root = std::find(root_names.begin(), root_names.end(), root_name);
    if (root != root_names.end()) {
      if ((*it)->Unmount(root_name.data())) {
        delete *it;
        open_packages_.erase(it);
      }
      return X_ERROR_SUCCESS;
    }
  }

  return X_ERROR_FILE_NOT_FOUND;
}

X_RESULT ContentManager::GetContentThumbnail(const XCONTENT_DATA& data,
                                             std::vector<uint8_t>* buffer) {
  auto global_lock = global_critical_region_.Acquire();

  auto package = ResolvePackage(data);
  if (!package) {
    return X_ERROR_FILE_NOT_FOUND;
  }

  return package->GetThumbnail(buffer);
}

X_RESULT ContentManager::SetContentThumbnail(const XCONTENT_DATA& data,
                                             std::vector<uint8_t> buffer) {
  auto global_lock = global_critical_region_.Acquire();

  auto package = ResolvePackage(data);
  if (!package) {
    return X_ERROR_FILE_NOT_FOUND;
  }

  return package->SetThumbnail(buffer);
}

X_RESULT ContentManager::DeleteContent(const XCONTENT_DATA& data) {
  auto global_lock = global_critical_region_.Acquire();

  auto package = ResolvePackage(data);
  if (!package) {
    return X_ERROR_FILE_NOT_FOUND;
  }

  auto result = package->Delete();
  if (XSUCCEEDED(result)) {
    auto it = std::find(open_packages_.begin(), open_packages_.end(), package);
    if (it != open_packages_.end()) {
      open_packages_.erase(it);
    }

    delete package;
  }

  return result;
}

std::filesystem::path ContentManager::ResolveGameUserContentPath() {
  auto title_id = fmt::format("{:8X}", kernel_state_->title_id());
  auto user_name = xe::to_path(kernel_state_->user_profile()->name());

  // Per-game per-profile data location:
  // content_root/title_id/profile/user_name
  return root_path_ / title_id / kGameUserContentDirName / user_name;
}

}  // namespace xam
}  // namespace kernel
}  // namespace xe
