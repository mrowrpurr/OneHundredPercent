#pragma once

#include <RE/Skyrim.h>

namespace PapyrusFunctions {
    void UpdateJournalWithLatestDiscoverableMapMarkers(RE::StaticFunctionTag*);
    bool BindFunctions(RE::BSScript::Internal::VirtualMachine* vm);
}

void SetupPapyrusFunctions();
