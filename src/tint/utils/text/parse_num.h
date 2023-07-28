// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_TEXT_PARSE_NUM_H_
#define SRC_TINT_UTILS_TEXT_PARSE_NUM_H_

#include <optional>
#include <string>

#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/result/result.h"

namespace tint {

/// Error returned by the number parsing functions
enum class ParseNumberError {
    /// The number was unparsable
    kUnparsable,
    /// The parsed number is not representable by the target datatype
    kResultOutOfRange,
};

/// @param str the string
/// @returns the string @p str parsed as a float
Result<float, ParseNumberError> ParseFloat(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a double
Result<double, ParseNumberError> ParseDouble(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int
Result<int, ParseNumberError> ParseInt(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a unsigned int
Result<unsigned int, ParseNumberError> ParseUint(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int64_t
Result<int64_t, ParseNumberError> ParseInt64(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a uint64_t
Result<uint64_t, ParseNumberError> ParseUint64(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int32_t
Result<int32_t, ParseNumberError> ParseInt32(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a uint32_t
Result<uint32_t, ParseNumberError> ParseUint32(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int16_t
Result<int16_t, ParseNumberError> ParseInt16(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a uint16_t
Result<uint16_t, ParseNumberError> ParseUint16(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int8_t
Result<int8_t, ParseNumberError> ParseInt8(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a uint8_t
Result<uint8_t, ParseNumberError> ParseUint8(std::string_view str);

/// Disables the false-positive unreachable-code compiler warnings
TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);

/// @param str the string
/// @returns the string @p str parsed as a the number @p T
template <typename T>
inline Result<T, ParseNumberError> ParseNumber(std::string_view str) {
    if constexpr (std::is_same_v<T, float>) {
        return ParseFloat(str);
    }
    if constexpr (std::is_same_v<T, double>) {
        return ParseDouble(str);
    }
    if constexpr (std::is_same_v<T, int>) {
        return ParseInt(str);
    }
    if constexpr (std::is_same_v<T, unsigned int>) {
        return ParseUint(str);
    }
    if constexpr (std::is_same_v<T, int64_t>) {
        return ParseInt64(str);
    }
    if constexpr (std::is_same_v<T, uint64_t>) {
        return ParseUint64(str);
    }
    if constexpr (std::is_same_v<T, int32_t>) {
        return ParseInt32(str);
    }
    if constexpr (std::is_same_v<T, uint32_t>) {
        return ParseUint32(str);
    }
    if constexpr (std::is_same_v<T, int16_t>) {
        return ParseInt16(str);
    }
    if constexpr (std::is_same_v<T, uint16_t>) {
        return ParseUint16(str);
    }
    if constexpr (std::is_same_v<T, int8_t>) {
        return ParseInt8(str);
    }
    if constexpr (std::is_same_v<T, uint8_t>) {
        return ParseUint8(str);
    }
    return ParseNumberError::kUnparsable;
}

/// Re-enables the unreachable-code compiler warnings
TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);

}  // namespace tint

#endif  // SRC_TINT_UTILS_TEXT_PARSE_NUM_H_
