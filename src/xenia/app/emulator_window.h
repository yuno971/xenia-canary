/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_MAIN_WINDOW_H_
#define XENIA_APP_MAIN_WINDOW_H_

#include "xenia/emulator.h"
#include "xenia/ui/graphics_context.h"
#include "xenia/ui/qt/graphics_window.h"
#include "xenia/ui/qt/loop_qt.h"
#include "xenia/ui/qt/window_qt.h"

namespace xe {
namespace app {

class VulkanWindow;
class VulkanRenderer;

using ui::Loop;
using ui::qt::GraphicsWindow;
using ui::qt::QtWindow;

class EmulatorWindow : public QtWindow {
  Q_OBJECT

 public:
  EmulatorWindow(Loop* loop, const std::string& title);

  bool Launch(const std::string& path);

  Emulator* emulator() const { return emulator_.get(); }
  GraphicsWindow* graphics_window() const { return graphics_window_.get(); }
  hid::InputSystem* input_system() const { return input_system_.get(); }
 private:
  bool Initialize() override;
  void CreateMenuBar();

  std::unique_ptr<Emulator> emulator_;
  std::unique_ptr<GraphicsWindow> graphics_window_;
  std::unique_ptr<hid::InputSystem> input_system_;
};

}  // namespace app
}  // namespace xe

#endif  // XENIA_UI_QT_MAIN_WINDOW_H_
