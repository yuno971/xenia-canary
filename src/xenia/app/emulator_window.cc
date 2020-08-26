/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2018 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/app/emulator_window.h"

#include <QVulkanWindow>

#include "emulator_window.h"
#include "third_party/imgui/imgui.h"
#include "xenia/apu/nop/nop_audio_system.h"
#include "xenia/apu/sdl/sdl_audio_system.h"
#include "xenia/apu/xaudio2/xaudio2_audio_system.h"
#include "xenia/base/cvar.h"
#include "xenia/base/debugging.h"
#include "xenia/base/factory.h"
#include "xenia/base/logging.h"
#include "xenia/base/platform.h"
#include "xenia/base/profiling.h"
#include "xenia/base/threading.h"
#include "xenia/emulator.h"
#include "xenia/gpu/command_processor.h"
#include "xenia/gpu/d3d12/d3d12_graphics_system.h"
#include "xenia/gpu/graphics_system.h"
#include "xenia/gpu/null/null_graphics_system.h"
#include "xenia/gpu/vulkan/vulkan_graphics_system.h"
#include "xenia/hid/input_system.h"
#include "xenia/hid/nop/nop_hid.h"
#include "xenia/hid/sdl/sdl_hid.h"
#include "xenia/hid/winkey/winkey_hid.h"
#include "xenia/hid/xinput/xinput_hid.h"

DEFINE_string(apu, "any", "Audio system. Use: [any, nop, xaudio2]", "General");
DEFINE_string(gpu, "any", "Graphics system. Use: [any, vulkan, null]",
              "General");
DEFINE_string(hid, "any", "Input system. Use: [any, nop, winkey, xinput]",
              "General");

DEFINE_string(target, "", "Specifies the target .xex or .iso to execute.",
              "General");
DEFINE_bool(fullscreen, false, "Toggles fullscreen", "General");

namespace xe {
namespace app {

std::unique_ptr<apu::AudioSystem> CreateAudioSystem(cpu::Processor* processor) {
  Factory<apu::AudioSystem, cpu::Processor*> factory;
#if XE_PLATFORM_WIN32
  factory.Add<apu::xaudio2::XAudio2AudioSystem>("xaudio2");
#endif  // XE_PLATFORM_WIN32
  factory.Add<apu::sdl::SDLAudioSystem>("sdl");
  factory.Add<apu::nop::NopAudioSystem>("nop");
  return factory.Create(cvars::apu, processor);
}

std::unique_ptr<gpu::GraphicsSystem> CreateGraphicsSystem() {
  Factory<gpu::GraphicsSystem> factory;
#if XE_PLATFORM_WIN32
  factory.Add<gpu::d3d12::D3D12GraphicsSystem>("d3d12");
#endif  // XE_PLATFORM_WIN32
  factory.Add<gpu::vulkan::VulkanGraphicsSystem>("vulkan");
  factory.Add<gpu::null::NullGraphicsSystem>("null");
  return factory.Create(cvars::gpu);
}

std::vector<std::unique_ptr<hid::InputDriver>> CreateInputDrivers(
    ui::Window* window) {
  std::vector<std::unique_ptr<hid::InputDriver>> drivers;
  if (cvars::hid.compare("nop") == 0) {
    drivers.emplace_back(hid::nop::Create(window));
  } else {
    Factory<hid::InputDriver, ui::Window*> factory;
#if XE_PLATFORM_WIN32
    factory.Add("xinput", hid::xinput::Create);
    // WinKey input driver should always be the last input driver added!
    factory.Add("winkey", hid::winkey::Create);
#endif  // XE_PLATFORM_WIN32
    factory.Add("sdl", hid::sdl::Create);
    for (auto& driver : factory.CreateAll(cvars::hid, window)) {
      if (XSUCCEEDED(driver->Setup())) {
        drivers.emplace_back(std::move(driver));
      }
    }
    if (drivers.empty()) {
      // Fallback to nop if none created.
      drivers.emplace_back(xe::hid::nop::Create(window));
    }
  }
  return drivers;
}

EmulatorWindow::EmulatorWindow(Loop* loop, const std::string& title)
    : QtWindow(loop, title) {
  // TODO(DrChat): Pass in command line arguments.
  emulator_ = std::make_unique<xe::Emulator>("", "", "");

  X_STATUS result = emulator_->Setup(this, CreateAudioSystem,
                                     CreateGraphicsSystem, CreateInputDrivers);
  if (result == X_STATUS_SUCCESS) {
    // Setup a callback called when the emulator wants to swap.
    emulator_->graphics_system()->command_processor()->set_swap_request_handler(
        [&]() {
          auto graphics_window =
              reinterpret_cast<QWindow*>(this->graphics_window_.get());
          QMetaObject::invokeMethod(graphics_window, "requestUpdate",
                                    Qt::QueuedConnection);
        });
  }

  if (!EmulatorWindow::Initialize()) {
    return;
  }

  // Set a callback on launch
  emulator_->on_launch.AddListener(
      [this](unsigned int title_id, std ::string_view title) {
        auto title_db = this->emulator()->game_data();
        if (title_db) {
          QPixmap p;
          auto icon_block = title_db->icon();
          if (icon_block.buffer &&
              p.loadFromData(icon_block.buffer, uint(icon_block.size), "PNG")) {
            this->setWindowIcon(QIcon(p));
          }
        }
      });
}

bool EmulatorWindow::Initialize() {
  if (!graphics_window_->Initialize()) {
    XELOGE("Could not initialize graphics window");
    return false;
  }
  // Now set the graphics window as our central widget.
  QWidget* wrapper = QWidget::createWindowContainer(
      dynamic_cast<QWindow*>(graphics_window_.get()));
  setCentralWidget(wrapper);

  return true;
}

bool EmulatorWindow::Launch(const std::string& path) {
  return emulator_->LaunchPath(path) == X_STATUS_SUCCESS;
}

}  // namespace app
}  // namespace xe
