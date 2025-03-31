#include <SkyrimScripting/Plugin.h>
#include <collections.h>

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
        if (location->worldLocMarker) {
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
    if (_currentCalculation_isRunning.exchange(true)) {
        Log("Recalculation already in progress, skipping...");
        return;
    }

    Log("Recalculating total number of discovered locations with map markers...");

    _currentCalculation_totalCount      = 0;
    _currentCalculation_discoveredCount = 0;
    _currentCalculation_markersInfo.clear();
    _currentCalculation_startTime = std::chrono::steady_clock::now();

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) return;

    auto* objectHandlePolicy = vm->GetObjectHandlePolicy();
    if (!objectHandlePolicy) return;

    auto allLocationsInGame = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSLocation>();
    for (const auto* location : allLocationsInGame) {
        if (location->fullName.empty()) continue;
        Log("Checking location: {}", location->fullName.c_str());
        auto markerHandle = location->worldLocMarker;
        if (markerHandle) {
            Log("markerHandle is true... does get() explode?");
            auto marker = markerHandle.get();
            Log("Nope, we're good for this one...");
            if (marker) {
                Log("Marker is valid, checking stuff...");
                _currentCalculation_totalCount++;
                auto& markerInfo = _currentCalculation_markersInfo.emplace(location, LocationMarkerInfo{location, marker.get(), {}, {}}).first->second;
                Log("A");
                auto markerVmHandle = objectHandlePolicy->GetHandleForObject(marker->GetFormType(), marker.get());
                Log("B");
                auto scriptsAttachedToThisMarker = vm->attachedScripts.find(markerVmHandle);
                Log("C");
                if (scriptsAttachedToThisMarker != vm->attachedScripts.end()) {
                    for (auto& script : scriptsAttachedToThisMarker->second) {
                        Log("Script: {}", script->type->name.c_str());
                        if (script->type->name == OBJECT_REFERENCE_SCRIPT_NAME) {
                            // We found ObjectReference for this marker
                            // Use it to check if the player has discovered this location
                            auto functionCount = script->type->GetNumMemberFuncs();
                            Log("Function count: {}", functionCount);
                            for (auto i = 0; i < functionCount; i++) {
                                Log("Function: {}", i);
                                auto* functionIterator = script->type->GetMemberFuncIter();
                                if (functionIterator) {
                                    auto function = functionIterator[i].func;
                                    Log("Function name: {}", function->GetName().c_str());
                                    if (function->GetName() == FN_IS_MAP_MARKER_VISIBLE) {
                                        // IsMapMarkerVisible()
                                        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callbackPtr{nullptr};
                                        callbackPtr = RE::make_smart<PapyrusBoolFunctionCallback>([&](bool isVisible) {
                                            Log("!! IsMapMarkerVisible: {} - {}", location->fullName.c_str(), isVisible ? "true" : "false");
                                            markerInfo.isVisible = isVisible;
                                        });
                                        auto args   = RE::MakeFunctionArguments();
                                        Log("Dispatching IsMapMarkerVisible...");
                                        vm->DispatchMethodCall(markerVmHandle, OBJECT_REFERENCE_SCRIPT_NAME, FN_IS_MAP_MARKER_VISIBLE, args, callbackPtr);
                                        Log("Dispatched IsMapMarkerVisible");
                                    } else if (function->GetName() == FN_CAN_FAST_TRAVEL_TO_MARKER) {
                                        // CanFastTravelToMarker()
                                        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callbackPtr{nullptr};
                                        callbackPtr = RE::make_smart<PapyrusBoolFunctionCallback>([&](bool isFastTravelable) {
                                            Log("!! CanFastTravelToMarker: {} - {}", location->fullName.c_str(), isFastTravelable ? "true" : "false");
                                            markerInfo.isFastTravelable = isFastTravelable;
                                        });
                                        auto args   = RE::MakeFunctionArguments();
                                        Log("Dispatching CanFastTravelToMarker...");
                                        vm->DispatchMethodCall(markerVmHandle, OBJECT_REFERENCE_SCRIPT_NAME, FN_CAN_FAST_TRAVEL_TO_MARKER, args, callbackPtr);
                                        Log("Dispatched CanFastTravelToMarker");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
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
