/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <cmath>
#include <vector>

#include "xenia/base/platform.h"
#include "xenia/ui/imgui_guest_notification.h"
#include "xenia/ui/imgui_notification.h"

#if XE_PLATFORM_WIN32
#include <playsoundapi.h>
#endif

DEFINE_string(notification_sound_path, "",
              "Path (including filename) to selected notification sound. Sound "
              "MUST be in wav format!",
              "General");

namespace xe {
namespace ui {

ImGuiGuestNotification::ImGuiGuestNotification(ui::ImGuiDrawer* imgui_drawer,
                                               std::string title,
                                               std::string description,
                                               uint8_t user_index,
                                               uint8_t position_id)
    : ImGuiNotification(imgui_drawer, title, description, user_index,
                        position_id) {}

ImGuiGuestNotification::~ImGuiGuestNotification() {}

void GuestNotificationWindow::OnUpdate() {
  switch (current_stage_) {
    case NotificationStage::kAwaiting:
      // TODO(Gliniak): Implement delayed notifications.
      current_stage_ = NotificationStage::kFazeIn;
      notification_draw_progress_ = 0.2f;
#if XE_PLATFORM_WIN32
      if (!cvars::notification_sound_path.empty()) {
        auto notification_sound_path = cvars::notification_sound_path;
        if (std::filesystem::exists(notification_sound_path)) {
          PlaySound(std::wstring(notification_sound_path.begin(),
                                 notification_sound_path.end())
                        .c_str(),
                    NULL,
                    SND_FILENAME | SND_NODEFAULT | SND_NOSTOP | SND_ASYNC);
        }
      }
#endif

      break;
    case NotificationStage::kFazeIn: {
      creation_time_ = Clock::QueryHostUptimeMillis();
      if (notification_draw_progress_ < 1.1f) {
        notification_draw_progress_ += 0.02f;
      }

      // Mimics a bit original console behaviour when it makes window a bit
      // longer for few frames then decreases size
      if (notification_draw_progress_ >= 1.1f) {
        current_stage_ = NotificationStage::kPresent;
        notification_draw_progress_ = 1.0f;
      }
      break;
    }
    case NotificationStage::kPresent:
      if (IsNotificationClosingTime()) {
        current_stage_ = NotificationStage::kFazeOut;
      }
      break;
    case NotificationStage::kFazeOut: {
      if (notification_draw_progress_ > 0.2f) {
        notification_draw_progress_ -= 0.02f;
      } else {
        current_stage_ = NotificationStage::kFinished;
      }
      break;
    }
    default:
      break;
  }
}

void GuestNotificationWindow::OnDraw(ImGuiIO& io) {
  OnUpdate();

  if (IsNotificationExpired()) {
    delete this;
    return;
  }

  const std::string longest_notification_text_line =
      GetTitle().size() > GetDescription().size() ? GetTitle().c_str()
                                                  : GetDescription().c_str();

  const ImVec2 screen_size = io.DisplaySize;
  const float window_scale =
      std::fminf(screen_size.x / default_drawing_resolution.x,
                 screen_size.y / default_drawing_resolution.y);
  const float font_scale = guest_font_size / io.Fonts->Fonts[0]->FontSize;
  const ImVec2 text_size = io.Fonts->Fonts[0]->CalcTextSizeA(
      guest_font_size * guest_notification_text_scale * window_scale, FLT_MAX,
      -1.0f, longest_notification_text_line.c_str());

  const ImVec2 final_notification_size =
      CalculateNotificationSize(text_size, window_scale);

  const ImVec2 notification_position = CalculateNotificationScreenPosition(
      screen_size, final_notification_size, GetPositionId());

  if (isnan(notification_position.x) || isnan(notification_position.y)) {
    return;
  }

  ImVec2 current_notification_size = final_notification_size;
  current_notification_size.x *= notification_draw_progress_;
  current_notification_size.x = std::floorf(current_notification_size.x);

  // Initialize position and window size
  ImGui::SetNextWindowSize(current_notification_size);
  ImGui::SetNextWindowPos(notification_position);

  // Set new window style before drawing window
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,
                      guest_notification_rounding * window_scale);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, guest_notification_background_color);
  ImGui::PushStyleColor(ImGuiCol_Separator,
                        guest_notification_background_color);
  ImGui::PushStyleColor(ImGuiCol_Border, guest_notification_border_color);

  ImGui::Begin("Guest Notification Window", NULL, NOTIFY_TOAST_FLAGS);
  {
    ImGui::SetWindowFontScale(guest_notification_text_scale * font_scale *
                              window_scale);
    // Set offset to image to prevent it from being right on border.
    ImGui::SetCursorPos(ImVec2(final_notification_size.x * 0.005f,
                               final_notification_size.y * 0.05f));
    // Elements of window
    ImGui::Image(reinterpret_cast<ImTextureID>(
                     GetDrawer()->GetNotificationIcon(GetUserIndex())),
                 ImVec2(guest_notification_icon_size.x * window_scale,
                        guest_notification_icon_size.y * window_scale));

    ImGui::SameLine();
    if (notification_draw_progress_ > 0.5f) {
      ImGui::TextColored(white_color, GetNotificationText().c_str());
    }
  }
  // Restore previous style
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(3);

  ImGui::End();
}

const ImVec2 GuestNotificationWindow::CalculateNotificationSize(
    ImVec2 text_size, float scale) {
  const ImVec2 result = ImVec2(std::floorf((guest_notification_icon_size.x +
                                            guest_notification_margin_size.x) *
                                           scale) +
                                   text_size.x,
                               std::floorf((guest_notification_icon_size.y +
                                            guest_notification_margin_size.y) *
                                           scale));

  return result;
}

const NotificationAlignment GuestNotificationWindow::GetNotificationAlignment(
    const uint8_t notification_position_id) {
  NotificationAlignment alignment = NotificationAlignment::kAlignUnknown;

  if (notification_position_id >=
      guest_notification_position_id_screen_offset.size()) {
    return alignment;
  }

  const ImVec2 screen_offset =
      guest_notification_position_id_screen_offset.at(notification_position_id);

  if (screen_offset.x < 0.3f) {
    alignment = NotificationAlignment::kAlignLeft;
  } else if (screen_offset.x > 0.7f) {
    alignment = NotificationAlignment::kAlignRight;
  } else {
    alignment = NotificationAlignment::kAlignCenter;
  }

  return alignment;
}

const ImVec2 GuestNotificationWindow::CalculateNotificationScreenPosition(
    const ImVec2 screen_size, const ImVec2 window_size,
    const uint8_t notification_position_id) {
  ImVec2 result = {NAN, NAN};

  if (window_size.x >= screen_size.x || window_size.y >= screen_size.y) {
    return result;
  }

  const NotificationAlignment alignment =
      GetNotificationAlignment(notification_position_id);

  if (alignment == NotificationAlignment::kAlignUnknown) {
    return result;
  }

  const ImVec2 screen_offset =
      guest_notification_position_id_screen_offset.at(notification_position_id);

  switch (alignment) {
    case NotificationAlignment::kAlignLeft:
      result.x = std::roundf(screen_size.x * screen_offset.x);
      break;

    case NotificationAlignment::kAlignRight:
      result.x = std::roundf((screen_size.x * screen_offset.x) - window_size.x);
      break;

    case NotificationAlignment::kAlignCenter:
      result.x = std::roundf((screen_size.x * 0.5f) - (window_size.x * 0.5f));
      break;

    default:
      break;
  }

  result.y = std::roundf(screen_size.y * screen_offset.y);
  return result;
}

}  // namespace ui
}  // namespace xe