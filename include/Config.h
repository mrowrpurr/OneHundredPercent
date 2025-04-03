#pragma once

#include <filesystem>

namespace Config {

    constexpr auto COSAVE_ID      = 'MpHc';
    constexpr auto COSAVE_VERSION = 1;

    constexpr auto QUEST_EDITOR_ID           = "MP_HazTheCompletionizt_Quest";
    constexpr auto RECENT_LOCATIONS_FORMLIST = "MP_HazTheCompletionizt_Locations";

    const std::filesystem::path JSON_FILES_FOLDER = "Data/SKSE/Plugins/HazTheCompletionizt";
}