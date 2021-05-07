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

#include "src/ast/builtin_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {
namespace {

class ResolverEntryPointValidationTest : public TestHelper,
                                         public testing::Test {};

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Location) {
  // [[stage(fragment)]]
  // fn main() -> [[location(0)]] f32 { return 1.0; }
  Func(Source{{12, 34}}, "main", {}, ty.f32(), {Return(1.0f)},
       {Stage(ast::PipelineStage::kFragment)}, {Location(0)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Builtin) {
  // [[stage(vertex)]]
  // fn main() -> [[builtin(position)]] vec4<f32> { return vec4<f32>(); }
  Func(Source{{12, 34}}, "main", {}, ty.vec4<f32>(),
       {Return(Construct(ty.vec4<f32>()))},
       {Stage(ast::PipelineStage::kVertex)},
       {Builtin(ast::Builtin::kPosition)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Missing) {
  // [[stage(vertex)]]
  // fn main() -> f32 {
  //   return 1.0;
  // }
  Func(Source{{12, 34}}, "main", {}, ty.vec4<f32>(),
       {Return(Construct(ty.vec4<f32>()))},
       {Stage(ast::PipelineStage::kVertex)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: missing entry point IO attribute on return type");
}

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Multiple) {
  // [[stage(vertex)]]
  // fn main() -> [[location(0)]] [[builtin(position)]] vec4<f32> {
  //   return vec4<f32>();
  // }
  Func(Source{{12, 34}}, "main", {}, ty.vec4<f32>(),
       {Return(Construct(ty.vec4<f32>()))},
       {Stage(ast::PipelineStage::kVertex)},
       {Location(Source{{13, 43}}, 0),
        Builtin(Source{{14, 52}}, ast::Builtin::kPosition)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed location(0))");
}

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Struct) {
  // struct Output {
  // };
  // [[stage(vertex)]]
  // fn main() -> [[location(0)]] Output {
  //   return Output();
  // }
  auto* output = Structure("Output", {});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kVertex)}, {Location(Source{{13, 43}}, 0)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "13:43 error: entry point IO attributes must not be used on structure "
      "return types");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_Valid) {
  // struct Output {
  //   [[location(0)]] a : f32;
  //   [[builtin(frag_depth)]] b : f32;
  // };
  // [[stage(fragment)]]
  // fn main() -> Output {
  //   return Output();
  // }
  auto* output = Structure(
      "Output", {Member("a", ty.f32(), {Location(0)}),
                 Member("b", ty.f32(), {Builtin(ast::Builtin::kFragDepth)})});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest,
       ReturnType_Struct_MemberMultipleAttributes) {
  // struct Output {
  //   [[location(0)]] [[builtin(frag_depth)]] a : f32;
  // };
  // [[stage(fragment)]]
  // fn main() -> Output {
  //   return Output();
  // }
  auto* output = Structure(
      "Output",
      {Member("a", ty.f32(),
              {Location(Source{{13, 43}}, 0),
               Builtin(Source{{14, 52}}, ast::Builtin::kFragDepth)})});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed location(0)
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest,
       ReturnType_Struct_MemberMissingAttribute) {
  // struct Output {
  //   [[location(0)]] a : f32;
  //   b : f32;
  // };
  // [[stage(fragment)]]
  // fn main() -> Output {
  //   return Output();
  // }
  auto* output = Structure(
      "Output", {Member(Source{{13, 43}}, "a", ty.f32(), {Location(0)}),
                 Member(Source{{14, 52}}, "b", ty.f32(), {})});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            R"(14:52 error: missing entry point IO attribute
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_NestedStruct) {
  // struct Inner {
  //   [[location(0)]] b : f32;
  // };
  // struct Output {
  //   [[location(0)]] a : Inner;
  // };
  // [[stage(fragment)]]
  // fn main() -> Output {
  //   return Output();
  // }
  auto* inner = Structure(
      "Inner", {Member(Source{{13, 43}}, "a", ty.f32(), {Location(0)})});
  auto* output = Structure(
      "Output", {Member(Source{{14, 52}}, "a", inner, {Location(0)})});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(14:52 error: entry point IO types cannot contain nested structures
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_RuntimeArray) {
  // [[block]]
  // struct Output {
  //   [[location(0)]] a : array<f32>;
  // };
  // [[stage(fragment)]]
  // fn main() -> Output {
  //   return Output();
  // }
  auto* output = Structure(
      "Output",
      {Member(Source{{13, 43}}, "a", ty.array<float>(), {Location(0)})},
      {create<ast::StructBlockDecoration>()});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(13:43 error: entry point IO types cannot contain runtime sized arrays
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_DuplicateBuiltins) {
  // struct Output {
  //   [[builtin(frag_depth)]] a : f32;
  //   [[builtin(frag_depth)]] b : f32;
  // };
  // [[stage(fragment)]]
  // fn main() -> Output {
  //   return Output();
  // }
  auto* output = Structure(
      "Output", {Member("a", ty.f32(), {Builtin(ast::Builtin::kFragDepth)}),
                 Member("b", ty.f32(), {Builtin(ast::Builtin::kFragDepth)})});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: builtin(frag_depth) attribute appears multiple times as pipeline output
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_DuplicateLocation) {
  // struct Output {
  //   [[location(1)]] a : f32;
  //   [[location(1)]] b : f32;
  // };
  // [[stage(fragment)]]
  // fn main() -> Output {
  //   return Output();
  // }
  auto* output = Structure("Output", {Member("a", ty.f32(), {Location(1)}),
                                      Member("b", ty.f32(), {Location(1)})});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: location(1) attribute appears multiple times as pipeline output
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Location) {
  // [[stage(fragment)]]
  // fn main([[location(0)]] param : f32) {}
  auto* param = Param("param", ty.f32(), {Location(0)});
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Builtin) {
  // [[stage(fragment)]]
  // fn main([[builtin(frag_depth)]] param : f32) {}
  auto* param =
      Param("param", ty.vec4<f32>(), {Builtin(ast::Builtin::kFragDepth)});
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Missing) {
  // [[stage(fragment)]]
  // fn main(param : f32) {}
  auto* param = Param(Source{{13, 43}}, "param", ty.vec4<f32>());
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "13:43 error: missing entry point IO attribute on parameter");
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Multiple) {
  // [[stage(fragment)]]
  // fn main([[location(0)]] [[builtin(vertex_index)]] param : u32) {}
  auto* param = Param("param", ty.u32(),
                      {Location(Source{{13, 43}}, 0),
                       Builtin(Source{{14, 52}}, ast::Builtin::kVertexIndex)});
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed location(0))");
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Struct) {
  // struct Input {
  // };
  // [[stage(fragment)]]
  // fn main([[location(0)]] param : Input) {}
  auto* input = Structure("Input", {});
  auto* param = Param("param", input, {Location(Source{{13, 43}}, 0)});
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "13:43 error: entry point IO attributes must not be used on structure "
      "parameters");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_Valid) {
  // struct Input {
  //   [[location(0)]] a : f32;
  //   [[builtin(sample_index)]] b : u32;
  // };
  // [[stage(fragment)]]
  // fn main(param : Input) {}
  auto* input = Structure(
      "Input", {Member("a", ty.f32(), {Location(0)}),
                Member("b", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)})});
  auto* param = Param("param", input);
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest,
       Parameter_Struct_MemberMultipleAttributes) {
  // struct Input {
  //   [[location(0)]] [[builtin(sample_index)]] a : u32;
  // };
  // [[stage(fragment)]]
  // fn main(param : Input) {}
  auto* input = Structure(
      "Input",
      {Member("a", ty.f32(),
              {Location(Source{{13, 43}}, 0),
               Builtin(Source{{14, 52}}, ast::Builtin::kSampleIndex)})});
  auto* param = Param("param", input);
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed location(0)
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest,
       Parameter_Struct_MemberMissingAttribute) {
  // struct Input {
  //   [[location(0)]] a : f32;
  //   b : f32;
  // };
  // [[stage(fragment)]]
  // fn main(param : Input) {}
  auto* input = Structure(
      "Input", {Member(Source{{13, 43}}, "a", ty.f32(), {Location(0)}),
                Member(Source{{14, 52}}, "b", ty.f32(), {})});
  auto* param = Param("param", input);
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), R"(14:52 error: missing entry point IO attribute
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_NestedStruct) {
  // struct Inner {
  //   [[location(0)]] b : f32;
  // };
  // struct Input {
  //   [[location(0)]] a : Inner;
  // };
  // [[stage(fragment)]]
  // fn main(param : Input) {}
  auto* inner = Structure(
      "Inner", {Member(Source{{13, 43}}, "a", ty.f32(), {Location(0)})});
  auto* input =
      Structure("Input", {Member(Source{{14, 52}}, "a", inner, {Location(0)})});
  auto* param = Param("param", input);
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(14:52 error: entry point IO types cannot contain nested structures
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_RuntimeArray) {
  // [[block]]
  // struct Input {
  //   [[location(0)]] a : array<f32>;
  // };
  // [[stage(fragment)]]
  // fn main(param : Input) {}
  auto* input = Structure(
      "Input",
      {Member(Source{{13, 43}}, "a", ty.array<float>(), {Location(0)})},
      {create<ast::StructBlockDecoration>()});
  auto* param = Param("param", input);
  Func(Source{{12, 34}}, "main", {param}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(13:43 error: entry point IO types cannot contain runtime sized arrays
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_DuplicateBuiltins) {
  // [[stage(fragment)]]
  // fn main([[builtin(sample_index)]] param_a : u32,
  //         [[builtin(sample_index)]] param_b : u32) {}
  auto* param_a =
      Param("param_a", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)});
  auto* param_b =
      Param("param_b", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)});
  Func(Source{{12, 34}}, "main", {param_a, param_b}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: builtin(sample_index) attribute appears multiple times as "
      "pipeline input");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_DuplicateBuiltins) {
  // struct InputA {
  //   [[builtin(sample_index)]] a : u32;
  // };
  // struct InputB {
  //   [[builtin(sample_index)]] a : u32;
  // };
  // [[stage(fragment)]]
  // fn main(param_a : InputA, param_b : InputB) {}
  auto* input_a = Structure(
      "InputA", {Member("a", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)})});
  auto* input_b = Structure(
      "InputB", {Member("a", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)})});
  auto* param_a = Param("param_a", input_a);
  auto* param_b = Param("param_b", input_b);
  Func(Source{{12, 34}}, "main", {param_a, param_b}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: builtin(sample_index) attribute appears multiple times as pipeline input
12:34 note: while analysing entry point main)");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_DuplicateLocation) {
  // [[stage(fragment)]]
  // fn main([[location(1)]] param_a : f32,
  //         [[location(1)]] param_b : f32) {}
  auto* param_a = Param("param_a", ty.u32(), {Location(1)});
  auto* param_b = Param("param_b", ty.u32(), {Location(1)});
  Func(Source{{12, 34}}, "main", {param_a, param_b}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: location(1) attribute appears multiple times as "
            "pipeline input");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_DuplicateLocation) {
  // struct InputA {
  //   [[location(1)]] a : f32;
  // };
  // struct InputB {
  //   [[location(1)]] a : f32;
  // };
  // [[stage(fragment)]]
  // fn main(param_a : InputA, param_b : InputB) {}
  auto* input_a = Structure("InputA", {Member("a", ty.f32(), {Location(1)})});
  auto* input_b = Structure("InputB", {Member("a", ty.f32(), {Location(1)})});
  auto* param_a = Param("param_a", input_a);
  auto* param_b = Param("param_b", input_b);
  Func(Source{{12, 34}}, "main", {param_a, param_b}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: location(1) attribute appears multiple times as pipeline input
12:34 note: while analysing entry point main)");
}

// TODO(bclayton): Reenable after CTS is updated
TEST_F(ResolverEntryPointValidationTest,
       DISABLED_VertexShaderMustReturnPosition) {
  // [[stage(vertex)]]
  // fn main() {}
  Func(Source{{12, 34}}, "main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kVertex)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: a vertex shader must include the 'position' builtin "
            "in its return type");
}

namespace TypeValidationTests {
struct Params {
  create_ast_type_func_ptr create_ast_type;
  bool is_valid;
};

using TypeValidationTest = resolver::ResolverTestWithParam<Params>;

static constexpr Params cases[] = {
    {ast_f32, true},
    {ast_i32, true},
    {ast_u32, true},
    {ast_bool, false},
    {ast_vec2<ast_f32>, true},
    {ast_vec3<ast_f32>, true},
    {ast_vec4<ast_f32>, true},
    {ast_mat2x2<ast_f32>, false},
    {ast_mat2x2<ast_i32>, false},
    {ast_mat2x2<ast_u32>, false},
    {ast_mat2x2<ast_bool>, false},
    {ast_mat3x3<ast_f32>, false},
    {ast_mat3x3<ast_i32>, false},
    {ast_mat3x3<ast_u32>, false},
    {ast_mat3x3<ast_bool>, false},
    {ast_mat4x4<ast_f32>, false},
    {ast_mat4x4<ast_i32>, false},
    {ast_mat4x4<ast_u32>, false},
    {ast_mat4x4<ast_bool>, false},
    {ast_alias<ast_f32>, true},
    {ast_alias<ast_i32>, true},
    {ast_alias<ast_u32>, true},
    {ast_alias<ast_bool>, false},
};

TEST_P(TypeValidationTest, BareInputs) {
  // [[stage(fragment)]]
  // fn main([[location(0)]] a : *) {}
  auto params = GetParam();
  auto* a = Param("a", params.create_ast_type(ty), {Location(0)});
  Func(Source{{12, 34}}, "main", {a}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
  }
}

TEST_P(TypeValidationTest, StructInputs) {
  // struct Input {
  //   [[location(0)]] a : *;
  // };
  // [[stage(fragment)]]
  // fn main(a : Input) {}
  auto params = GetParam();
  auto* input = Structure(
      "Input", {Member("a", params.create_ast_type(ty), {Location(0)})});
  auto* a = Param("a", input, {});
  Func(Source{{12, 34}}, "main", {a}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
  }
}

TEST_P(TypeValidationTest, BareOutputs) {
  // [[stage(fragment)]]
  // fn main() -> [[location(0)]] * {
  //   return *();
  // }
  auto params = GetParam();
  Func(Source{{12, 34}}, "main", {}, params.create_ast_type(ty),
       {Return(Construct(params.create_ast_type(ty)))},
       {Stage(ast::PipelineStage::kFragment)}, {Location(0)});

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
  }
}

TEST_P(TypeValidationTest, StructOutputs) {
  // struct Output {
  //   [[location(0)]] a : *;
  // };
  // [[stage(fragment)]]
  // fn main() -> Output {
  //   return Output();
  // }
  auto params = GetParam();
  auto* output = Structure(
      "Output", {Member("a", params.create_ast_type(ty), {Location(0)})});
  Func(Source{{12, 34}}, "main", {}, output, {Return(Construct(output))},
       {Stage(ast::PipelineStage::kFragment)});

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverEntryPointValidationTest,
                         TypeValidationTest,
                         testing::ValuesIn(cases));

}  // namespace TypeValidationTests

}  // namespace
}  // namespace resolver
}  // namespace tint
