#pragma once

#include <RE/Skyrim.h>

namespace PapyrusFunctions {
    void UpdateJournalWithLatestDiscoverableLocationInfo(RE::StaticFunctionTag*);
    bool BindFunctions(RE::BSScript::Internal::VirtualMachine* vm);
}

void SetupPapyrusFunctions();
