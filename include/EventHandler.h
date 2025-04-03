#pragma once

#include <RE/Skyrim.h>

#include "BGSLocationEx.h"

namespace EventHandler {
    void UpdateJournalWithLatestStats(bool showSillyMessage = false);
    void OnLocationDiscovered(const RE::MapMarkerData* mapMarkerData);
    void OnLocationCleared(const BGSLocationEx* locationEx);
}
