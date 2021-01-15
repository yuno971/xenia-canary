/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_KERNEL_XAM_CONTENT_MANAGER_H_
#define XENIA_KERNEL_XAM_CONTENT_MANAGER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "xenia/base/memory.h"
#include "xenia/base/mutex.h"
#include "xenia/base/string_key.h"
#include "xenia/base/string_util.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {
class KernelState;
}  // namespace kernel
}  // namespace xe

namespace xe {
namespace kernel {
namespace xam {

class ContentPackage;

struct XCONTENT_DATA {
  static const size_t kSize = 4 + 4 + 128 * 2 + 42 + 2;  // = 306 + 2b padding
  uint32_t device_id;
  uint32_t content_type;
  std::u16string display_name;  // 128 chars
  std::string file_name;

  XCONTENT_DATA() = default;
  explicit XCONTENT_DATA(const uint8_t* ptr) {
    device_id = xe::load_and_swap<uint32_t>(ptr + 0);
    content_type = xe::load_and_swap<uint32_t>(ptr + 4);
    display_name = xe::load_and_swap<std::u16string>(ptr + 8);
    file_name = xe::load_and_swap<std::string>(ptr + 8 + 128 * 2);
  }

  void Write(uint8_t* ptr) {
    xe::store_and_swap<uint32_t>(ptr + 0, device_id);
    xe::store_and_swap<uint32_t>(ptr + 4, content_type);
    xe::store_and_swap<std::u16string>(ptr + 8, display_name);
    xe::store_and_swap<std::string>(ptr + 8 + 128 * 2, file_name);
  }

  void Write(XCONTENT_DATA* data) const {
    data->device_id = device_id;
    data->content_type = content_type;
    data->display_name = display_name;
    data->file_name = file_name;
  }
};

struct XCONTENT_AGGREGATE_DATA {
  be<uint32_t> device_id;
  be<uint32_t> content_type;
  union {
    be<uint16_t> display_name[128];
    char16_t display_name_chars[128];
  };
  char file_name[42];
  uint8_t padding[2];
  be<uint32_t> title_id;
};
static_assert_size(XCONTENT_AGGREGATE_DATA, 312);

struct ContentAggregateData {
  uint32_t device_id;
  uint32_t content_type;
  std::u16string display_name;
  std::string file_name;
  uint32_t title_id;

  ContentAggregateData() = default;

  explicit ContentAggregateData(const XCONTENT_AGGREGATE_DATA& data) {
    device_id = data.device_id;
    content_type = data.content_type;
    display_name = xe::load_and_swap<std::u16string>(data.display_name);
    file_name = xe::load_and_swap<std::string>(data.file_name);
    title_id = data.title_id;
  }

  void Write(XCONTENT_AGGREGATE_DATA* data) const {
    data->device_id = device_id;
    data->content_type = content_type;
    xe::string_util::copy_and_swap_truncating(
        data->display_name_chars, display_name,
        xe::countof(data->display_name_chars));
    xe::string_util::copy_maybe_truncating<
        string_util::Safety::IKnowWhatIAmDoing>(data->file_name, file_name,
                                                xe::countof(data->file_name));
    data->title_id = title_id;
  }
};

class ContentManager {
 public:
  // Extension to append to folder path when searching for STFS headers
  static constexpr const wchar_t* const kStfsHeadersExtension = L".headers.bin";

  ContentManager(KernelState* kernel_state,
                 const std::filesystem::path& root_path);
  ~ContentManager();

  std::vector<XCONTENT_DATA> ListContent(uint32_t device_id,
                                         uint32_t content_type);

  ContentPackage* ResolvePackage(const XCONTENT_DATA& data);

  bool ContentExists(const XCONTENT_DATA& data);
  X_RESULT CreateContent(const std::string_view root_name,
                         const XCONTENT_DATA& data);
  X_RESULT OpenContent(const std::string_view root_name,
                       const XCONTENT_DATA& data);
  X_RESULT CloseContent(const std::string_view root_name);
  X_RESULT GetContentThumbnail(const XCONTENT_DATA& data,
                               std::vector<uint8_t>* buffer);
  X_RESULT SetContentThumbnail(const XCONTENT_DATA& data,
                               std::vector<uint8_t> buffer);
  X_RESULT DeleteContent(const XCONTENT_DATA& data);
  std::filesystem::path ResolveGameUserContentPath();
  void CloseOpenedFilesFromContent(const std::string_view root_name);

  void SetTitleIdOverride(uint32_t title_id) { title_id_override_ = title_id; }

 private:
  std::filesystem::path ResolvePackageRoot(uint32_t content_type);
  std::filesystem::path ResolvePackagePath(const XCONTENT_DATA& data);

  uint32_t title_id();

  KernelState* kernel_state_;
  std::filesystem::path root_path_;

  // TODO(benvanik): remove use of global lock, it's bad here!
  xe::global_critical_region global_critical_region_;
  std::vector<ContentPackage*> open_packages_;

  uint32_t title_id_override_ =
      0;  // can be used for games/apps that request content for other IDs
};

}  // namespace xam
}  // namespace kernel
}  // namespace xe

#endif  // XENIA_KERNEL_XAM_CONTENT_MANAGER_H_