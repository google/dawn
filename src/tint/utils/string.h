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

#include <string>
#include <variant>

#include "src/tint/utils/slice.h"
#include "src/tint/utils/string_stream.h"

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
/// @returns value printed as a string via the stream `<<` operator
template <typename T>
std::string ToString(const T& value) {
    utils::StringStream s;
    s << value;
    return s.str();
}

/// @param value the variant to be printed as a string
/// @returns value printed as a string via the stream `<<` operator
template <typename... TYs>
std::string ToString(const std::variant<TYs...>& value) {
    utils::StringStream s;
    s << std::visit([&](auto& v) { return ToString(v); }, value);
    return s.str();
}

/// @param str the input string
/// @param prefix the prefix string
/// @returns true iff @p str has the prefix @p prefix
inline size_t HasPrefix(std::string_view str, std::string_view prefix) {
    return str.length() >= prefix.length() && str.substr(0, prefix.length()) == prefix;
}

/// @param str the input string
/// @param suffix the suffix string
/// @returns true iff @p str has the suffix @p suffix
inline size_t HasSuffix(std::string_view str, std::string_view suffix) {
    return str.length() >= suffix.length() && str.substr(str.length() - suffix.length()) == suffix;
}

/// @param a the first string
/// @param b the second string
/// @returns the Levenshtein distance between @p a and @p b
size_t Distance(std::string_view a, std::string_view b);

/// Suggest alternatives for an unrecognized string from a list of possible values.
/// @param got the unrecognized string
/// @param strings the list of possible values
/// @param ss the stream to write the suggest and list of possible values to
/// @param prefix the prefix to apply to the strings when printing (optional)
void SuggestAlternatives(std::string_view got,
                         Slice<char const* const> strings,
                         utils::StringStream& ss,
                         std::string_view prefix = "");

/// @param str the input string
/// @param pred the predicate function
/// @return @p str with characters passing the predicate function @p pred removed from the start of
/// the string.
template <typename PREDICATE>
std::string_view TrimLeft(std::string_view str, PREDICATE&& pred) {
    while (!str.empty() && pred(str.front())) {
        str = str.substr(1);
    }
    return str;
}

/// @param str the input string
/// @param pred the predicate function
/// @return @p str with characters passing the predicate function @p pred removed from the end of
/// the string.
template <typename PREDICATE>
std::string_view TrimRight(std::string_view str, PREDICATE&& pred) {
    while (!str.empty() && pred(str.back())) {
        str = str.substr(0, str.length() - 1);
    }
    return str;
}

/// @param str the input string
/// @param prefix the prefix to trim from @p str
/// @return @p str with the prefix removed, if @p str has the prefix.
inline std::string_view TrimPrefix(std::string_view str, std::string_view prefix) {
    return HasPrefix(str, prefix) ? str.substr(prefix.length()) : str;
}

/// @param str the input string
/// @param suffix the suffix to trim from @p str
/// @return @p str with the suffix removed, if @p str has the suffix.
inline std::string_view TrimSuffix(std::string_view str, std::string_view suffix) {
    return HasSuffix(str, suffix) ? str.substr(0, str.length() - suffix.length()) : str;
}

/// @param str the input string
/// @param pred the predicate function
/// @return @p str with characters passing the predicate function @p pred removed from the start and
/// end of the string.
template <typename PREDICATE>
std::string_view Trim(std::string_view str, PREDICATE&& pred) {
    return TrimLeft(TrimRight(str, pred), pred);
}

/// @param c the character to test
/// @returns true if @p c is one of the following:
/// * space (' ')
/// * form feed ('\f')
/// * line feed ('\n')
/// * carriage return ('\r')
/// * horizontal tab ('\t')
/// * vertical tab ('\v')
inline bool IsSpace(char c) {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

/// @param str the input string
/// @return @p str with all whitespace (' ') removed from the start and end of the string.
inline std::string_view TrimSpace(std::string_view str) {
    return Trim(str, IsSpace);
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_STRING_H_
