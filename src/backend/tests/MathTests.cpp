// Copyright 2017 The NXT Authors
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

#include "common/Math.h"

using namespace backend;

// Tests for ScanForward
TEST(Math, ScanForward) {
    // Test extrema
    ASSERT_EQ(ScanForward(1), 0);
    ASSERT_EQ(ScanForward(0x8000000000000000), 63);

    // Test with more than one bit set.
    ASSERT_EQ(ScanForward(256), 8);
    ASSERT_EQ(ScanForward(256 + 32), 5);
    ASSERT_EQ(ScanForward(1024 + 256 + 32), 5);
}

// Tests for Log2
TEST(Math, Log2) {
    // Test extrema
    ASSERT_EQ(Log2(1), 0);
    ASSERT_EQ(Log2(0xFFFFFFFF), 31);

    // Test boundary between two logs
    ASSERT_EQ(Log2(0x80000000), 31);
    ASSERT_EQ(Log2(0x7FFFFFFF), 30);

    ASSERT_EQ(Log2(16), 4);
    ASSERT_EQ(Log2(15), 3);
}

// Tests for IsPowerOfTwo
TEST(Math, IsPowerOfTwo) {
    ASSERT_TRUE(IsPowerOfTwo(1));
    ASSERT_TRUE(IsPowerOfTwo(2));
    ASSERT_FALSE(IsPowerOfTwo(3));

    ASSERT_TRUE(IsPowerOfTwo(0x8000000));
    ASSERT_FALSE(IsPowerOfTwo(0x8000400));
}

// Tests for Align
TEST(Math, Align) {
    constexpr size_t kTestAlignment = 8;

    char buffer[kTestAlignment * 4];

    for (size_t i = 0; i < 2 * kTestAlignment; ++i) {
        char* unaligned = &buffer[i];
        char* aligned = Align(unaligned, kTestAlignment);

        ASSERT_GE(aligned - unaligned, 0);
        ASSERT_LT(aligned - unaligned, kTestAlignment);
        ASSERT_EQ(reinterpret_cast<intptr_t>(aligned) & (kTestAlignment -1), 0);
    }
}

// Tests for IsAligned
TEST(Math, IsAligned) {
    constexpr size_t kTestAlignment = 8;

    char buffer[kTestAlignment * 4];

    for (size_t i = 0; i < 2 * kTestAlignment; ++i) {
        char* unaligned = &buffer[i];
        char* aligned = Align(unaligned, kTestAlignment);

        ASSERT_EQ(IsAligned(unaligned, kTestAlignment), unaligned == aligned);
    }
}
