// Copyright 2023 The Tint Authors.
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

#include "src/tint/writer/spirv/ir/test_helper_ir.h"

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Constant_Bool) {
    generator_.Constant(b.Constant(true));
    generator_.Constant(b.Constant(false));
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeBool
%1 = OpConstantTrue %2
%3 = OpConstantFalse %2
)");
}

TEST_F(SpvGeneratorImplTest, Constant_I32) {
    generator_.Constant(b.Constant(i32(42)));
    generator_.Constant(b.Constant(i32(-1)));
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeInt 32 1
%1 = OpConstant %2 42
%3 = OpConstant %2 -1
)");
}

TEST_F(SpvGeneratorImplTest, Constant_U32) {
    generator_.Constant(b.Constant(u32(42)));
    generator_.Constant(b.Constant(u32(4000000000)));
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeInt 32 0
%1 = OpConstant %2 42
%3 = OpConstant %2 4000000000
)");
}

TEST_F(SpvGeneratorImplTest, Constant_F32) {
    generator_.Constant(b.Constant(f32(42)));
    generator_.Constant(b.Constant(f32(-1)));
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeFloat 32
%1 = OpConstant %2 42
%3 = OpConstant %2 -1
)");
}

TEST_F(SpvGeneratorImplTest, Constant_F16) {
    generator_.Constant(b.Constant(f16(42)));
    generator_.Constant(b.Constant(f16(-1)));
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeFloat 16
%1 = OpConstant %2 0x1.5p+5
%3 = OpConstant %2 -0x1p+0
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec4Bool) {
    auto* v = b.create<constant::Composite>(
        mod.types.vec4(mod.types.Get<type::Bool>()),
        utils::Vector{b.Bool(true), b.Bool(false), b.Bool(false), b.Bool(true)}, false, true);

    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeBool
%2 = OpTypeVector %3 4
%4 = OpConstantTrue %3
%5 = OpConstantFalse %3
%1 = OpConstantComposite %2 %4 %5 %5 %4
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec2i) {
    auto* v = b.create<constant::Composite>(mod.types.vec2(mod.types.Get<type::I32>()),
                                            utils::Vector{b.I32(42), b.I32(-1)}, false, false);
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeInt 32 1
%2 = OpTypeVector %3 2
%4 = OpConstant %3 42
%5 = OpConstant %3 -1
%1 = OpConstantComposite %2 %4 %5
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec3u) {
    auto* v = b.create<constant::Composite>(mod.types.vec3(mod.types.Get<type::U32>()),
                                            utils::Vector{b.U32(42), b.U32(0), b.U32(4000000000)},
                                            false, true);
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeInt 32 0
%2 = OpTypeVector %3 3
%4 = OpConstant %3 42
%5 = OpConstant %3 0
%6 = OpConstant %3 4000000000
%1 = OpConstantComposite %2 %4 %5 %6
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec4f) {
    auto* v = b.create<constant::Composite>(
        mod.types.vec4(mod.types.Get<type::F32>()),
        utils::Vector{b.F32(42), b.F32(0), b.F32(0.25), b.F32(-1)}, false, true);
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%4 = OpConstant %3 42
%5 = OpConstant %3 0
%6 = OpConstant %3 0.25
%7 = OpConstant %3 -1
%1 = OpConstantComposite %2 %4 %5 %6 %7
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec2h) {
    auto* v = b.create<constant::Composite>(mod.types.vec2(mod.types.Get<type::F16>()),
                                            utils::Vector{b.F16(42), b.F16(0.25)}, false, false);
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 2
%4 = OpConstant %3 0x1.5p+5
%5 = OpConstant %3 0x1p-2
%1 = OpConstantComposite %2 %4 %5
)");
}

// Test that we do not emit the same constant more than once.
TEST_F(SpvGeneratorImplTest, Constant_Deduplicate) {
    generator_.Constant(b.Constant(i32(42)));
    generator_.Constant(b.Constant(i32(42)));
    generator_.Constant(b.Constant(i32(42)));
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeInt 32 1
%1 = OpConstant %2 42
)");
}

}  // namespace
}  // namespace tint::writer::spirv
