#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include <cstdint>
#include <optional>
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

    inline std::optional<std::string> GetObjectiveText(std::uint32_t index) {
        if (auto* objective = GetObjective(index)) return objective->displayText.c_str();
        return std::nullopt;
    }

    inline bool HasObjectiveText(std::uint32_t index) {
        if (auto* objective = GetObjective(index)) return !objective->displayText.empty();
        return false;
    }

    inline bool IsObjectiveVisible(std::uint32_t index) {
        if (auto* objective = GetObjective(index)) return objective->state.any(RE::QUEST_OBJECTIVE_STATE::kDisplayed);
        return false;
    }

    void UpdateObjectiveText(std::uint32_t index, std::string_view text);
    void SetStatus(std::uint32_t index, bool visible, bool completed, bool active = false);

    inline void ClearObjective(std::uint32_t index) {
        if (auto* objective = GetObjective(index)) {
            objective->displayText = "";
            SetStatus(index, false, false, false);
            Log("Cleared objective {} from quest {}", index, Config::QUEST_EDITOR_ID);
        }
    }
}
