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

#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

using BinaryWriterTest = TestHelper;

TEST_F(BinaryWriterTest, Preamble) {
    BinaryWriter bw;
    bw.WriteHeader(5);

    auto res = bw.result();
    ASSERT_EQ(res.size(), 5u);
    EXPECT_EQ(res[0], spv::MagicNumber);
    EXPECT_EQ(res[1], 0x00010300u);  // SPIR-V 1.3
    EXPECT_EQ(res[2], 23u << 16);    // Generator ID
    EXPECT_EQ(res[3], 5u);           // ID Bound
    EXPECT_EQ(res[4], 0u);           // Reserved
}

TEST_F(BinaryWriterTest, Float) {
    Module m;

    m.PushAnnot(spv::Op::OpKill, {Operand(2.4f)});
    BinaryWriter bw;
    bw.WriteModule(&m);

    auto res = bw.result();
    ASSERT_EQ(res.size(), 2u);
    float f;
    memcpy(&f, res.data() + 1, 4);
    EXPECT_EQ(f, 2.4f);
}

TEST_F(BinaryWriterTest, Int) {
    Module m;

    m.PushAnnot(spv::Op::OpKill, {Operand(2u)});
    BinaryWriter bw;
    bw.WriteModule(&m);

    auto res = bw.result();
    ASSERT_EQ(res.size(), 2u);
    EXPECT_EQ(res[1], 2u);
}

TEST_F(BinaryWriterTest, String) {
    Module m;

    m.PushAnnot(spv::Op::OpKill, {Operand("my_string")});
    BinaryWriter bw;
    bw.WriteModule(&m);

    auto res = bw.result();
    ASSERT_EQ(res.size(), 4u);

    uint8_t* v = reinterpret_cast<uint8_t*>(res.data() + 1);
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
    Module m;

    m.PushAnnot(spv::Op::OpKill, {Operand("mystring")});
    BinaryWriter bw;
    bw.WriteModule(&m);

    auto res = bw.result();
    ASSERT_EQ(res.size(), 4u);

    uint8_t* v = reinterpret_cast<uint8_t*>(res.data() + 1);
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

TEST_F(BinaryWriterTest, TestInstructionWriter) {
    Instruction i1{spv::Op::OpKill, {Operand(2u)}};
    Instruction i2{spv::Op::OpKill, {Operand(4u)}};

    BinaryWriter bw;
    bw.WriteInstruction(i1);
    bw.WriteInstruction(i2);

    auto res = bw.result();
    ASSERT_EQ(res.size(), 4u);
    EXPECT_EQ(res[1], 2u);
    EXPECT_EQ(res[3], 4u);
}

}  // namespace
}  // namespace tint::writer::spirv
