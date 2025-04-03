#pragma once

#include "JournalManager.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "Config.h"
#include "SetQuestObjectiveState.h"

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

    void UpdateObjectiveText(std::uint32_t index, std::string_view text) {
        Log(">> Updating objective {} text to: {}", index, text);
        if (auto* objective = GetObjective(index)) {
            if (objective->displayText != text) objective->displayText = text;
        }
    }

    void SetStatus(std::uint32_t index, bool visible, bool completed, bool active) {
        // If it's objective index 0, we ALWAYS use this...
        // if (index == 0) {
        //     visible   = true;
        //     completed = false;
        //     active    = true;
        // } else {
        //     // it should NEVER be active if it's not 0
        //     active = false;
        // }

        Log(">> Setting status for objective {}: visible={}, completed={}", index, visible, completed);
        if (auto* objective = GetObjective(index)) {
            if (visible) {
                if (completed) SetObjectiveState(objective, RE::QUEST_OBJECTIVE_STATE::kCompletedDisplayed);
                else SetObjectiveState(objective, active ? RE::QUEST_OBJECTIVE_STATE::kDisplayed : RE::QUEST_OBJECTIVE_STATE::kDisplayed);
            } else {
                if (completed) SetObjectiveState(objective, RE::QUEST_OBJECTIVE_STATE::kCompleted);
                else SetObjectiveState(objective, RE::QUEST_OBJECTIVE_STATE::kDormant);
            }
        }
    }
}
