#include <SkyrimScripting/Plugin.h>
#include <collections.h>

constexpr auto QUEST_EDITOR_NAME            = "MP_HazTheCompletionizt";
constexpr auto OBJECT_REFERENCE_SCRIPT_NAME = "ObjectReference";
constexpr auto FN_IS_MAP_MARKER_VISIBLE     = "IsMapMarkerVisible";
constexpr auto FN_CAN_FAST_TRAVEL_TO_MARKER = "CanFastTravelToMarker";

auto totalLocationsWithMarkersCount      = 0;
auto totalPlayerDiscoveredLocationsCount = 0;

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
    Log("Recalculating total number of discovered locations with map markers...");
    auto now = std::chrono::steady_clock::now();

    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) return;

    auto* objectHandlePolicy = vm->GetObjectHandlePolicy();
    if (!objectHandlePolicy) return;

    auto freshCountOfLocationsWithMarkers = 0;
    auto freshCountOfDiscoveredLocations  = 0;
    auto allLocationsInGame               = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSLocation>();
    for (const auto* location : allLocationsInGame) {
        if (location->fullName.empty()) continue;
        auto markerHandle = location->worldLocMarker;
        if (markerHandle) {
            auto marker = markerHandle.get();
            if (marker) {
                freshCountOfLocationsWithMarkers++;

                std::optional<bool> isVisible     = std::nullopt;
                std::optional<bool> canFastTravel = std::nullopt;

                auto markerVmHandle              = objectHandlePolicy->GetHandleForObject(marker->GetFormType(), marker.get());
                auto scriptsAttachedToThisMarker = vm->attachedScripts.find(markerVmHandle);
                if (scriptsAttachedToThisMarker != vm->attachedScripts.end()) {
                    for (auto& script : scriptsAttachedToThisMarker->second) {
                        if (script->type->name == OBJECT_REFERENCE_SCRIPT_NAME) {
                            // We found ObjectReference for this marker
                            // Use it to check if the player has discovered this location
                            auto functionCount = script->type->GetNumMemberFuncs();
                            for (auto i = 0; i < functionCount; i++) {
                                auto function = script->type->GetMemberFuncIter()[i].func;
                                if (function->GetName() == FN_IS_MAP_MARKER_VISIBLE) {
                                    // RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callbackPtr{nullptr};
                                    // callbackPtr = RE::make_smart<PapyrusBoolFunctionCallback>(*callback);

                                    /*

            if (!self) self = _self;
            if (self)
                vm->DispatchMethodCall(
                    self.value(), _function->GetName(), argumentsToDispatch.get(), callbackPtr
                );
            else
                vm->DispatchStaticCall(
                    _function->GetObjectTypeName(), _function->GetName(), argumentsToDispatch.get(),
                    callbackPtr
                );
                */

                                    // vm->DispatchMethodCall(markerVmHandle, OBJECT_REFERENCE_SCRIPT_NAME, FN_IS_MAP_MARKER_VISIBLE, nullptr, &isVisible);
                                }
                            }
                        }
                    }
                }
            }
        }

        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - now).count();
        Log("Recalculation took {} ms", elapsedMs);
    }

    class EventSink : public RE::BSTEventSink<RE::TESActorLocationChangeEvent>, public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        static EventSink* instance() {
            static EventSink singleton;
            return &singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>* eventSource) override {
            if (!event) return RE::BSEventNotifyControl::kContinue;
            if (!event->actor) return RE::BSEventNotifyControl::kContinue;
            if (!event->actor->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
            if (!event->newLoc) return RE::BSEventNotifyControl::kContinue;

            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player) return RE::BSEventNotifyControl::kContinue;

            auto* hazQuest = RE::TESForm::LookupByEditorID<RE::TESQuest>("MP_HazTheCompletionizt");
            if (!hazQuest) return RE::BSEventNotifyControl::kContinue;

            auto* firstObjective = hazQuest->objectives.front();
            if (!firstObjective) return RE::BSEventNotifyControl::kContinue;

            Log("The player currently has {} map markers", player->mapMarkerIterator);

            firstObjective->displayText = std::format("The player currently has {} map markers", player->mapMarkerIterator);
            // firstObjective->displayText = std::format("We discovered {}", event->newLoc->GetName());

            auto mapMenu = RE::UI::GetSingleton()->GetMenu<RE::MapMenu>(RE::MapMenu::MENU_NAME);
            if (mapMenu) {
                auto shownMarkerCount = mapMenu->mapMarkers.size();
                Log("The map menu has {} markers", shownMarkerCount);
            }

            return RE::BSEventNotifyControl::kContinue;
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
        RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESActorLocationChangeEvent>(EventSink::instance());
        RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(EventSink::instance());
    }

    SKSEPlugin_Entrypoint {}