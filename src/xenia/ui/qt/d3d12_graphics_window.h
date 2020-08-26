/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_D3D12_GRAPHICS_WINDOW_H_
#define XENIA_UI_QT_D3D12_GRAPHICS_WINDOW_H_

#include "graphics_window.h"

namespace xe {
namespace ui {
namespace qt {

class D3D12GraphicsWindow : public GraphicsWindowImpl<> {
  Q_OBJECT
  D3D12GraphicsWindow(Emulator* emulator) : GraphicsWindowImpl(emulator) {}

 public:
  bool Initialize() override;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif
