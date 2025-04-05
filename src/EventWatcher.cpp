#include "EventWatcher.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "EventHandler.h"
#include "PlayerMapMarkers.h"

class EventSink : public RE::BSTEventSink<RE::LocationDiscovery::Event>, public RE::BSTEventSink<RE::TESActorLocationChangeEvent> {
public:
    static EventSink* instance() {
        static EventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationDiscovery::Event* event, RE::BSTEventSource<RE::LocationDiscovery::Event>*) override {
        EventHandler::OnMapMarkerDiscovered(event->mapMarkerData);
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*) override {
        if (event && event->actor && event->actor->IsPlayerRef()) UpdateSaveGameToIncludeDiscoveredPlayerMapMarkers();
        return RE::BSEventNotifyControl::kContinue;
    }
};

void WatchForEvents() {
    RE::LocationDiscovery::GetEventSource()->AddEventSink(EventSink::instance());
    RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESActorLocationChangeEvent>(EventSink::instance());
}
