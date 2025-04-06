#pragma once

#include "PapyrusFunctions.h"

#include <SKSE/SKSE.h>

#include "EventHandler.h"

namespace PapyrusFunctions {
    void UpdateJournalWithLatestDiscoverableMapMarkers(RE::StaticFunctionTag*) { EventHandler::UpdateJournalWithLatestStats(); }

    bool BindFunctions(RE::BSScript::Internal::VirtualMachine* vm) {
        vm->RegisterFunction("UpdateJournalWithLatestDiscoverableMapMarkers", "MP_OneHundredPercent", UpdateJournalWithLatestDiscoverableMapMarkers);
        return true;
    }
}

void SetupPapyrusFunctions() { SKSE::GetPapyrusInterface()->Register(PapyrusFunctions::BindFunctions); }
