// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/float_to_string.h"

#include <cmath>
#include <cstring>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>

#include "src/tint/debug.h"

namespace tint::writer {

std::string FloatToString(float f) {
    // Try printing the float in fixed point, with a smallish limit on the
    // precision
    std::stringstream fixed;
    fixed.flags(fixed.flags() | std::ios_base::showpoint | std::ios_base::fixed);
    fixed.precision(9);
    fixed << f;

    // If this string can be parsed without loss of information, use it
    auto float_equal_no_warning = std::equal_to<float>();
    if (float_equal_no_warning(std::stof(fixed.str()), f)) {
        auto str = fixed.str();
        while (str.length() >= 2 && str[str.size() - 1] == '0' && str[str.size() - 2] != '.') {
            str.pop_back();
        }

        return str;
    }

    // Resort to scientific, with the minimum precision needed to preserve the
    // whole float
    std::stringstream sci;
    sci.precision(std::numeric_limits<float>::max_digits10);
    sci << f;
    return sci.str();
}

std::string FloatToBitPreservingString(float f) {
    // For the NaN case, avoid handling the number as a floating point value.
    // Some machines will modify the top bit in the mantissa of a NaN.

    std::stringstream ss;

    uint32_t float_bits = 0u;
    std::memcpy(&float_bits, &f, sizeof(float_bits));

    // Handle the sign.
    const uint32_t kSignMask = 1u << 31;
    if (float_bits & kSignMask) {
        // If `f` is -0.0 print -0.0.
        ss << '-';
        // Strip sign bit.
        float_bits = float_bits & (~kSignMask);
    }

    switch (std::fpclassify(f)) {
        case FP_ZERO:
        case FP_NORMAL:
            std::memcpy(&f, &float_bits, sizeof(float_bits));
            ss << FloatToString(f);
            break;

        default: {
            // Infinity, NaN, and Subnormal
            // TODO(dneto): It's unclear how Infinity and NaN should be handled.
            // See https://github.com/gpuweb/gpuweb/issues/1769

            // std::hexfloat prints 'nan' and 'inf' instead of an
            // explicit representation like we want. Split it out
            // manually.
            const int kExponentBias = 127;
            const int kExponentMask = 0x7f800000;
            const int kMantissaMask = 0x007fffff;
            const int kMantissaBits = 23;

            int mantissaNibbles = (kMantissaBits + 3) / 4;

            const int biased_exponent =
                static_cast<int>((float_bits & kExponentMask) >> kMantissaBits);
            int exponent = biased_exponent - kExponentBias;
            uint32_t mantissa = float_bits & kMantissaMask;

            ss << "0x";

            if (exponent == 128) {
                if (mantissa == 0) {
                    //  Infinity case.
                    ss << "1p+128";
                } else {
                    //  NaN case.
                    //  Emit the mantissa bits as if they are left-justified after the
                    //  binary point.  This is what SPIRV-Tools hex float emitter does,
                    //  and it's a justifiable choice independent of the bit width
                    //  of the mantissa.
                    mantissa <<= (4 - (kMantissaBits % 4));
                    // Remove trailing zeroes, for tidyness.
                    while (0 == (0xf & mantissa)) {
                        mantissa >>= 4;
                        mantissaNibbles--;
                    }
                    ss << "1." << std::hex << std::setfill('0') << std::setw(mantissaNibbles)
                       << mantissa << "p+128";
                }
            } else {
                // Subnormal, and not zero.
                TINT_ASSERT(Writer, mantissa != 0);
                const int kTopBit = (1 << kMantissaBits);

                // Shift left until we get 1.x
                while (0 == (kTopBit & mantissa)) {
                    mantissa <<= 1;
                    exponent--;
                }
                // Emit the leading 1, and remove it from the mantissa.
                ss << "1";
                mantissa = mantissa ^ kTopBit;
                mantissa <<= 1;
                exponent++;

                // Emit the fractional part.
                if (mantissa) {
                    // Remove trailing zeroes, for tidyness
                    while (0 == (0xf & mantissa)) {
                        mantissa >>= 4;
                        mantissaNibbles--;
                    }
                    ss << "." << std::hex << std::setfill('0') << std::setw(mantissaNibbles)
                       << mantissa;
                }
                // Emit the exponent
                ss << "p" << std::showpos << std::dec << exponent;
            }
        }
    }
    return ss.str();
}

}  // namespace tint::writer
