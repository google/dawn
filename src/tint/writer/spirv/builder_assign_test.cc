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

#include "gmock/gmock.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Assign_Var) {
    auto* v = GlobalVar("var", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign("var", 1_f);

    WrapInFunction(assign);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpConstant %3 1
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %1 %5
)");
}

TEST_F(BuilderTest, Assign_Var_OutsideFunction_IsError) {
    auto* v = GlobalVar("var", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign("var", Expr(1_f));

    WrapInFunction(assign);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    tint::SetInternalCompilerErrorReporter(nullptr);

    EXPECT_FALSE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_TRUE(b.has_error());
    EXPECT_THAT(b.Diagnostics().str(),
                ::testing::HasSubstr("trying to add SPIR-V instruction 62 outside a function"));
}

TEST_F(BuilderTest, Assign_Var_ZeroInitializer) {
    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* val = vec3<f32>();
    auto* assign = Assign("var", val);

    WrapInFunction(assign);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %1 %5
)");
}

TEST_F(BuilderTest, Assign_Var_Complex_InitializerNestedVector) {
    auto* init = vec3<f32>(vec2<f32>(1_f, 2_f), 3_f);

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign("var", init);

    WrapInFunction(assign);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%6 = OpConstant %4 1
%7 = OpConstant %4 2
%8 = OpConstant %4 3
%9 = OpConstantComposite %3 %6 %7 %8
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %1 %9
)");
}

TEST_F(BuilderTest, Assign_Var_Complex_Initializer) {
    auto* init = vec3<f32>(1_f, 2_f, 3_f);

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign("var", init);

    WrapInFunction(assign);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%6 = OpConstant %4 1
%7 = OpConstant %4 2
%8 = OpConstant %4 3
%9 = OpConstantComposite %3 %6 %7 %8
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %1 %9
)");
}

TEST_F(BuilderTest, Assign_StructMember) {
    // my_struct {
    //   a : f32
    //   b : f32
    // }
    // var ident : my_struct
    // ident.b = 4.0;

    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.f32()),
                                         Member("b", ty.f32()),
                                     });

    auto* v = Var("ident", ty.Of(s));

    auto* assign = Assign(MemberAccessor("ident", "b"), Expr(4_f));

    WrapInFunction(v, assign);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpTypePointer Function %4
%10 = OpConstant %4 4
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpAccessChain %8 %1 %7
OpStore %9 %10
)");
}

TEST_F(BuilderTest, Assign_Vector) {
    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* val = vec3<f32>(1_f, 1_f, 3_f);
    auto* assign = Assign("var", val);

    WrapInFunction(assign);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%6 = OpConstant %4 1
%7 = OpConstant %4 3
%8 = OpConstantComposite %3 %6 %6 %7
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %1 %8
)");
}

TEST_F(BuilderTest, Assign_Vector_MemberByName) {
    // var.y = 1

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign(MemberAccessor("var", "y"), Expr(1_f));

    WrapInFunction(assign);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpTypePointer Private %4
%10 = OpConstant %4 1
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpAccessChain %8 %1 %7
OpStore %9 %10
)");
}

TEST_F(BuilderTest, Assign_Vector_MemberByIndex) {
    // var[1] = 1

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign(IndexAccessor("var", 1_i), Expr(1_f));

    WrapInFunction(assign);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateAssignStatement(assign)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpTypePointer Private %4
%10 = OpConstant %4 1
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpAccessChain %8 %1 %7
OpStore %9 %10
)");
}

}  // namespace
}  // namespace tint::writer::spirv
