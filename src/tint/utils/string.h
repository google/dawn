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

namespace tint::utils {

/// @param str the string to apply replacements to
/// @param substr the string to search for
/// @param replacement the replacement string to use instead of `substr`
/// @returns `str` with all occurrences of `substr` replaced with `replacement`
inline std::string ReplaceAll(std::string str,
                              const std::string& substr,
                              const std::string& replacement) {
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != std::string::npos) {
        str.replace(pos, substr.length(), replacement);
        pos += replacement.length();
    }
    return str;
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_STRING_H_
