#include "IniFile.h"

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
    } else {
        g_iniConfig.notification_on_location_discovered = true;
        g_iniConfig.notification_on_location_cleared    = true;
        g_iniConfig.percentage_based_message_in_journal = true;
    }
}
