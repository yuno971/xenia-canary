/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_KERNEL_PROFILE_MANAGER_H_
#define XENIA_KERNEL_PROFILE_MANAGER_H_

#include <random>
#include <map>
#include <vector>
#include "xenia/kernel/xam/user_profile.h"

namespace xe {
namespace kernel {

class ProfileIOHandler {
 public:
 virtual bool LoadProfile(uint64_t xuid);
 virtual bool SaveProfile(uint64_t xuid);
};

class ProfileIOHandlerToml : public ProfileIOHandler {
 public:
  bool LoadProfile(uint64_t xuid);
  bool SaveProfile(uint64_t xuid);
};

class ProfileManager {
 public:

  ProfileManager(){};
  ~ProfileManager(){};

  void CreateProfile();
  void DeleteProfile(uint64_t xuid);

  void Login(uint64_t xuid, uint32_t user_index);
  void Logout(uint64_t xuid);

  xam::UserProfile* GetProfile(uint32_t user_index);
  xam::UserProfile* GetProfile(uint64_t xuid);

 private:
  static uint64_t GenerateXuid() {
    std::random_device rd;
    std::mt19937 gen(rd());

    return ((uint64_t)0xE03 << 52) + (gen() % (1 << 31));
  }

  void LoadProfile(ProfileIOHandler* io, uint64_t xuid);
  void SaveProfile(ProfileIOHandler* io, uint64_t xuid);


  std::vector<uint32_t> profiles_;
  std::map<uint8_t, xam::UserProfile*> logged_profile_;

  //KernelState* kernel_state_;
};

}  // namespace kernel
}  // namespace xe

#endif  // XENIA_KERNEL_PROFILE_MANAGER_H_