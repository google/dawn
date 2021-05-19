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
#include "src/writer/hlsl/test_helper.h"

using ::testing::HasSubstr;

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Function = TestHelper;

TEST_F(HlslGeneratorImplTest_Function, Emit_Function) {
  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           Return(),
       });

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
       {
           Return(),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(R"(  void tint_symbol() {
    return;
  })"));
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithParams) {
  Func("my_func", ast::VariableList{Param("a", ty.f32()), Param("b", ty.i32())},
       ty.void_(),
       {
           Return(),
       });

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
  Func("main", ast::VariableList{}, ty.void_(), {/* no explicit return */},
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(void main() {
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function, PtrParameter) {
  // fn f(foo : ptr<function, f32>) -> f32 {
  //   return *foo;
  // }
  Func("f", {Param("foo", ty.pointer<f32>(ast::StorageClass::kFunction))},
       ty.f32(), {Return(Deref("foo"))});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(R"(float f(inout float foo) {
  return foo;
}

)"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_WithInOutVars) {
  // fn frag_main([[location(0)]] foo : f32) -> [[location(1)]] f32 {
  //   return foo;
  // }
  auto* foo_in = Param("foo", ty.f32(), {Location(0)});
  Func("frag_main", ast::VariableList{foo_in}, ty.f32(), {Return("foo")},
       {Stage(ast::PipelineStage::kFragment)}, {Location(1)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_1 {
  float foo : TEXCOORD0;
};
struct tint_symbol_2 {
  float value : SV_Target1;
};

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  const float foo = tint_symbol.foo;
  const tint_symbol_2 tint_symbol_3 = {foo};
  return tint_symbol_3;
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
      Param("coord", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)});
  Func("frag_main", ast::VariableList{coord_in}, ty.f32(),
       {Return(MemberAccessor("coord", "x"))},
       {Stage(ast::PipelineStage::kFragment)},
       {Builtin(ast::Builtin::kFragDepth)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_1 {
  float4 coord : SV_Position;
};
struct tint_symbol_2 {
  float value : SV_Depth;
};

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  const float4 coord = tint_symbol.coord;
  const tint_symbol_2 tint_symbol_3 = {coord.x};
  return tint_symbol_3;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_SharedStruct_DifferentStages) {
  // struct Interface {
  //   [[builtin(position)]] pos : vec4<f32>;
  //   [[location(1)]] col1 : f32;
  //   [[location(2)]] col2 : f32;
  // };
  // fn vert_main() -> Interface {
  //   return Interface(vec4<f32>(), 0.4, 0.6);
  // }
  // fn frag_main(inputs : Interface) {
  //   const r = inputs.col1;
  //   const g = inputs.col2;
  //   const p = inputs.pos;
  // }
  auto* interface_struct = Structure(
      "Interface",
      {
          Member("pos", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)}),
          Member("col1", ty.f32(), {Location(1)}),
          Member("col2", ty.f32(), {Location(2)}),
      });

  Func("vert_main", {}, interface_struct,
       {Return(Construct(interface_struct, Construct(ty.vec4<f32>()),
                         Expr(0.5f), Expr(0.25f)))},
       {Stage(ast::PipelineStage::kVertex)});

  Func("frag_main", {Param("inputs", interface_struct)}, ty.void_(),
       {
           Decl(Const("r", ty.f32(), MemberAccessor("inputs", "col1"))),
           Decl(Const("g", ty.f32(), MemberAccessor("inputs", "col2"))),
           Decl(Const("p", ty.vec4<f32>(), MemberAccessor("inputs", "pos"))),
       },
       {Stage(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct Interface {
  float4 pos;
  float col1;
  float col2;
};
struct tint_symbol {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};
struct tint_symbol_3 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

tint_symbol vert_main() {
  const Interface tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f), 0.5f, 0.25f};
  const tint_symbol tint_symbol_4 = {tint_symbol_1.col1, tint_symbol_1.col2, tint_symbol_1.pos};
  return tint_symbol_4;
}

void frag_main(tint_symbol_3 tint_symbol_2) {
  const Interface inputs = {tint_symbol_2.pos, tint_symbol_2.col1, tint_symbol_2.col2};
  const float r = inputs.col1;
  const float g = inputs.col2;
  const float4 p = inputs.pos;
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
      {Member("pos", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)})});

  Func("foo", {Param("x", ty.f32())}, vertex_output_struct,
       {Return(Construct(vertex_output_struct,
                         Construct(ty.vec4<f32>(), "x", "x", "x", Expr(1.f))))},
       {});

  Func("vert_main1", {}, vertex_output_struct,
       {Return(Construct(vertex_output_struct, Expr(Call("foo", Expr(0.5f)))))},
       {Stage(ast::PipelineStage::kVertex)});

  Func(
      "vert_main2", {}, vertex_output_struct,
      {Return(Construct(vertex_output_struct, Expr(Call("foo", Expr(0.25f)))))},
      {Stage(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct VertexOutput {
  float4 pos;
};
struct tint_symbol {
  float4 pos : SV_Position;
};
struct tint_symbol_2 {
  float4 pos : SV_Position;
};

VertexOutput foo(float x) {
  const VertexOutput tint_symbol_4 = {float4(x, x, x, 1.0f)};
  return tint_symbol_4;
}

tint_symbol vert_main1() {
  const VertexOutput tint_symbol_1 = {foo(0.5f)};
  const tint_symbol tint_symbol_5 = {tint_symbol_1.pos};
  return tint_symbol_5;
}

tint_symbol_2 vert_main2() {
  const VertexOutput tint_symbol_3 = {foo(0.25f)};
  const tint_symbol_2 tint_symbol_6 = {tint_symbol_3.pos};
  return tint_symbol_6;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_Uniform) {
  auto* ubo_ty = Structure("UBO", {Member("coord", ty.vec4<f32>())},
                           {create<ast::StructBlockDecoration>()});
  auto* ubo = Global(
      "ubo", ubo_ty, ast::StorageClass::kUniform, nullptr,
      {create<ast::BindingDecoration>(0), create<ast::GroupDecoration>(1)});

  Func("sub_func",
       {
           Param("param", ty.f32()),
       },
       ty.f32(),
       {
           Return(MemberAccessor(MemberAccessor(ubo, "coord"), "x")),
       });

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kNone, Call("sub_func", 1.0f));

  Func("frag_main", {}, ty.void_(),
       {
           Decl(var),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct UBO {
  float4 coord;
};

ConstantBuffer<UBO> ubo : register(b0, space1);

float sub_func(float param) {
  return ubo.coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_UniformStruct) {
  auto* s = Structure("Uniforms", {Member("coord", ty.vec4<f32>())},
                      {create<ast::StructBlockDecoration>()});

  Global("uniforms", s, ast::StorageClass::kUniform, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                  MemberAccessor(MemberAccessor("uniforms", "coord"), "x"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
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
  auto* s = Structure("Data",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});

  auto* ac = ty.access(ast::AccessControl::kReadWrite, s);

  Global("coord", ac, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                  MemberAccessor("coord", "b"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(),
            R"(
RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_RO_StorageBuffer_Read) {
  auto* s = Structure("Data",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});

  auto* ac = ty.access(ast::AccessControl::kReadOnly, s);

  Global("coord", ac, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                  MemberAccessor("coord", "b"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(),
            R"(
ByteAddressBuffer coord : register(t0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_WO_StorageBuffer_Store) {
  auto* s = Structure("Data",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});

  auto* ac = ty.access(ast::AccessControl::kWriteOnly, s);

  Global("coord", ac, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("frag_main", ast::VariableList{}, ty.void_(),
       {
           Assign(MemberAccessor("coord", "b"), Expr(2.0f)),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(),
            R"(
RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_StorageBuffer_Store) {
  auto* s = Structure("Data",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});

  auto* ac = ty.access(ast::AccessControl::kReadWrite, s);

  Global("coord", ac, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("frag_main", ast::VariableList{}, ty.void_(),
       {
           Assign(MemberAccessor("coord", "b"), Expr(2.0f)),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(),
            R"(
RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}

)");

  Validate();
}

// TODO(crbug.com/tint/697): Remove this test
TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_Decoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {  // NOLINT
  Global("foo", ty.f32(), ast::StorageClass::kInput, nullptr,
         {
             Location(0),
         });

  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         {
             Location(1),
         });

  Global("val", ty.f32(), ast::StorageClass::kOutput, nullptr,
         {
             Location(0),
         });

  Func("sub_func", ast::VariableList{Param("param", ty.f32())}, ty.f32(),
       {
           Assign("bar", "foo"),
           Assign("val", "param"),
           Return("foo"),
       });

  Func("ep_1", ast::VariableList{}, ty.void_(),
       {
           Assign("bar", Call("sub_func", 1.0f)),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
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
         {
             Builtin(ast::Builtin::kFragDepth),
         });

  Func("sub_func", ast::VariableList{Param("param", ty.f32())}, ty.f32(),
       {
           Return("param"),
       });

  Func("ep_1", ast::VariableList{}, ty.void_(),
       {
           Assign("depth", Call("sub_func", 1.0f)),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
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
         {
             Builtin(ast::Builtin::kPosition),
         });

  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         {
             Builtin(ast::Builtin::kFragDepth),
         });

  Func("sub_func", ast::VariableList{Param("param", ty.f32())}, ty.f32(),
       {
           Assign("depth", MemberAccessor("coord", "x")),
           Return("param"),
       });

  Func("ep_1", ast::VariableList{}, ty.void_(),
       {
           Assign("depth", Call("sub_func", 1.0f)),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
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
  auto* s = Structure("S", {Member("x", ty.f32())},
                      {create<ast::StructBlockDecoration>()});
  Global("coord", s, ast::StorageClass::kUniform, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("sub_func", ast::VariableList{Param("param", ty.f32())}, ty.f32(),
       {
           Return(MemberAccessor("coord", "x")),
       });

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kNone, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct S {
  float x;
};

ConstantBuffer<S> coord : register(b0, space1);

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
  auto* s = Structure("S", {Member("x", ty.f32())},
                      {create<ast::StructBlockDecoration>()});
  auto* ac = ty.access(ast::AccessControl::kReadWrite, s);
  Global("coord", ac, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("sub_func", ast::VariableList{Param("param", ty.f32())}, ty.f32(),
       {
           Return(MemberAccessor("coord", "x")),
       });

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kNone, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           Return(),
       },
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(),
            R"(
RWByteAddressBuffer coord : register(u0, space1);

float sub_func(float param) {
  return asfloat(coord.Load(0u));
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoints_WithGlobal_Nested_Return) {
  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         {
             Location(1),
         });

  Func(
      "ep_1", ast::VariableList{}, ty.void_(),
      {
          Assign("bar", Expr(1.0f)),
          create<ast::IfStatement>(create<ast::BinaryExpression>(
                                       ast::BinaryOp::kEqual, Expr(1), Expr(1)),
                                   Block(Return()), ast::ElseStatementList{}),
          Return(),
      },
      {
          Stage(ast::PipelineStage::kFragment),
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
  Func("GeometryShader", ast::VariableList{}, ty.void_(), {},
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(void tint_symbol() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Decoration_EntryPoint_Compute) {
  Func("main", ast::VariableList{}, ty.void_(),
       {
           Return(),
       },
       {
           Stage(ast::PipelineStage::kCompute),
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
       Emit_Decoration_EntryPoint_Compute_WithWorkgroup_Literal) {
  Func("main", ast::VariableList{}, ty.void_(), {},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(2, 4, 6),
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

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_Compute_WithWorkgroup_Const) {
  GlobalConst("width", ty.i32(), Construct(ty.i32(), 2));
  GlobalConst("height", ty.i32(), Construct(ty.i32(), 3));
  GlobalConst("depth", ty.i32(), Construct(ty.i32(), 4));
  Func("main", ast::VariableList{}, ty.void_(), {},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize("width", "height", "depth"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(static const int width = int(2);
static const int height = int(3);
static const int depth = int(4);
[numthreads(2, 3, 4)]
void main() {
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_Compute_WithWorkgroup_OverridableConst) {
  GlobalConst("width", ty.i32(), Construct(ty.i32(), 2), {Override(7u)});
  GlobalConst("height", ty.i32(), Construct(ty.i32(), 3), {Override(8u)});
  GlobalConst("depth", ty.i32(), Construct(ty.i32(), 4), {Override(9u)});
  Func("main", ast::VariableList{}, ty.void_(), {},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize("width", "height", "depth"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(#ifndef WGSL_SPEC_CONSTANT_7
#define WGSL_SPEC_CONSTANT_7 int(2)
#endif
static const int width = WGSL_SPEC_CONSTANT_7;
#ifndef WGSL_SPEC_CONSTANT_8
#define WGSL_SPEC_CONSTANT_8 int(3)
#endif
static const int height = WGSL_SPEC_CONSTANT_8;
#ifndef WGSL_SPEC_CONSTANT_9
#define WGSL_SPEC_CONSTANT_9 int(4)
#endif
static const int depth = WGSL_SPEC_CONSTANT_9;
[numthreads(WGSL_SPEC_CONSTANT_7, WGSL_SPEC_CONSTANT_8, WGSL_SPEC_CONSTANT_9)]
void main() {
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
  Func("my_func", ast::VariableList{Param("a", ty.array<f32, 5>())}, ty.void_(),
       {
           Return(),
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
  // fn a() {
  //   var v = data.d;
  //   return;
  // }
  //
  // [[stage(compute)]]
  // fn b() {
  //   var v = data.d;
  //   return;
  // }

  auto* s = Structure("Data", {Member("d", ty.f32())},
                      {create<ast::StructBlockDecoration>()});

  auto* ac = ty.access(ast::AccessControl::kReadWrite, s);

  Global("data", ac, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                    MemberAccessor("data", "d"));

    Func("a", ast::VariableList{}, ty.void_(),
         {
             Decl(var),
             Return(),
         },
         {
             Stage(ast::PipelineStage::kCompute),
         });
  }

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                    MemberAccessor("data", "d"));

    Func("b", ast::VariableList{}, ty.void_(),
         {
             Decl(var),
             Return(),
         },
         {
             Stage(ast::PipelineStage::kCompute),
         });
  }

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(
RWByteAddressBuffer data : register(u0, space0);

[numthreads(1, 1, 1)]
void a() {
  float v = asfloat(data.Load(0u));
  return;
}

[numthreads(1, 1, 1)]
void b() {
  float v = asfloat(data.Load(0u));
  return;
}

)");

  Validate();
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
