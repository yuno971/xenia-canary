/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2015 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_VFS_FILE_H_
#define XENIA_VFS_FILE_H_

#include <cstdint>

#include "xenia/xbox.h"

namespace xe {
namespace vfs {

struct CreateOptions {
  // https://processhacker.sourceforge.io/doc/ntioapi_8h.html
  static const uint32_t FILE_DIRECTORY_FILE = 0x00000001;
  // Optimization - files access will be sequential, not random.
  static const uint32_t FILE_SEQUENTIAL_ONLY = 0x00000004;
  static const uint32_t FILE_SYNCHRONOUS_IO_ALERT = 0x00000010;
  static const uint32_t FILE_SYNCHRONOUS_IO_NONALERT = 0x00000020;
  static const uint32_t FILE_NON_DIRECTORY_FILE = 0x00000040;
  // Optimization - file access will be random, not sequential.
  static const uint32_t FILE_RANDOM_ACCESS = 0x00000800;
};

class Entry;

class File {
 public:
  File(uint32_t file_access, Entry* entry)
      : file_access_(file_access), entry_(entry) {}
  virtual ~File() = default;

  virtual void Destroy() = 0;

  virtual X_STATUS ReadSync(void* buffer, size_t buffer_length,
                            size_t byte_offset, size_t* out_bytes_read) = 0;
  virtual X_STATUS WriteSync(const void* buffer, size_t buffer_length,
                             size_t byte_offset, size_t* out_bytes_written) = 0;

  // TODO: Parameters
  virtual X_STATUS ReadAsync(void* buffer, size_t buffer_length,
                             size_t byte_offset, size_t* out_bytes_read) {
    return X_STATUS_NOT_IMPLEMENTED;
  }

  // TODO: Parameters
  virtual X_STATUS WriteAsync(const void* buffer, size_t buffer_length,
                              size_t byte_offset, size_t* out_bytes_written) {
    return X_STATUS_NOT_IMPLEMENTED;
  }

  virtual X_STATUS SetLength(size_t length) { return X_STATUS_NOT_IMPLEMENTED; }

  // xe::filesystem::FileAccess
  uint32_t file_access() const { return file_access_; }
  const Entry* entry() const { return entry_; }
  Entry* entry() { return entry_; }

 protected:
  // xe::filesystem::FileAccess
  uint32_t file_access_ = 0;
  Entry* entry_ = nullptr;
};

}  // namespace vfs
}  // namespace xe

#endif  // XENIA_VFS_FILE_H_