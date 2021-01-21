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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/block_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/f32_type.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Block) {
  // Note, this test uses shadow variables which aren't allowed in WGSL but
  // serves to prove the block code is pushing new scopes as needed.
  auto* inner = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(
          Var("var", ast::StorageClass::kFunction, ty.f32)),
      create<ast::AssignmentStatement>(Expr("var"), Expr(2.f))});
  auto* outer = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(
          Var("var", ast::StorageClass::kFunction, ty.f32)),
      create<ast::AssignmentStatement>(Expr("var"), Expr(1.f)), inner,
      create<ast::AssignmentStatement>(Expr("var"), Expr(3.f))});

  ASSERT_TRUE(td.DetermineResultType(outer)) << td.error();

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateStatement(outer)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
%5 = OpConstant %3 1
%7 = OpConstant %3 2
%8 = OpConstant %3 3
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %4
%6 = OpVariable %2 Function %4
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpStore %1 %5
OpStore %6 %7
OpStore %1 %8
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
