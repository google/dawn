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
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/writer/wgsl/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_GlobalDeclAfterFunction) {
    auto* func_var = Var("a", ty.f32());
    WrapInFunction(func_var);

    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  @compute @workgroup_size(1i, 1i, 1i)
  fn test_function() {
    var a : f32;
  }

  var<private> a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, Emit_GlobalsInterleaved) {
    GlobalVar("a0", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* s0 = Structure("S0", utils::Vector{
                                   Member("a", ty.i32()),
                               });

    Func("func", {}, ty.f32(),
         utils::Vector{
             Return("a0"),
         },
         utils::Empty);

    GlobalVar("a1", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* s1 = Structure("S1", utils::Vector{
                                   Member("a", ty.i32()),
                               });

    Func("main", {}, ty.void_(),
         utils::Vector{
             Decl(Var("s0", ty.Of(s0))),
             Decl(Var("s1", ty.Of(s1))),
             Assign("a1", Call("func")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
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

  @compute @workgroup_size(1i)
  fn main() {
    var s0 : S0;
    var s1 : S1;
    a1 = func();
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Global_Sampler) {
    GlobalVar("s", ty.sampler(type::SamplerKind::kSampler), Group(0_a), Binding(0_a));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), "  @group(0) @binding(0) var s : sampler;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_Global_Texture) {
    auto st = ty.sampled_texture(type::TextureDimension::k1d, ty.f32());
    GlobalVar("t", st, Group(0_a), Binding(0_a));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), "  @group(0) @binding(0) var t : texture_1d<f32>;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_GlobalConst) {
    GlobalConst("explicit", ty.f32(), Expr(1_f));
    GlobalConst("inferred", Expr(1_f));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  const explicit : f32 = 1.0f;

  const inferred = 1.0f;
)");
}

TEST_F(WgslGeneratorImplTest, Emit_OverridableConstants) {
    Override("a", ty.f32());
    Override("b", ty.f32(), Id(7_a));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  override a : f32;

  @id(7) override b : f32;
)");
}

}  // namespace
}  // namespace tint::writer::wgsl
