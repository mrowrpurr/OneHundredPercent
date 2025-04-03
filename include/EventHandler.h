#pragma once

#include <RE/Skyrim.h>

#include <optional>

#include "BGSLocationEx.h"

namespace EventHandler {
    void UpdateJournalWithLatestStats(std::optional<std::string> sillyMessage = std::nullopt);
    void OnLocationDiscovered(const RE::MapMarkerData* mapMarkerData);
    void OnLocationCleared(const BGSLocationEx* locationEx);
}
