// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_UNICODE_H_
#define SRC_TINT_UTILS_UNICODE_H_

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

namespace tint::utils {

/// CodePoint is a unicode code point.
struct CodePoint {
    /// Constructor
    inline CodePoint() = default;

    /// Constructor
    /// @param v the code point value
    inline explicit CodePoint(uint32_t v) : value(v) {}

    /// @returns the code point value
    inline operator uint32_t() const { return value; }

    /// Assignment operator
    /// @param v the new value for the code point
    /// @returns this CodePoint
    inline CodePoint& operator=(uint32_t v) {
        value = v;
        return *this;
    }

    /// @returns true if this CodePoint is in the XID_Start set.
    /// @see https://unicode.org/reports/tr31/
    bool IsXIDStart() const;

    /// @returns true if this CodePoint is in the XID_Continue set.
    /// @see https://unicode.org/reports/tr31/
    bool IsXIDContinue() const;

    /// The code point value
    uint32_t value = 0;
};

namespace utf8 {

/// Decodes the first code point in the utf8 string.
/// @param ptr the pointer to the first byte of the utf8 sequence
/// @param len the maximum number of bytes to read
/// @returns a pair of CodePoint and width in code units (bytes).
///          If the next code point cannot be decoded then returns [0,0].
std::pair<CodePoint, size_t> Decode(const uint8_t* ptr, size_t len);

/// Decodes the first code point in the utf8 string.
/// @param utf8_string the string view that contains the utf8 sequence
/// @returns a pair of CodePoint and width in code units (bytes).
///          If the next code point cannot be decoded then returns [0,0].
std::pair<CodePoint, size_t> Decode(std::string_view utf8_string);

/// @returns true if all the utf-8 code points in the string are ASCII
/// (code-points 0x00..0x7f).
bool IsASCII(std::string_view);

}  // namespace utf8

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_UNICODE_H_
