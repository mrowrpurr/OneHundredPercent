#pragma once

#include "JournalManager.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include "SetQuestObjectiveState.h"

namespace JournalManager {
    void UpdateObjectiveText(std::uint32_t index, std::string_view text) {
        Log(">> Updating objective {} text to: {}", index, text);
        if (auto* objective = GetObjective(index)) {
            if (objective->displayText != text) objective->displayText = text;
        }
    }

    void SetStatus(std::uint32_t index, bool visible, bool completed, bool active) {
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
