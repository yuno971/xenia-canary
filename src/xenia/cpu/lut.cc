/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/cpu/lut.h"
#include "xenia/base/assert.h"
#include "xenia/base/logging.h"
#include "xenia/base/memory.h"
#include "xenia/base/platform.h"
#include "xenia/cpu/cpu_flags.h"

#include <lz4frame.h>

#include <cstdlib>
#include <fstream>
#include <thread>

namespace xe::cpu {

LUT::LUT() {
  std::vector<std::thread> loaders;

#define XE_LUT(name)                                               \
  lut_##name##_ = nullptr;                                         \
  if (cvars::lut_##name) {                                         \
    loaders.push_back(std::thread([&]() {                          \
      load_lut(std::filesystem::path("luts") / (#name ".lut.lz4"), \
               lut_##name##_);                                     \
    }));                                                           \
  }
#include "xenia/cpu/lut_table.inc"
#undef XE_LUT

  for (auto& loader : loaders) {
    loader.join();
  }
}

LUT::~LUT() {
#define XE_LUT(name)                        \
  if (lut_##name##_) {                      \
    xe::memory::AlignedFree(lut_##name##_); \
  }
#include "xenia/cpu/lut_table.inc"
#undef XE_LUT
}

LUT* LUT::singleton_ = nullptr;
std::mutex LUT::construct_mutex_;
LUT& LUT::getInstance() {
  if (!singleton_) {
    std::unique_lock<std::mutex> lock(construct_mutex_, std::defer_lock);
    if (lock.try_lock()) {
      singleton_ = new LUT();
    } else {
      // Wait until another thread has finished construction.
      lock.lock();
    }
  }
  return *singleton_;
}

const char* load_err = "Unable to load LUT";

void LUT::load_lut(const std::filesystem::path& file, float*& lut) {
  std::ifstream lutfile(file, std::ios::binary | std::ios::in);

  if (lutfile.fail()) {
    xe::FatalError(fmt::format("{}, file '{}' not found", load_err,
                               xe::path_to_utf8(file)));
  }

  load_lut(lutfile, lut);
}

// This is basically just decompression into memory plus endian swap:
void LUT::load_lut(std::istream& in, float*& lut) {
  assert_null(lut);
  LZ4F_errorCode_t lz4f_err;
  std::vector<char> file_buf;
  auto size = sizeof(float) * (size_t(1) << 32);

  auto lut_ = xe::memory::AlignedAlloc(size, 16);
  assert_zero(reinterpret_cast<uintptr_t>(lut_) & 0xF);
  if (!lut_) {
    xe::FatalError(
        fmt::format("{}, could not allocate 16GB of memory", load_err));
  }

  LZ4F_dctx* dctx;
  lz4f_err = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
  if (LZ4F_isError(lz4f_err)) {
    xe::FatalError(fmt::format("{}, could not create decompression context: {}",
                               load_err, LZ4F_getErrorName(lz4f_err)));
  }

  uint8_t* lut_cur = reinterpret_cast<uint8_t*>(lut_);
  void* lut_swap_cur = lut_cur;
  size_t lut_remains = size;
  while (!in.eof()) {
    if (!in.good()) {
      xe::FatalError(fmt::format("{}, istream broken", load_err));
    }
    {
      auto residual_count = file_buf.size();
      file_buf.resize(1024 * 1024);
      in.read(&file_buf[residual_count], file_buf.size() - residual_count);
      file_buf.resize(residual_count + in.gcount());
    }

    {
      size_t read = file_buf.size();
      size_t written = lut_remains;
      lz4f_err = LZ4F_decompress(dctx, lut_cur, &written, file_buf.data(),
                                 &read, nullptr);
      if (LZ4F_isError(lz4f_err)) {
        xe::FatalError(fmt::format("{}, decompression error: {}", load_err,
                                   LZ4F_getErrorName(lz4f_err)));
      }

      file_buf.erase(file_buf.begin(), file_buf.begin() + read);
      lut_remains -= written;
      lut_cur += written;
    }

    // Swap in the loop in case mem pages will be saved to disk
    if constexpr (std::endian::native == std::endian::little) {
      constexpr uintptr_t swap_alignment_mask = ~uintptr_t(16 - 1);
      uintptr_t lut_swap_cur_new =
          reinterpret_cast<uintptr_t>(lut_cur) & swap_alignment_mask;
      size_t elem_count =
          (lut_swap_cur_new - reinterpret_cast<uintptr_t>(lut_swap_cur)) / 4;
      xe::copy_and_swap_32_aligned(lut_swap_cur, lut_swap_cur, elem_count);
      lut_swap_cur = reinterpret_cast<void*>(lut_swap_cur_new);
    }
  }
  assert_zero(lut_remains);
  assert_true((std::endian::native == std::endian::big) ||
              (lut_cur == lut_swap_cur));

  lz4f_err = LZ4F_freeDecompressionContext(dctx);
  assert_zero(lz4f_err);
  lut = reinterpret_cast<float*>(lut_);
}

}  // namespace xe::cpu
