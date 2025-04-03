#pragma once

#include <RE/Skyrim.h>

namespace PapyrusFunctions {
    void UpdateJournalWithLatestDiscoveredLocationStats(RE::StaticFunctionTag*);
    bool BindFunctions(RE::BSScript::Internal::VirtualMachine* vm);
}

void SetupPapyrusFunctions();
