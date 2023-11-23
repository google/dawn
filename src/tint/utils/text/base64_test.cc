// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/text/base64.h"

#include <optional>

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

struct Case {
    char in;
    std::optional<uint8_t> out;
};

using DecodeBase64Test = testing::TestWithParam<Case>;

TEST_P(DecodeBase64Test, Char) {
    EXPECT_EQ(DecodeBase64(GetParam().in), GetParam().out);
}

INSTANTIATE_TEST_SUITE_P(Valid,
                         DecodeBase64Test,
                         testing::Values(Case{'A', 0},
                                         Case{'B', 1},
                                         Case{'C', 2},
                                         Case{'D', 3},
                                         Case{'E', 4},
                                         Case{'F', 5},
                                         Case{'G', 6},
                                         Case{'H', 7},
                                         Case{'I', 8},
                                         Case{'J', 9},
                                         Case{'K', 10},
                                         Case{'L', 11},
                                         Case{'M', 12},
                                         Case{'N', 13},
                                         Case{'O', 14},
                                         Case{'P', 15},
                                         Case{'Q', 16},
                                         Case{'R', 17},
                                         Case{'S', 18},
                                         Case{'T', 19},
                                         Case{'U', 20},
                                         Case{'V', 21},
                                         Case{'W', 22},
                                         Case{'X', 23},
                                         Case{'Y', 24},
                                         Case{'Z', 25},
                                         Case{'a', 26},
                                         Case{'b', 27},
                                         Case{'c', 28},
                                         Case{'d', 29},
                                         Case{'e', 30},
                                         Case{'f', 31},
                                         Case{'g', 32},
                                         Case{'h', 33},
                                         Case{'i', 34},
                                         Case{'j', 35},
                                         Case{'k', 36},
                                         Case{'l', 37},
                                         Case{'m', 38},
                                         Case{'n', 39},
                                         Case{'o', 40},
                                         Case{'p', 41},
                                         Case{'q', 42},
                                         Case{'r', 43},
                                         Case{'s', 44},
                                         Case{'t', 45},
                                         Case{'u', 46},
                                         Case{'v', 47},
                                         Case{'w', 48},
                                         Case{'x', 49},
                                         Case{'y', 50},
                                         Case{'z', 51},
                                         Case{'0', 52},
                                         Case{'1', 53},
                                         Case{'2', 54},
                                         Case{'3', 55},
                                         Case{'4', 56},
                                         Case{'5', 57},
                                         Case{'6', 58},
                                         Case{'7', 59},
                                         Case{'8', 60},
                                         Case{'9', 61},
                                         Case{'+', 62},
                                         Case{'/', 63}));

INSTANTIATE_TEST_SUITE_P(Invalid,
                         DecodeBase64Test,
                         testing::Values(Case{'@', std::nullopt},
                                         Case{'#', std::nullopt},
                                         Case{'^', std::nullopt},
                                         Case{'&', std::nullopt},
                                         Case{'!', std::nullopt},
                                         Case{'*', std::nullopt},
                                         Case{'(', std::nullopt},
                                         Case{')', std::nullopt},
                                         Case{'-', std::nullopt},
                                         Case{'.', std::nullopt},
                                         Case{'\0', std::nullopt},
                                         Case{'\n', std::nullopt}));

INSTANTIATE_TEST_SUITE_P(Padding, DecodeBase64Test, testing::Values(Case{'=', std::nullopt}));

}  // namespace
}  // namespace tint::utils
