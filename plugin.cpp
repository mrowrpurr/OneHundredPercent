#include <SkyrimScripting/Plugin.h>

#include "DiscoverableMapMarkers.h"
#include "EventHandler.h"
#include "EventWatcher.h"
#include "HudNotifications.h"
#include "JsonFiles.h"
#include "PapyrusFunctions.h"
#include "PlayerMapMarkers.h"
#include "SaveData.h"
#include "TomlFile.h"

SKSEPlugin_Entrypoint {
    LoadTomlConfigFile();
    SetupSaveCallbacks();
    SetupPapyrusFunctions();
    HudNotifications::InstallHook();
}

SKSEPlugin_OnDataLoaded {
    WatchForEvents();
    FindAndLoadAllJsonFiles();       // OnDataLoaded because it requires data to lookup forms
    ReloadDiscoverableMapMarkers();  // Must be done AFTER we load the .json
}

void OnGameLoad() {
    ResetPlayerMapMarkerLookupCache();
    UpdateSaveGameToIncludeDiscoveredPlayerMapMarkers();
    GetSaveData().RemoveLocationsForModsWhichAreNoLongerLoaded();
    EventHandler::UpdateJournalWithLatestStats();
}

SKSEPlugin_OnPostLoadGame { OnGameLoad(); }
SKSEPlugin_OnNewGame { OnGameLoad(); }
