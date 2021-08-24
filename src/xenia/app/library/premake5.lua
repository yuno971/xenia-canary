project_root = "../../../.."
include(project_root.."/tools/build")

group("src")
project("xenia-app-library")
  uuid("bb78039c-0150-4be5-a1d8-e7e00b574b63")
  kind("StaticLib")
  language("C++")
  defines({
  })
  local_platform_files()
