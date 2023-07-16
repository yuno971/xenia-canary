/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_IMGUI_NOTIFICATION_H_
#define XENIA_UI_IMGUI_NOTIFICATION_H_

#include "third_party/imgui/imgui.h"
#include "xenia/ui/imgui_dialog.h"
#include "xenia/ui/imgui_drawer.h"

#define NOTIFY_TOAST_FLAGS                                            \
  ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | \
      ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |            \
      ImGuiWindowFlags_NoBringToFrontOnFocus |                        \
      ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize

// Parameters based on 1280x720 resolution
constexpr ImVec2 default_drawing_resolution = ImVec2(1280.f, 720.f);

constexpr ImVec4 white_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

enum class NotificationAlignment : uint8_t {
  kAlignLeft = 0,
  kAlignRight = 1,
  kAlignCenter = 2,
  kAlignUnknown = 0xFF
};

namespace xe {
namespace ui {
class ImGuiNotification {
 public:
  ImGuiNotification(ui::ImGuiDrawer* imgui_drawer, std::string title,
                    std::string description, uint8_t user_index,
                    uint8_t position_id = 0);

  ~ImGuiNotification();

  void Draw();

 protected:
  enum class NotificationStage : uint8_t {
    kAwaiting = 0,
    kFazeIn = 1,
    kPresent = 2,
    kFazeOut = 3,
    kFinished = 4
  };

  ImGuiDrawer* GetDrawer() { return imgui_drawer_; }

  const bool IsNotificationClosingTime() {
    return Clock::QueryHostUptimeMillis() - creation_time_ > time_to_close_;
  }

  const bool IsNotificationExpired() {
    return current_stage_ == NotificationStage::kFinished;
  }

  const std::string GetNotificationText() {
    std::string text = title_;

    if (!description_.empty()) {
      text.append("\n" + description_);
    }
    return text;
  }

  const std::string GetTitle() { return title_; }
  const std::string GetDescription() { return description_; }

  const uint8_t GetPositionId() { return position_; }
  const uint8_t GetUserIndex() { return user_index_; }

  void SetNewLifeTime(uint32_t ms_to_close) { time_to_close_ = ms_to_close; }

  virtual void OnDraw(ImGuiIO& io) {}
  virtual void OnUpdate() {}

  virtual const ImVec2 CalculateNotificationSize(ImVec2 text_size, float scale);

  virtual const ImVec2 CalculateNotificationScreenPosition(
      const ImVec2 screen_size, const ImVec2 window_size,
      const uint8_t notification_position_id);

  virtual const NotificationAlignment GetNotificationAlignment(
      const uint8_t notification_position_id);

  NotificationStage current_stage_;
  uint64_t creation_time_;
  uint8_t position_;
  float notification_draw_progress_;

 private:
  uint8_t user_index_;

  uint32_t delay_ = 0;
  uint32_t time_to_close_ = 4500;

  std::string title_;
  std::string description_;

  ImGuiDrawer* imgui_drawer_ = nullptr;
};

class HostNotificationWindow final : ImGuiNotification {
 public:
  HostNotificationWindow(ui::ImGuiDrawer* imgui_drawer, std::string title,
                         std::string description, uint8_t user_index,
                         uint8_t position_id = 0)
      : ImGuiNotification(imgui_drawer, title, description, user_index,
                          position_id) {
    SetNewLifeTime(1500);
  }

  void OnDraw(ImGuiIO& io) override;
  void OnUpdate() override;
};

}  // namespace ui
}  // namespace xe

#endif