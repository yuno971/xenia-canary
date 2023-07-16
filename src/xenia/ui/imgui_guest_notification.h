/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_IMGUI_GUEST_NOTIFICATION_H_
#define XENIA_UI_IMGUI_GUEST_NOTIFICATION_H_

#include <cmath>
#include <vector>

#include "xenia/base/logging.h"
#include "xenia/base/platform.h"
#include "xenia/ui/imgui_notification.h"

constexpr ImVec2 guest_notification_icon_size = ImVec2(58.0f, 58.0f);
constexpr ImVec2 guest_notification_margin_size = ImVec2(50.f, 5.f);
constexpr float guest_notification_text_scale = 2.3f;
constexpr float guest_notification_rounding = 30.f;
constexpr float guest_font_size = 12.f;

constexpr ImVec4 guest_notification_background_color =
    ImVec4(0.215f, 0.215f, 0.215f, 1.0f);
constexpr ImVec4 guest_notification_border_color = white_color;

const static std::vector<ImVec2> guest_notification_position_id_screen_offset =
    {
        {0.50f, 0.45f},  // CENTER-CENTER - 0
        {0.50f, 0.10f},  // CENTER-TOP - 1
        {0.50f, 0.80f},  // CENTER-BOTTOM - 2
        {NAN, NAN},      // NOT EXIST - 3
        {0.07f, 0.45f},  // LEFT-CENTER - 4
        {0.07f, 0.10f},  // LEFT-TOP - 5
        {0.07f, 0.80f},  // LEFT-BOTTOM - 6
        {NAN, NAN},      // NOT EXIST - 7
        {0.93f, 0.45f},  // RIGHT-CENTER - 8
        {0.93f, 0.10f},  // RIGHT-TOP - 9
        {0.93f, 0.80f}   // RIGHT-BOTTOM - 10
};

namespace xe {
namespace ui {

class ImGuiGuestNotification : public ImGuiNotification {
 public:
  ImGuiGuestNotification(ui::ImGuiDrawer* imgui_drawer, std::string title,
                         std::string description, uint8_t user_index,
                         uint8_t position_id = 0);

  ~ImGuiGuestNotification();

  virtual void Draw() {}
};

class GuestNotificationWindow final : ImGuiGuestNotification {
 public:
  GuestNotificationWindow(ui::ImGuiDrawer* imgui_drawer, std::string title,
                          std::string description, uint8_t user_index,
                          uint8_t position_id = 0)
      : ImGuiGuestNotification(imgui_drawer, title, description, user_index,
                               position_id){};

  void OnDraw(ImGuiIO& io) override;
  void OnUpdate() override;

  virtual const ImVec2 CalculateNotificationSize(ImVec2 text_size,
                                                 float scale) override;

  virtual const ImVec2 CalculateNotificationScreenPosition(
      const ImVec2 screen_size, const ImVec2 window_size,
      const uint8_t notification_position_id) override;

  virtual const NotificationAlignment GetNotificationAlignment(
      const uint8_t notification_position_id) override;
};

}  // namespace ui
}  // namespace xe
#endif
