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

public:
    collections_map<FormIdentifier, LocationEvent> GetLocationEvents() const { return locationEvents; }

    LocationEvent* LookupLocation(const RE::BGSLocation* location) {
        auto it = locationEvents.find(FormIdentifier::CreateIdentifier(location));
        if (it != locationEvents.end()) return &it->second;
        return nullptr;
    }

    auto GetLocationCount() const { return locationEvents.size(); }

    LocationEvent* SaveLocationEvent(LocationEventType type, const RE::BGSLocation* location);

    inline bool ContainsLocation(const RE::BGSLocation* location) { return locationEvents.contains(FormIdentifier::CreateIdentifier(location)); }

    void DiscoveredLocation(const RE::BGSLocation* location);
    void ClearedLocation(const RE::BGSLocation* location);
    void FoundPreviouslyDiscoveredLocationOnPlayersMap(const RE::BGSLocation* location);

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
