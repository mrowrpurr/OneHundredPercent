#include "EventWatcher.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "BGSLocationEx.h"
#include "EventHandler.h"

class EventSink : public RE::BSTEventSink<RE::LocationDiscovery::Event>, public RE::BSTEventSink<RE::LocationCleared::Event> {
public:
    static EventSink* instance() {
        static EventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationDiscovery::Event* event, RE::BSTEventSource<RE::LocationDiscovery::Event>*) override {
        Log("Location Discovered: {} - {}", event->worldspaceID, event->mapMarkerData->locationName.GetFullName());
        EventHandler::OnLocationDiscovered(event->mapMarkerData);
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationCleared::Event* event, RE::BSTEventSource<RE::LocationCleared::Event>*) override {
        auto* location = BGSLocationEx::GetLastChecked();
        if (location && location->IsLoaded()) {
            Log("Location Cleared: {} - {}", location->GetName(), location->GetFormID());
            EventHandler::OnLocationCleared(location);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

void WatchForEvents() {
    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(256);

    RE::LocationDiscovery::GetEventSource()->AddEventSink(EventSink::instance());

    // This is for the BGSLocationEx::GetLastChecked() function (used in Location Cleared event)
    BGSLocationEx::Install(trampoline);

    RE::LocationCleared::GetEventSource()->AddEventSink(EventSink::instance());
}
