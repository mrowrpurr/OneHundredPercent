add_rules("mode.debug")
-- add_rules("mode.debug", "mode.release", "mode.releasedbg")

set_languages("c++23")

option("commonlib")
    set_default("skyrim-commonlib-ae")
option_end()

if not has_config("commonlib") then
    return
end

add_repositories("SkyrimScripting     https://github.com/SkyrimScripting/Packages.git")
add_repositories("SkyrimScriptingBeta https://github.com/SkyrimScriptingBeta/Packages.git")
add_repositories("MrowrLib            https://github.com/MrowrLib/Packages.git")

includes("xmake/*.lua")

-- add_requires(get_config("commonlib"))

-- add_requires("skyrim-commonlib-ae")
-- add_requires("skyrim-commonlib-ae 438b23a0016ee9dd2761b380bd103ea960400b7d")

-- Old one which works with RE::DebugNotification (April 2)
-- add_requires("skyrim-commonlib-ae 0701c84")

-- Newer one, does it still work? Still with RE::DebugNotification thought...
-- add_requires("skyrim-commonlib-ae c0debe9e127b1cea431ec21de34fce8324cb1036")

-- Trying a little newer, Sept 7...
-- add_requires("skyrim-commonlib-ae c8e90bc26b07fb394dfcd271cfbee94b95d1c505")

-- Sept 11 ... does it work? .... memory system broken
-- add_requires("skyrim-commonlib-ae a73394e1b3aea58a9050ad59ea6a78e467ba63cc")

-- Later on Sept 11 same or??? ... BGSEntryPointFunctionDataActivateChoice fucked ...
-- add_requires("skyrim-commonlib-ae 7c3377ba984540b60b7e3465be08df5248736920")

-- Sept 13 -- same fucking BGSEntryPointFunctionDataActivateChoice ugh ...
-- add_requires("skyrim-commonlib-ae 8d9ac75bf348e34035ed8b67aaffb7f21e94b20b")

-- 1533381f7e431c2d1c37ace868ffd9442ccbb2c3 Sept 20
-- BGSEntryPointFunctionDataActivateChoice fuckin breaks on this but is ok on Sept 7 ...
-- add_requires("skyrim-commonlib-ae 1533381f7e431c2d1c37ace868ffd9442ccbb2c3")

-- Newer, Oct 7, still uses RE::DebugNotification
-- Won't compile:
-- add_requires("skyrim-commonlib-ae 70a45aea9b835ba66edf259ac5595eda01d77fba")

-- Oct 24 ...  almost the latest ... compiles with the new Hud stuff but that CTDs still ...
-- add_requires("skyrim-commonlib-ae c46b8a39d955071eec41d60e722f72b45d5b4b5a")

-- Back to latest
add_requires("skyrim-commonlib-ae")

add_requires("SkyrimScripting.Plugin", { configs = { commonlib = get_config("commonlib") } })
add_requires(
    "collections",
    "unordered_dense",
    "nlohmann_json",
    "toml++"
)

-- target("Build Papyrus Scripts")
--     set_kind("phony")
--     compile_papyrus_scripts()
    
skse_plugin({
    name = "One Hundred Percent",
    version = "1.0.3",
    author = "Mrowr Purr",
    email = "mrowr.purr@gmail.com",
    mod_files = {"Scripts", "OneHundredPercent.esp", "SKSE"},
    -- deps = {"Build Papyrus Scripts"},
    packages = {"SkyrimScripting.Plugin", "collections", "unordered_dense", "nlohmann_json", "toml++"},
})
