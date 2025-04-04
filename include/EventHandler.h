#pragma once

#include <RE/Skyrim.h>

#include <optional>

namespace EventHandler {
    void UpdateJournalWithLatestStats(std::optional<std::string> sillyMessage = std::nullopt);
    void OnLocationDiscovered(const RE::BGSLocation* location);
    void OnLocationCleared(const RE::BGSLocation* location);
}
