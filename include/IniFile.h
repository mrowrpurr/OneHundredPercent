#pragma once

#include <toml++/toml.hpp>

struct IniConfig {
    bool notification_on_location_discovered = true;
    bool notification_on_location_cleared    = true;
    bool percentage_based_message_in_journal = true;
};

void LoadIni();

inline IniConfig         g_iniConfig;
inline std::atomic<bool> g_iniConfigLoaded = false;

inline IniConfig& GetIni() {
    LoadIni();
    return g_iniConfig;
}
