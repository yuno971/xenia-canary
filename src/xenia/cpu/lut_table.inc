/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

// This is a partial file designed to be included by other files when
// constructing various tables.

#ifndef XE_LUT
#define XE_LUT(name)
#define __XE_LUT_UNSET
#endif

XE_LUT(vexptefp)
XE_LUT(vlogefp)
XE_LUT(vrefp)
XE_LUT(vrsqrtefp)

#ifdef __XE_LUT_UNSET
#undef __XE_LUT_UNSET
#undef XE_LUT
#endif
