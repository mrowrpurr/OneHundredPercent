#pragma once

#include <toml++/toml.hpp>

struct IniConfig {
    bool message_on_location_discovered                   = true;
    bool message_on_location_cleared                      = true;
    bool percentage_based_message_in_journal              = true;
    bool show_silly_message_in_journal                    = true;
    bool show_recent_locations_in_journal                 = true;
    bool show_message_for_most_recent_location_in_journal = true;
};

void LoadIni();

inline IniConfig         g_iniConfig;
inline std::atomic<bool> g_iniConfigLoaded = false;

inline IniConfig& GetIni() {
    LoadIni();
    return g_iniConfig;
}
