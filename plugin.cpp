#include <SkyrimScripting/Plugin.h>

#include "DiscoverableLocations.h"
#include "EventHandler.h"
#include "EventWatcher.h"
#include "JsonFiles.h"
#include "PapyrusFunctions.h"
#include "SaveData.h"
#include "TomlFile.h"

SKSEPlugin_Entrypoint {
    LoadIni();
    SetupSaveCallbacks();
    SetupPapyrusFunctions();
}

SKSEPlugin_OnDataLoaded {
    WatchForEvents();
    FindAndLoadAllJsonFiles();  // OnDataLoaded because it requires data to lookup forms
    ReloadDiscoverableLocationInfo();
}

void OnGameLoad() {
    EventHandler::UpdateJournalWithLatestStats();
    ReloadDiscoverableLocationInfo();
}

SKSEPlugin_OnPostLoadGame { OnGameLoad(); }
SKSEPlugin_OnNewGame { OnGameLoad(); }
