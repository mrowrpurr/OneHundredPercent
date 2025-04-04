#include "IniFile.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "Config.h"

void LoadIni() {
    if (g_iniConfigLoaded.exchange(true)) return;

    try {
        if (!std::filesystem::exists(Config::INI_FILE_PATH)) {
            Log("INI file not found at: {}. Using default values.", Config::INI_FILE_PATH.string());
            goto use_defaults;
        }

        auto tomlConfig = toml::parse_file(Config::INI_FILE_PATH.string());
        if (tomlConfig.contains("OnScreenMessages")) {
            auto onScreenMessagesSection               = tomlConfig["OnScreenMessages"];
            g_iniConfig.message_on_location_discovered = onScreenMessagesSection["message_on_location_discovered"].value_or(true);
            g_iniConfig.message_on_location_cleared    = onScreenMessagesSection["message_on_location_cleared"].value_or(true);
        } else {
            Log("'OnScreenMessages' section not found in INI. Using default values.");
            goto use_defaults;
        }

        if (tomlConfig.contains("Journal")) {
            auto journalSection                                          = tomlConfig["Journal"];
            g_iniConfig.percentage_based_message_in_journal              = journalSection["percentage_based_message_in_journal"].value_or(true);
            g_iniConfig.show_silly_message_in_journal                    = journalSection["show_silly_message_in_journal"].value_or(true);
            g_iniConfig.show_recent_locations_in_journal                 = journalSection["show_recent_locations_in_journal"].value_or(true);
            g_iniConfig.show_message_for_most_recent_location_in_journal = journalSection["show_message_for_most_recent_location_in_journal"].value_or(true);
        } else {
            Log("'Journal' section not found in INI. Using default values.");
            goto use_defaults;
        }
    } catch (const toml::parse_error& e) {
        Log("Error parsing INI file: {}", e.what());
        goto use_defaults;
    } catch (const std::exception& e) {
        Log("Unexpected error loading INI file: {}", e.what());
        goto use_defaults;
    }

    Log("INI configuration loaded successfully.");
    goto log_values;

use_defaults:
    g_iniConfig.message_on_location_discovered                   = true;
    g_iniConfig.message_on_location_cleared                      = true;
    g_iniConfig.percentage_based_message_in_journal              = true;
    g_iniConfig.show_silly_message_in_journal                    = true;
    g_iniConfig.show_recent_locations_in_journal                 = true;
    g_iniConfig.show_message_for_most_recent_location_in_journal = true;
    Log("Using default configuration values.");

log_values:
    Log("Current configuration:");
    Log("  message_on_location_discovered: {}", g_iniConfig.message_on_location_discovered);
    Log("  message_on_location_cleared: {}", g_iniConfig.message_on_location_cleared);
    Log("  percentage_based_message_in_journal: {}", g_iniConfig.percentage_based_message_in_journal);
    Log("  show_silly_message_in_journal: {}", g_iniConfig.show_silly_message_in_journal);
    Log("  show_recent_locations_in_journal: {}", g_iniConfig.show_recent_locations_in_journal);
    Log("  show_message_for_most_recent_location_in_journal: {}", g_iniConfig.show_message_for_most_recent_location_in_journal);
}
