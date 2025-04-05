#include <SkyrimScripting/Plugin.h>

#include "DiscoverableLocations.h"
#include "EventHandler.h"
#include "EventWatcher.h"
#include "JsonFiles.h"
#include "PapyrusFunctions.h"
#include "PlayerMapMarkers.h"
#include "SaveData.h"
#include "TomlFile.h"

SKSEPlugin_Entrypoint {
    LoadTomlConfigFile();
    SetupSaveCallbacks();
    SetupPapyrusFunctions();
}

SKSEPlugin_OnDataLoaded {
    WatchForEvents();
    FindAndLoadAllJsonFiles();  // OnDataLoaded because it requires data to lookup forms
    ReloadDiscoverableLocationInfo();
}

void OnGameLoad() {
    ResetPlayerMapMarkerLookupCache();
    ReloadDiscoverableLocationInfo();
    EventHandler::UpdateJournalWithLatestStats();
}

SKSEPlugin_OnPostLoadGame { OnGameLoad(); }
SKSEPlugin_OnNewGame { OnGameLoad(); }
