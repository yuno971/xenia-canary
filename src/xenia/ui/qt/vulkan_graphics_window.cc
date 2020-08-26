/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "vulkan_graphics_window.h"

#include <QVulkanWindow>

#include "xenia/gpu/command_processor.h"
#include "xenia/gpu/vulkan/vulkan_graphics_system.h"
#include "xenia/ui/vulkan/vulkan_instance.h"
#include "xenia/ui/vulkan/vulkan_provider.h"

namespace xe {
namespace ui {
namespace qt {

class VulkanRenderer : public QVulkanWindowRenderer {
 public:
  VulkanRenderer(VulkanGraphicsWindow* window,
                 gpu::vulkan::VulkanGraphicsSystem* graphics_system)
      : window_(window), graphics_system_(graphics_system) {}

  void startNextFrame() override {
    // NEED TO DO STUFF HERE IDK
  }

 private:
  VulkanGraphicsWindow* window_;
  gpu::vulkan::VulkanGraphicsSystem* graphics_system_;
};

QVulkanWindowRenderer* VulkanGraphicsWindow::createRenderer() {
  auto graphics_system = reinterpret_cast<gpu::vulkan::VulkanGraphicsSystem*>(
      emulator()->graphics_system());
  return new VulkanRenderer(
      this, graphics_system);
}

bool VulkanGraphicsWindow::Initialize() {
  // copied from DrChat's original Qt branch
  auto provider = reinterpret_cast<ui::vulkan::VulkanProvider*>(
      emulator()->graphics_system()->provider());

  // Create a Qt wrapper around our vulkan instance.
  vulkan_instance_.setVkInstance(*provider->instance());
  if (!vulkan_instance_.create()) {
    return false;
  }
  setVulkanInstance(&vulkan_instance_);

  return true;
}

}  // namespace qt
}  // namespace ui
}  // namespace xe
