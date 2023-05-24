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
    auto const_bool = [&](bool val) { return mod.constant_values.Get(val); };
    auto* v = mod.constant_values.Composite(
        mod.Types().vec4(mod.Types().bool_()),
        utils::Vector{const_bool(true), const_bool(false), const_bool(false), const_bool(true)});

    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeBool
%2 = OpTypeVector %3 4
%4 = OpConstantTrue %3
%5 = OpConstantFalse %3
%1 = OpConstantComposite %2 %4 %5 %5 %4
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec2i) {
    auto const_i32 = [&](float val) { return mod.constant_values.Get(i32(val)); };
    auto* v = mod.constant_values.Composite(mod.Types().vec2(mod.Types().i32()),
                                            utils::Vector{const_i32(42), const_i32(-1)});
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeInt 32 1
%2 = OpTypeVector %3 2
%4 = OpConstant %3 42
%5 = OpConstant %3 -1
%1 = OpConstantComposite %2 %4 %5
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec3u) {
    auto const_u32 = [&](float val) { return mod.constant_values.Get(u32(val)); };
    auto* v = mod.constant_values.Composite(
        mod.Types().vec3(mod.Types().u32()),
        utils::Vector{const_u32(42), const_u32(0), const_u32(4000000000)});
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
    auto const_f32 = [&](float val) { return mod.constant_values.Get(f32(val)); };
    auto* v = mod.constant_values.Composite(
        mod.Types().vec4(mod.Types().f32()),
        utils::Vector{const_f32(42), const_f32(0), const_f32(0.25), const_f32(-1)});
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
    auto const_f16 = [&](float val) { return mod.constant_values.Get(f16(val)); };
    auto* v = mod.constant_values.Composite(mod.Types().vec2(mod.Types().f16()),
                                            utils::Vector{const_f16(42), const_f16(0.25)});
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 2
%4 = OpConstant %3 0x1.5p+5
%5 = OpConstant %3 0x1p-2
%1 = OpConstantComposite %2 %4 %5
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Mat2x3f) {
    auto const_f32 = [&](float val) { return mod.constant_values.Get(f32(val)); };
    auto* f32 = mod.Types().f32();
    auto* v = mod.constant_values.Composite(
        mod.Types().mat2x3(f32),
        utils::Vector{
            mod.constant_values.Composite(
                mod.Types().vec3(f32),
                utils::Vector{const_f32(42), const_f32(-1), const_f32(0.25)}),
            mod.constant_values.Composite(
                mod.Types().vec3(f32),
                utils::Vector{const_f32(-42), const_f32(0), const_f32(-0.25)}),
        });
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypeMatrix %3 2
%6 = OpConstant %4 42
%7 = OpConstant %4 -1
%8 = OpConstant %4 0.25
%5 = OpConstantComposite %3 %6 %7 %8
%10 = OpConstant %4 -42
%11 = OpConstant %4 0
%12 = OpConstant %4 -0.25
%9 = OpConstantComposite %3 %10 %11 %12
%1 = OpConstantComposite %2 %5 %9
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Mat4x2h) {
    auto const_f16 = [&](float val) { return mod.constant_values.Get(f16(val)); };
    auto* f16 = mod.Types().f16();
    auto* v = mod.constant_values.Composite(
        mod.Types().mat4x2(f16),
        utils::Vector{
            mod.constant_values.Composite(mod.Types().vec2(f16),
                                          utils::Vector{const_f16(42), const_f16(-1)}),
            mod.constant_values.Composite(mod.Types().vec2(f16),
                                          utils::Vector{const_f16(0), const_f16(0.25)}),
            mod.constant_values.Composite(mod.Types().vec2(f16),
                                          utils::Vector{const_f16(-42), const_f16(1)}),
            mod.constant_values.Composite(mod.Types().vec2(f16),
                                          utils::Vector{const_f16(0.5), const_f16(-0)}),
        });
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%4 = OpTypeFloat 16
%3 = OpTypeVector %4 2
%2 = OpTypeMatrix %3 4
%6 = OpConstant %4 0x1.5p+5
%7 = OpConstant %4 -0x1p+0
%5 = OpConstantComposite %3 %6 %7
%9 = OpConstant %4 0x0p+0
%10 = OpConstant %4 0x1p-2
%8 = OpConstantComposite %3 %9 %10
%12 = OpConstant %4 -0x1.5p+5
%13 = OpConstant %4 0x1p+0
%11 = OpConstantComposite %3 %12 %13
%15 = OpConstant %4 0x1p-1
%14 = OpConstantComposite %3 %15 %9
%1 = OpConstantComposite %2 %5 %8 %11 %14
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
