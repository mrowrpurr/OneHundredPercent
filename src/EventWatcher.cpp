#include "EventWatcher.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "BGSLocationEx.h"
#include "EventHandler.h"
#include "PlayerMapMarkers.h"

class EventSink : public RE::BSTEventSink<RE::LocationDiscovery::Event>,
                  public RE::BSTEventSink<RE::LocationCleared::Event>,
                  public RE::BSTEventSink<RE::TESActorLocationChangeEvent> {
public:
    static EventSink* instance() {
        static EventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationDiscovery::Event* event, RE::BSTEventSource<RE::LocationDiscovery::Event>*) override {
        EventHandler::OnMapMarkerDiscovered(event->mapMarkerData);
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationCleared::Event* event, RE::BSTEventSource<RE::LocationCleared::Event>*) override {
        auto* location = BGSLocationEx::GetLastChecked();
        if (location && location->IsLoaded()) {
            if (location->worldLocMarker && location->worldLocMarker.get()) {
                if (auto* extraMapData = location->worldLocMarker.get()->extraList.GetByType<RE::ExtraMapMarker>()) {
                    if (auto* mapMarkerData = extraMapData->mapData) {
                        EventHandler::OnMapMarkerCleared(mapMarkerData);
                    }
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*) override {
        if (event && event->actor && event->actor->IsPlayerRef()) UpdateSaveGameToIncludeDiscoveredPlayerMapMarkers();
        return RE::BSEventNotifyControl::kContinue;
    }
};

void WatchForEvents() {
    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(256);

    // This is for the BGSLocationEx::GetLastChecked() function (used in Location Cleared event)
    BGSLocationEx::Install(trampoline);

    RE::LocationDiscovery::GetEventSource()->AddEventSink(EventSink::instance());
    RE::LocationCleared::GetEventSource()->AddEventSink(EventSink::instance());
    RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESActorLocationChangeEvent>(EventSink::instance());
}
