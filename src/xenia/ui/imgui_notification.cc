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

#include "xenia/base/logging.h"
#include "xenia/base/platform.h"
#include "xenia/ui/imgui_notification.h"

namespace xe {
namespace ui {

ImGuiNotification::ImGuiNotification(ui::ImGuiDrawer* imgui_drawer,
                                     std::string title, std::string description,
                                     uint8_t user_index, uint8_t position_id)
    : imgui_drawer_(imgui_drawer),
      title_(title),
      description_(description),
      user_index_(user_index),
      position_(position_id),
      creation_time_(0),
      current_stage_(NotificationStage::kAwaiting),
      notification_draw_progress_(0.0f) {
  imgui_drawer->AddNotification(this);
}

ImGuiNotification::~ImGuiNotification() {
  imgui_drawer_->RemoveNotification(this);
}

const ImVec2 ImGuiNotification::CalculateNotificationSize(ImVec2 text_size,
                                                          float scale) {
  return ImVec2(180.f * scale, 60.f * scale);
}

const NotificationAlignment ImGuiNotification::GetNotificationAlignment(
    const uint8_t notification_position_id) {
  NotificationAlignment alignment = NotificationAlignment::kAlignUnknown;
  return alignment;
}

const ImVec2 ImGuiNotification::CalculateNotificationScreenPosition(
    const ImVec2 screen_size, const ImVec2 window_size,
    const uint8_t notification_position_id) {
  ImVec2 result = {NAN, NAN};

  if (window_size.x >= screen_size.x || window_size.y >= screen_size.y) {
    return result;
  }

  result.x = std::roundf(screen_size.x * 0.85f);
  result.y = std::roundf(screen_size.y * 0.90f);
  return result;
}

void ImGuiNotification::Draw() { OnDraw(imgui_drawer_->GetIO()); }

void HostNotificationWindow::OnUpdate() {
  switch (current_stage_) {
    case NotificationStage::kAwaiting:
      current_stage_ = NotificationStage::kFazeIn;
      notification_draw_progress_ = 1.0f;
      break;
    case NotificationStage::kFazeIn:
      creation_time_ = Clock::QueryHostUptimeMillis();
      current_stage_ = NotificationStage::kPresent;
      break;
    case NotificationStage::kPresent:
      if (IsNotificationClosingTime()) {
        current_stage_ = NotificationStage::kFazeOut;
      }
      break;
    case NotificationStage::kFazeOut:
      current_stage_ = NotificationStage::kFinished;
      break;
    default:
      break;
  }
}

void HostNotificationWindow::OnDraw(ImGuiIO& io) {
  OnUpdate();

  if (IsNotificationExpired()) {
    delete this;
    return;
  }

  const ImVec2 screen_size = io.DisplaySize;
  const float window_scale =
      std::fminf(screen_size.x / default_drawing_resolution.x,
                 screen_size.y / default_drawing_resolution.y);
  const float font_scale = 12.f / io.Fonts->Fonts[0]->FontSize;
  const ImVec2 text_size = io.Fonts->Fonts[0]->CalcTextSizeA(
      12.f * 1.0f * window_scale, FLT_MAX, -1.0f, GetTitle().c_str());

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

  ImGui::Begin("Host Notification Window", NULL, NOTIFY_TOAST_FLAGS);
  {
    ImGui::Text(GetTitle().c_str());
    ImGui::Separator();
    ImGui::Text(GetDescription().c_str());
  }

  ImGui::End();
}
}  // namespace ui
}  // namespace xe