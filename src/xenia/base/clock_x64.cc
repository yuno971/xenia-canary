/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2019 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/platform.h"

#if XE_ARCH_AMD64

#include "xenia/base/clock.h"

// Wrap all these different cpu compiler intrinsics.
// So no inline assembler here and the compiler will remove the clutter.
#if XE_COMPILER_MSVC
#include <intrin.h>
#define xe_cpu_cpuid(level, eax, ebx, ecx, edx) \
  {                                             \
    int __xe_cpuid_registers_[4];               \
    __cpuid(__xe_cpuid_registers_, (level));    \
    (eax) = __xe_cpuid_registers_[0];           \
    (ebx) = __xe_cpuid_registers_[1];           \
    (ecx) = __xe_cpuid_registers_[2];           \
    (edx) = __xe_cpuid_registers_[3];           \
  }
#define xe_cpu_rdtsc() __rdtsc()
#elif XE_COMPILER_CLANG
#include <cpuid.h>
#define xe_cpu_cpuid(level, eax, ebx, ecx, edx) \
  __cpuid((level), (eax), (ebx), (ecx), (edx));
#define xe_cpu_rdtsc() __rdtsc()
#elif XE_COMPILER_GNUC
#include <cpuid.h>
#include <x86intrin.h>
#define xe_cpu_cpuid(level, eax, ebx, ecx, edx) \
  __cpuid((level), (eax), (ebx), (ecx), (edx));
#define xe_cpu_rdtsc() __rdtsc()
#else
#error "No cpu instruction wrappers for current compiler implemented."
#endif

namespace xe {

uint64_t Clock::host_tick_frequency_raw() {
  uint32_t eax, ebx, ecx, edx;

  // 00H Get max supported cpuid level.
  xe_cpu_cpuid(0x0, eax, ebx, ecx, edx);
  auto max_cpuid = eax;
  assert(max_cpuid >= 0x16);

  // 15H Get TSC/Crystal ratio and Crystal Hz.
  xe_cpu_cpuid(0x15, eax, ebx, ecx, edx);
  uint64_t ratio_num = ebx;
  uint64_t ratio_den = eax;
  uint64_t cryst_freq = ecx;
  assert(ratio_num && ratio_den);

  // For some CPUs, Crystal frequency is not reported.
  if (cryst_freq) {
    // If it is, calculate the tsr frequency
    return cryst_freq * ratio_num / ratio_den;
  } else {
    // 16H Get CPU base frequency MHz in EAX.
    xe_cpu_cpuid(0x16, eax, ebx, ecx, edx);
    uint64_t cpu_base_freq = static_cast<uint64_t>(eax) * 1000000;
    assert(cpu_base_freq);
    return cpu_base_freq;
  }
}

uint64_t Clock::host_tick_count_raw() { return xe_cpu_rdtsc(); }

}  // namespace xe

#endif
