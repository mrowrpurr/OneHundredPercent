#include "DiscoveredLocations.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include <chrono>
#include <unordered_set>

std::string ToLowerCase(std::string_view text) {
    std::string result;
    std::ranges::transform(text, std::back_inserter(result), [](unsigned char c) { return std::tolower(c); });
    return result;
}

DiscoveredLocationStats GetDiscoveredLocationStats() {
    Log("Recalculating total number of discovered locations with map markers...");
    auto now = std::chrono::steady_clock::now();

    DiscoveredLocationStats statsFromPlayer;
    DiscoveredLocationStats statsFromReferences;

    std::unordered_map<RE::TESFile*, std::uint32_t> markersPerFile;

    auto* player           = RE::PlayerCharacter::GetSingleton();
    auto* clearableKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypeClearable");

    std::unordered_set<RE::MapMarkerData*> discoveredLocations;

    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    statsFromPlayer.totalLocations++;
                    markersPerFile[marker->GetFile(0)]++;
                    if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) statsFromPlayer.discoveredLocations++;
                    if (marker->HasKeyword(clearableKeyword)) statsFromPlayer.clearedLocations++;
                    discoveredLocations.insert(mapData);
                }
            }
        }
    }

    Log("PLAYER:");
    for (auto& [file, count] : markersPerFile) {
        if (file) {
            Log("Found {} markers in file: {}", count, file->GetFilename());
        }
    }

    Log("Found {} markers in player", statsFromPlayer.discoveredLocations);

    //

    markersPerFile.clear();

    auto skipped                      = 0;
    auto workspaceBasedCountOfMarkers = 0;
    auto worldspaces                  = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESWorldSpace>();
    for (auto* worldspace : worldspaces) {
        if (!worldspace) continue;

        auto* persistent = worldspace->persistentCell;
        if (persistent) {
            for (auto& refHandle : persistent->references) {
                auto* ref = refHandle.get();
                if (!ref) continue;

                // 3. Location must exist and have a name (i.e., not a dummy marker)
                auto* location = ref->GetCurrentLocation();
                if (!location || location->fullName.empty()) {
                    skipped++;
                    continue;
                }

                if (auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>()) {
                    auto* mapData = marker->mapData;
                    if (mapData) {
                        // 1. Must have a name
                        if (mapData->locationName.fullName.empty()) {
                            skipped++;
                            continue;
                        }

                        // 2. Should NOT be visible by default
                        // (If visible at game start, it's probably a quest point, not a discoverable)
                        if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible)) continue;

                        // Did the player NOT have this one?
                        if (discoveredLocations.find(mapData) == discoveredLocations.end()) {
                            Log("THIS MARKER is NOT IN THE PLAYER LIST: {} [FormID: {:X}]", mapData->locationName.GetFullName(), ref->GetFormID());
                            if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible)) {
                                Log("THIS MARKER is VISIBLE: {} [FormID: {:X}]", mapData->locationName.GetFullName(), ref->GetFormID());
                            } else {
                                Log("THIS MARKER is NOT VISIBLE: {} [FormID: {:X}]", mapData->locationName.GetFullName(), ref->GetFormID());
                            }
                        }

                        workspaceBasedCountOfMarkers++;

                        statsFromReferences.totalLocations++;
                        markersPerFile[ref->GetFile(0)]++;
                        if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo))
                            statsFromReferences.discoveredLocations++;
                        if (ref->HasKeyword(clearableKeyword)) statsFromReferences.clearedLocations++;
                    }
                }
            }
        }
    }

    Log("Found {} markers in worldspaces", workspaceBasedCountOfMarkers);
    Log("Skipped {} markers in worldspaces", skipped);

    Log("WORLDSPACES:");
    for (auto& [file, count] : markersPerFile) {
        if (file) {
            Log("Found {} markers in file: {}", count, file->GetFilename());
        }
    }

    //

    // Print out the stats from the player
    Log("Player discovered locations: {} / {} (cleared: {})", statsFromPlayer.discoveredLocations, statsFromPlayer.totalLocations, statsFromPlayer.clearedLocations);

    // Print out the stats from the references
    Log("References discovered locations: {} / {} (cleared: {})", statsFromReferences.discoveredLocations, statsFromReferences.totalLocations,
        statsFromReferences.clearedLocations);

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    Log("Recalculation took {} ms", durationMs);

    return statsFromPlayer;
}
