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
#include "src/ast/identifier_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/f32_type.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement) {
  auto* var = Global("a", ast::StorageClass::kNone, ty.f32());

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  var a : f32;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Function) {
  // Variable declarations with Function storage class don't mention their
  // storage class.  Rely on defaulting.
  // https://github.com/gpuweb/gpuweb/issues/654

  auto* var = Global("a", ast::StorageClass::kFunction, ty.f32());

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  var a : f32;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Private) {
  auto* var = Global("a", ast::StorageClass::kPrivate, ty.f32());

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  var<private> a : f32;\n");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
