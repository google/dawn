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

#include "src/ast/variable_decl_statement.h"
#include "src/type/access_control_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement) {
  auto* var = Global("a", ty.f32(), ast::StorageClass::kInput);

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  var<in> a : f32;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Function) {
  // Variable declarations with Function storage class don't mention their
  // storage class.  Rely on defaulting.
  // https://github.com/gpuweb/gpuweb/issues/654

  auto* var = Global("a", ty.f32(), ast::StorageClass::kFunction);

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  var a : f32;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Private) {
  auto* var = Global("a", ty.f32(), ast::StorageClass::kPrivate);

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  var<private> a : f32;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Sampler) {
  auto* var = Global("s", create<type::Sampler>(type::SamplerKind::kSampler),
                     ast::StorageClass::kUniformConstant);

  auto* stmt = create<ast::VariableDeclStatement>(var);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  var s : sampler;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Texture) {
  auto* st =
      create<type::SampledTexture>(type::TextureDimension::k1d, ty.f32());
  auto* var = Global(
      "t", create<type::AccessControl>(ast::AccessControl::kReadOnly, st),
      ast::StorageClass::kUniformConstant);

  auto* stmt = create<ast::VariableDeclStatement>(var);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  var t : [[access(read)]] texture_1d<f32>;\n");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
