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

#include "gmock/gmock.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/writer/glsl/test_helper.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_Function = TestHelper;

TEST_F(GlslGeneratorImplTest_Function, Emit_Function) {
    Func("my_func", ast::VariableList{}, ty.void_(),
         {
             Return(),
         });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(  #version 310 es

  void my_func() {
    return;
  }

)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Function_Name_Collision) {
    Func("centroid", ast::VariableList{}, ty.void_(),
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

TEST_F(GlslGeneratorImplTest_Function, Emit_Function_WithParams) {
    Func("my_func", ast::VariableList{Param("a", ty.f32()), Param("b", ty.i32())}, ty.void_(),
         {
             Return(),
         });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(  #version 310 es

  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_NoReturn_Void) {
    Func("func", ast::VariableList{}, ty.void_(), {/* no explicit return */},
         {
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

void func() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, PtrParameter) {
    // fn f(foo : ptr<function, f32>) -> f32 {
    //   return *foo;
    // }
    Func("f", {Param("foo", ty.pointer<f32>(ast::StorageClass::kFunction))}, ty.f32(),
         {Return(Deref("foo"))});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(R"(float f(inout float foo) {
  return foo;
}
)"));
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_WithInOutVars) {
    // fn frag_main(@location(0) foo : f32) -> @location(1) f32 {
    //   return foo;
    // }
    auto* foo_in = Param("foo", ty.f32(), {Location(0)});
    Func("frag_main", ast::VariableList{foo_in}, ty.f32(), {Return("foo")},
         {Stage(ast::PipelineStage::kFragment)}, {Location(1)});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

layout(location = 0) in float foo_1;
layout(location = 1) out float value;
float frag_main(float foo) {
  return foo;
}

void main() {
  float inner_result = frag_main(foo_1);
  value = inner_result;
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_WithInOut_Builtins) {
    // fn frag_main(@position(0) coord : vec4<f32>) -> @frag_depth f32 {
    //   return coord.x;
    // }
    auto* coord_in = Param("coord", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)});
    Func("frag_main", ast::VariableList{coord_in}, ty.f32(), {Return(MemberAccessor("coord", "x"))},
         {Stage(ast::PipelineStage::kFragment)}, {Builtin(ast::Builtin::kFragDepth)});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

float frag_main(vec4 coord) {
  return coord.x;
}

void main() {
  float inner_result = frag_main(gl_FragCoord);
  gl_FragDepth = inner_result;
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_SharedStruct_DifferentStages) {
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
        "Interface", {
                         Member("pos", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)}),
                         Member("col1", ty.f32(), {Location(1)}),
                         Member("col2", ty.f32(), {Location(2)}),
                     });

    Func("vert_main", {}, ty.Of(interface_struct),
         {Return(Construct(ty.Of(interface_struct), Construct(ty.vec4<f32>()), Expr(0.5f),
                           Expr(0.25f)))},
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
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

layout(location = 1) out float col1_1;
layout(location = 2) out float col2_1;
layout(location = 1) in float col1_2;
layout(location = 2) in float col2_2;
struct Interface {
  vec4 pos;
  float col1;
  float col2;
};

Interface vert_main() {
  Interface tint_symbol = Interface(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0.5f, 0.25f);
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  Interface inner_result = vert_main();
  gl_Position = inner_result.pos;
  col1_1 = inner_result.col1;
  col2_1 = inner_result.col2;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
void frag_main(Interface inputs) {
  float r = inputs.col1;
  float g = inputs.col2;
  vec4 p = inputs.pos;
}

void main_1() {
  Interface tint_symbol_1 = Interface(gl_FragCoord, col1_2, col2_2);
  frag_main(tint_symbol_1);
  return;
}
)");
}

#if 0
TEST_F(GlslGeneratorImplTest_Function,
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
       {Return(Construct(ty.Of(vertex_output_struct),
                         Expr(Call("foo", Expr(0.5f)))))},
       {Stage(ast::PipelineStage::kVertex)});

  Func("vert_main2", {}, ty.Of(vertex_output_struct),
       {Return(Construct(ty.Of(vertex_output_struct),
                         Expr(Call("foo", Expr(0.25f)))))},
       {Stage(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct VertexOutput {
  float4 pos;
};

VertexOutput foo(float x) {
  const VertexOutput tint_symbol_4 = {float4(x, x, x, 1.0f)};
  return tint_symbol_4;
}

struct tint_symbol {
  float4 pos : SV_Position;
};

tint_symbol vert_main1() {
  const VertexOutput tint_symbol_1 = {foo(0.5f)};
  const tint_symbol tint_symbol_5 = {tint_symbol_1.pos};
  return tint_symbol_5;
}

struct tint_symbol_2 {
  float4 pos : SV_Position;
};

tint_symbol_2 vert_main2() {
  const VertexOutput tint_symbol_3 = {foo(0.25f)};
  const tint_symbol_2 tint_symbol_6 = {tint_symbol_3.pos};
  return tint_symbol_6;
}
)");
}
#endif

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_Uniform) {
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

    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, Call("sub_func", 1.0f));

    Func("frag_main", {}, ty.void_(),
         {
             Decl(var),
             Return(),
         },
         {
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

struct UBO {
  vec4 coord;
};

layout(binding = 0) uniform UBO_1 {
  vec4 coord;
} ubo;

float sub_func(float param) {
  return ubo.coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_UniformStruct) {
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
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

struct Uniforms {
  vec4 coord;
};

layout(binding = 0) uniform Uniforms_1 {
  vec4 coord;
} uniforms;

void frag_main() {
  float v = uniforms.coord.x;
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_RW_StorageBuffer_Read) {
    auto* s = Structure("Data", {
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    Global("coord", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(1),
           });

    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, MemberAccessor("coord", "b"));

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
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

struct Data {
  int a;
  float b;
};

layout(binding = 0, std430) buffer Data_1 {
  int a;
  float b;
} coord;
void frag_main() {
  float v = coord.b;
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_RO_StorageBuffer_Read) {
    auto* s = Structure("Data", {
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    Global("coord", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(1),
           });

    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, MemberAccessor("coord", "b"));

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
              R"(#version 310 es
precision mediump float;

struct Data {
  int a;
  float b;
};

layout(binding = 0, std430) buffer Data_1 {
  int a;
  float b;
} coord;
void frag_main() {
  float v = coord.b;
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_WO_StorageBuffer_Store) {
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
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

struct Data {
  int a;
  float b;
};

layout(binding = 0, std430) buffer Data_1 {
  int a;
  float b;
} coord;
void frag_main() {
  coord.b = 2.0f;
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_StorageBuffer_Store) {
    auto* s = Structure("Data", {
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    Global("coord", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
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
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

struct Data {
  int a;
  float b;
};

layout(binding = 0, std430) buffer Data_1 {
  int a;
  float b;
} coord;
void frag_main() {
  coord.b = 2.0f;
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_Called_By_EntryPoint_With_Uniform) {
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

    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, Call("sub_func", 1.0f));

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
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

struct S {
  float x;
};

layout(binding = 0) uniform S_1 {
  float x;
} coord;

float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_Called_By_EntryPoint_With_StorageBuffer) {
    auto* s = Structure("S", {Member("x", ty.f32())});
    Global("coord", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(1),
           });

    Func("sub_func", ast::VariableList{Param("param", ty.f32())}, ty.f32(),
         {
             Return(MemberAccessor("coord", "x")),
         });

    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, Call("sub_func", 1.0f));

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
              R"(#version 310 es
precision mediump float;

struct S {
  float x;
};

layout(binding = 0, std430) buffer S_1 {
  float x;
} coord;
float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_WithNameCollision) {
    Func("centroid", ast::VariableList{}, ty.void_(), {},
         {
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

void tint_symbol() {
}

void main() {
  tint_symbol();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute) {
    Func("main", ast::VariableList{}, ty.void_(),
         {
             Return(),
         },
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Literal) {
    Func("main", ast::VariableList{}, ty.void_(), {},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(2_i, 4_i, 6_i),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 2, local_size_y = 4, local_size_z = 6) in;
void main() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Const) {
    GlobalConst("width", ty.i32(), Construct(ty.i32(), 2_i));
    GlobalConst("height", ty.i32(), Construct(ty.i32(), 3_i));
    GlobalConst("depth", ty.i32(), Construct(ty.i32(), 4_i));
    Func("main", ast::VariableList{}, ty.void_(), {},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize("width", "height", "depth"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

const int width = int(2);
const int height = int(3);
const int depth = int(4);
layout(local_size_x = 2, local_size_y = 3, local_size_z = 4) in;
void main() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_Compute_WithWorkgroup_OverridableConst) {
    Override("width", ty.i32(), Construct(ty.i32(), 2_i), {Id(7u)});
    Override("height", ty.i32(), Construct(ty.i32(), 3_i), {Id(8u)});
    Override("depth", ty.i32(), Construct(ty.i32(), 4_i), {Id(9u)});
    Func("main", ast::VariableList{}, ty.void_(), {},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize("width", "height", "depth"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

#ifndef WGSL_SPEC_CONSTANT_7
#define WGSL_SPEC_CONSTANT_7 int(2)
#endif
const int width = WGSL_SPEC_CONSTANT_7;
#ifndef WGSL_SPEC_CONSTANT_8
#define WGSL_SPEC_CONSTANT_8 int(3)
#endif
const int height = WGSL_SPEC_CONSTANT_8;
#ifndef WGSL_SPEC_CONSTANT_9
#define WGSL_SPEC_CONSTANT_9 int(4)
#endif
const int depth = WGSL_SPEC_CONSTANT_9;
layout(local_size_x = WGSL_SPEC_CONSTANT_7, local_size_y = WGSL_SPEC_CONSTANT_8, local_size_z = WGSL_SPEC_CONSTANT_9) in;
void main() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
    Func("my_func", ast::VariableList{Param("a", ty.array<f32, 5>())}, ty.void_(),
         {
             Return(),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

void my_func(float a[5]) {
  return;
}

)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Function_WithArrayReturn) {
    Func("my_func", {}, ty.array<f32, 5>(),
         {
             Return(Construct(ty.array<f32, 5>())),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

float[5] my_func() {
  return float[5](0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

)");
}

// https://crbug.com/tint/297
TEST_F(GlslGeneratorImplTest_Function, Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
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
        auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, MemberAccessor("data", "d"));

        Func("a", ast::VariableList{}, ty.void_(),
             {
                 Decl(var),
                 Return(),
             },
             {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});
    }

    {
        auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, MemberAccessor("data", "d"));

        Func("b", ast::VariableList{}, ty.void_(),
             {
                 Decl(var),
                 Return(),
             },
             {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});
    }

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

struct Data {
  float d;
};

layout(binding = 0, std430) buffer Data_1 {
  float d;
} data;
void a() {
  float v = data.d;
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a();
  return;
}
void b() {
  float v = data.d;
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main_1() {
  b();
  return;
}
)");
}

}  // namespace
}  // namespace tint::writer::glsl
