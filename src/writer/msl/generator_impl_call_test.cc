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

#include "src/ast/call_statement.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitExpression_Call_WithoutParams) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "my_func()");
}

TEST_F(MslGeneratorImplTest, EmitExpression_Call_WithParams) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{});
  Global("param1", ty.f32(), ast::StorageClass::kInput);
  Global("param2", ty.f32(), ast::StorageClass::kInput);

  auto* call = Call("my_func", "param1", "param2");
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "my_func(param1, param2)");
}

TEST_F(MslGeneratorImplTest, EmitStatement_Call) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{});
  Global("param1", ty.f32(), ast::StorageClass::kInput);
  Global("param2", ty.f32(), ast::StorageClass::kInput);

  auto* call = Call("my_func", "param1", "param2");
  auto* stmt = create<ast::CallStatement>(call);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  my_func(param1, param2);\n");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
