#pragma once

#include <RE/Skyrim.h>

namespace EventHandler {
    void UpdateJournalWithLatestStats(std::string_view sillyMessage = "");
    void OnLocationDiscovered(const RE::BGSLocation* location);
    void OnLocationCleared(const RE::BGSLocation* location);
}
