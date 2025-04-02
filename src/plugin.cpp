#include <SkyrimScripting/Plugin.h>

#include "EventHandler.h"
#include "EventWatcher.h"
#include "PapyrusFunctions.h"

SKSEPlugin_Entrypoint {
    spdlog::set_pattern("%v");
    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions::BindFunctions);
}
SKSEPlugin_OnDataLoaded { WatchForEvents(); }
SKSEPlugin_OnPostLoadGame { EventHandler::UpdateJournalWithLatestStats(); }
