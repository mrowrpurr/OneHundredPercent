#include "DiscoverableLocations.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>
#include <collections.h>

#include <atomic>
#include <chrono>

#include "JsonFiles.h"

std::unique_ptr<DiscoverableLocationInfo> g_DiscoverableLocations = nullptr;
std::atomic<bool>                         g_hasLoggedFullListOfDiscoverableLocations{false};
std::atomic<bool>                         g_DiscoverableLocationsReloading{false};
DiscoverableLocationInfo*                 GetDiscoverableLocationInfo() { return g_DiscoverableLocations.get(); }

void ReloadDiscoverableLocationInfo() {
    if (g_DiscoverableLocationsReloading.exchange(true)) {
        Log("Reloading discovered locations already in progress, skipping...");
        return;
    }

    Log("Reloading discovered locations...");
    auto now = std::chrono::steady_clock::now();

    auto                                         locationInfo = std::make_unique<DiscoverableLocationInfo>();
    collections_map<RE::TESFile*, std::uint32_t> countOfDiscoverableLocationsPerFile;

    /*
     * Add all Location w/ World Markers into discoverable locations
     *
     * Note: not all of them have the MapMarkerData* but we can at least track them as discoverable locations
     */
    auto locationForms = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSLocation>();
    for (auto* location : locationForms) {
        if (!location) continue;
        if (IgnoredLocationIDs.contains(location->GetFormID())) continue;
        if (location->fullName.empty()) continue;
        if (location->worldLocMarker && location->worldLocMarker.get()) {
            // Mark this location as discoverable!
            locationInfo->discoverableLocations.insert(location);

            // See if we can get the marker to associate with it:
            auto foundMarker = false;
            if (auto* extraMapMarker = location->worldLocMarker.get()->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapMarkerData = extraMapMarker->mapData) {
                    // Associate the map marker with the location
                    locationInfo->discoverableMapMarkersToLocations[mapMarkerData] = location;
                    foundMarker                                                    = true;
                }
            }

            auto* file = location->GetFile(0);
            countOfDiscoverableLocationsPerFile[file]++;
            locationInfo->totalDiscoverableLocationCount++;

            if (!g_hasLoggedFullListOfDiscoverableLocations) {
                if (foundMarker) Log("Discoverable Location (with marker): {} - {:x} in {}", location->GetName(), location->GetLocalFormID(), location->GetFile(0)->GetFilename());
                else Log("Discoverable Location (without marker): {} - {:x} in {}", location->GetName(), location->GetLocalFormID(), location->GetFile(0)->GetFilename());
            }
        }
    }

    /*
     * Go through all references in the game looking for markers and associate the markers
     */
    auto worldspaces = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESWorldSpace>();
    for (auto* worldspace : worldspaces) {
        if (!worldspace) continue;

        auto* persistent = worldspace->persistentCell;
        if (persistent) {
            for (auto& refHandle : persistent->references) {
                auto* ref = refHandle.get();
                if (!ref) continue;

                auto* location = ref->GetCurrentLocation();
                if (!location || location->fullName.empty()) continue;

                if (auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>()) {
                    if (auto* mapData = marker->mapData) {
                        if (IgnoredLocationIDs.contains(location->GetFormID())) continue;
                        if (mapData->locationName.fullName.empty()) continue;

                        // Mark this location as discoverable!
                        locationInfo->discoverableLocations.insert(location);

                        // Associate the map marker with the location
                        locationInfo->discoverableMapMarkersToLocations[mapData] = location;

                        auto* file = location->GetFile(0);
                        countOfDiscoverableLocationsPerFile[file]++;
                        locationInfo->totalDiscoverableLocationCount++;

                        if (!g_hasLoggedFullListOfDiscoverableLocations)
                            Log("Discoverable Location (from ref marker): {} - {:x} in {}", location->GetName(), location->GetLocalFormID(), location->GetFile(0)->GetFilename());
                    }
                }
            }
        }
    }

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    Log("Reloading discovered locations took {} ms", durationMs);
    Log("Total discoverable locations: {} (location count: {}) (marker count: {})", locationInfo->totalDiscoverableLocationCount, locationInfo->discoverableLocations.size(),
        locationInfo->discoverableMapMarkersToLocations.size());
    Log("Count of discoverable locations per file:");
    for (const auto& [file, count] : countOfDiscoverableLocationsPerFile) Log("File: {} - {} discoverable locations", file->GetFilename(), count);

    g_DiscoverableLocations                    = std::move(locationInfo);
    g_DiscoverableLocationsReloading           = false;
    g_hasLoggedFullListOfDiscoverableLocations = true;
}
