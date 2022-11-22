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

#include "src/tint/utils/bitset.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(Bitset, Length) {
    Bitset<8> bits;
    EXPECT_EQ(bits.Length(), 0u);
    bits.Resize(100u);
    EXPECT_EQ(bits.Length(), 100u);
}

TEST(Bitset, AllBitsZero) {
    Bitset<8> bits;
    EXPECT_TRUE(bits.AllBitsZero());

    bits.Resize(4u);
    EXPECT_TRUE(bits.AllBitsZero());

    bits.Resize(100u);
    EXPECT_TRUE(bits.AllBitsZero());

    bits[63] = true;
    EXPECT_FALSE(bits.AllBitsZero());

    bits.Resize(60);
    EXPECT_TRUE(bits.AllBitsZero());

    bits.Resize(64);
    EXPECT_TRUE(bits.AllBitsZero());

    bits[4] = true;
    EXPECT_FALSE(bits.AllBitsZero());

    bits.Resize(8);
    EXPECT_FALSE(bits.AllBitsZero());
}

TEST(Bitset, InitCleared_NoSpill) {
    Bitset<256> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_FALSE(bits[i]);
    }
}

TEST(Bitset, InitCleared_Spill) {
    Bitset<64> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_FALSE(bits[i]);
    }
}

TEST(Bitset, ReadWrite_NoSpill) {
    Bitset<256> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = (i & 0x2) == 0;
    }
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], (i & 0x2) == 0);
    }
}

TEST(Bitset, ReadWrite_Spill) {
    Bitset<64> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = (i & 0x2) == 0;
    }
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], (i & 0x2) == 0);
    }
}

TEST(Bitset, ShinkGrowAlignedClears_NoSpill) {
    Bitset<256> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = true;
    }
    bits.Resize(64);
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], i < 64u);
    }
}

TEST(Bitset, ShinkGrowAlignedClears_Spill) {
    Bitset<64> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = true;
    }
    bits.Resize(64);
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], i < 64u);
    }
}
TEST(Bitset, ShinkGrowMisalignedClears_NoSpill) {
    Bitset<256> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = true;
    }
    bits.Resize(42);
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], i < 42u);
    }
}

TEST(Bitset, ShinkGrowMisalignedClears_Spill) {
    Bitset<64> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = true;
    }
    bits.Resize(42);
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], i < 42u);
    }
}

}  // namespace
}  // namespace tint::utils
