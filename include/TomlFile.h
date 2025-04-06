#pragma once

#include <toml++/toml.hpp>

struct IniConfig {
    bool          enable_notifications                             = true;  // New field
    bool          on_location_discovered_notification              = true;
    bool          on_location_cleared_notification                 = true;
    std::string   on_location_discovered_notification_color        = "";
    std::string   on_location_cleared_notification_color           = "";
    bool          enable_journal                                   = true;  // New field
    bool          show_percentage_in_journal                       = true;
    bool          show_message_for_percentage_in_journal           = true;
    bool          show_recent_locations_in_journal                 = true;
    bool          show_message_for_most_recent_location_in_journal = true;
    std::uint32_t max_recent_locations_in_journal                  = 50;
    bool          enable_on_screen_messages                        = true;
    bool          show_percentage_updates_on_screen                = true;
    bool          show_message_for_percentage_on_screen            = true;
    bool          show_message_for_most_recent_location_on_screen  = true;
    bool          show_recently_discovered_locations_on_screen     = true;
};

void LoadTomlConfigFile();

inline IniConfig         g_iniConfig;
inline std::atomic<bool> g_iniConfigLoaded = false;

inline IniConfig& GetConfig() {
    LoadTomlConfigFile();
    return g_iniConfig;
}
