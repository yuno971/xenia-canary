project_root = "../../../.."
include(project_root.."/tools/build")

group("src")
project("xenia-app-settings")
  uuid("d2031dc9-e643-4d46-b252-65c825895a3d")
  kind("StaticLib")
  language("C++")
  defines({
  })
  links({
    "pugixml",
  })
  local_platform_files()
