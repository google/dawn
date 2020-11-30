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

#include "src/ast/constant_id_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_ModuleConstant = TestHelper;

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_ModuleConstant) {
  ast::type::F32 f32;
  ast::type::Array ary(&f32, 3);

  ast::ExpressionList exprs;
  exprs.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  exprs.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.0f)));
  exprs.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  auto* var = create<ast::Variable>("pos", ast::StorageClass::kNone, &ary);
  var->set_is_const(true);
  var->set_constructor(create<ast::TypeConstructorExpression>(&ary, exprs));

  ASSERT_TRUE(gen.EmitProgramConstVariable(out, var)) << gen.error();
  EXPECT_EQ(result(), "static const float pos[3] = {1.0f, 2.0f, 3.0f};\n");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_SpecConstant) {
  ast::type::F32 f32;

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::ConstantIdDecoration>(23, Source{}));

  auto* var = create<ast::DecoratedVariable>(
      create<ast::Variable>("pos", ast::StorageClass::kNone, &f32));
  var->set_decorations(decos);
  var->set_is_const(true);
  var->set_constructor(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ASSERT_TRUE(gen.EmitProgramConstVariable(out, var)) << gen.error();
  EXPECT_EQ(result(), R"(#ifndef WGSL_SPEC_CONSTANT_23
#define WGSL_SPEC_CONSTANT_23 3.0f
#endif
static const float pos = WGSL_SPEC_CONSTANT_23;
#undef WGSL_SPEC_CONSTANT_23
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_SpecConstant_NoConstructor) {
  ast::type::F32 f32;

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::ConstantIdDecoration>(23, Source{}));

  auto* var = create<ast::DecoratedVariable>(
      create<ast::Variable>("pos", ast::StorageClass::kNone, &f32));
  var->set_decorations(decos);
  var->set_is_const(true);

  ASSERT_TRUE(gen.EmitProgramConstVariable(out, var)) << gen.error();
  EXPECT_EQ(result(), R"(#ifndef WGSL_SPEC_CONSTANT_23
#error spec constant required for constant id 23
#endif
static const float pos = WGSL_SPEC_CONSTANT_23;
#undef WGSL_SPEC_CONSTANT_23
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
