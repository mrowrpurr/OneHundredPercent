#pragma once

#include <toml++/toml.hpp>

struct IniConfig {
    bool          enable_on_screen_messages                        = true;  // New field
    bool          message_on_location_discovered                   = true;
    bool          message_on_location_cleared                      = true;
    std::string   color_on_location_discovered                     = "";
    std::string   color_on_location_cleared                        = "";
    bool          enable_journal                                   = true;  // New field
    bool          show_percentage_in_journal                       = true;
    bool          show_message_for_percentage_in_journal           = true;
    bool          show_recent_locations_in_journal                 = true;
    bool          show_message_for_most_recent_location_in_journal = true;
    std::uint32_t max_recent_locations_in_journal                  = 50;
};

void LoadTomlConfigFile();

inline IniConfig         g_iniConfig;
inline std::atomic<bool> g_iniConfigLoaded = false;

inline IniConfig& GetConfig() {
    LoadTomlConfigFile();
    return g_iniConfig;
}
