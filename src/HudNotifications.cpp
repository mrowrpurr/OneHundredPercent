#pragma once

#include "HudNotifications.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>
#include <collections.h>

#include "Config.h"
#include "TomlFile.h"

namespace stl {
    using namespace SKSE::stl;

    template <class T>
    void write_thunk_call(std::uintptr_t a_src) {
        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();
        T::func          = trampoline.write_call<5>(a_src, T::thunk);
    }

    template <class F, std::size_t idx, class T>
    void write_vfunc() {
        REL::Relocation<std::uintptr_t> vtbl{F::VTABLE[0]};
        T::func = vtbl.write_vfunc(idx, T::thunk);
    }

    // NG
    // template <std::size_t idx, class T>
    // void write_vfunc(REL::VariantID id) {
    //     REL::Relocation<std::uintptr_t> vtbl{id};
    //     T::func = vtbl.write_vfunc(idx, T::thunk);
    // }

    template <class T>
    void write_thunk_jump(std::uintptr_t a_src) {
        SKSE::AllocTrampoline(14);

        auto& trampoline = SKSE::GetTrampoline();
        T::func          = trampoline.write_branch<5>(a_src, T::thunk);
    }
}

namespace HudNotifications {
    collections_set<std::string> ignoredObjectiveTexts;

    void IgnoreQuestObjectiveWithText(std::string_view text) {
        if (text.empty()) return;
        Log("Queued ignore of quest objective with text: {}", text);
        ignoredObjectiveTexts.insert(text.data());
    }

    inline bool ShouldSkip(const RE::HUDNotifications::Notification& notification) {
        if (notification.quest && notification.quest->formEditorID == Config::QUEST_EDITOR_ID) {
            if (!GetConfig().enable_on_screen_messages) return true;
        }
        return false;
    }

    char HUDNotifications_Update::thunk(RE::HUDNotifications* This) {
        if (This->queue.size()) {
            if (ShouldSkip(This->queue.front())) {
                auto& front  = This->queue.front();
                front.text   = "";
                front.status = "";
                front.sound  = "";
                front.quest  = nullptr;
                front.word   = nullptr;
                front.type   = 0;
                front.time   = 0;
            }
        }
        return func(This);
    }

    void InstallHook() { stl::write_vfunc<RE::HUDNotifications, 0x1, HUDNotifications_Update>(); }
}