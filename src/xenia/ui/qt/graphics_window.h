/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_GRAPHICS_WINDOW_H_
#define XENIA_UI_QT_GRAPHICS_WINDOW_H_

#include <QWindow>

#include "xenia/emulator.h"

namespace xe {
namespace ui {
namespace qt {

class GraphicsWindow {
 public:
  GraphicsWindow(Emulator* emulator) : emulator_(emulator) {}
  virtual ~GraphicsWindow() = default;

  Emulator* emulator() const { return emulator_; }
  virtual bool Initialize() = 0;

 private:
  Emulator* emulator_;
};

template <typename T = QWindow>
class GraphicsWindowImpl : public T, public GraphicsWindow {
  static_assert(std::is_base_of_v<QWindow, T>,
                "GraphicsWindow must inherit from a QWindow-based class");
 public:
  GraphicsWindowImpl(Emulator* emulator) : T(), GraphicsWindow(emulator) {}
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif
