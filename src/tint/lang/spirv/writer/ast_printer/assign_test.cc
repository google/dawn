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
#include "gtest/gtest-spi.h"
#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, Assign_Var) {
    auto* v = GlobalVar("var", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign("var", 1_f);

    WrapInFunction(assign);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Assign_Var_OutsideFunction_IsError) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder pb;

            auto* v = pb.GlobalVar("var", pb.ty.f32(), builtin::AddressSpace::kPrivate);

            auto* assign = pb.Assign("var", pb.Expr(1_f));

            pb.WrapInFunction(assign);

            auto program = std::make_unique<Program>(resolver::Resolve(pb));
            auto b = std::make_unique<Builder>(program.get());

            b->GenerateGlobalVariable(v);
            b->GenerateAssignStatement(assign);
        },
        "trying to add SPIR-V instruction 62 outside a function");
}

TEST_F(SpirvASTPrinterTest, Assign_Var_ZeroInitializer) {
    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* val = Call<vec3<f32>>();
    auto* assign = Assign("var", val);

    WrapInFunction(assign);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Assign_Var_Complex_InitializerNestedVector) {
    auto* init = Call<vec3<f32>>(Call<vec2<f32>>(1_f, 2_f), 3_f);

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign("var", init);

    WrapInFunction(assign);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Assign_Var_Complex_Initializer) {
    auto* init = Call<vec3<f32>>(1_f, 2_f, 3_f);

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign("var", init);

    WrapInFunction(assign);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Assign_StructMember) {
    // my_struct {
    //   a : f32
    //   b : f32
    // }
    // var ident : my_struct
    // ident.b = 4.0;

    auto* s = Structure("my_struct", Vector{
                                         Member("a", ty.f32()),
                                         Member("b", ty.f32()),
                                     });

    auto* v = Var("ident", ty.Of(s));

    auto* assign = Assign(MemberAccessor("ident", "b"), Expr(4_f));

    WrapInFunction(v, assign);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Assign_Vector) {
    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* val = Call<vec3<f32>>(1_f, 1_f, 3_f);
    auto* assign = Assign("var", val);

    WrapInFunction(assign);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Assign_Vector_MemberByName) {
    // var.y = 1

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign(MemberAccessor("var", "y"), Expr(1_f));

    WrapInFunction(assign);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Assign_Vector_MemberByIndex) {
    // var[1] = 1

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* assign = Assign(IndexAccessor("var", 1_i), Expr(1_f));

    WrapInFunction(assign);

    Builder& b = Build();

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
}  // namespace tint::spirv::writer
