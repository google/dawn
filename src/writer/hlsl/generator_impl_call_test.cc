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

#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/program.h"
#include "src/type/void_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Call = TestHelper;

TEST_F(HlslGeneratorImplTest_Call, EmitExpression_Call_WithoutParams) {
  auto* call = Call("my_func");

  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::FunctionDecorationList{});
  AST().Functions().Add(func);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_EQ(result(), "my_func()");
}

TEST_F(HlslGeneratorImplTest_Call, EmitExpression_Call_WithParams) {
  auto* call = Call("my_func", "param1", "param2");

  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::FunctionDecorationList{});
  AST().Functions().Add(func);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_EQ(result(), "my_func(param1, param2)");
}

TEST_F(HlslGeneratorImplTest_Call, EmitStatement_Call) {
  auto* call = create<ast::CallStatement>(Call("my_func", "param1", "param2"));

  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::FunctionDecorationList{});
  AST().Functions().Add(func);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitStatement(out, call)) << gen.error();
  EXPECT_EQ(result(), "  my_func(param1, param2);\n");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
