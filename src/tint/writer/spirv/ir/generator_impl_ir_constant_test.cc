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
    auto* t = b.Constant(true);
    auto* f = b.Constant(false);
    auto* v = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(mod.types.Get<type::Bool>(), 4u),
        utils::Vector{t->Value(), f->Value(), f->Value(), t->Value()}, false, true);
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeBool
%2 = OpTypeVector %3 4
%4 = OpConstantTrue %3
%5 = OpConstantFalse %3
%1 = OpConstantComposite %2 %4 %5 %5 %4
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec2i) {
    auto* i = mod.types.Get<type::I32>();
    auto* i_42 = b.Constant(i32(42));
    auto* i_n1 = b.Constant(i32(-1));
    auto* v = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(i, 2u), utils::Vector{i_42->Value(), i_n1->Value()}, false,
        false);
    generator_.Constant(b.Constant(v));
    EXPECT_EQ(DumpTypes(), R"(%3 = OpTypeInt 32 1
%2 = OpTypeVector %3 2
%4 = OpConstant %3 42
%5 = OpConstant %3 -1
%1 = OpConstantComposite %2 %4 %5
)");
}

TEST_F(SpvGeneratorImplTest, Constant_Vec3u) {
    auto* u = mod.types.Get<type::U32>();
    auto* u_42 = b.Constant(u32(42));
    auto* u_0 = b.Constant(u32(0));
    auto* u_4b = b.Constant(u32(4000000000));
    auto* v = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(u, 3u),
        utils::Vector{u_42->Value(), u_0->Value(), u_4b->Value()}, false, true);
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
    auto* f = mod.types.Get<type::F32>();
    auto* f_42 = b.Constant(f32(42));
    auto* f_0 = b.Constant(f32(0));
    auto* f_q = b.Constant(f32(0.25));
    auto* f_n1 = b.Constant(f32(-1));
    auto* v = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(f, 4u),
        utils::Vector{f_42->Value(), f_0->Value(), f_q->Value(), f_n1->Value()}, false, true);
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
    auto* h = mod.types.Get<type::F16>();
    auto* h_42 = b.Constant(f16(42));
    auto* h_q = b.Constant(f16(0.25));
    auto* v = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(h, 2u), utils::Vector{h_42->Value(), h_q->Value()}, false,
        false);
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
