#include <SkyrimScripting/Plugin.h>

#include "EventHandler.h"
#include "EventWatcher.h"

SKSEPlugin_OnDataLoaded { WatchForEvents(); }
SKSEPlugin_OnPostLoadGame { EventHandler::UpdateJournalWithLatestStats(); }
