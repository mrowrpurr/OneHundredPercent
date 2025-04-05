#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <collections.h>

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

#include "FormUtils.h"

enum class LocationEventType : std::uint32_t {
    None                    = 0,
    Discovered              = 1,
    DiscoveredFromPlayerMap = 2,
};

inline std::string LocationEventTypeToString(LocationEventType type) {
    switch (type) {
        case LocationEventType::Discovered:
            return "Discovered";
        case LocationEventType::DiscoveredFromPlayerMap:
            return "Discovered from player map";
        default:
            return "None";
    }
}

struct LocationEvent {
    FormIdentifier    formIdentifier;  // The TESObjectREFR* owner of the MapMarkerData*
    std::string       locationName;    // From the MapMarkerData* (to match with what is shown on screen)
    LocationEventType eventType;       // How did we discover this location?
    float             eventTime;       // When did we discover this location? (in-game time)

    // And, just for fun, WHERE WERE WE when we discovered it?
    RE::NiPoint3 eventPosition;
    RE::NiPoint3 eventRotation;
    std::string  eventCellName;

    void Save(SKSE::SerializationInterface* intfc) const;
    void Load(SKSE::SerializationInterface* intfc);
};

class SaveData {
    collections_map<FormIdentifier, LocationEvent> discoveryEvents;
    std::vector<FormIdentifier>                    recentlyDiscoveredMarkers;

public:
    collections_map<FormIdentifier, LocationEvent>& GetDiscoveryEvents();
    std::vector<FormIdentifier>&                    GetRecentlyDiscoveredMapMarkerIDs();

    std::uint32_t GetTotalDiscoveredMapMarkersCount() const;
    std::uint32_t GetRecentlyDiscoveredMapMarkersCount() const;

    LocationEvent* LookupMapMarker(const RE::MapMarkerData* mapMarkerData);
    LocationEvent* LookupMapMarker(const FormIdentifier& mapMarkerReferenceFormID);
    LocationEvent* GetMostRecentlyDiscoveredLocation();
    LocationEvent* GetRecentlyDiscoveredLocation(std::uint32_t index);

    bool IsMapMarkerDiscovered(const RE::MapMarkerData* mapMarkerData) const;

    void SaveDiscoveryEvent(LocationEventType type, const RE::MapMarkerData* mapMarkerData);

    void Save(SKSE::SerializationInterface* intfc);
    void Load(SKSE::SerializationInterface* intfc);
};

void SaveCallback(SKSE::SerializationInterface* a_intfc);
void LoadCallback(SKSE::SerializationInterface* a_intfc);
void RevertCallback(SKSE::SerializationInterface* a_intfc);

void SetupSaveCallbacks();

inline SaveData          g_saveData;
inline std::atomic<bool> g_isSaveDataLoaded{false};

inline SaveData& GetSaveData() { return g_saveData; }
