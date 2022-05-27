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

#include "src/tint/number.h"

#include <algorithm>
#include <cstring>
#include <ostream>

namespace tint {

std::ostream& operator<<(std::ostream& out, ConversionFailure failure) {
    switch (failure) {
        case ConversionFailure::kExceedsPositiveLimit:
            return out << "value exceeds positive limit for type";
        case ConversionFailure::kExceedsNegativeLimit:
            return out << "value exceeds negative limit for type";
    }
    return out << "<unknown>";
}

f16::type f16::Quantize(f16::type value) {
    if (value > kHighest) {
        return std::numeric_limits<f16::type>::infinity();
    }
    if (value < kLowest) {
        return -std::numeric_limits<f16::type>::infinity();
    }
    // Below value must be within the finite range of a f16.
    uint32_t u32;
    memcpy(&u32, &value, 4);
    if ((u32 & 0x7fffffffu) == 0) {  // ~sign
        return value;                // +/- zero
    }
    if ((u32 & 0x7f800000) == 0x7f800000) {  // exponent all 1's
        return value;                        // inf or nan
    }
    // f32 bits :  1 sign,  8 exponent, 23 mantissa
    // f16 bits :  1 sign,  5 exponent, 10 mantissa
    // Mask the value to preserve the sign, exponent and most-significant 10 mantissa bits.
    u32 = u32 & 0xffffe000u;
    memcpy(&value, &u32, 4);
    return value;
}

}  // namespace tint
