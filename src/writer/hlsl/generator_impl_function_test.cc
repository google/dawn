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

#include "gmock/gmock.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/type/access_control_type.h"
#include "src/writer/hlsl/test_helper.h"

using ::testing::HasSubstr;

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Function = TestHelper;

TEST_F(HlslGeneratorImplTest_Function, Emit_Function) {
  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_Name_Collision) {
  Func("GeometryShader", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void GeometryShader_tint_0() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithParams) {
  Func("my_func",
       ast::VariableList{Var("a", ty.f32(), ast::StorageClass::kNone),
                         Var("b", ty.i32(), ast::StorageClass::kNone)},
       ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_NoReturn_Void) {
  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{/* no explicit return */},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(void main() {
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_WithInOutVars) {
  // fn frag_main([[location(0)]] foo : f32) -> [[location(1)]] f32 {
  //   return foo;
  // }
  auto* foo_in =
      Const("foo", ty.f32(), nullptr, {create<ast::LocationDecoration>(0)});
  Func("frag_main", ast::VariableList{foo_in}, ty.f32(),
       {create<ast::ReturnStatement>(Expr("foo"))},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)},
       {create<ast::LocationDecoration>(1)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_5 {
  float foo : TEXCOORD0;
};
struct tint_symbol_2 {
  float value : SV_Target1;
};

tint_symbol_2 frag_main(tint_symbol_5 tint_symbol_7) {
  const float foo = tint_symbol_7.foo;
  const tint_symbol_2 tint_symbol_1 = {foo};
  return tint_symbol_1;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_WithInOut_Builtins) {
  // fn frag_main([[position(0)]] coord : vec4<f32>) -> [[frag_depth]] f32 {
  //   return coord.x;
  // }
  auto* coord_in =
      Const("coord", ty.vec4<f32>(), nullptr,
            {create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord)});
  Func("frag_main", ast::VariableList{coord_in}, ty.f32(),
       {create<ast::ReturnStatement>(MemberAccessor("coord", "x"))},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)},
       {create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_6 {
  float4 coord : SV_Position;
};
struct tint_symbol_2 {
  float value : SV_Depth;
};

tint_symbol_2 frag_main(tint_symbol_6 tint_symbol_8) {
  const float4 coord = tint_symbol_8.coord;
  const tint_symbol_2 tint_symbol_1 = {coord.x};
  return tint_symbol_1;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_SharedStruct_DifferentStages) {
  // struct Interface {
  //   [[location(1)]] col1 : f32;
  //   [[location(2)]] col2 : f32;
  // };
  // fn vert_main() -> Interface {
  //   return Interface(0.4, 0.6);
  // }
  // fn frag_main(colors : Interface) -> void {
  //   const r = colors.col1;
  //   const g = colors.col2;
  // }
  auto* interface_struct = Structure(
      "Interface",
      {Member("col1", ty.f32(), {create<ast::LocationDecoration>(1)}),
       Member("col2", ty.f32(), {create<ast::LocationDecoration>(2)})});

  Func("vert_main", {}, interface_struct,
       {create<ast::ReturnStatement>(
           Construct(interface_struct, Expr(0.5f), Expr(0.25f)))},
       {create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  Func("frag_main", {Const("colors", interface_struct)}, ty.void_(),
       {
           WrapInStatement(
               Const("r", ty.f32(), MemberAccessor(Expr("colors"), "col1"))),
           WrapInStatement(
               Const("g", ty.f32(), MemberAccessor(Expr("colors"), "col2"))),
       },
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct Interface {
  float col1;
  float col2;
};
struct tint_symbol_2 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
};
struct tint_symbol_8 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
};

tint_symbol_2 vert_main() {
  const Interface tint_symbol_5 = {0.5f, 0.25f};
  const tint_symbol_2 tint_symbol_1 = {tint_symbol_5.col1, tint_symbol_5.col2};
  return tint_symbol_1;
}

void frag_main(tint_symbol_8 tint_symbol_10) {
  const Interface colors = {tint_symbol_10.col1, tint_symbol_10.col2};
  const float r = colors.col1;
  const float g = colors.col2;
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_SharedStruct_HelperFunction) {
  // struct VertexOutput {
  //   [[builtin(position)]] pos : vec4<f32>;
  // };
  // fn foo(x : f32) -> VertexOutput {
  //   return VertexOutput(vec4<f32>(x, x, x, 1.0));
  // }
  // fn vert_main1() -> VertexOutput {
  //   return foo(0.5);
  // }
  // fn vert_main2() -> VertexOutput {
  //   return foo(0.25);
  // }
  auto* vertex_output_struct = Structure(
      "VertexOutput",
      {Member("pos", ty.vec4<f32>(),
              {create<ast::BuiltinDecoration>(ast::Builtin::kPosition)})});

  Func("foo", {Const("x", ty.f32())}, vertex_output_struct,
       {create<ast::ReturnStatement>(Construct(
           vertex_output_struct, Construct(ty.vec4<f32>(), Expr("x"), Expr("x"),
                                           Expr("x"), Expr(1.f))))},
       {});

  Func("vert_main1", {}, vertex_output_struct,
       {create<ast::ReturnStatement>(
           Construct(vertex_output_struct, Expr(Call("foo", Expr(0.5f)))))},
       {create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  Func("vert_main2", {}, vertex_output_struct,
       {create<ast::ReturnStatement>(
           Construct(vertex_output_struct, Expr(Call("foo", Expr(0.25f)))))},
       {create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct VertexOutput {
  float4 pos;
};
struct tint_symbol_2 {
  float4 pos : SV_Position;
};
struct tint_symbol_6 {
  float4 pos : SV_Position;
};

VertexOutput foo(float x) {
  const VertexOutput tint_symbol_8 = {float4(x, x, x, 1.0f)};
  return tint_symbol_8;
}

tint_symbol_2 vert_main1() {
  const VertexOutput tint_symbol_4 = {foo(0.5f)};
  const tint_symbol_2 tint_symbol_1 = {tint_symbol_4.pos};
  return tint_symbol_1;
}

tint_symbol_6 vert_main2() {
  const VertexOutput tint_symbol_7 = {foo(0.25f)};
  const tint_symbol_6 tint_symbol_5 = {tint_symbol_7.pos};
  return tint_symbol_5;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_Uniform) {
  Global("coord", ty.vec4<f32>(), ast::StorageClass::kUniform, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "x"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(cbuffer cbuffer_coord : register(b0, space1) {
  float4 coord;
};

void frag_main() {
  float v = coord.x;
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_UniformStruct) {
  auto* s = Structure("Uniforms", {Member("coord", ty.vec4<f32>())});

  Global("uniforms", s, ast::StorageClass::kUniform, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  create<ast::MemberAccessorExpression>(
                      MemberAccessor("uniforms", "coord"), Expr("x")));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct Uniforms {
  float4 coord;
};

ConstantBuffer<Uniforms> uniforms : register(b0, space1);

void frag_main() {
  float v = uniforms.coord.x;
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_RW_StorageBuffer_Read) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "b"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4));
  return;
})"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_RO_StorageBuffer_Read) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadOnly, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "b"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(ByteAddressBuffer coord : register(t0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4));
  return;
})"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_WO_StorageBuffer_Store) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kWriteOnly, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(MemberAccessor("coord", "b"),
                                            Expr(2.0f)),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4, asuint(2.0f));
  return;
})"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_StorageBuffer_Store) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(MemberAccessor("coord", "b"),
                                            Expr(2.0f)),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4, asuint(2.0f));
  return;
})"));

  Validate();
}

// TODO(crbug.com/tint/697): Remove this test
TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_Decoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {  // NOLINT
  Global("foo", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(0),
         });

  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(1),
         });

  Global("val", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(0),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kNone)},
       ty.f32(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
           create<ast::AssignmentStatement>(Expr("val"), Expr("param")),
           create<ast::ReturnStatement>(Expr("foo")),
       },
       ast::DecorationList{});

  Func(
      "ep_1", ast::VariableList{}, ty.void_(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("bar"), Call("sub_func", 1.0f)),
          create<ast::ReturnStatement>(),
      },
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment),
      });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_in {
  float foo : TEXCOORD0;
};

struct ep_1_out {
  float bar : SV_Target1;
  float val : SV_Target0;
};

float sub_func_ep_1(in ep_1_in tint_in, out ep_1_out tint_out, float param) {
  tint_out.bar = tint_in.foo;
  tint_out.val = param;
  return tint_in.foo;
}

ep_1_out ep_1(ep_1_in tint_in) {
  ep_1_out tint_out = (ep_1_out)0;
  tint_out.bar = sub_func_ep_1(tint_in, tint_out, 1.0f);
  return tint_out;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_Called_By_EntryPoints_NoUsedGlobals) {
  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kFunction)},
       ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr("param")),
       },
       ast::DecorationList{});

  Func("ep_1", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("depth"),
                                            Call("sub_func", 1.0f)),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_out {
  float depth : SV_Depth;
};

float sub_func(float param) {
  return param;
}

ep_1_out ep_1() {
  ep_1_out tint_out = (ep_1_out)0;
  tint_out.depth = sub_func(1.0f);
  return tint_out;
}

)");

  Validate();
}

// TODO(crbug.com/tint/697): Remove this test
TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_Decoration_Called_By_EntryPoints_WithBuiltinGlobals_And_Params) {  // NOLINT
  Global("coord", ty.vec4<f32>(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord),
         });

  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kNone)},
       ty.f32(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("depth"),
                                            MemberAccessor("coord", "x")),
           create<ast::ReturnStatement>(Expr("param")),
       },
       ast::DecorationList{});

  Func("ep_1", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("depth"),
                                            Call("sub_func", 1.0f)),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_in {
  float4 coord : SV_Position;
};

struct ep_1_out {
  float depth : SV_Depth;
};

float sub_func_ep_1(in ep_1_in tint_in, out ep_1_out tint_out, float param) {
  tint_out.depth = tint_in.coord.x;
  return param;
}

ep_1_out ep_1(ep_1_in tint_in) {
  ep_1_out tint_out = (ep_1_out)0;
  tint_out.depth = sub_func_ep_1(tint_in, tint_out, 1.0f);
  return tint_out;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_Called_By_EntryPoint_With_Uniform) {
  Global("coord", ty.vec4<f32>(), ast::StorageClass::kUniform, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kFunction)},
       ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
       },
       ast::DecorationList{});

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kFunction, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(cbuffer cbuffer_coord : register(b0, space1) {
  float4 coord;
};

float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_Called_By_EntryPoint_With_StorageBuffer) {
  type::AccessControl ac(ast::AccessControl::kReadWrite, ty.vec4<f32>());
  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kFunction)},
       ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
       },
       ast::DecorationList{});

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kFunction, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(RWByteAddressBuffer coord : register(u0, space1);

float sub_func(float param) {
  return asfloat(coord.Load((4 * 0)));
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
})"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoints_WithGlobal_Nested_Return) {
  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(1),
         });

  auto* list = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  Func(
      "ep_1", ast::VariableList{}, ty.void_(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("bar"), Expr(1.0f)),
          create<ast::IfStatement>(create<ast::BinaryExpression>(
                                       ast::BinaryOp::kEqual, Expr(1), Expr(1)),
                                   list, ast::ElseStatementList{}),
          create<ast::ReturnStatement>(),
      },
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment),
      });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_out {
  float bar : SV_Target1;
};

ep_1_out ep_1() {
  ep_1_out tint_out = (ep_1_out)0;
  tint_out.bar = 1.0f;
  if ((1 == 1)) {
    return tint_out;
  }
  return tint_out;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_WithNameCollision) {
  Func("GeometryShader", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(void GeometryShader_tint_0() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Decoration_EntryPoint_Compute) {
  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(1, 1, 1)]
void main() {
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_Compute_WithWorkgroup) {
  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           create<ast::WorkgroupDecoration>(2u, 4u, 6u),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(2, 4, 6)]
void main() {
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
  Func(
      "my_func",
      ast::VariableList{Var("a", ty.array<f32, 5>(), ast::StorageClass::kNone)},
      ty.void_(),
      ast::StatementList{
          create<ast::ReturnStatement>(),
      });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func(float a[5]) {
    return;
  }

)");
}

// https://crbug.com/tint/297
TEST_F(HlslGeneratorImplTest_Function,
       Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   d : f32;
  // };
  // [[binding(0), group(0)]] var<storage> data : Data;
  //
  // [[stage(compute)]]
  // fn a() -> void {
  //   return;
  // }
  //
  // [[stage(compute)]]
  // fn b() -> void {
  //   return;
  // }

  auto* s =
      Structure("Data", {Member("d", ty.f32())},
                ast::DecorationList{create<ast::StructBlockDecoration>()});

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("data", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                    MemberAccessor("data", "d"));

    Func("a", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             create<ast::VariableDeclStatement>(var),
             create<ast::ReturnStatement>(),
         },
         ast::DecorationList{
             create<ast::StageDecoration>(ast::PipelineStage::kCompute),
         });
  }

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                    MemberAccessor("data", "d"));

    Func("b", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             create<ast::VariableDeclStatement>(var),
             create<ast::ReturnStatement>(),
         },
         ast::DecorationList{
             create<ast::StageDecoration>(ast::PipelineStage::kCompute),
         });
  }

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(
RWByteAddressBuffer data : register(u0, space0);

[numthreads(1, 1, 1)]
void a() {
  float v = asfloat(data.Load(0));
  return;
}

[numthreads(1, 1, 1)]
void b() {
  float v = asfloat(data.Load(0));
  return;
}

)");

  Validate();
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
