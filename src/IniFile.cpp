#include "IniFile.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "Config.h"

/*
    [SillyMessages]
    notification_on_location_discovered=1
    notification_on_location_cleared=1
*/

void LoadIni() {
    if (g_iniConfigLoaded.exchange(true)) return;

    auto tomlConfig = toml::parse_file(Config::INI_FILE_PATH.string());
    if (tomlConfig.contains("SillyMessages")) {
        auto section                                    = tomlConfig["SillyMessages"];
        g_iniConfig.notification_on_location_discovered = section["notification_on_location_discovered"].value_or(true);
        g_iniConfig.notification_on_location_cleared    = section["notification_on_location_cleared"].value_or(true);
        g_iniConfig.percentage_based_message_in_journal = section["percentage_based_message_in_journal"].value_or(true);
        g_iniConfig.silly_message_in_journal            = section["silly_message_in_journal"].value_or(true);
        g_iniConfig.recent_locations_in_journal         = section["recent_locations_in_journal"].value_or(true);
        g_iniConfig.most_recent_location_in_journal     = section["most_recent_location_in_journal"].value_or(true);
    } else {
        g_iniConfig.notification_on_location_discovered = true;
        g_iniConfig.notification_on_location_cleared    = true;
        g_iniConfig.percentage_based_message_in_journal = true;
        g_iniConfig.silly_message_in_journal            = true;
        g_iniConfig.recent_locations_in_journal         = true;
        g_iniConfig.most_recent_location_in_journal     = true;
    }

    Log("Loaded INI configuration:");
    Log("  notification_on_location_discovered: {}", g_iniConfig.notification_on_location_discovered);
    Log("  notification_on_location_cleared: {}", g_iniConfig.notification_on_location_cleared);
    Log("  percentage_based_message_in_journal: {}", g_iniConfig.percentage_based_message_in_journal);
    Log("  silly_message_in_journal: {}", g_iniConfig.silly_message_in_journal);
    Log("  recent_locations_in_journal: {}", g_iniConfig.recent_locations_in_journal);
    Log("  most_recent_location_in_journal: {}", g_iniConfig.most_recent_location_in_journal);
    Log("INI configuration loaded successfully.");
}
