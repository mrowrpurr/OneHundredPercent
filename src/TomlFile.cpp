#include "TomlFile.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "Config.h"

void LoadTomlConfigFile() {
    if (g_iniConfigLoaded.exchange(true)) return;

    try {
        if (!std::filesystem::exists(Config::INI_FILE_PATH)) {
            Log("TOML config file not found at: {}. Using default values.", Config::INI_FILE_PATH.string());
            goto use_defaults;
        }

        auto tomlConfig = toml::parse_file(Config::INI_FILE_PATH.string());
        if (tomlConfig.contains("Notifications")) {
            auto NotificationsSection                             = tomlConfig["Notifications"];
            g_iniConfig.enable_notifications                      = NotificationsSection["enable_notifications"].value_or(true);
            g_iniConfig.on_location_discovered_notification       = NotificationsSection["on_location_discovered_notification"].value_or(true);
            g_iniConfig.on_location_cleared_notification          = NotificationsSection["on_location_cleared_notification"].value_or(true);
            g_iniConfig.on_location_discovered_notification_color = NotificationsSection["on_location_discovered_notification_color"].value_or("");
            g_iniConfig.on_location_cleared_notification_color    = NotificationsSection["on_location_cleared_notification_color"].value_or("");
        } else {
            Log("'Notifications' section not found in INI. Using default values.");
            goto use_defaults;
        }

        if (tomlConfig.contains("Journal")) {
            auto journalSection                                          = tomlConfig["Journal"];
            g_iniConfig.enable_journal                                   = journalSection["enable_journal"].value_or(true);
            g_iniConfig.show_percentage_in_journal                       = journalSection["show_percentage_in_journal"].value_or(true);
            g_iniConfig.show_message_for_percentage_in_journal           = journalSection["show_message_for_percentage_in_journal"].value_or(true);
            g_iniConfig.show_recent_locations_in_journal                 = journalSection["show_recent_locations_in_journal"].value_or(true);
            g_iniConfig.show_message_for_most_recent_location_in_journal = journalSection["show_message_for_most_recent_location_in_journal"].value_or(true);
            g_iniConfig.max_recent_locations_in_journal                  = std::min(journalSection["max_recent_locations_in_journal"].value_or(500), 500);
        } else {
            Log("'Journal' section not found in INI. Using default values.");
            goto use_defaults;
        }

        if (tomlConfig.contains("OnScreenMessages")) {
            auto onScreenMessagesSection                                = tomlConfig["OnScreenMessages"];
            g_iniConfig.enable_on_screen_messages                       = onScreenMessagesSection["enable_on_screen_messages"].value_or(true);
            g_iniConfig.show_percentage_updates_on_screen               = onScreenMessagesSection["show_percentage_updates_on_screen"].value_or(true);
            g_iniConfig.show_message_for_percentage_on_screen           = onScreenMessagesSection["show_message_for_percentage_on_screen"].value_or(true);
            g_iniConfig.show_message_for_most_recent_location_on_screen = onScreenMessagesSection["show_message_for_most_recent_location_on_screen"].value_or(true);
            g_iniConfig.show_recently_discovered_locations_on_screen    = onScreenMessagesSection["show_recently_discovered_locations_on_screen"].value_or(true);
        } else {
            Log("'OnScreenMessages' section not found in INI. Using default values.");
        }
    } catch (const toml::parse_error& e) {
        Log("Error parsing TOML config file: {}", e.what());
        goto use_defaults;
    } catch (const std::exception& e) {
        Log("Unexpected error loading TOML config file: {}", e.what());
        goto use_defaults;
    }

    Log("INI configuration loaded successfully.");
    goto log_values;

use_defaults:
    g_iniConfig.enable_notifications                             = true;
    g_iniConfig.on_location_discovered_notification              = true;
    g_iniConfig.on_location_cleared_notification                 = true;
    g_iniConfig.on_location_discovered_notification_color        = "";
    g_iniConfig.on_location_cleared_notification_color           = "";
    g_iniConfig.enable_journal                                   = true;
    g_iniConfig.show_percentage_in_journal                       = true;
    g_iniConfig.show_message_for_percentage_in_journal           = true;
    g_iniConfig.show_recent_locations_in_journal                 = true;
    g_iniConfig.show_message_for_most_recent_location_in_journal = true;
    g_iniConfig.max_recent_locations_in_journal                  = 50;
    g_iniConfig.enable_on_screen_messages                        = true;
    g_iniConfig.show_percentage_updates_on_screen                = true;
    g_iniConfig.show_message_for_percentage_on_screen            = true;
    g_iniConfig.show_message_for_most_recent_location_on_screen  = true;
    g_iniConfig.show_recently_discovered_locations_on_screen     = true;
    Log("Using default configuration values.");

log_values:
    Log("Configuration:");
    Log("- enable_notifications: {}", g_iniConfig.enable_notifications);
    Log("- on_location_discovered_notification: {}", g_iniConfig.on_location_discovered_notification);
    Log("- on_location_cleared_notification: {}", g_iniConfig.on_location_cleared_notification);
    Log("- on_location_discovered_notification_color: {}", g_iniConfig.on_location_discovered_notification_color);
    Log("- on_location_cleared_notification_color: {}", g_iniConfig.on_location_cleared_notification_color);
    Log("- enable_journal: {}", g_iniConfig.enable_journal);
    Log("- show_percentage_in_journal: {}", g_iniConfig.show_percentage_in_journal);
    Log("- show_message_for_percentage_in_journal: {}", g_iniConfig.show_message_for_percentage_in_journal);
    Log("- show_recent_locations_in_journal: {}", g_iniConfig.show_recent_locations_in_journal);
    Log("- show_message_for_most_recent_location_in_journal: {}", g_iniConfig.show_message_for_most_recent_location_in_journal);
    Log("- max_recent_locations_in_journal: {}", g_iniConfig.max_recent_locations_in_journal);
    Log("- enable_on_screen_messages: {}", g_iniConfig.enable_on_screen_messages);
    Log("- show_percentage_updates_on_screen: {}", g_iniConfig.show_percentage_updates_on_screen);
    Log("- show_message_for_percentage_on_screen: {}", g_iniConfig.show_message_for_percentage_on_screen);
    Log("- show_message_for_most_recent_location_on_screen: {}", g_iniConfig.show_message_for_most_recent_location_on_screen);
    Log("- show_recently_discovered_locations_on_screen: {}", g_iniConfig.show_recently_discovered_locations_on_screen);
}
