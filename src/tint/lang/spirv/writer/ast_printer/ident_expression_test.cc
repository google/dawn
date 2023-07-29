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

#include "gtest/gtest-spi.h"
#include "src/tint/lang/spirv/writer/ast_printer/test_helper.h"
#include "src/tint/lang/spirv/writer/spv_dump.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, IdentifierExpression_GlobalConst) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder pb;

            auto* init = pb.Call<vec3<f32>>(1_f, 1_f, 3_f);

            auto* v = pb.GlobalConst("c", pb.ty.vec3<f32>(), init);

            auto* expr = pb.Expr("c");
            pb.WrapInFunction(expr);

            auto program = std::make_unique<Program>(resolver::Resolve(pb));
            auto b = std::make_unique<Builder>(program.get());

            b->GenerateGlobalVariable(v);
            b->GenerateIdentifierExpression(expr);
        },
        "internal compiler error: unable to find ID for variable: c");
}

TEST_F(SpirvASTPrinterTest, IdentifierExpression_GlobalVar) {
    auto* v = GlobalVar("var", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* expr = Expr("var");
    WrapInFunction(expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %1 "var"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");

    EXPECT_EQ(b.GenerateIdentifierExpression(expr), 1u);
}

TEST_F(SpirvASTPrinterTest, IdentifierExpression_FunctionConst) {
    auto* init = Call<vec3<f32>>(1_f, 1_f, 3_f);

    auto* v = Let("var", ty.vec3<f32>(), init);

    auto* expr = Expr("var");
    WrapInFunction(v, expr);

    Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");

    EXPECT_EQ(b.GenerateIdentifierExpression(expr), 5u);
}

TEST_F(SpirvASTPrinterTest, IdentifierExpression_FunctionVar) {
    auto* v = Var("var", ty.f32(), builtin::AddressSpace::kFunction);
    auto* expr = Expr("var");
    WrapInFunction(v, expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %1 "var"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
)");

    const auto& func = b.CurrentFunction();
    EXPECT_EQ(DumpInstructions(func.variables()),
              R"(%1 = OpVariable %2 Function %4
)");

    EXPECT_EQ(b.GenerateIdentifierExpression(expr), 1u);
}

TEST_F(SpirvASTPrinterTest, IdentifierExpression_Load) {
    auto* var = GlobalVar("var", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* expr = Add("var", "var");
    WrapInFunction(expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr->As<ast::BinaryExpression>()), 7u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%5 = OpLoad %3 %1
%6 = OpLoad %3 %1
%7 = OpIAdd %3 %5 %6
)");
}

TEST_F(SpirvASTPrinterTest, IdentifierExpression_NoLoadConst) {
    auto* let = Let("let", ty.i32(), Expr(2_i));
    auto* expr = Add("let", "let");
    WrapInFunction(let, expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(let)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr->As<ast::BinaryExpression>()), 3u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%3 = OpIAdd %1 %2 %2
)");
}

}  // namespace
}  // namespace tint::spirv::writer
