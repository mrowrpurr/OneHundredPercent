#include "EventWatcher.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "BGSLocationEx.h"
#include "EventHandler.h"

class EventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>, public RE::BSTEventSink<RE::LocationDiscovery::Event>, public RE::BSTEventSink<RE::LocationCleared::Event> {
public:
    static EventSink* instance() {
        static EventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
        Log("Menu Open/Close Event: {} - {}", event->opening ? "Opened" : "Closed", event->menuName.c_str());
        if (event->opening && event->menuName == RE::JournalMenu::MENU_NAME) {
            EventHandler::OnOpenJournal();
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationDiscovery::Event* event, RE::BSTEventSource<RE::LocationDiscovery::Event>*) override {
        Log("LOCATION DISCOVERED: {} - {}", event->worldspaceID, event->mapMarkerData->locationName.GetFullName());
        EventHandler::OnLocationDiscovered(event->mapMarkerData);
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationCleared::Event* event, RE::BSTEventSource<RE::LocationCleared::Event>*) override {
        auto* location = BGSLocationEx::GetLastChecked();
        if (location && location->IsLoaded()) {
            Log("Location cleared: {} - {}", location->GetName(), location->GetFormID());
            EventHandler::OnLocationCleared(location);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

void WatchForEvents() {
    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(256);

    BGSLocationEx::Install(trampoline);
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(EventSink::instance());
    RE::LocationDiscovery::GetEventSource()->AddEventSink(EventSink::instance());
    RE::LocationCleared::GetEventSource()->AddEventSink(EventSink::instance());
}
