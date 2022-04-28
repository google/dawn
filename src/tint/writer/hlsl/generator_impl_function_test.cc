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
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/writer/hlsl/test_helper.h"

using ::testing::HasSubstr;

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_Function = TestHelper;

TEST_F(HlslGeneratorImplTest_Function, Emit_Function) {
  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           Return(),
       });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(  void my_func() {
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr(R"(  void tint_symbol() {
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(  void my_func(float a, int b) {
    return;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_NoReturn_Void) {
  Func("main", ast::VariableList{}, ty.void_(), {/* no explicit return */},
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(void main() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, PtrParameter) {
  // fn f(foo : ptr<function, f32>) -> f32 {
  //   return *foo;
  // }
  Func("f", {Param("foo", ty.pointer<f32>(ast::StorageClass::kFunction))},
       ty.f32(), {Return(Deref("foo"))});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr(R"(float f(inout float foo) {
  return foo;
}
)"));
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_WithInOutVars) {
  // fn frag_main(@location(0) foo : f32) -> @location(1) f32 {
  //   return foo;
  // }
  auto* foo_in = Param("foo", ty.f32(), {Location(0)});
  Func("frag_main", ast::VariableList{foo_in}, ty.f32(), {Return("foo")},
       {Stage(ast::PipelineStage::kFragment)}, {Location(1)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct tint_symbol_1 {
  float foo : TEXCOORD0;
};
struct tint_symbol_2 {
  float value : SV_Target1;
};

float frag_main_inner(float foo) {
  return foo;
}

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  const float inner_result = frag_main_inner(tint_symbol.foo);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_WithInOut_Builtins) {
  // fn frag_main(@position(0) coord : vec4<f32>) -> @frag_depth f32 {
  //   return coord.x;
  // }
  auto* coord_in =
      Param("coord", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)});
  Func("frag_main", ast::VariableList{coord_in}, ty.f32(),
       {Return(MemberAccessor("coord", "x"))},
       {Stage(ast::PipelineStage::kFragment)},
       {Builtin(ast::Builtin::kFragDepth)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct tint_symbol_1 {
  float4 coord : SV_Position;
};
struct tint_symbol_2 {
  float value : SV_Depth;
};

float frag_main_inner(float4 coord) {
  return coord.x;
}

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  const float inner_result = frag_main_inner(tint_symbol.coord);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_SharedStruct_DifferentStages) {
  // struct Interface {
  //   @builtin(position) pos : vec4<f32>;
  //   @location(1) col1 : f32;
  //   @location(2) col2 : f32;
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

  Func("vert_main", {}, ty.Of(interface_struct),
       {Return(Construct(ty.Of(interface_struct), Construct(ty.vec4<f32>()),
                         Expr(0.5f), Expr(0.25f)))},
       {Stage(ast::PipelineStage::kVertex)});

  Func("frag_main", {Param("inputs", ty.Of(interface_struct))}, ty.void_(),
       {
           Decl(Let("r", ty.f32(), MemberAccessor("inputs", "col1"))),
           Decl(Let("g", ty.f32(), MemberAccessor("inputs", "col2"))),
           Decl(Let("p", ty.vec4<f32>(), MemberAccessor("inputs", "pos"))),
       },
       {Stage(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct Interface {
  float4 pos;
  float col1;
  float col2;
};
struct tint_symbol {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

Interface vert_main_inner() {
  const Interface tint_symbol_3 = {float4(0.0f, 0.0f, 0.0f, 0.0f), 0.5f, 0.25f};
  return tint_symbol_3;
}

tint_symbol vert_main() {
  const Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  return wrapper_result;
}

struct tint_symbol_2 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

void frag_main_inner(Interface inputs) {
  const float r = inputs.col1;
  const float g = inputs.col2;
  const float4 p = inputs.pos;
}

void frag_main(tint_symbol_2 tint_symbol_1) {
  const Interface tint_symbol_4 = {tint_symbol_1.pos, tint_symbol_1.col1, tint_symbol_1.col2};
  frag_main_inner(tint_symbol_4);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_SharedStruct_HelperFunction) {
  // struct VertexOutput {
  //   @builtin(position) pos : vec4<f32>;
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

  Func("foo", {Param("x", ty.f32())}, ty.Of(vertex_output_struct),
       {Return(Construct(ty.Of(vertex_output_struct),
                         Construct(ty.vec4<f32>(), "x", "x", "x", Expr(1.f))))},
       {});

  Func("vert_main1", {}, ty.Of(vertex_output_struct),
       {Return(Call("foo", Expr(0.5f)))}, {Stage(ast::PipelineStage::kVertex)});

  Func("vert_main2", {}, ty.Of(vertex_output_struct),
       {Return(Call("foo", Expr(0.25f)))},
       {Stage(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct VertexOutput {
  float4 pos;
};

VertexOutput foo(float x) {
  const VertexOutput tint_symbol_2 = {float4(x, x, x, 1.0f)};
  return tint_symbol_2;
}

struct tint_symbol {
  float4 pos : SV_Position;
};

VertexOutput vert_main1_inner() {
  return foo(0.5f);
}

tint_symbol vert_main1() {
  const VertexOutput inner_result = vert_main1_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_1 {
  float4 pos : SV_Position;
};

VertexOutput vert_main2_inner() {
  return foo(0.25f);
}

tint_symbol_1 vert_main2() {
  const VertexOutput inner_result_1 = vert_main2_inner();
  tint_symbol_1 wrapper_result_1 = (tint_symbol_1)0;
  wrapper_result_1.pos = inner_result_1.pos;
  return wrapper_result_1;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_Uniform) {
  auto* ubo_ty = Structure("UBO", {Member("coord", ty.vec4<f32>())});
  auto* ubo = Global("ubo", ty.Of(ubo_ty), ast::StorageClass::kUniform,
                     ast::AttributeList{
                         create<ast::BindingAttribute>(0),
                         create<ast::GroupAttribute>(1),
                     });

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

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(cbuffer cbuffer_ubo : register(b0, space1) {
  uint4 ubo[1];
};

float sub_func(float param) {
  return asfloat(ubo[0].x);
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_With_UniformStruct) {
  auto* s = Structure("Uniforms", {Member("coord", ty.vec4<f32>())});

  Global("uniforms", ty.Of(s), ast::StorageClass::kUniform,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(1),
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(cbuffer cbuffer_uniforms : register(b0, space1) {
  uint4 uniforms[1];
};

void frag_main() {
  float v = uniforms.coord.x;
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_With_RW_StorageBuffer_Read) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  Global("coord", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kReadWrite,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(1),
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(),
            R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_With_RO_StorageBuffer_Read) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  Global("coord", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(1),
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(),
            R"(ByteAddressBuffer coord : register(t0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_With_WO_StorageBuffer_Store) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  Global("coord", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kWrite,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(1),
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(),
            R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_With_StorageBuffer_Store) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  Global("coord", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kReadWrite,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(1),
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(),
            R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_Called_By_EntryPoint_With_Uniform) {
  auto* s = Structure("S", {Member("x", ty.f32())});
  Global("coord", ty.Of(s), ast::StorageClass::kUniform,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(1),
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(cbuffer cbuffer_coord : register(b0, space1) {
  uint4 coord[1];
};

float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_Called_By_EntryPoint_With_StorageBuffer) {
  auto* s = Structure("S", {Member("x", ty.f32())});
  Global("coord", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kReadWrite,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(1),
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

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(),
            R"(RWByteAddressBuffer coord : register(u0, space1);

float sub_func(float param) {
  return asfloat(coord.Load(0u));
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_WithNameCollision) {
  Func("GeometryShader", ast::VariableList{}, ty.void_(), {},
       {
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(void tint_symbol() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute) {
  Func("main", ast::VariableList{}, ty.void_(),
       {
           Return(),
       },
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"([numthreads(1, 1, 1)]
void main() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Literal) {
  Func("main", ast::VariableList{}, ty.void_(), {},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(2, 4, 6),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"([numthreads(2, 4, 6)]
void main() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Const) {
  GlobalConst("width", ty.i32(), Construct(ty.i32(), 2));
  GlobalConst("height", ty.i32(), Construct(ty.i32(), 3));
  GlobalConst("depth", ty.i32(), Construct(ty.i32(), 4));
  Func("main", ast::VariableList{}, ty.void_(), {},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize("width", "height", "depth"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(static const int width = int(2);
static const int height = int(3);
static const int depth = int(4);

[numthreads(2, 3, 4)]
void main() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_Compute_WithWorkgroup_OverridableConst) {
  Override("width", ty.i32(), Construct(ty.i32(), 2), {Id(7u)});
  Override("height", ty.i32(), Construct(ty.i32(), 3), {Id(8u)});
  Override("depth", ty.i32(), Construct(ty.i32(), 4), {Id(9u)});
  Func("main", ast::VariableList{}, ty.void_(), {},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize("width", "height", "depth"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#ifndef WGSL_SPEC_CONSTANT_7
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
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
  Func("my_func", ast::VariableList{Param("a", ty.array<f32, 5>())}, ty.void_(),
       {
           Return(),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(void my_func(float a[5]) {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayReturn) {
  Func("my_func", {}, ty.array<f32, 5>(),
       {
           Return(Construct(ty.array<f32, 5>())),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(typedef float my_func_ret[5];
my_func_ret my_func() {
  return (float[5])0;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithDiscardAndVoidReturn) {
  Func("my_func", {Param("a", ty.i32())}, ty.void_(),
       {
           If(Equal("a", 0),  //
              Block(create<ast::DiscardStatement>())),
           Return(),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(void my_func(int a) {
  if ((a == 0)) {
    discard;
  }
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Function_WithDiscardAndNonVoidReturn) {
  Func("my_func", {Param("a", ty.i32())}, ty.i32(),
       {
           If(Equal("a", 0),  //
              Block(create<ast::DiscardStatement>())),
           Return(42),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(int my_func(int a) {
  if (true) {
    if ((a == 0)) {
      discard;
    }
    return 42;
  }
  int unused;
  return unused;
}
)");
}

// https://crbug.com/tint/297
TEST_F(HlslGeneratorImplTest_Function,
       Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
  // struct Data {
  //   d : f32;
  // };
  // @binding(0) @group(0) var<storage> data : Data;
  //
  // @stage(compute) @workgroup_size(1)
  // fn a() {
  //   var v = data.d;
  //   return;
  // }
  //
  // @stage(compute) @workgroup_size(1)
  // fn b() {
  //   var v = data.d;
  //   return;
  // }

  auto* s = Structure("Data", {Member("d", ty.f32())});

  Global("data", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(0),
         });

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                    MemberAccessor("data", "d"));

    Func("a", ast::VariableList{}, ty.void_(),
         {
             Decl(var),
             Return(),
         },
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});
  }

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                    MemberAccessor("data", "d"));

    Func("b", ast::VariableList{}, ty.void_(),
         {
             Decl(var),
             Return(),
         },
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});
  }

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(RWByteAddressBuffer data : register(u0, space0);

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
}

}  // namespace
}  // namespace tint::writer::hlsl
