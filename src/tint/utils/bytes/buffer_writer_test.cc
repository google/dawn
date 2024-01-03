// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/utils/bytes/buffer_writer.h"

#include "gmock/gmock.h"
#include "src/tint/utils/containers/transform.h"

namespace tint::bytes {
namespace {

template <typename T, typename U, size_t N>
Vector<T, N> Cast(const Vector<U, N>& in) {
    return Transform(in, [](auto el) { return static_cast<T>(el); });
}

TEST(BufferWriterTest, IntegerBigEndian) {
    BufferWriter<16> writer;
    EXPECT_EQ(writer.Int(0x10203040u, Endianness::kBig), Success);
    EXPECT_THAT(Cast<int>(writer.buffer), testing::ElementsAre(0x10, 0x20, 0x30, 0x40));
}

TEST(BufferWriterTest, IntegerLittleEndian) {
    BufferWriter<16> writer;
    EXPECT_EQ(writer.Int(0x10203040u, Endianness::kLittle), Success);
    EXPECT_THAT(Cast<int>(writer.buffer), testing::ElementsAre(0x40, 0x30, 0x20, 0x10));
}

TEST(BufferWriterTest, Float) {
    BufferWriter<16> writer;
    EXPECT_EQ(writer.Float<float>(8.5f), Success);
    EXPECT_THAT(Cast<int>(writer.buffer), testing::ElementsAre(0x00, 0x00, 0x08, 0x41));
}

TEST(BufferWriterTest, Bool) {
    BufferWriter<16> writer;
    EXPECT_EQ(writer.Bool(true), Success);
    EXPECT_THAT(Cast<int>(writer.buffer), testing::ElementsAre(0x01));
}

}  // namespace
}  // namespace tint::bytes
