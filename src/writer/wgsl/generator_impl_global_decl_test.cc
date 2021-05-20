// Copyright 2021 The Tint Authors.
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

#include "src/ast/stage_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/sem/sampled_texture_type.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_GlobalDeclAfterFunction) {
  auto* func_var = Var("a", ty.f32());
  WrapInFunction(func_var);

  Global("a", ty.f32(), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(  [[stage(compute)]]
  fn test_function() {
    var a : f32;
  }

  var<private> a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, Emit_GlobalsInterleaved) {
  Global("a0", ty.f32(), ast::StorageClass::kPrivate);

  auto* s0 = Structure("S0", {Member("a", ty.i32())});

  Func("func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           Return("a0"),
       },
       ast::DecorationList{});

  Global("a1", ty.f32(), ast::StorageClass::kOutput);

  auto* s1 = Structure("S1", {Member("a", ty.i32())});

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Var("s0", s0)),
           Decl(Var("s1", s1)),
           Assign("a1", Call("func")),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(  var<private> a0 : f32;

  struct S0 {
    a : i32;
  };

  fn func() -> f32 {
    return a0;
  }

  var<out> a1 : f32;

  struct S1 {
    a : i32;
  };

  [[stage(compute)]]
  fn main() {
    var s0 : S0;
    var s1 : S1;
    a1 = func();
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Global_Sampler) {
  Global("s", ty.sampler(ast::SamplerKind::kSampler), ast::StorageClass::kNone,
         nullptr,
         {create<ast::GroupDecoration>(0), create<ast::BindingDecoration>(0)});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), "  [[group(0), binding(0)]] var s : sampler;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_Global_Texture) {
  auto* st = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
  Global("t", ty.access(ast::AccessControl::kReadOnly, st),
         ast::StorageClass::kNone, nullptr,
         {create<ast::GroupDecoration>(0), create<ast::BindingDecoration>(0)});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(
      gen.result(),
      "  [[group(0), binding(0)]] var t : [[access(read)]] texture_1d<f32>;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_OverridableConstants) {
  GlobalConst("a", ty.f32(), nullptr, {Override()});
  GlobalConst("b", ty.f32(), nullptr, {Override(7u)});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(  [[override]] let a : f32;

  [[override(7)]] let b : f32;
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
