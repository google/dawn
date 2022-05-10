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

#include <cstring>

#include "src/tint/writer/wgsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

// Makes an IEEE 754 binary32 floating point number with
// - 0 sign if sign is 0, 1 otherwise
// - 'exponent_bits' is placed in the exponent space.
//   So, the exponent bias must already be included.
f32 MakeFloat(int sign, int biased_exponent, int mantissa) {
    const uint32_t sign_bit = sign ? 0x80000000u : 0u;
    // The binary32 exponent is 8 bits, just below the sign.
    const uint32_t exponent_bits = (biased_exponent & 0xffu) << 23;
    // The mantissa is the bottom 23 bits.
    const uint32_t mantissa_bits = (mantissa & 0x7fffffu);

    uint32_t bits = sign_bit | exponent_bits | mantissa_bits;
    float result = 0.0f;
    static_assert(sizeof(result) == sizeof(bits),
                  "expected float and uint32_t to be the same size");
    std::memcpy(&result, &bits, sizeof(bits));
    return f32(result);
}

struct FloatData {
    f32 value;
    std::string expected;
};
inline std::ostream& operator<<(std::ostream& out, FloatData data) {
    out << "{" << data.value << "," << data.expected << "}";
    return out;
}

using WgslGenerator_FloatLiteralTest = TestParamHelper<FloatData>;

TEST_P(WgslGenerator_FloatLiteralTest, Emit) {
    auto* v = Expr(GetParam().value);

    SetResolveOnBuild(false);
    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitLiteral(out, v)) << gen.error();
    EXPECT_EQ(out.str(), GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(Zero,
                         WgslGenerator_FloatLiteralTest,
                         ::testing::ValuesIn(std::vector<FloatData>{{0_f, "0.0"},
                                                                    {MakeFloat(0, 0, 0), "0.0"},
                                                                    {MakeFloat(1, 0, 0), "-0.0"}}));

INSTANTIATE_TEST_SUITE_P(Normal,
                         WgslGenerator_FloatLiteralTest,
                         ::testing::ValuesIn(std::vector<FloatData>{{1_f, "1.0"},
                                                                    {-1_f, "-1.0"},
                                                                    {101.375_f, "101.375"}}));

INSTANTIATE_TEST_SUITE_P(Subnormal,
                         WgslGenerator_FloatLiteralTest,
                         ::testing::ValuesIn(std::vector<FloatData>{
                             {MakeFloat(0, 0, 1), "0x1p-149"},  // Smallest
                             {MakeFloat(1, 0, 1), "-0x1p-149"},
                             {MakeFloat(0, 0, 2), "0x1p-148"},
                             {MakeFloat(1, 0, 2), "-0x1p-148"},
                             {MakeFloat(0, 0, 0x7fffff), "0x1.fffffcp-127"},   // Largest
                             {MakeFloat(1, 0, 0x7fffff), "-0x1.fffffcp-127"},  // Largest
                             {MakeFloat(0, 0, 0xcafebe), "0x1.2bfaf8p-127"},   // Scattered bits
                             {MakeFloat(1, 0, 0xcafebe), "-0x1.2bfaf8p-127"},  // Scattered bits
                             {MakeFloat(0, 0, 0xaaaaa), "0x1.55554p-130"},     // Scattered bits
                             {MakeFloat(1, 0, 0xaaaaa), "-0x1.55554p-130"},    // Scattered bits
                         }));

INSTANTIATE_TEST_SUITE_P(Infinity,
                         WgslGenerator_FloatLiteralTest,
                         ::testing::ValuesIn(std::vector<FloatData>{
                             {MakeFloat(0, 255, 0), "0x1p+128"},
                             {MakeFloat(1, 255, 0), "-0x1p+128"}}));

INSTANTIATE_TEST_SUITE_P(
    // TODO(dneto): It's unclear how Infinity and NaN should be handled.
    // https://github.com/gpuweb/gpuweb/issues/1769
    // This test fails on Windows x86-64 because the machine sets the high
    // mantissa bit on NaNs.
    DISABLED_NaN,
    // In the NaN case, the top bit in the mantissa is often used to encode
    // whether the NaN is signalling or quiet, but no agreement between
    // different machine architectures on whether 1 means signalling or
    // if 1 means quiet.
    WgslGenerator_FloatLiteralTest,
    ::testing::ValuesIn(std::vector<FloatData>{
        // LSB only.  Smallest mantissa.
        {MakeFloat(0, 255, 1), "0x1.000002p+128"},  // Smallest mantissa
        {MakeFloat(1, 255, 1), "-0x1.000002p+128"},
        // MSB only.
        {MakeFloat(0, 255, 0x400000), "0x1.8p+128"},
        {MakeFloat(1, 255, 0x400000), "-0x1.8p+128"},
        // All 1s in the mantissa.
        {MakeFloat(0, 255, 0x7fffff), "0x1.fffffep+128"},
        {MakeFloat(1, 255, 0x7fffff), "-0x1.fffffep+128"},
        // Scattered bits, with 0 in top mantissa bit.
        {MakeFloat(0, 255, 0x20101f), "0x1.40203ep+128"},
        {MakeFloat(1, 255, 0x20101f), "-0x1.40203ep+128"},
        // Scattered bits, with 1 in top mantissa bit.
        {MakeFloat(0, 255, 0x40101f), "0x1.80203ep+128"},
        {MakeFloat(1, 255, 0x40101f), "-0x1.80203ep+128"}}));

}  // namespace
}  // namespace tint::writer::wgsl
