// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gtest/gtest-spi.h"
#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, IdentifierExpression_GlobalConst) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder pb;

            auto* init = pb.Call<vec3<f32>>(1_f, 1_f, 3_f);

            auto* v = pb.GlobalConst("c", pb.ty.vec3<f32>(), init);

            auto* expr = pb.Expr("c");
            pb.WrapInFunction(expr);

            auto program = resolver::Resolve(pb);
            Builder b(program);

            b.GenerateGlobalVariable(v);
            b.GenerateIdentifierExpression(expr);
        },
        "internal compiler error: unable to find ID for variable: c");
}

TEST_F(SpirvASTPrinterTest, IdentifierExpression_GlobalVar) {
    auto* v = GlobalVar("var", ty.f32(), core::AddressSpace::kPrivate);

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
    auto* v = Var("var", ty.f32(), core::AddressSpace::kFunction);
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
    auto* var = GlobalVar("var", ty.i32(), core::AddressSpace::kPrivate);
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
