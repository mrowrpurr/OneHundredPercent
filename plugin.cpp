#include <SkyrimScripting/Plugin.h>

#include "EventHandler.h"
#include "EventWatcher.h"
#include "PapyrusFunctions.h"
#include "SaveData.h"

SKSEPlugin_Entrypoint {
    SetupSaveCallbacks();
    SetupPapyrusFunctions();
}
SKSEPlugin_OnDataLoaded { WatchForEvents(); }
SKSEPlugin_OnPostLoadGame { EventHandler::UpdateJournalWithLatestStats(); }
