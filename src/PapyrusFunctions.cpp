#pragma once

#include "PapyrusFunctions.h"

#include "EventHandler.h"

namespace PapyrusFunctions {
    void UpdateJournalWithLatestDiscoveredLocationStats(RE::StaticFunctionTag*) { EventHandler::UpdateJournalWithLatestStats(); }

    bool BindFunctions(RE::BSScript::Internal::VirtualMachine* vm) {
        vm->RegisterFunction("UpdateJournalWithLatestDiscoveredLocationStats", "MP_HazTheCompletionizt", UpdateJournalWithLatestDiscoveredLocationStats);
        return true;
    }
}
