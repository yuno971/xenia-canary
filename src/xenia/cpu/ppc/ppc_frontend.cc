/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/cpu/ppc/ppc_frontend.h"

#include "xenia/base/atomic.h"
#include "xenia/base/logging.h"
#include "xenia/cpu/ppc/ppc_context.h"
#include "xenia/cpu/ppc/ppc_emit.h"
#include "xenia/cpu/ppc/ppc_opcode_info.h"
#include "xenia/cpu/ppc/ppc_translator.h"
#include "xenia/cpu/processor.h"

namespace xe {
namespace cpu {
namespace ppc {

void InitializeIfNeeded();
void CleanupOnShutdown();

void InitializeIfNeeded() {
  static bool has_initialized = false;
  if (has_initialized) {
    return;
  }
  has_initialized = true;

  RegisterEmitCategoryAltivec();
  RegisterEmitCategoryALU();
  RegisterEmitCategoryControl();
  RegisterEmitCategoryFPU();
  RegisterEmitCategoryMemory();

  atexit(CleanupOnShutdown);
}

void CleanupOnShutdown() {}

PPCFrontend::PPCFrontend(Processor* processor) : processor_(processor) {
  InitializeIfNeeded();
}

PPCFrontend::~PPCFrontend() {
  // Force cleanup now before we deinit.
  translator_pool_.Reset();
}

Memory* PPCFrontend::memory() const { return processor_->memory(); }

 void SUB_82B471F8(PPCContext* ppc_context, void* arg0, void* arg1) {
  uint32_t r3 = uint32_t(ppc_context->r[3]);
  uint32_t r10 = uint32_t(ppc_context->r[10]);
  uint32_t r11 = uint32_t(ppc_context->r[11]);

  XELOGE("trapPointCrashUp1 {:08X} {:08X} {:08X}", r3, r10, r11);
}

// XMemCpy
void SUB_82208FAC(PPCContext* ppc_context, void* arg0, void* arg1) {
  uint32_t pDest = uint32_t(ppc_context->r[3]);
  uint32_t pSrc = uint32_t(ppc_context->r[4]);
  uint32_t size = uint32_t(ppc_context->r[5]);
  uint32_t adr = uint32_t(ppc_context->r[9]);

  uint32_t src_c1 = uint32_t(ppc_context->r[11]);
  uint32_t src_c2 = uint32_t(ppc_context->r[26]);
  XELOGE("XMemCpy({:08X}, {:08X}, {:08X} = {:08X} + {:08X})", pDest, pSrc, size,
         src_c1, src_c2);

  if (pSrc == 0x46B9BD34) {
    XELOGE("WTF");
    assert_always();
  }
}

void SUB_82B47708(PPCContext* ppc_context, void* arg0, void* arg1) {
  uint32_t r29 = uint32_t(ppc_context->r[29]);
  XELOGE("Size to allocate: {:08X}", r29);
}

void SUB_82BF6044(PPCContext* ppc_context, void* arg0, void* arg1) {
  uint32_t function_call = uint32_t(ppc_context->r[11]);
  XELOGE("luaZ_fill call to: {:08X}", function_call);
}

void SUB_82BF5FCC(PPCContext* ppc_context, void* arg0, void* arg1) {
  uint32_t zio_ptr = uint32_t(ppc_context->r[3]);
  uint32_t dest_ptr = uint32_t(ppc_context->r[4]);
  uint32_t size = uint32_t(ppc_context->r[5]);

  auto r11 = uint32_t(ppc_context->r[11]);
  XELOGE("lua_mem_copy({:08X}, {:08X}, {:08X})", zio_ptr, dest_ptr, size);
}
    // Checks the state of the global lock and sets scratch to the current MSR
// value.
void CheckGlobalLock(PPCContext* ppc_context, void* arg0, void* arg1) {
  auto global_mutex = reinterpret_cast<std::recursive_mutex*>(arg0);
  auto global_lock_count = reinterpret_cast<int32_t*>(arg1);
  std::lock_guard<std::recursive_mutex> lock(*global_mutex);
  ppc_context->scratch = *global_lock_count ? 0 : 0x8000;
}

// Enters the global lock. Safe to recursion.
void EnterGlobalLock(PPCContext* ppc_context, void* arg0, void* arg1) {
  auto global_mutex = reinterpret_cast<std::recursive_mutex*>(arg0);
  auto global_lock_count = reinterpret_cast<int32_t*>(arg1);
  global_mutex->lock();
  xe::atomic_inc(global_lock_count);
}

// Leaves the global lock. Safe to recursion.
void LeaveGlobalLock(PPCContext* ppc_context, void* arg0, void* arg1) {
  auto global_mutex = reinterpret_cast<std::recursive_mutex*>(arg0);
  auto global_lock_count = reinterpret_cast<int32_t*>(arg1);
  auto new_lock_count = xe::atomic_dec(global_lock_count);
  assert_true(new_lock_count >= 0);
  global_mutex->unlock();
}

void SyscallHandler(PPCContext* ppc_context, void* arg0, void* arg1) {
  uint64_t syscall_number = ppc_context->r[0];
  switch (syscall_number) {
    default:
      assert_unhandled_case(syscall_number);
      XELOGE("Unhandled syscall {}!", syscall_number);
      break;
#pragma warning(suppress : 4065)
  }
}

bool PPCFrontend::Initialize() {
  void* arg0 = reinterpret_cast<void*>(&xe::global_critical_region::mutex());
  void* arg1 = reinterpret_cast<void*>(&builtins_.global_lock_count);

  builtins_.sub_82B471F8 =
      processor_->DefineBuiltin("SUB_82B471F8", SUB_82B471F8, arg0, arg1);


  // XMemCpy
  builtins_.sub_82208FAC =
      processor_->DefineBuiltin("SUB_82208FAC", SUB_82208FAC, arg0, arg1);

  // DEEP SHIT
  builtins_.sub_82B47708 =
      processor_->DefineBuiltin("SUB_82B47708", SUB_82B47708, arg0, arg1);

  // MAIN SHIT
  builtins_.sub_82BF6044 =
      processor_->DefineBuiltin("SUB_82BF6044", SUB_82BF6044, arg0, arg1);
  builtins_.sub_82BF5FCC =
      processor_->DefineBuiltin("SUB_82BF5FCC", SUB_82BF5FCC, arg0, arg1);
  builtins_.check_global_lock =
      processor_->DefineBuiltin("CheckGlobalLock", CheckGlobalLock, arg0, arg1);
  builtins_.enter_global_lock =
      processor_->DefineBuiltin("EnterGlobalLock", EnterGlobalLock, arg0, arg1);
  builtins_.leave_global_lock =
      processor_->DefineBuiltin("LeaveGlobalLock", LeaveGlobalLock, arg0, arg1);
  builtins_.syscall_handler = processor_->DefineBuiltin(
      "SyscallHandler", SyscallHandler, nullptr, nullptr);
  return true;
}

bool PPCFrontend::DeclareFunction(GuestFunction* function) {
  // Could scan or something here.
  // Could also check to see if it's a well-known function type and classify
  // for later.
  // Could also kick off a precompiler, since we know it's likely the function
  // will be demanded soon.
  return true;
}

bool PPCFrontend::DefineFunction(GuestFunction* function,
                                 uint32_t debug_info_flags) {
  auto translator = translator_pool_.Allocate(this);
  bool result = translator->Translate(function, debug_info_flags);
  translator_pool_.Release(translator);
  return result;
}

}  // namespace ppc
}  // namespace cpu
}  // namespace xe
