// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_UTILS_STRING_H_
#define SRC_TINT_UTILS_STRING_H_

#include <sstream>
#include <string>
#include <variant>

namespace tint::utils {

/// @param str the string to apply replacements to
/// @param substr the string to search for
/// @param replacement the replacement string to use instead of `substr`
/// @returns `str` with all occurrences of `substr` replaced with `replacement`
[[nodiscard]] inline std::string ReplaceAll(std::string str,
                                            std::string_view substr,
                                            std::string_view replacement) {
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != std::string_view::npos) {
        str.replace(pos, substr.length(), replacement);
        pos += replacement.length();
    }
    return str;
}

/// @param value the value to be printed as a string
/// @returns value printed as a string via the std::ostream `<<` operator
template <typename T>
std::string ToString(const T& value) {
    std::stringstream s;
    s << value;
    return s.str();
}

/// @param value the variant to be printed as a string
/// @returns value printed as a string via the std::ostream `<<` operator
template <typename... TYs>
std::string ToString(const std::variant<TYs...>& value) {
    std::stringstream s;
    s << std::visit([&](auto& v) { return ToString(v); }, value);
    return s.str();
}

/// @param str the input string
/// @param prefix the prefix string
/// @returns true iff @p str has the prefix @p prefix
inline size_t HasPrefix(std::string_view str, std::string_view prefix) {
    return str.compare(0, prefix.size(), prefix) == 0;
}

/// @param a the first string
/// @param b the second string
/// @returns the Levenshtein distance between @p a and @p b
size_t Distance(std::string_view a, std::string_view b);

/// Suggest alternatives for an unrecognized string from a list of expected values.
/// @param got the unrecognized string
/// @param strings the list of expected values
/// @param ss the stream to write the suggest and list of possible values to
template <size_t N>
void SuggestAlternatives(std::string_view got,
                         const char* const (&strings)[N],
                         std::ostringstream& ss) {
    // If the string typed was within kSuggestionDistance of one of the possible enum values,
    // suggest that. Don't bother with suggestions if the string was extremely long.
    constexpr size_t kSuggestionDistance = 5;
    constexpr size_t kSuggestionMaxLength = 64;
    if (!got.empty() && got.size() < kSuggestionMaxLength) {
        size_t candidate_dist = kSuggestionDistance;
        const char* candidate = nullptr;
        for (auto* str : strings) {
            auto dist = utils::Distance(str, got);
            if (dist < candidate_dist) {
                candidate = str;
                candidate_dist = dist;
            }
        }
        if (candidate) {
            ss << "Did you mean '" << candidate << "'?\n";
        }
    }

    // List all the possible enumerator values
    ss << "Possible values: ";
    for (auto* str : strings) {
        if (str != strings[0]) {
            ss << ", ";
        }
        ss << "'" << str << "'";
    }
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_STRING_H_
