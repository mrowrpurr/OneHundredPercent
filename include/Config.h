#pragma once

#include <filesystem>

namespace Config {

    constexpr auto COSAVE_ID      = 'MpHc';
    constexpr auto COSAVE_VERSION = 1;

    constexpr auto QUEST_EDITOR_ID = "MP_OneHundredPercent_Quest";

    const std::filesystem::path JSON_FILES_FOLDER = "Data/SKSE/Plugins/OneHundredPercent";
    const std::filesystem::path INI_FILE_PATH     = "Data/SKSE/Plugins/OneHundredPercent.toml";
}