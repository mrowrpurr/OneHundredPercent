#include <SkyrimScripting/Plugin.h>

#include "EventHandler.h"
#include "EventWatcher.h"
#include "JsonFiles.h"
#include "PapyrusFunctions.h"
#include "SaveData.h"

SKSEPlugin_Entrypoint {
    SetupSaveCallbacks();
    SetupPapyrusFunctions();
}

SKSEPlugin_OnDataLoaded {
    WatchForEvents();
    FindAndLoadAllJsonFiles();  // Requires data to lookup forms
}

SKSEPlugin_OnPostLoadGame { EventHandler::UpdateJournalWithLatestStats(); }
