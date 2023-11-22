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

#include "src/tint/utils/bytes/reader.h"

#include "gtest/gtest.h"

namespace tint::bytes {
namespace {

template <typename... ARGS>
auto Data(ARGS&&... args) {
    return std::array{std::byte{static_cast<uint8_t>(args)}...};
}

TEST(BytesReaderTest, IntegerBigEndian) {
    auto data = Data(0x10, 0x20, 0x30, 0x40);
    auto u32 = Reader{Slice{data}, 0, Endianness::kBig}.Int<uint32_t>();
    EXPECT_EQ(u32, 0x10203040u);
    auto i32 = Reader{Slice{data}, 0, Endianness::kBig}.Int<int32_t>();
    EXPECT_EQ(i32, 0x10203040);
}

TEST(BytesReaderTest, IntegerBigEndian_Offset) {
    auto data = Data(0x10, 0x20, 0x30, 0x40, 0x50, 0x60);
    auto u32 = Reader{Slice{data}, 2, Endianness::kBig}.Int<uint32_t>();
    EXPECT_EQ(u32, 0x30405060u);
    auto i32 = Reader{Slice{data}, 2, Endianness::kBig}.Int<int32_t>();
    EXPECT_EQ(i32, 0x30405060);
}

TEST(BytesReaderTest, IntegerBigEndian_Clipped) {
    auto data = Data(0x10, 0x20, 0x30, 0x40);
    auto u32 = Reader{Slice{data}, 2, Endianness::kBig}.Int<uint32_t>();
    EXPECT_EQ(u32, 0x30400000u);
    auto i32 = Reader{Slice{data}, 2, Endianness::kBig}.Int<int32_t>();
    EXPECT_EQ(i32, 0x30400000);
}

TEST(BytesReaderTest, IntegerLittleEndian) {
    auto data = Data(0x10, 0x20, 0x30, 0x40);
    auto u32 = Reader{Slice{data}, 0, Endianness::kLittle}.Int<uint32_t>();
    EXPECT_EQ(u32, 0x40302010u);
    auto i32 = Reader{Slice{data}, 0, Endianness::kLittle}.Int<int32_t>();
    EXPECT_EQ(i32, 0x40302010);
}

TEST(BytesReaderTest, IntegerLittleEndian_Offset) {
    auto data = Data(0x10, 0x20, 0x30, 0x40, 0x50, 0x60);
    auto u32 = Reader{Slice{data}, 2, Endianness::kLittle}.Int<uint32_t>();
    EXPECT_EQ(u32, 0x60504030u);
    auto i32 = Reader{Slice{data}, 2, Endianness::kLittle}.Int<int32_t>();
    EXPECT_EQ(i32, 0x60504030);
}

TEST(BytesReaderTest, IntegerLittleEndian_Clipped) {
    auto data = Data(0x10, 0x20, 0x30, 0x40);
    auto u32 = Reader{Slice{data}, 2, Endianness::kLittle}.Int<uint32_t>();
    EXPECT_EQ(u32, 0x00004030u);
    auto i32 = Reader{Slice{data}, 2, Endianness::kLittle}.Int<int32_t>();
    EXPECT_EQ(i32, 0x00004030);
}

TEST(BytesReaderTest, Float) {
    auto data = Data(0x00, 0x00, 0x08, 0x41);
    float f32 = Reader{Slice{data}}.Float<float>();
    EXPECT_EQ(f32, 8.5f);
}

TEST(BytesReaderTest, Float_Offset) {
    auto data = Data(0x00, 0x00, 0x08, 0x41, 0x80, 0x3e);
    float f32 = Reader{Slice{data}, 2}.Float<float>();
    EXPECT_EQ(f32, 0.25049614f);
}

TEST(BytesReaderTest, Float_Clipped) {
    auto data = Data(0x00, 0x00, 0x08, 0x41);
    float f32 = Reader{Slice{data}, 2}.Float<float>();
    EXPECT_EQ(f32, 2.3329e-41f);
}

}  // namespace
}  // namespace tint::bytes
