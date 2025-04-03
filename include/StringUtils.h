#pragma once

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>

inline std::string ToLowerCase(std::string_view text) {
    std::string result;
    std::ranges::transform(text, std::back_inserter(result), [](unsigned char c) { return std::tolower(c); });
    return result;
}
