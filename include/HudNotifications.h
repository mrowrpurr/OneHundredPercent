#pragma once

#include <RE/Skyrim.h>

#include <string_view>

namespace HudNotifications {
    void IgnoreQuestObjectiveWithText(std::string_view text);

    struct HUDNotifications_Update {
        static char                                    thunk(RE::HUDNotifications* This);
        static inline REL::Relocation<decltype(thunk)> func;
    };

    void InstallHook();
}