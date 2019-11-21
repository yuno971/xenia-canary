/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2015 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/clock.h"

#include "xenia/base/platform_win.h"

namespace xe {

uint64_t Clock::host_tick_frequency() {
  if (cvars::clock_source_raw) {
    int id[4];

    // 00H Get max supported cpuid level.
    __cpuid(id, 0x0);
    auto max_cpuid = id[0];  // EAX
    assert(max_cpuid >= 0x16);

    // 15H Get TSC/Crystal ratio and Crystal Hz.
    __cpuid(id, 0x15);
    uint64_t ratio_num = id[1];   // EBX
    uint64_t ratio_den = id[0];   // EAX
    uint64_t cryst_freq = id[2];  // ECX
    assert(ratio_num && ratio_den);

    // For some CPUs, Crystal frequency is not reported.
    if (cryst_freq) {
      // If it is, calculate the tsr frequency
      return cryst_freq * ratio_num / ratio_den;
    } else {
      // 16H Get CPU base frequency MHz in EAX.
      __cpuid(id, 0x16);
      uint64_t cpu_base_freq = id[0] * 1000000ull;
      assert(cpu_base_freq);
      return cpu_base_freq;
    }
  } else {
    static LARGE_INTEGER frequency = {{0}};
    if (!frequency.QuadPart) {
      QueryPerformanceFrequency(&frequency);
    }
    return frequency.QuadPart;
  }
}

uint64_t Clock::QueryHostTickCount() {
  if (cvars::clock_source_raw) {
    return __rdtsc();
  } else {
    LARGE_INTEGER counter;
    uint64_t time = 0;
    if (QueryPerformanceCounter(&counter)) {
      time = counter.QuadPart;
    }
    return time;
  }
}

uint64_t Clock::QueryHostSystemTime() {
  FILETIME t;
  GetSystemTimeAsFileTime(&t);
  return (uint64_t(t.dwHighDateTime) << 32) | t.dwLowDateTime;
}

uint64_t Clock::QueryHostUptimeMillis() {
  return QueryHostTickCount() / (host_tick_frequency() / 1000);
}

}  // namespace xe
