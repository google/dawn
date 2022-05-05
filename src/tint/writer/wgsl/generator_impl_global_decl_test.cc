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

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/writer/wgsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_GlobalDeclAfterFunction) {
    auto* func_var = Var("a", ty.f32());
    WrapInFunction(func_var);

    Global("a", ty.f32(), ast::StorageClass::kPrivate);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(  @stage(compute) @workgroup_size(1i, 1i, 1i)
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
         ast::AttributeList{});

    Global("a1", ty.f32(), ast::StorageClass::kPrivate);

    auto* s1 = Structure("S1", {Member("a", ty.i32())});

    Func("main", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(Var("s0", ty.Of(s0))),
             Decl(Var("s1", ty.Of(s1))),
             Assign("a1", Call("func")),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(  var<private> a0 : f32;

  struct S0 {
    a : i32,
  }

  fn func() -> f32 {
    return a0;
  }

  var<private> a1 : f32;

  struct S1 {
    a : i32,
  }

  @stage(compute) @workgroup_size(1i)
  fn main() {
    var s0 : S0;
    var s1 : S1;
    a1 = func();
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Global_Sampler) {
    Global("s", ty.sampler(ast::SamplerKind::kSampler),
           ast::AttributeList{
               create<ast::GroupAttribute>(0),
               create<ast::BindingAttribute>(0),
           });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), "  @group(0) @binding(0) var s : sampler;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_Global_Texture) {
    auto* st = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
    Global("t", st,
           ast::AttributeList{
               create<ast::GroupAttribute>(0),
               create<ast::BindingAttribute>(0),
           });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), "  @group(0) @binding(0) var t : texture_1d<f32>;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_OverridableConstants) {
    Override("a", ty.f32(), nullptr);
    Override("b", ty.f32(), nullptr, {Id(7u)});

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(  override a : f32;

  @id(7) override b : f32;
)");
}

}  // namespace
}  // namespace tint::writer::wgsl
