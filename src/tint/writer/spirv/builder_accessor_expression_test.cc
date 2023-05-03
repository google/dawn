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

#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Let_IndexAccessor_Vector) {
    // let ary = vec3<i32>(1, 2, 3);
    // var x = ary[1i];

    auto* ary = Let("ary", vec3<i32>(1_i, 2_i, 3_i));
    auto* x = Var("x", IndexAccessor(ary, 1_i));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 3
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%9 = OpConstant %6 3
%10 = OpConstantComposite %5 %7 %8 %9
%13 = OpTypePointer Function %6
%14 = OpConstantNull %6
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%12 = OpVariable %13 Function %14
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%11 = OpCompositeExtract %6 %10 1
OpStore %12 %11
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Const_IndexAccessor_Vector) {
    // const ary = vec3<i32>(1, 2, 3);
    // var x = ary[1i];

    auto* ary = Const("ary", vec3<i32>(1_i, 2_i, 3_i));
    auto* x = Var("x", IndexAccessor(ary, 1_i));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 1
%6 = OpConstant %5 2
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%7 = OpVariable %8 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpStore %7 %6
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Runtime_IndexAccessor_Vector) {
    // var ary : vec3<u32>;
    // var x = ary[1i];

    auto* ary = Var("ary", ty.vec3<u32>());
    auto* x = Var("x", IndexAccessor(ary, 1_i));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%10 = OpTypeInt 32 1
%11 = OpConstant %10 1
%12 = OpTypePointer Function %8
%16 = OpConstantNull %8
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
%15 = OpVariable %12 Function %16
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%13 = OpAccessChain %12 %5 %11
%14 = OpLoad %8 %13
OpStore %15 %14
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Dynamic_IndexAccessor_Vector) {
    // var ary : vec3<f32>;
    // var idx : i32;
    // var x = ary[idx];

    auto* ary = Var("ary", ty.vec3<f32>());
    auto* idx = Var("idx", ty.i32());
    auto* x = Var("x", IndexAccessor(ary, idx));
    WrapInFunction(ary, idx, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%12 = OpTypeInt 32 1
%11 = OpTypePointer Function %12
%13 = OpConstantNull %12
%15 = OpTypePointer Function %8
%19 = OpConstantNull %8
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
%10 = OpVariable %11 Function %13
%18 = OpVariable %15 Function %19
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(%14 = OpLoad %12 %10
%16 = OpAccessChain %15 %5 %14
%17 = OpLoad %8 %16
OpStore %18 %17
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Const_IndexAccessor_Vector2) {
    // let ary : vec3<i32>(1, 2, 3);
    // var x = ary[1i + 1i];

    auto* ary = Let("ary", vec3<i32>(1_i, 2_i, 3_i));
    auto* x = Var("x", IndexAccessor(ary, Add(1_i, 1_i)));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 3
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%9 = OpConstant %6 3
%10 = OpConstantComposite %5 %7 %8 %9
%13 = OpTypePointer Function %6
%14 = OpConstantNull %6
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%12 = OpVariable %13 Function %14
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%11 = OpCompositeExtract %6 %10 2
OpStore %12 %11
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Runtime_IndexAccessor_Vector2) {
    // var ary : vec3<f32>;
    // var x = ary[1i + 1i];

    auto* ary = Var("ary", ty.vec3<f32>());
    auto* x = Var("x", IndexAccessor(ary, Add(1_i, 1_i)));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%10 = OpTypeInt 32 1
%11 = OpConstant %10 2
%12 = OpTypePointer Function %8
%16 = OpConstantNull %8
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
%15 = OpVariable %12 Function %16
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%13 = OpAccessChain %12 %5 %11
%14 = OpLoad %8 %13
OpStore %15 %14
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Dynamic_IndexAccessor_Vector2) {
    // var ary : vec3<f32>;
    // var one = 1i;
    // var x = ary[one + 2i];

    auto* ary = Var("ary", ty.vec3<f32>());
    auto* one = Var("one", Expr(1_i));
    auto* x = Var("x", IndexAccessor(ary, Add(one, 2_i)));
    WrapInFunction(ary, one, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%10 = OpTypeInt 32 1
%11 = OpConstant %10 1
%13 = OpTypePointer Function %10
%14 = OpConstantNull %10
%16 = OpConstant %10 2
%18 = OpTypePointer Function %8
%22 = OpConstantNull %8
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
%12 = OpVariable %13 Function %14
%21 = OpVariable %18 Function %22
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %12 %11
%15 = OpLoad %10 %12
%17 = OpIAdd %10 %15 %16
%19 = OpAccessChain %18 %5 %17
%20 = OpLoad %8 %19
OpStore %21 %20
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Let_IndexAccessor_Array_MultiLevel) {
    // let ary = array<vec3<f32>, 2u>(vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(4.0f, 5.0f, 6.0f));
    // var x = ary[1i][2i];

    auto* ary = Let("ary", array(ty.vec3<f32>(), 2_u, vec3<f32>(1._f, 2._f, 3._f),
                                 vec3<f32>(4._f, 5._f, 6._f)));
    auto* x = Var("x", IndexAccessor(IndexAccessor(ary, 1_i), 2_i));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 3
%8 = OpTypeInt 32 0
%9 = OpConstant %8 2
%5 = OpTypeArray %6 %9
%10 = OpConstant %7 1
%11 = OpConstant %7 2
%12 = OpConstant %7 3
%13 = OpConstantComposite %6 %10 %11 %12
%14 = OpConstant %7 4
%15 = OpConstant %7 5
%16 = OpConstant %7 6
%17 = OpConstantComposite %6 %14 %15 %16
%18 = OpConstantComposite %5 %13 %17
%19 = OpTypeInt 32 1
%20 = OpConstant %19 1
%22 = OpConstant %19 2
%25 = OpTypePointer Function %7
%26 = OpConstantNull %7
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%24 = OpVariable %25 Function %26
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%21 = OpCompositeExtract %6 %18 1
%23 = OpCompositeExtract %7 %21 2
OpStore %24 %23
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Const_IndexAccessor_Array_MultiLevel) {
    // const ary = array<vec3<f32>, 2u>(vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(4.0f, 5.0f, 6.0f));
    // var x = ary[1i][2i];

    auto* ary = Const("ary", array(ty.vec3<f32>(), 2_u, vec3<f32>(1._f, 2._f, 3._f),
                                   vec3<f32>(4._f, 5._f, 6._f)));
    auto* x = Var("x", IndexAccessor(IndexAccessor(ary, 1_i), 2_i));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 6
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%7 = OpVariable %8 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpStore %7 %6
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Runtime_IndexAccessor_Array_MultiLevel) {
    // var ary : array<vec3<f32>, 4u>;
    // var x = ary[1i][2i];

    auto* ary = Var("ary", ty.array(ty.vec3<f32>(), 4_u));
    auto* x = Var("x", IndexAccessor(IndexAccessor(ary, 1_i), 2_i));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%10 = OpTypeInt 32 0
%11 = OpConstant %10 4
%7 = OpTypeArray %8 %11
%6 = OpTypePointer Function %7
%12 = OpConstantNull %7
%13 = OpTypeInt 32 1
%14 = OpConstant %13 1
%15 = OpConstant %13 2
%16 = OpTypePointer Function %9
%20 = OpConstantNull %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %12
%19 = OpVariable %16 Function %20
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%17 = OpAccessChain %16 %5 %14 %15
%18 = OpLoad %9 %17
OpStore %19 %18
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Dynamic_IndexAccessor_Array_MultiLevel) {
    // var ary : array<vec3<f32>, 4u>;
    // var one = 1i;
    // var x = ary[one][2i];

    auto* ary = Var("ary", ty.array(ty.vec3<f32>(), 4_u));
    auto* one = Var("one", Expr(3_i));
    auto* x = Var("x", IndexAccessor(IndexAccessor(ary, one), 2_i));
    WrapInFunction(ary, one, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%10 = OpTypeInt 32 0
%11 = OpConstant %10 4
%7 = OpTypeArray %8 %11
%6 = OpTypePointer Function %7
%12 = OpConstantNull %7
%13 = OpTypeInt 32 1
%14 = OpConstant %13 3
%16 = OpTypePointer Function %13
%17 = OpConstantNull %13
%19 = OpConstant %13 2
%20 = OpTypePointer Function %9
%24 = OpConstantNull %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %12
%15 = OpVariable %16 Function %17
%23 = OpVariable %20 Function %24
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %15 %14
%18 = OpLoad %13 %15
%21 = OpAccessChain %20 %5 %18 %19
%22 = OpLoad %9 %21
OpStore %23 %22
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Const_IndexAccessor_Array_ArrayWithSwizzle) {
    // let ary = array<vec3<f32>, 2u>(vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(4.0f, 5.0f, 6.0f));
    // var x = a[1i].xy;

    auto* ary = Let("ary", array(ty.vec3<f32>(), 2_u, vec3<f32>(1._f, 2._f, 3._f),
                                 vec3<f32>(4._f, 5._f, 6._f)));
    auto* x = Var("x", MemberAccessor(IndexAccessor("ary", 1_i), "xy"));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 3
%8 = OpTypeInt 32 0
%9 = OpConstant %8 2
%5 = OpTypeArray %6 %9
%10 = OpConstant %7 1
%11 = OpConstant %7 2
%12 = OpConstant %7 3
%13 = OpConstantComposite %6 %10 %11 %12
%14 = OpConstant %7 4
%15 = OpConstant %7 5
%16 = OpConstant %7 6
%17 = OpConstantComposite %6 %14 %15 %16
%18 = OpConstantComposite %5 %13 %17
%19 = OpTypeInt 32 1
%20 = OpConstant %19 1
%22 = OpTypeVector %7 2
%25 = OpTypePointer Function %22
%26 = OpConstantNull %22
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%24 = OpVariable %25 Function %26
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%21 = OpCompositeExtract %6 %18 1
%23 = OpVectorShuffle %22 %21 %21 0 1
OpStore %24 %23
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Runtime_IndexAccessor_Array_ArrayWithSwizzle) {
    // var ary : array<vec3<f32>, 4u>;
    // var x = ary[1i].xy;

    auto* ary = Var("ary", ty.array(ty.vec3<f32>(), 4_u));
    auto* x = Var("x", MemberAccessor(IndexAccessor("ary", 1_i), "xy"));
    WrapInFunction(ary, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%10 = OpTypeInt 32 0
%11 = OpConstant %10 4
%7 = OpTypeArray %8 %11
%6 = OpTypePointer Function %7
%12 = OpConstantNull %7
%13 = OpTypeInt 32 1
%14 = OpConstant %13 1
%15 = OpTypePointer Function %8
%17 = OpTypeVector %9 2
%21 = OpTypePointer Function %17
%22 = OpConstantNull %17
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %12
%20 = OpVariable %21 Function %22
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%16 = OpAccessChain %15 %5 %14
%18 = OpLoad %8 %16
%19 = OpVectorShuffle %17 %18 %18 0 1
OpStore %20 %19
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Dynamic_IndexAccessor_Array_ArrayWithSwizzle) {
    // var ary : array<vec3<f32>, 4u>;
    // var one = 1i;
    // var x = ary[one].xy;

    auto* ary = Var("ary", ty.array(ty.vec3<f32>(), 4_u));
    auto* one = Var("one", Expr(1_i));
    auto* x = Var("x", MemberAccessor(IndexAccessor("ary", one), "xy"));
    WrapInFunction(ary, one, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%10 = OpTypeInt 32 0
%11 = OpConstant %10 4
%7 = OpTypeArray %8 %11
%6 = OpTypePointer Function %7
%12 = OpConstantNull %7
%13 = OpTypeInt 32 1
%14 = OpConstant %13 1
%16 = OpTypePointer Function %13
%17 = OpConstantNull %13
%19 = OpTypePointer Function %8
%21 = OpTypeVector %9 2
%25 = OpTypePointer Function %21
%26 = OpConstantNull %21
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %12
%15 = OpVariable %16 Function %17
%24 = OpVariable %25 Function %26
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %15 %14
%18 = OpLoad %13 %15
%20 = OpAccessChain %19 %5 %18
%22 = OpLoad %8 %20
%23 = OpVectorShuffle %21 %22 %22 0 1
OpStore %24 %23
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Let_IndexAccessor_Nested_Array_f32) {
    // let pos : array<array<f32, 2>, 3u> = array<vec2<f32, 2>, 3u>(
    //   array<f32, 2>(0.0, 0.5),
    //   array<f32, 2>(-0.5, -0.5),
    //   array<f32, 2>(0.5, -0.5));
    // var x = pos[1u][0u];

    auto* pos = Let("pos", ty.array(ty.vec2<f32>(), 3_u),
                    Call(ty.array(ty.vec2<f32>(), 3_u), vec2<f32>(0_f, 0.5_f),
                         vec2<f32>(-0.5_f, -0.5_f), vec2<f32>(0.5_f, -0.5_f)));
    auto* x = Var("x", IndexAccessor(IndexAccessor(pos, 1_u), 0_u));
    WrapInFunction(pos, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%8 = OpTypeInt 32 0
%9 = OpConstant %8 3
%5 = OpTypeArray %6 %9
%10 = OpConstantNull %7
%11 = OpConstant %7 0.5
%12 = OpConstantComposite %6 %10 %11
%13 = OpConstant %7 -0.5
%14 = OpConstantComposite %6 %13 %13
%15 = OpConstantComposite %6 %11 %13
%16 = OpConstantComposite %5 %12 %14 %15
%17 = OpConstant %8 1
%19 = OpConstantNull %8
%22 = OpTypePointer Function %7
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%21 = OpVariable %22 Function %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%18 = OpCompositeExtract %6 %16 1
%20 = OpCompositeExtract %7 %18 0
OpStore %21 %20
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Const_IndexAccessor_Nested_Array_f32) {
    // const pos : array<array<f32, 2>, 3u> = array<vec2<f32, 2>, 3u>(
    //   array<f32, 2>(0.0, 0.5),
    //   array<f32, 2>(-0.5, -0.5),
    //   array<f32, 2>(0.5, -0.5));
    // var x = pos[1u][0u];

    auto* pos = Const("pos", ty.array(ty.vec2<f32>(), 3_u),
                      Call(ty.array(ty.vec2<f32>(), 3_u), vec2<f32>(0_f, 0.5_f),
                           vec2<f32>(-0.5_f, -0.5_f), vec2<f32>(0.5_f, -0.5_f)));
    auto* x = Var("x", IndexAccessor(IndexAccessor(pos, 1_u), 0_u));
    WrapInFunction(pos, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 -0.5
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%7 = OpVariable %8 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpStore %7 %6
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Runtime_IndexAccessor_Array_Vec3_f32) {
    // var pos : array<vec3<f32>, 3u>;
    // var x = pos[1u][2u];

    auto* pos = Var("pos", ty.array(ty.vec3<f32>(), 3_a));
    auto* x = Var("x", IndexAccessor(IndexAccessor(pos, 1_u), 2_u));
    WrapInFunction(pos, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%10 = OpTypeInt 32 0
%11 = OpConstant %10 3
%7 = OpTypeArray %8 %11
%6 = OpTypePointer Function %7
%12 = OpConstantNull %7
%13 = OpConstant %10 1
%14 = OpConstant %10 2
%15 = OpTypePointer Function %9
%19 = OpConstantNull %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %12
%18 = OpVariable %15 Function %19
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%16 = OpAccessChain %15 %5 %13 %14
%17 = OpLoad %9 %16
OpStore %18 %17
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Dynamic_IndexAccessor_Nested_Array_f32) {
    // var pos : array<array<f32, 4>, 3u>;
    // var one = 1u;
    // var x = pos[one][2u];

    auto* pos = Var("pos", ty.array(ty.array<f32, 4>(), 3_u));
    auto* one = Var("one", Expr(2_u));
    auto* x = Var("x", IndexAccessor(IndexAccessor(pos, "one"), 2_u));
    WrapInFunction(pos, one, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%10 = OpTypeInt 32 0
%11 = OpConstant %10 4
%8 = OpTypeArray %9 %11
%12 = OpConstant %10 3
%7 = OpTypeArray %8 %12
%6 = OpTypePointer Function %7
%13 = OpConstantNull %7
%14 = OpConstant %10 2
%16 = OpTypePointer Function %10
%17 = OpConstantNull %10
%19 = OpTypePointer Function %9
%23 = OpConstantNull %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %13
%15 = OpVariable %16 Function %17
%22 = OpVariable %19 Function %23
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %15 %14
%18 = OpLoad %10 %15
%20 = OpAccessChain %19 %5 %18 %14
%21 = OpLoad %9 %20
OpStore %22 %21
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Let_IndexAccessor_Matrix) {
    // let a : mat2x2<f32>(vec2<f32>(1., 2.), vec2<f32>(3., 4.));
    // var x = a[1i]

    auto* a =
        Let("a", ty.mat2x2<f32>(),
            Call(ty.mat2x2<f32>(), Call(ty.vec2<f32>(), 1_f, 2_f), Call(ty.vec2<f32>(), 3_f, 4_f)));
    auto* x = Var("x", IndexAccessor("a", 1_i));
    WrapInFunction(a, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%5 = OpTypeMatrix %6 2
%8 = OpConstant %7 1
%9 = OpConstant %7 2
%10 = OpConstantComposite %6 %8 %9
%11 = OpConstant %7 3
%12 = OpConstant %7 4
%13 = OpConstantComposite %6 %11 %12
%14 = OpConstantComposite %5 %10 %13
%15 = OpTypeInt 32 1
%16 = OpConstant %15 1
%19 = OpTypePointer Function %6
%20 = OpConstantNull %6
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%18 = OpVariable %19 Function %20
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%17 = OpCompositeExtract %6 %14 1
OpStore %18 %17
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Const_IndexAccessor_Matrix) {
    // const a : mat2x2<f32>(vec2<f32>(1., 2.), vec2<f32>(3., 4.));
    // var x = a[1i]

    auto* a = Const(
        "a", ty.mat2x2<f32>(),
        Call(ty.mat2x2<f32>(), Call(ty.vec2<f32>(), 1_f, 2_f), Call(ty.vec2<f32>(), 3_f, 4_f)));
    auto* x = Var("x", IndexAccessor("a", 1_i));
    WrapInFunction(a, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 3
%8 = OpConstant %6 4
%9 = OpConstantComposite %5 %7 %8
%11 = OpTypePointer Function %5
%12 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%10 = OpVariable %11 Function %12
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpStore %10 %9
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Runtime_IndexAccessor_Matrix) {
    // var a : mat2x2<f32>;
    // var x = a[1i]

    auto* a = Var("a", ty.mat2x2<f32>());
    auto* x = Var("x", IndexAccessor("a", 1_i));
    WrapInFunction(a, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 2
%7 = OpTypeMatrix %8 2
%6 = OpTypePointer Function %7
%10 = OpConstantNull %7
%11 = OpTypeInt 32 1
%12 = OpConstant %11 1
%13 = OpTypePointer Function %8
%17 = OpConstantNull %8
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %10
%16 = OpVariable %13 Function %17
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%14 = OpAccessChain %13 %5 %12
%15 = OpLoad %8 %14
OpStore %16 %15
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, Dynamic_IndexAccessor_Matrix) {
    // var a : mat2x2<f32>;
    // var idx : i32
    // var x = a[idx]

    auto* a = Var("a", ty.mat2x2<f32>());
    auto* idx = Var("idx", ty.i32());
    auto* x = Var("x", IndexAccessor("a", idx));
    WrapInFunction(a, idx, x);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 2
%7 = OpTypeMatrix %8 2
%6 = OpTypePointer Function %7
%10 = OpConstantNull %7
%13 = OpTypeInt 32 1
%12 = OpTypePointer Function %13
%14 = OpConstantNull %13
%16 = OpTypePointer Function %8
%20 = OpConstantNull %8
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %10
%11 = OpVariable %12 Function %14
%19 = OpVariable %16 Function %20
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(%15 = OpLoad %13 %11
%17 = OpAccessChain %16 %5 %15
%18 = OpLoad %8 %17
OpStore %19 %18
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor) {
    // my_struct {
    //   a : f32
    //   b : f32
    // }
    // var ident : my_struct
    // ident.b

    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.f32()),
                                         Member("b", ty.f32()),
                                     });

    auto* var = Var("ident", ty.Of(s));

    auto* expr = MemberAccessor("ident", "b");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeStruct %8 %8
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%10 = OpTypeInt 32 0
%11 = OpConstant %10 1
%12 = OpTypePointer Function %8
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%13 = OpAccessChain %12 %5 %11
%14 = OpLoad %8 %13
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Nested) {
    // inner_struct {
    //   a : f32
    //   b : f32
    // }
    // my_struct {
    //   inner : inner_struct
    // }
    //
    // var ident : my_struct
    // ident.inner.a
    auto* inner_struct = Structure("Inner", utils::Vector{
                                                Member("a", ty.f32()),
                                                Member("b", ty.f32()),
                                            });

    auto* s_type = Structure("my_struct", utils::Vector{Member("inner", ty.Of(inner_struct))});

    auto* var = Var("ident", ty.Of(s_type));
    auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "b");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeStruct %9 %9
%7 = OpTypeStruct %8
%6 = OpTypePointer Function %7
%10 = OpConstantNull %7
%11 = OpTypeInt 32 0
%12 = OpConstant %11 0
%13 = OpConstant %11 1
%14 = OpTypePointer Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%15 = OpAccessChain %14 %5 %12 %13
%16 = OpLoad %9 %15
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_NonPointer) {
    // my_struct {
    //   a : f32
    //   b : f32
    // }
    // let ident : my_struct = my_struct();
    // ident.b

    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.f32()),
                                         Member("b", ty.f32()),
                                     });

    auto* var = Let("ident", ty.Of(s), Call(ty.Of(s), 0_f, 0_f));

    auto* expr = MemberAccessor("ident", "b");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeStruct %6 %6
%7 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%8 = OpCompositeExtract %6 %7 1
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Nested_NonPointer) {
    // inner_struct {
    //   a : f32
    //   b : f32
    // }
    // my_struct {
    //   inner : inner_struct
    // }
    //
    // let ident : my_struct = my_struct();
    // ident.inner.a
    auto* inner_struct = Structure("Inner", utils::Vector{
                                                Member("a", ty.f32()),
                                                Member("b", ty.f32()),
                                            });

    auto* s_type = Structure("my_struct", utils::Vector{Member("inner", ty.Of(inner_struct))});

    auto* var =
        Let("ident", ty.Of(s_type), Call(ty.Of(s_type), Call(ty.Of(inner_struct), 0_f, 0_f)));
    auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "b");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeStruct %7 %7
%5 = OpTypeStruct %6
%8 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%9 = OpCompositeExtract %6 %8 0
%10 = OpCompositeExtract %7 %9 1
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Nested_WithAlias) {
    // struct Inner {
    //   a : f32
    //   b : f32
    // };
    // type Alias = Inner;
    // my_struct {
    //   inner : Inner
    // }
    //
    // var ident : my_struct
    // ident.inner.a
    auto* inner_struct = Structure("Inner", utils::Vector{
                                                Member("a", ty.f32()),
                                                Member("b", ty.f32()),
                                            });

    auto* alias = Alias("Alias", ty.Of(inner_struct));
    auto* s_type = Structure("Outer", utils::Vector{Member("inner", ty.Of(alias))});

    auto* var = Var("ident", ty.Of(s_type));
    auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "a");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeStruct %9 %9
%7 = OpTypeStruct %8
%6 = OpTypePointer Function %7
%10 = OpConstantNull %7
%11 = OpTypeInt 32 0
%12 = OpConstant %11 0
%13 = OpTypePointer Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%14 = OpAccessChain %13 %5 %12 %12
%15 = OpLoad %9 %14
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Nested_Assignment_LHS) {
    // inner_struct {
    //   a : f32
    // }
    // my_struct {
    //   inner : inner_struct
    // }
    //
    // var ident : my_struct
    // ident.inner.a = 2.0f;
    auto* inner_struct = Structure("Inner", utils::Vector{
                                                Member("a", ty.f32()),
                                                Member("b", ty.f32()),
                                            });

    auto* s_type = Structure("my_struct", utils::Vector{Member("inner", ty.Of(inner_struct))});

    auto* var = Var("ident", ty.Of(s_type));
    auto* expr = Assign(MemberAccessor(MemberAccessor("ident", "inner"), "a"), Expr(2_f));
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeStruct %9 %9
%7 = OpTypeStruct %8
%6 = OpTypePointer Function %7
%10 = OpConstantNull %7
%11 = OpTypeInt 32 0
%12 = OpConstant %11 0
%13 = OpTypePointer Function %9
%15 = OpConstant %9 2
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%14 = OpAccessChain %13 %5 %12 %12
OpStore %14 %15
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Nested_Assignment_RHS) {
    // inner_struct {
    //   a : f32
    // }
    // my_struct {
    //   inner : inner_struct
    // }
    //
    // var ident : my_struct
    // var store : f32 = ident.inner.a

    auto* inner_struct = Structure("Inner", utils::Vector{
                                                Member("a", ty.f32()),
                                                Member("b", ty.f32()),
                                            });

    auto* s_type = Structure("my_struct", utils::Vector{Member("inner", ty.Of(inner_struct))});

    auto* var = Var("ident", ty.Of(s_type));
    auto* store = Var("store", ty.f32());

    auto* rhs = MemberAccessor(MemberAccessor("ident", "inner"), "a");
    auto* expr = Assign("store", rhs);
    WrapInFunction(var, store, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeStruct %9 %9
%7 = OpTypeStruct %8
%6 = OpTypePointer Function %7
%10 = OpConstantNull %7
%12 = OpTypePointer Function %9
%13 = OpConstantNull %9
%14 = OpTypeInt 32 0
%15 = OpConstant %14 0
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %10
%11 = OpVariable %12 Function %13
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%16 = OpAccessChain %12 %5 %15 %15
%17 = OpLoad %9 %16
OpStore %11 %17
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_Single) {
    // ident.y

    auto* var = Var("ident", ty.vec3<f32>());

    auto* expr = MemberAccessor("ident", "y");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%10 = OpTypeInt 32 0
%11 = OpConstant %10 1
%12 = OpTypePointer Function %8
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%13 = OpAccessChain %12 %5 %11
%14 = OpLoad %8 %13
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_MultipleNames) {
    // ident.yx

    auto* var = Var("ident", ty.vec3<f32>());

    auto* expr = MemberAccessor("ident", "yx");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%11 = OpTypeVector %8 2
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(%10 = OpLoad %7 %5
%12 = OpVectorShuffle %11 %10 %10 1 0
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_of_Swizzle) {
    // ident.yxz.xz

    auto* var = Var("ident", ty.vec3<f32>());

    auto* expr = MemberAccessor(MemberAccessor("ident", "yxz"), "xz");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%12 = OpTypeVector %8 2
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(%10 = OpLoad %7 %5
%11 = OpVectorShuffle %7 %10 %10 1 0 2
%13 = OpVectorShuffle %12 %11 %11 0 2
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Member_of_Swizzle) {
    // ident.yxz.x

    auto* var = Var("ident", ty.vec3<f32>());

    auto* expr = MemberAccessor(MemberAccessor("ident", "yxz"), "x");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(%10 = OpLoad %7 %5
%11 = OpVectorShuffle %7 %10 %10 1 0 2
%12 = OpCompositeExtract %8 %11 0
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, MemberAccessor_Array_of_Swizzle) {
    // index.yxz[1i]

    auto* var = Var("ident", ty.vec3<f32>());

    auto* expr = IndexAccessor(MemberAccessor("ident", "yxz"), 1_i);
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%12 = OpTypeInt 32 1
%13 = OpConstant %12 1
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(%10 = OpLoad %7 %5
%11 = OpVectorShuffle %7 %10 %10 1 0 2
%14 = OpCompositeExtract %8 %11 1
OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, IndexAccessor_Mixed_ArrayAndMember) {
    // type C = struct {
    //   baz : vec3<f32>
    // }
    // type B = struct {
    //  bar : C;
    // }
    // type A = struct {
    //   foo : array<B, 3>
    // }
    // var index : array<A, 2u>
    // index[0i].foo[2i].bar.baz.yx

    auto* c_type = Structure("C", utils::Vector{Member("baz", ty.vec3<f32>())});

    auto* b_type = Structure("B", utils::Vector{Member("bar", ty.Of(c_type))});
    auto b_ary_type = ty.array(ty.Of(b_type), 3_u);
    auto* a_type = Structure("A", utils::Vector{Member("foo", b_ary_type)});

    auto a_ary_type = ty.array(ty.Of(a_type), 2_u);
    auto* var = Var("index", a_ary_type);
    auto* expr = MemberAccessor(
        MemberAccessor(
            MemberAccessor(IndexAccessor(MemberAccessor(IndexAccessor("index", 0_i), "foo"), 2_i),
                           "bar"),
            "baz"),
        "yx");
    WrapInFunction(var, expr);

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%13 = OpTypeFloat 32
%12 = OpTypeVector %13 3
%11 = OpTypeStruct %12
%10 = OpTypeStruct %11
%14 = OpTypeInt 32 0
%15 = OpConstant %14 3
%9 = OpTypeArray %10 %15
%8 = OpTypeStruct %9
%16 = OpConstant %14 2
%7 = OpTypeArray %8 %16
%6 = OpTypePointer Function %7
%17 = OpConstantNull %7
%18 = OpTypeInt 32 1
%19 = OpConstantNull %18
%20 = OpConstant %14 0
%21 = OpConstant %18 2
%22 = OpTypePointer Function %12
%24 = OpTypeVector %13 2
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()),
              R"(%5 = OpVariable %6 Function %17
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%23 = OpAccessChain %22 %5 %19 %20 %21 %20 %20
%25 = OpLoad %12 %23
%26 = OpVectorShuffle %24 %25 %25 1 0
OpReturn
)");

    Validate(b);
}

}  // namespace
}  // namespace tint::writer::spirv
