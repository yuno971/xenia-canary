/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_VULKAN_GRAPHICS_WINDOW_H_
#define XENIA_UI_QT_VULKAN_GRAPHICS_WINDOW_H_

#include <QVulkanWindow>
#include "graphics_window.h"

namespace xe {
namespace ui {
namespace qt {

class VulkanGraphicsWindow : public GraphicsWindowImpl<QVulkanWindow> {
  Q_OBJECT
public:
  VulkanGraphicsWindow(Emulator* emulator)
      : GraphicsWindowImpl(emulator) {}

  bool Initialize() override;

 private:
  QVulkanWindowRenderer* createRenderer() override;

  QVulkanInstance vulkan_instance_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif