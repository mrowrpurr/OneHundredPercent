#include "PlayerMapMarkers.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>
#include <collections.h>

#include <atomic>

#include "DiscoverableLocations.h"
#include "JsonFiles.h"
#include "SaveData.h"

std::atomic<bool> g_isUpdateSaveGameToIncludeDiscoveredPlayerMapMarkersCurrentlyRunning{false};

collections_set<RE::FormID> SearchedWorldSpaces;

void ResetPlayerMapMarkerLookupCache() {
    SearchedWorldSpaces.clear();
    g_isUpdateSaveGameToIncludeDiscoveredPlayerMapMarkersCurrentlyRunning = false;
}

void UpdateSaveGameToIncludeDiscoveredPlayerMapMarkers() {
    if (g_isUpdateSaveGameToIncludeDiscoveredPlayerMapMarkersCurrentlyRunning.exchange(true)) {
        Log("UpdateSaveGameToIncludeDiscoveredPlayerMapMarkers is already running, skipping this call.");
        return;
    }

    // Get the player character
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        Log("Player character not found, skipping update.");
        g_isUpdateSaveGameToIncludeDiscoveredPlayerMapMarkersCurrentlyRunning = false;
        return;
    }

    if (player->GetWorldspace()) {
        auto worldSpaceFormId = player->GetWorldspace()->GetFormID();
        if (SearchedWorldSpaces.contains(worldSpaceFormId)) {
            Log("Already searched world space {}, skipping update.", player->GetWorldspace()->GetName());
            g_isUpdateSaveGameToIncludeDiscoveredPlayerMapMarkersCurrentlyRunning = false;
            return;
        }
        SearchedWorldSpaces.insert(worldSpaceFormId);
        Log("Searching world space {} for discovered map markers", player->GetWorldspace()->GetName());
    }

    // Get full list of all discoverable locations (which we check the player markers against)
    auto* discoverableLocations = GetDiscoverableLocationInfo();
    if (!discoverableLocations) {
        Log("Discoverable locations not found, skipping update.");
        g_isUpdateSaveGameToIncludeDiscoveredPlayerMapMarkersCurrentlyRunning = false;
        return;
    }

    auto& saveData = GetSaveData();
    
    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                        Log("[Debug] [Discovered Location from Player Map Markers]: {}", mapData->locationName.GetFullName());

                        auto findLocation = discoverableLocations->discoverableMapMarkersToLocations.find(mapData);
                        if (findLocation != discoverableLocations->discoverableMapMarkersToLocations.end()) {
                            Log("[Debug] [Found location]: {}::{}", findLocation->second->GetFullName(), mapData->locationName.GetFullName());

                            if (IgnoredLocationIDs.contains(findLocation->second->GetFormID())) continue;  // Skip any ignored location IDs
                            else Log("[Debug] [Not ignored location]: {}::{}", findLocation->second->GetFullName(), mapData->locationName.GetFullName());

                            if (IgnoredMapMarkers.contains(ToLowerCase(std::format("{}::{}", findLocation->second->GetFullName(), mapData->locationName.GetFullName()))))
                                continue;  // Skip any ignored map markers
                            else Log("[Debug] [Not ignored map marker]: {}::{}", findLocation->second->GetFullName(), mapData->locationName.GetFullName());

                            if (!saveData.ContainsLocation(findLocation->second)) {
                                // It's discovered
                                // It's on the player's map
                                // And it's not yet in the save game
                                saveData.FoundPreviouslyDiscoveredLocationOnPlayersMap(findLocation->second);

                                Log("[Player Map Markers] Discovered location: {}::{} ... {:x} @ {}", findLocation->second->GetFullName(), mapData->locationName.GetFullName(),
                                    findLocation->second->GetLocalFormID(), findLocation->second->GetFile(0)->GetFilename());
                            } else {
                                Log("[Debug] [Already discovered location]: {}::{}", findLocation->second->GetFullName(), mapData->locationName.GetFullName());
                            }
                        } else {
                            Log("[Debug] [Location not in the overall Discovereable locations list]: {}::{}", mapData->locationName.GetFullName(),
                                mapData->locationName.GetFullName());
                        }
                    }
                }
            }
        }
    }

    // Debug...
    auto& allLocations = GetSaveData().GetLocationEvents();
    for (auto& [id, locationEvent] : allLocations) {
        if (locationEvent.eventType == LocationEventType::DiscoveredFromMapMarker) {
            Log("[FROM SAVE] [DISCOVERED]: {} - {} - {}", locationEvent.locationName, LocationEventTypeToString(locationEvent.eventType), locationEvent.eventCellName);
        }
    }

    g_isUpdateSaveGameToIncludeDiscoveredPlayerMapMarkersCurrentlyRunning = false;
}
