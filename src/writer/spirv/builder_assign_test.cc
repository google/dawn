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
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Assign_Var) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);

  auto ident = std::make_unique<ast::IdentifierExpression>("var");
  auto val = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f));

  ast::AssignmentStatement assign(std::move(ident), std::move(val));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%1 = OpVariable %2 Output
%4 = OpConstant %3 1
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpStore %1 %4
)");
}

TEST_F(BuilderTest, DISABLED_Assign_StructMember) {}

TEST_F(BuilderTest, DISABLED_Assign_Vector) {}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
