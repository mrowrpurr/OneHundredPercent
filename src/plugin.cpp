#include <SkyrimScripting/Plugin.h>
#include <collections.h>

#include <unordered_set>

constexpr auto QUEST_EDITOR_NAME            = "MP_HazTheCompletionizt";
constexpr auto OBJECT_REFERENCE_SCRIPT_NAME = "ObjectReference";
constexpr auto FN_IS_MAP_MARKER_VISIBLE     = "IsMapMarkerVisible";
constexpr auto FN_CAN_FAST_TRAVEL_TO_MARKER = "CanFastTravelToMarker";

auto totalLocationsWithMarkersCount      = 0;
auto totalPlayerDiscoveredLocationsCount = 0;

struct LocationMarkerInfo {
    const RE::BGSLocation* location;
    RE::TESObjectREFR*     marker;
    std::optional<bool>    isFastTravelable;
    std::optional<bool>    isVisible;
};

std::atomic<bool>                                           _currentCalculation_isRunning       = false;
auto                                                        _currentCalculation_totalCount      = 0;
auto                                                        _currentCalculation_discoveredCount = 0;
collections_map<const RE::BGSLocation*, LocationMarkerInfo> _currentCalculation_markersInfo;
std::chrono::steady_clock::time_point                       _currentCalculation_startTime;

/*
    https://ck.uesp.net/wiki/IsMapMarkerVisible_-_ObjectReference

    ; Is this location discovered?
    Bool Function IsLocationDiscovered(ObjectReference akMapMarker)
    ; Returns true if the location's map marker is visible and
    ; can be fast traveled to, indicating it has been discovered.

    if (akMapMarker.IsMapMarkerVisible() == TRUE && akMapMarker.CanFastTravelToMarker() == TRUE)
        return true
    else
        return false
    endif
    EndFunction

    This function only returns if the map marker is visible or not. If you want to know if a location is visible and already discovered, use GetMapMarkerVisible.
    Checking if a location is discovered, requires both this and CanFastTravelToMarker - ObjectReference. If you wish to avoid a scripted method, the above will suffice.
*/

class PapyrusBoolFunctionCallback : public RE::BSScript::IStackCallbackFunctor {
    std::function<void(bool)> _callback;

public:
    PapyrusBoolFunctionCallback(std::function<void(bool)> callback) : _callback(callback) {}
    void operator()(RE::BSScript::Variable variable) override { _callback(variable.GetBool()); }
    bool CanSave() const override { return false; }
    void SetObject(const RE::BSTSmartPointer<RE::BSScript::Object>&) override {}
};

class PapyrusFunctionArguments : public RE::BSScript::IFunctionArguments {
    std::vector<RE::BSScript::Variable> _arguments;

public:
    void AddArgument(const RE::BSScript::Variable& argument) { _arguments.emplace_back(argument); }
    bool operator()(RE::BSScrapArray<RE::BSScript::Variable>& variableArray) const override {
        if (!_arguments.empty()) variableArray.resize(_arguments.size());
        for (size_t i = 0; i < _arguments.size(); ++i) variableArray[i] = _arguments[i];
        return true;
    }
};

void GetTotalCountOfLocationsWithMarkers() {
    auto                                         countOfLocationsWithoutMarkers = 0;
    collections_map<RE::TESFile*, std::uint32_t> locationCountPerFile;
    auto                                         allLocationsInGame = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSLocation>();
    for (const auto* location : allLocationsInGame) {
        if (location->fullName.empty()) continue;
        if (location->worldLocMarker && location->worldLocMarker.get()) {
            auto* file = location->GetFile(0);
            if (file) {
                auto& count = locationCountPerFile[file];
                count++;
            }
            totalLocationsWithMarkersCount++;
        } else {
            countOfLocationsWithoutMarkers++;
        }
    }
    Log("{} locations with map markers in the game", totalLocationsWithMarkersCount);
    Log("{} locations without map markers in the game", countOfLocationsWithoutMarkers);
    for (const auto& [file, count] : locationCountPerFile) {
        Log("File {} has {} locations with map markers", file->GetFilename(), count);
    }
}

void RecalculateTotalNumberOfDiscoveredLocationsWithMapMarkers() {
    std::unordered_set<const RE::TESObjectREFR*> mapMarkersSet;

    Log("Recalculating total number of discovered locations with map markers...");

    auto now = std::chrono::steady_clock::now();

    auto  playerTotalCount     = 0;
    auto  playerTotalBothCount = 0;
    auto* player               = RE::PlayerCharacter::GetSingleton();
    Log("Player has {} map markers", player->currentMapMarkers.size());
    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    auto isVisible        = mapData->flags.any(RE::MapMarkerData::Flag::kVisible);
                    auto isFastTravelable = mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo);
                    auto isBoth           = isVisible && isFastTravelable;

                    playerTotalCount++;
                    if (isBoth) playerTotalBothCount++;

                    Log("Player location {}, isVisible: {}, isFastTravelable: {}, isBoth: {}", mapData->locationName.GetFullName(), isVisible, isFastTravelable, isBoth);

                    mapMarkersSet.insert(marker.get());
                }
            }
        }
    }

    std::unordered_set<const RE::TESObjectREFR*> uniqueMapMarkerDataInTheseObjects;

    auto countOfDifferent             = 0;
    auto isVisibleCount               = 0;
    auto isFastTravelableCount        = 0;
    auto isBothCount                  = 0;
    auto totalCount                   = 0;
    auto allObjectReferencesInTheGame = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESObjectREFR>();
    for (const auto* reference : allObjectReferencesInTheGame) {
        if (reference->extraList.HasType<RE::ExtraMapMarker>()) {
            if (auto* extraMapMarker = reference->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    uniqueMapMarkerDataInTheseObjects.insert(reference);

                    auto isVisible        = mapData->flags.any(RE::MapMarkerData::Flag::kVisible);
                    auto isFastTravelable = mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo);
                    auto isBoth           = isVisible && isFastTravelable;
                    totalCount++;
                    if (isVisible) isVisibleCount++;
                    if (isFastTravelable) isFastTravelableCount++;
                    if (isBoth) isBothCount++;

                    // Is this in the list of markers which the player stored?
                    if (!mapMarkersSet.contains(reference)) {
                        Log("------> LOCATION *not* IN PLAYER MAP MARKERS: {}", mapData->locationName.GetFullName());
                        countOfDifferent++;
                    }
                }
            }
        }
    }

    Log("--> UNIQUE MAP MARKERS IN OBJECTS: {}", uniqueMapMarkerDataInTheseObjects.size());
    Log("DIFFERENT COUNT: {}", countOfDifferent);

    Log("Player has {} map markers", playerTotalCount);
    Log("Player has {} map markers that are visible", playerTotalBothCount);

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    Log("Total locations with map markers: {}", totalLocationsWithMarkersCount);
    Log("Total locations with map markers that are visible: {}", isVisibleCount);
    Log("Total locations with map markers that are fast travelable: {}", isFastTravelableCount);
    Log("Total locations with map markers that are both visible and fast travelable: {}", isBothCount);
    Log("Duration of calculation: {} ms", durationMs);
}

class EventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
public:
    static EventSink* instance() {
        static EventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
        if (event->opening && event->menuName == RE::JournalMenu::MENU_NAME) {
            std::thread([&]() { RecalculateTotalNumberOfDiscoveredLocationsWithMapMarkers(); }).detach();
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

SKSEPlugin_OnDataLoaded {
    GetTotalCountOfLocationsWithMarkers();
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(EventSink::instance());
}
