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

#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, Literal_Bool_True) {
    auto* b_true = create<ast::BoolLiteralExpression>(true);
    WrapInFunction(b_true);

    Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(b_true);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
}

TEST_F(SpirvASTPrinterTest, Literal_Bool_False) {
    auto* b_false = create<ast::BoolLiteralExpression>(false);
    WrapInFunction(b_false);

    Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(b_false);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantFalse %1
)");
}

TEST_F(SpirvASTPrinterTest, Literal_Bool_Dedup) {
    auto* b_true = create<ast::BoolLiteralExpression>(true);
    auto* b_false = create<ast::BoolLiteralExpression>(false);
    WrapInFunction(b_true, b_false);

    Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(b_true), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    ASSERT_NE(b.GenerateLiteralIfNeeded(b_false), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    ASSERT_NE(b.GenerateLiteralIfNeeded(b_true), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%3 = OpConstantFalse %1
)");
}

TEST_F(SpirvASTPrinterTest, Literal_I32) {
    auto* i = Expr(-23_i);
    WrapInFunction(i);
    Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(i);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 -23
)");
}

TEST_F(SpirvASTPrinterTest, Literal_I32_Dedup) {
    auto* i1 = Expr(-23_i);
    auto* i2 = Expr(-23_i);
    WrapInFunction(i1, i2);

    Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(i1), 0u);
    ASSERT_NE(b.GenerateLiteralIfNeeded(i2), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 -23
)");
}

TEST_F(SpirvASTPrinterTest, Literal_U32) {
    auto* i = Expr(23_u);
    WrapInFunction(i);

    Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(i);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 23
)");
}

TEST_F(SpirvASTPrinterTest, Literal_U32_Dedup) {
    auto* i1 = Expr(23_u);
    auto* i2 = Expr(23_u);
    WrapInFunction(i1, i2);

    Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(i1), 0u);
    ASSERT_NE(b.GenerateLiteralIfNeeded(i2), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 23
)");
}

TEST_F(SpirvASTPrinterTest, Literal_F32) {
    auto* i = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kF);
    WrapInFunction(i);

    Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(i);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 23.2450008
)");
}

TEST_F(SpirvASTPrinterTest, Literal_F32_Dedup) {
    auto* i1 = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kF);
    auto* i2 = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kF);
    WrapInFunction(i1, i2);

    Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(i1), 0u);
    ASSERT_NE(b.GenerateLiteralIfNeeded(i2), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 23.2450008
)");
}

TEST_F(SpirvASTPrinterTest, Literal_F16) {
    Enable(wgsl::Extension::kF16);

    auto* i = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kH);
    WrapInFunction(i);

    Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(i);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1.73cp+4
)");
}

TEST_F(SpirvASTPrinterTest, Literal_F16_Dedup) {
    Enable(wgsl::Extension::kF16);

    auto* i1 = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kH);
    auto* i2 = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kH);
    WrapInFunction(i1, i2);

    Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(i1), 0u);
    ASSERT_NE(b.GenerateLiteralIfNeeded(i2), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1.73cp+4
)");
}

}  // namespace tint::spirv::writer
