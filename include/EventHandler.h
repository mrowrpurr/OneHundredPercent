#pragma once

#include <RE/Skyrim.h>

namespace EventHandler {
    void UpdateJournalWithLatestStats();
    void OnMapMarkerDiscovered(const RE::MapMarkerData* mapMarkerData);
    void OnMapMarkerCleared(const RE::MapMarkerData* mapMarkerData);
}
