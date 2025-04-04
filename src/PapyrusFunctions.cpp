#pragma once

#include "PapyrusFunctions.h"

#include <SKSE/SKSE.h>

#include "EventHandler.h"

namespace PapyrusFunctions {
    void UpdateJournalWithLatestDiscoverableLocationInfo(RE::StaticFunctionTag*) { EventHandler::UpdateJournalWithLatestStats(); }

    bool BindFunctions(RE::BSScript::Internal::VirtualMachine* vm) {
        vm->RegisterFunction("UpdateJournalWithLatestDiscoverableLocationInfo", "MP_HazTheCompletionizt", UpdateJournalWithLatestDiscoverableLocationInfo);
        return true;
    }
}

void SetupPapyrusFunctions() { SKSE::GetPapyrusInterface()->Register(PapyrusFunctions::BindFunctions); }
