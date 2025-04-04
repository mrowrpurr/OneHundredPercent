#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

enum class LocationEventType : std::uint32_t {
    None       = 0,
    Discovered = 1,
    Cleared    = 2,
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
    std::string       locationName;
    LocationEventType eventType;
    float             eventTime;
    RE::NiPoint3      eventPosition;
    RE::NiPoint3      eventRotation;
    std::string       eventCellName;
    RE::FormID        locationFormID;
    std::string       locationPluginName;
};

struct SaveData {
    std::vector<LocationEvent> locationEvents;

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

void SaveLocationDiscoveredEvent(const RE::BGSLocation* location);
void SaveLocationClearedEvent(const RE::BGSLocation* location);
