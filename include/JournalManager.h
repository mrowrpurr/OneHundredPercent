#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include <cstdint>
#include <string_view>

#include "Config.h"

namespace JournalManager {
    inline RE::TESQuest* GetQuest() { return RE::TESForm::LookupByEditorID<RE::TESQuest>(Config::QUEST_EDITOR_ID); }

    inline RE::BGSQuestObjective* GetObjective(std::uint32_t index) {
        if (auto* quest = GetQuest()) {
            auto i = 0;
            for (auto& objective : quest->objectives) {
                if (i == index) return objective;
                ++i;
            }
        }
        Log("Failed to get objective {} from quest {}", index, Config::QUEST_EDITOR_ID);
        return nullptr;
    }

    void UpdateObjectiveText(std::uint32_t index, std::string_view text);
    void SetStatus(std::uint32_t index, bool visible, bool completed, bool active = false);
}
