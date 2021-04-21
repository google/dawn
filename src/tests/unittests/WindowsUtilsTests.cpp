// Copyright 2021 The Dawn Authors
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

#include <gtest/gtest.h>

#include "common/WindowsUtils.h"

TEST(WindowsUtilsTests, WCharToUTF8) {
    // Test the empty string
    ASSERT_EQ("", WCharToUTF8(L""));

    // Test ASCII characters
    ASSERT_EQ("abc", WCharToUTF8(L"abc"));

    // Test ASCII characters
    ASSERT_EQ("abc", WCharToUTF8(L"abc"));

    // Test two-byte utf8 character
    ASSERT_EQ("\xd1\x90", WCharToUTF8(L"\x450"));

    // Test three-byte utf8 codepoint
    ASSERT_EQ("\xe1\x81\x90", WCharToUTF8(L"\x1050"));
}

TEST(WindowsUtilsTests, UTF8ToWStr) {
    // Test the empty string
    ASSERT_EQ(L"", UTF8ToWStr(""));

    // Test ASCII characters
    ASSERT_EQ(L"abc", UTF8ToWStr("abc"));

    // Test ASCII characters
    ASSERT_EQ(L"abc", UTF8ToWStr("abc"));

    // Test two-byte utf8 character
    ASSERT_EQ(L"\x450", UTF8ToWStr("\xd1\x90"));

    // Test three-byte utf8 codepoint
    ASSERT_EQ(L"\x1050", UTF8ToWStr("\xe1\x81\x90"));
}