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

#include "src/tint/writer/spirv/scalar_constant.h"
#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

using SpirvScalarConstantTest = TestHelper;

TEST_F(SpirvScalarConstantTest, Equality) {
    ScalarConstant a{};
    ScalarConstant b{};
    EXPECT_EQ(a, b);

    a.kind = ScalarConstant::Kind::kU32;
    EXPECT_NE(a, b);
    b.kind = ScalarConstant::Kind::kU32;
    EXPECT_EQ(a, b);

    a.value.b = true;
    EXPECT_NE(a, b);
    b.value.b = true;
    EXPECT_EQ(a, b);
}

TEST_F(SpirvScalarConstantTest, U32) {
    auto c = ScalarConstant::U32(123);
    EXPECT_EQ(c.value.u32, 123u);
    EXPECT_EQ(c.kind, ScalarConstant::Kind::kU32);
}

TEST_F(SpirvScalarConstantTest, F16) {
    auto c = ScalarConstant::F16(123.456f);
    // 123.456f will be quantized to f16 123.4375h, bit pattern 0x57b7
    EXPECT_EQ(c.value.f16.bits_representation, 0x57b7u);
    EXPECT_EQ(c.kind, ScalarConstant::Kind::kF16);
}

}  // namespace
}  // namespace tint::writer::spirv
