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

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Literal_Bool_True) {
    auto* b_true = create<ast::BoolLiteralExpression>(true);
    WrapInFunction(b_true);

    spirv::Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(b_true);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
}

TEST_F(BuilderTest, Literal_Bool_False) {
    auto* b_false = create<ast::BoolLiteralExpression>(false);
    WrapInFunction(b_false);

    spirv::Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(b_false);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantFalse %1
)");
}

TEST_F(BuilderTest, Literal_Bool_Dedup) {
    auto* b_true = create<ast::BoolLiteralExpression>(true);
    auto* b_false = create<ast::BoolLiteralExpression>(false);
    WrapInFunction(b_true, b_false);

    spirv::Builder& b = Build();

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

TEST_F(BuilderTest, Literal_I32) {
    auto* i = Expr(-23_i);
    WrapInFunction(i);
    spirv::Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(i);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 -23
)");
}

TEST_F(BuilderTest, Literal_I32_Dedup) {
    auto* i1 = Expr(-23_i);
    auto* i2 = Expr(-23_i);
    WrapInFunction(i1, i2);

    spirv::Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(i1), 0u);
    ASSERT_NE(b.GenerateLiteralIfNeeded(i2), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 -23
)");
}

TEST_F(BuilderTest, Literal_U32) {
    auto* i = Expr(23_u);
    WrapInFunction(i);

    spirv::Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(i);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 23
)");
}

TEST_F(BuilderTest, Literal_U32_Dedup) {
    auto* i1 = Expr(23_u);
    auto* i2 = Expr(23_u);
    WrapInFunction(i1, i2);

    spirv::Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(i1), 0u);
    ASSERT_NE(b.GenerateLiteralIfNeeded(i2), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 23
)");
}

TEST_F(BuilderTest, Literal_F32) {
    auto* i = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kF);
    WrapInFunction(i);

    spirv::Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(i);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 23.2450008
)");
}

TEST_F(BuilderTest, Literal_F32_Dedup) {
    auto* i1 = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kF);
    auto* i2 = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kF);
    WrapInFunction(i1, i2);

    spirv::Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(i1), 0u);
    ASSERT_NE(b.GenerateLiteralIfNeeded(i2), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 23.2450008
)");
}

TEST_F(BuilderTest, Literal_F16) {
    Enable(builtin::Extension::kF16);

    auto* i = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kH);
    WrapInFunction(i);

    spirv::Builder& b = Build();

    auto id = b.GenerateLiteralIfNeeded(i);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(2u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1.73cp+4
)");
}

TEST_F(BuilderTest, Literal_F16_Dedup) {
    Enable(builtin::Extension::kF16);

    auto* i1 = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kH);
    auto* i2 = create<ast::FloatLiteralExpression>(23.245, ast::FloatLiteralExpression::Suffix::kH);
    WrapInFunction(i1, i2);

    spirv::Builder& b = Build();

    ASSERT_NE(b.GenerateLiteralIfNeeded(i1), 0u);
    ASSERT_NE(b.GenerateLiteralIfNeeded(i2), 0u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1.73cp+4
)");
}

}  // namespace tint::writer::spirv
