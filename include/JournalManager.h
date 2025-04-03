#pragma once

#include <cstdint>
#include <string_view>

namespace JournalManager {
    void UpdateObjectiveText(std::uint32_t index, std::string_view text);
    void SetStatus(std::uint32_t index, bool visible, bool completed, bool active = false);
}
