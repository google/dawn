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

#include "src/tint/utils/strconv/parse_num.h"

#include <charconv>

#include "absl/strings/charconv.h"

namespace tint {

namespace {

template <typename T>
Result<T, ParseNumberError> Parse(std::string_view number) {
    T val = 0;
    if constexpr (std::is_floating_point_v<T>) {
        auto result = absl::from_chars(number.data(), number.data() + number.size(), val);
        if (result.ec == std::errc::result_out_of_range) {
            return ParseNumberError::kResultOutOfRange;
        }
        if (result.ec != std::errc() || result.ptr != number.data() + number.size()) {
            return ParseNumberError::kUnparsable;
        }
    } else {
        auto result = std::from_chars(number.data(), number.data() + number.size(), val);
        if (result.ec == std::errc::result_out_of_range) {
            return ParseNumberError::kResultOutOfRange;
        }
        if (result.ec != std::errc() || result.ptr != number.data() + number.size()) {
            return ParseNumberError::kUnparsable;
        }
    }
    return val;
}

}  // namespace

Result<float, ParseNumberError> ParseFloat(std::string_view str) {
    return Parse<float>(str);
}

Result<double, ParseNumberError> ParseDouble(std::string_view str) {
    return Parse<double>(str);
}

Result<int, ParseNumberError> ParseInt(std::string_view str) {
    return Parse<int>(str);
}

Result<unsigned int, ParseNumberError> ParseUint(std::string_view str) {
    return Parse<unsigned int>(str);
}

Result<int64_t, ParseNumberError> ParseInt64(std::string_view str) {
    return Parse<int64_t>(str);
}

Result<uint64_t, ParseNumberError> ParseUint64(std::string_view str) {
    return Parse<uint64_t>(str);
}

Result<int32_t, ParseNumberError> ParseInt32(std::string_view str) {
    return Parse<int32_t>(str);
}

Result<uint32_t, ParseNumberError> ParseUint32(std::string_view str) {
    return Parse<uint32_t>(str);
}

Result<int16_t, ParseNumberError> ParseInt16(std::string_view str) {
    return Parse<int16_t>(str);
}

Result<uint16_t, ParseNumberError> ParseUint16(std::string_view str) {
    return Parse<uint16_t>(str);
}

Result<int8_t, ParseNumberError> ParseInt8(std::string_view str) {
    return Parse<int8_t>(str);
}

Result<uint8_t, ParseNumberError> ParseUint8(std::string_view str) {
    return Parse<uint8_t>(str);
}

}  // namespace tint
