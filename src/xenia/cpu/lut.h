/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_CPU_LUT_H_
#define XENIA_CPU_LUT_H_

#include <filesystem>
#include <memory>
#include <mutex>
#include <string_view>

namespace xe::cpu {

class LUT {
 private:
  LUT();
  ~LUT();

  static LUT* singleton_;
  static std::mutex construct_mutex_;

 public:
  static LUT& getInstance();

#define XE_LUT(name) \
  float* name() { return lut_##name##_; }
#include "xenia/cpu/lut_table.inc"
#undef XE_LUT

 protected:
  static void load_lut(const std::filesystem::path& file, float*& lut);
  static void load_lut(std::istream& in, float*& lut);

 private:
#define XE_LUT(name) float* lut_##name##_;
#include "xenia/cpu/lut_table.inc"
#undef XE_LUT
};
}  // namespace xe::cpu

#endif
