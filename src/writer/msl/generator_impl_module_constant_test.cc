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
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/float_literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_ModuleConstant) {
  ast::type::F32 f32;
  ast::type::Array ary(&f32, 3, ast::ArrayDecorationList{});

  ast::ExpressionList exprs;
  exprs.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.0f)));
  exprs.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 2.0f)));
  exprs.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 3.0f)));

  auto* var =
      create<ast::Variable>(Source{},                  // source
                            "pos",                     // name
                            ast::StorageClass::kNone,  // storage_class
                            &ary,                      // type
                            true,                      // is_const
                            create<ast::TypeConstructorExpression>(
                                Source{}, &ary, exprs),      // constructor
                            ast::VariableDecorationList{});  // decorations

  ASSERT_TRUE(gen.EmitProgramConstVariable(var)) << gen.error();
  EXPECT_EQ(gen.result(), "constant float pos[3] = {1.0f, 2.0f, 3.0f};\n");
}

TEST_F(MslGeneratorImplTest, Emit_SpecConstant) {
  ast::type::F32 f32;

  auto* var = create<ast::Variable>(
      Source{},                  // source
      "pos",                     // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      true,                      // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 3.0f)),  // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::ConstantIdDecoration>(23, Source{}),
      });

  ASSERT_TRUE(gen.EmitProgramConstVariable(var)) << gen.error();
  EXPECT_EQ(gen.result(), "constant float pos [[function_constant(23)]];\n");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
