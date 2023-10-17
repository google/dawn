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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, UnaryOp_Negation_Integer) {
    auto* expr = create<ast::UnaryOpExpression>(core::UnaryOp::kNegation, Expr(1_i));
    WrapInFunction(expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateUnaryOpExpression(expr), 1u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%1 = OpSNegate %2 %3
)");
}

TEST_F(SpirvASTPrinterTest, UnaryOp_Negation_Float) {
    auto* expr = create<ast::UnaryOpExpression>(core::UnaryOp::kNegation, Expr(1_f));
    WrapInFunction(expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateUnaryOpExpression(expr), 1u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%3 = OpConstant %2 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%1 = OpFNegate %2 %3
)");
}

TEST_F(SpirvASTPrinterTest, UnaryOp_Complement) {
    auto* expr = create<ast::UnaryOpExpression>(core::UnaryOp::kComplement, Expr(1_i));
    WrapInFunction(expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateUnaryOpExpression(expr), 1u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%1 = OpNot %2 %3
)");
}

TEST_F(SpirvASTPrinterTest, UnaryOp_Not) {
    auto* expr = create<ast::UnaryOpExpression>(core::UnaryOp::kNot, Expr(false));
    WrapInFunction(expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateUnaryOpExpression(expr), 1u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%3 = OpConstantNull %2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%1 = OpLogicalNot %2 %3
)");
}

TEST_F(SpirvASTPrinterTest, UnaryOp_LoadRequired) {
    auto* var = Var("param", ty.vec3<f32>());

    auto* expr = create<ast::UnaryOpExpression>(core::UnaryOp::kNegation, Expr("param"));
    WrapInFunction(var, expr);

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateUnaryOpExpression(expr), 6u) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().variables()),
              R"(%1 = OpVariable %2 Function %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%7 = OpLoad %3 %1
%6 = OpFNegate %3 %7
)");
}

}  // namespace
}  // namespace tint::spirv::writer
