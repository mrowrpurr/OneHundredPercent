#pragma once

#include <RE/Skyrim.h>

#include "BGSLocationEx.h"

namespace EventHandler {
    void UpdateJournalWithLatestStats();
    void OnLocationDiscovered(const RE::MapMarkerData* mapMarkerData);
    void OnLocationCleared(const BGSLocationEx* locationEx);
    void OnOpenJournal();
}
