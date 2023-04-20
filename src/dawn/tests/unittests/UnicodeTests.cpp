// Copyright 2022 The Dawn Authors
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

#include "dawn/native/ShaderModule.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"

class CountUTF16CodeUnitsFromUTF8StringTest : public ValidationTest {};

TEST_F(CountUTF16CodeUnitsFromUTF8StringTest, ValidUnicodeString) {
    struct TestCase {
        const char* u8String;
        uint64_t lengthInUTF16;
    };

    // Referenced from src/tint/utils/unicode_test.cc
    constexpr std::array<TestCase, 12> kTestCases = {{
        {"", 0},
        {"abc", 3},
        {"\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c", 4},
        {"def\xf0\x9f\x91\x8b\xf0\x9f\x8c\x8e", 7},
        {"\xed\x9f\xbf", 1},      // CodePoint == 0xD7FF
        {"\xed\x9f\xbe", 1},      // CodePoint == 0xD7FF - 1
        {"\xee\x80\x80", 1},      // CodePoint == 0xE000
        {"\xee\x80\x81", 1},      // CodePoint == 0xE000 + 1
        {"\xef\xbf\xbf", 1},      // CodePoint == 0xFFFF
        {"\xef\xbf\xbe", 1},      // CodePoint == 0xFFFF - 1
        {"\xf0\x90\x80\x80", 2},  // CodePoint == 0x10000
        {"\xf0\x90\x80\x81", 2},  // CodePoint == 0x10000 + 1
    }};

    for (const TestCase& testCase : kTestCases) {
        dawn::native::ResultOrError<uint64_t> resultOrError =
            dawn::native::CountUTF16CodeUnitsFromUTF8String(std::string_view(testCase.u8String));
        ASSERT_TRUE(resultOrError.IsSuccess());
        ASSERT_EQ(testCase.lengthInUTF16, resultOrError.AcquireSuccess());
    }
}

TEST_F(CountUTF16CodeUnitsFromUTF8StringTest, InvalidUnicodeString) {
    // Referenced from src/tint/utils/unicode_test.cc
    constexpr std::array<const char*, 12> kTestCases = {{
        "\xed\xa0\x80",  // CodePoint == 0xD7FF + 1
        "\xed\xbf\xbf",  // CodePoint == 0xE000 - 1
        "ab\xed\xa0\x80",
        "\xd0",              // 2-bytes, missing second byte
        "\xe8\x8f",          // 3-bytes, missing third byte
        "\xf4\x8f\x8f",      // 4-bytes, missing fourth byte
        "\xd0\x7f",          // 2-bytes, second byte MSB unset
        "\xe8\x7f\x8f",      // 3-bytes, second byte MSB unset
        "\xe8\x8f\x7f",      // 3-bytes, third byte MSB unset
        "\xf4\x7f\x8f\x8f",  // 4-bytes, second byte MSB unset
        "\xf4\x8f\x7f\x8f",  // 4-bytes, third byte MSB unset
        "\xf4\x8f\x8f\x7f",  // 4-bytes, fourth byte MSB unset
    }};

    for (const char* testCase : kTestCases) {
        dawn::native::ResultOrError<uint64_t> resultOrError =
            dawn::native::CountUTF16CodeUnitsFromUTF8String(std::string_view(testCase));
        ASSERT_TRUE(resultOrError.IsError());
        std::ignore = resultOrError.AcquireError();
    }
}
