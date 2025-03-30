#include <SkyrimScripting/Plugin.h>
#include <collections.h>

constexpr auto QUEST_EDITOR_NAME = "MP_HazTheCompletionizt";

auto totalLocationsWithMarkersCount      = 0;
auto totalPlayerDiscoveredLocationsCount = 0;

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
        if (event->menuName == RE::MapMenu::MENU_NAME) {
            Log("Map menu opened");
            auto mapMenu = RE::UI::GetSingleton()->GetMenu<RE::MapMenu>(RE::MapMenu::MENU_NAME);
            if (mapMenu) {
                auto shownMarkerCount = mapMenu->mapMarkers.size();
                Log("The map menu has {} markers", shownMarkerCount);

                auto anotherCount = mapMenu->markerData.size();
                Log("The map menu has {} markers in markerData", anotherCount);

                for (auto& marker : mapMenu->mapMarkers) {
                    // marker.type = RE::MARKER_TYPE::kYouAreHere;
                    if (marker.customMarker) Log("There is a custom marker");
                    Log("Marker...");
                }
            } else {
                Log("Map menu not found");
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

void GetTotalCountOfLocationsWithMarkers() {
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
        }
    }
    Log("{} locations with map markers in the game", totalLocationsWithMarkersCount);
    for (const auto& [file, count] : locationCountPerFile) {
        Log("File {} has {} locations with map markers", file->GetFilename(), count);
    }
}

SKSEPlugin_OnDataLoaded {
    GetTotalCountOfLocationsWithMarkers();
    RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESActorLocationChangeEvent>(EventSink::instance());
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(EventSink::instance());
}

SKSEPlugin_Entrypoint {}