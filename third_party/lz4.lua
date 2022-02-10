--
-- On Linux we build against the system version (liblz4-dev for building),
-- since it's basically a system library there.
--

local third_party_path = os.getcwd()

if not os.istarget("linux") then
  -- build ourselves
  include("lz4-static.lua")
end


--
-- Call this function in project scope to make LZ4 available.
--
function lz4_depend()
  filter("platforms:not Linux")
    links({
      "lz4",
    })
    defines({
      "LZ4F_STATIC_LINKING_ONLY",
    })
    includedirs({
      path.getrelative(".", third_party_path) .. "/lz4/lib/",
    })
  filter({})
  -- Linux builds globally depend on lz4
end
