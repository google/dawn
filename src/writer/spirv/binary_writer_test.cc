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

#include "src/writer/spirv/binary_writer.h"

#include <cstring>

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/writer/spirv/builder.h"

namespace tint {
namespace writer {
namespace spirv {

using BinaryWriterTest = testing::Test;

TEST_F(BinaryWriterTest, Preamble) {
  Builder b;
  BinaryWriter bw;
  ASSERT_TRUE(bw.Write(b));

  auto res = bw.result();
  ASSERT_EQ(res.size(), 5);
  EXPECT_EQ(res[0], spv::MagicNumber);
  EXPECT_EQ(res[1], 0x00010300);  // SPIR-V 1.3
  EXPECT_EQ(res[2], 0);           // Generator ID
  EXPECT_EQ(res[3], 1);           // ID Bound
  EXPECT_EQ(res[4], 0);           // Reserved
}

TEST_F(BinaryWriterTest, Float) {
  Builder b;
  b.push_preamble(spv::Op::OpNop, {Operand::Float(2.4f)});
  BinaryWriter bw;
  ASSERT_TRUE(bw.Write(b));

  auto res = bw.result();
  ASSERT_EQ(res.size(), 7);
  float f;
  memcpy(&f, res.data() + 6, 4);
  EXPECT_EQ(f, 2.4f);
}

TEST_F(BinaryWriterTest, Int) {
  Builder b;
  b.push_preamble(spv::Op::OpNop, {Operand::Int(2)});
  BinaryWriter bw;
  ASSERT_TRUE(bw.Write(b));

  auto res = bw.result();
  ASSERT_EQ(res.size(), 7);
  EXPECT_EQ(res[6], 2);
}

TEST_F(BinaryWriterTest, String) {
  Builder b;
  b.push_preamble(spv::Op::OpNop, {Operand::String("my_string")});
  BinaryWriter bw;
  ASSERT_TRUE(bw.Write(b));

  auto res = bw.result();
  ASSERT_EQ(res.size(), 9);

  uint8_t* v = reinterpret_cast<uint8_t*>(res.data()) + (6 * 4);
  EXPECT_EQ(v[0], 'm');
  EXPECT_EQ(v[1], 'y');
  EXPECT_EQ(v[2], '_');
  EXPECT_EQ(v[3], 's');
  EXPECT_EQ(v[4], 't');
  EXPECT_EQ(v[5], 'r');
  EXPECT_EQ(v[6], 'i');
  EXPECT_EQ(v[7], 'n');
  EXPECT_EQ(v[8], 'g');
  EXPECT_EQ(v[9], '\0');
  EXPECT_EQ(v[10], '\0');
  EXPECT_EQ(v[11], '\0');
}

TEST_F(BinaryWriterTest, String_Multiple4Length) {
  Builder b;
  b.push_preamble(spv::Op::OpNop, {Operand::String("mystring")});
  BinaryWriter bw;
  ASSERT_TRUE(bw.Write(b));

  auto res = bw.result();
  ASSERT_EQ(res.size(), 9);

  uint8_t* v = reinterpret_cast<uint8_t*>(res.data()) + (6 * 4);
  EXPECT_EQ(v[0], 'm');
  EXPECT_EQ(v[1], 'y');
  EXPECT_EQ(v[2], 's');
  EXPECT_EQ(v[3], 't');
  EXPECT_EQ(v[4], 'r');
  EXPECT_EQ(v[5], 'i');
  EXPECT_EQ(v[6], 'n');
  EXPECT_EQ(v[7], 'g');
  EXPECT_EQ(v[8], '\0');
  EXPECT_EQ(v[9], '\0');
  EXPECT_EQ(v[10], '\0');
  EXPECT_EQ(v[11], '\0');
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
