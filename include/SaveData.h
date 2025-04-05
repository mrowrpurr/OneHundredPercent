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
    Cleared                 = 2,
    DiscoveredFromMapMarker = 3,
};

inline std::string LocationEventTypeToString(LocationEventType type) {
    switch (type) {
        case LocationEventType::Discovered:
            return "Discovered";
        case LocationEventType::Cleared:
            return "Cleared";
        case LocationEventType::DiscoveredFromMapMarker:
            return "DiscoveredFromMapMarker";
        default:
            return "None";
    }
}

struct LocationEvent {
    FormIdentifier    formIdentifier;
    std::string       locationName;
    LocationEventType eventType;
    float             eventTime;
    RE::NiPoint3      eventPosition;
    RE::NiPoint3      eventRotation;
    std::string       eventCellName;

    void Save(SKSE::SerializationInterface* intfc) const;
    void Load(SKSE::SerializationInterface* intfc);
};

class SaveData {
    collections_map<FormIdentifier, LocationEvent> locationEvents;
    std::vector<FormIdentifier>                    discoveredLocations;

public:
    collections_map<FormIdentifier, LocationEvent>& GetLocationEvents() { return locationEvents; }
    std::vector<FormIdentifier>&                    GetRecentlyDiscoveredLocationIDs() { return discoveredLocations; }

    LocationEvent* LookupLocation(const RE::BGSLocation* location);
    LocationEvent* LookupLocation(const FormIdentifier& formIdentifier);
    LocationEvent* GetMostRecentlyDiscoveredLocation();
    LocationEvent* GetRecentlyDiscoveredLocation(std::uint32_t index);

    bool          ContainsLocation(const RE::BGSLocation* location) const;
    std::uint32_t GetTotalDiscoveredLocationCount() const;
    std::uint32_t GetRecentlyDiscoveredLocationCount() const;

    void           DiscoveredLocation(const RE::BGSLocation* location);
    void           ClearedLocation(const RE::BGSLocation* location);
    void           FoundPreviouslyDiscoveredLocationOnPlayersMap(const RE::BGSLocation* location);
    LocationEvent* SaveLocationEvent(LocationEventType type, const RE::BGSLocation* location);

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
