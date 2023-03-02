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

#include "src/tint/debug.h"
#include "src/tint/utils/string_stream.h"

namespace tint::writer {

namespace {

template <typename T>
struct Traits;

template <>
struct Traits<float> {
    using uint_t = uint32_t;
    static constexpr int kExponentBias = 127;
    static constexpr uint_t kExponentMask = 0x7f800000;
    static constexpr uint_t kMantissaMask = 0x007fffff;
    static constexpr uint_t kSignMask = 0x80000000;
    static constexpr int kMantissaBits = 23;
};

template <>
struct Traits<double> {
    using uint_t = uint64_t;
    static constexpr int kExponentBias = 1023;
    static constexpr uint_t kExponentMask = 0x7ff0000000000000;
    static constexpr uint_t kMantissaMask = 0x000fffffffffffff;
    static constexpr uint_t kSignMask = 0x8000000000000000;
    static constexpr int kMantissaBits = 52;
};

template <typename F>
std::string ToString(F f) {
    utils::StringStream s;
    s << f;
    return s.str();
}

template <typename F>
std::string ToBitPreservingString(F f) {
    using T = Traits<F>;
    using uint_t = typename T::uint_t;

    // For the NaN case, avoid handling the number as a floating point value.
    // Some machines will modify the top bit in the mantissa of a NaN.

    std::stringstream ss;

    typename T::uint_t float_bits = 0u;
    static_assert(sizeof(float_bits) == sizeof(f));
    std::memcpy(&float_bits, &f, sizeof(float_bits));

    // Handle the sign.
    if (float_bits & T::kSignMask) {
        // If `f` is -0.0 print -0.0.
        ss << '-';
        // Strip sign bit.
        float_bits = float_bits & (~T::kSignMask);
    }

    switch (std::fpclassify(f)) {
        case FP_ZERO:
        case FP_NORMAL:
            std::memcpy(&f, &float_bits, sizeof(float_bits));
            ss << ToString(f);
            break;

        default: {
            // Infinity, NaN, and Subnormal
            // TODO(dneto): It's unclear how Infinity and NaN should be handled.
            // See https://github.com/gpuweb/gpuweb/issues/1769

            // std::hexfloat prints 'nan' and 'inf' instead of an explicit representation like we
            // want. Split it out manually.
            int mantissa_nibbles = (T::kMantissaBits + 3) / 4;

            const int biased_exponent =
                static_cast<int>((float_bits & T::kExponentMask) >> T::kMantissaBits);
            int exponent = biased_exponent - T::kExponentBias;
            uint_t mantissa = float_bits & T::kMantissaMask;

            ss << "0x";

            if (exponent == T::kExponentBias + 1) {
                if (mantissa == 0) {
                    //  Infinity case.
                    ss << "1p+" << exponent;
                } else {
                    // NaN case.
                    // Emit the mantissa bits as if they are left-justified after the binary point.
                    // This is what SPIRV-Tools hex float emitter does, and it's a justifiable
                    // choice independent of the bit width of the mantissa.
                    mantissa <<= (4 - (T::kMantissaBits % 4));
                    // Remove trailing zeroes, for tidiness.
                    while (0 == (0xf & mantissa)) {
                        mantissa >>= 4;
                        mantissa_nibbles--;
                    }
                    ss << "1." << std::hex << std::setfill('0') << std::setw(mantissa_nibbles)
                       << mantissa << "p+" << std::dec << exponent;
                }
            } else {
                // Subnormal, and not zero.
                TINT_ASSERT(Writer, mantissa != 0);
                const auto kTopBit = static_cast<uint_t>(1u) << T::kMantissaBits;

                // Shift left until we get 1.x
                while (0 == (kTopBit & mantissa)) {
                    mantissa <<= 1;
                    exponent--;
                }
                // Emit the leading 1, and remove it from the mantissa.
                ss << "1";
                mantissa = mantissa ^ kTopBit;
                exponent++;

                // Left-justify mantissa to whole nibble.
                mantissa <<= (4 - (T::kMantissaBits % 4));

                // Emit the fractional part.
                if (mantissa) {
                    // Remove trailing zeroes, for tidiness
                    while (0 == (0xf & mantissa)) {
                        mantissa >>= 4;
                        mantissa_nibbles--;
                    }
                    ss << "." << std::hex << std::setfill('0') << std::setw(mantissa_nibbles)
                       << mantissa;
                }
                // Emit the exponent
                ss << "p" << std::showpos << std::dec << exponent;
            }
        }
    }
    return ss.str();
}

}  // namespace

std::string FloatToString(float f) {
    return ToString(f);
}

std::string FloatToBitPreservingString(float f) {
    return ToBitPreservingString(f);
}

std::string DoubleToString(double f) {
    return ToString(f);
}

std::string DoubleToBitPreservingString(double f) {
    return ToBitPreservingString(f);
}

}  // namespace tint::writer
