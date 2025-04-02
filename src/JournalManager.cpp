#pragma once

#include "JournalManager.h"

#include <RE/Skyrim.h>

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
        return nullptr;
    }

    void UpdateObjectiveText(std::uint32_t index, std::string_view text) {
        if (auto* objective = GetObjective(index)) objective->displayText = text;
    }

    void SetStatus(std::uint32_t index, bool visible, bool completed) {
        if (auto* objective = GetObjective(index)) {
            if (visible) {
                if (completed) objective->state.set(RE::QUEST_OBJECTIVE_STATE::kCompletedDisplayed);
                else objective->state.set(RE::QUEST_OBJECTIVE_STATE::kDisplayed);
            } else {
                if (completed) objective->state.set(RE::QUEST_OBJECTIVE_STATE::kCompleted);
                else objective->state.reset(RE::QUEST_OBJECTIVE_STATE::kDisplayed);
            }
        }
    }
}
