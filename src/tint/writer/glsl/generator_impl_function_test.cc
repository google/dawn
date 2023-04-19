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
    Func("my_func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  #version 310 es

  void my_func() {
    return;
  }

)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Function_Name_Collision) {
    Func("centroid", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr(R"(  void tint_symbol() {
    return;
  })"));
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Function_WithParams) {
    Func("my_func",
         utils::Vector{
             Param("a", ty.f32()),
             Param("b", ty.i32()),
         },
         ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  #version 310 es

  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_NoReturn_Void) {
    Func("func", utils::Empty, ty.void_(), utils::Empty /* no explicit return */,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

void func() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, PtrParameter) {
    // fn f(foo : ptr<function, f32>) -> f32 {
    //   return *foo;
    // }
    Func("f", utils::Vector{Param("foo", ty.pointer<f32>(builtin::AddressSpace::kFunction))},
         ty.f32(), utils::Vector{Return(Deref("foo"))});

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr(R"(float f(inout float foo) {
  return foo;
}
)"));
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_WithInOutVars) {
    // fn frag_main(@location(0) foo : f32) -> @location(1) f32 {
    //   return foo;
    // }
    Func("frag_main",
         utils::Vector{
             Param("foo", ty.f32(), utils::Vector{Location(0_a)}),
         },
         ty.f32(),
         utils::Vector{
             Return("foo"),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(1_a),
         });

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

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
    auto* coord_in =
        Param("coord", ty.vec4<f32>(), utils::Vector{Builtin(builtin::BuiltinValue::kPosition)});
    Func("frag_main",
         utils::Vector{
             coord_in,
         },
         ty.f32(),
         utils::Vector{
             Return(MemberAccessor("coord", "x")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Builtin(builtin::BuiltinValue::kFragDepth),
         });

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

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
        "Interface",
        utils::Vector{
            Member("pos", ty.vec4<f32>(), utils::Vector{Builtin(builtin::BuiltinValue::kPosition)}),
            Member("col1", ty.f32(), utils::Vector{Location(1_a)}),
            Member("col2", ty.f32(), utils::Vector{Location(2_a)}),
        });

    Func("vert_main", utils::Empty, ty.Of(interface_struct),
         utils::Vector{Return(
             Call(ty.Of(interface_struct), Call(ty.vec4<f32>()), Expr(0.5_f), Expr(0.25_f)))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    Func("frag_main", utils::Vector{Param("inputs", ty.Of(interface_struct))}, ty.void_(),
         utils::Vector{
             Decl(Let("r", ty.f32(), MemberAccessor("inputs", "col1"))),
             Decl(Let("g", ty.f32(), MemberAccessor("inputs", "col2"))),
             Decl(Let("p", ty.vec4<f32>(), MemberAccessor("inputs", "pos"))),
         },
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

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
  Interface tint_symbol = Interface(vec4(0.0f), 0.5f, 0.25f);
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
      {Member("pos", ty.vec4<f32>(), {Builtin(builtin::BuiltinValue::kPosition)})});

  Func("foo", utils::Vector{Param("x", ty.f32())}, ty.Of(vertex_output_struct),
       {Return(Call(ty.Of(vertex_output_struct),
                         Call(ty.vec4<f32>(), "x", "x", "x", Expr(1_f))))},
       {});

  Func("vert_main1", utils::Empty, ty.Of(vertex_output_struct),
       {Return(Call(ty.Of(vertex_output_struct),
                         Expr(Call("foo", Expr(0.5_f)))))},
       {Stage(ast::PipelineStage::kVertex)});

  Func("vert_main2", utils::Empty, ty.Of(vertex_output_struct),
       {Return(Call(ty.Of(vertex_output_struct),
                         Expr(Call("foo", Expr(0.25_f)))))},
       {Stage(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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
    auto* ubo_ty = Structure("UBO", utils::Vector{Member("coord", ty.vec4<f32>())});
    auto* ubo =
        GlobalVar("ubo", ty.Of(ubo_ty), builtin::AddressSpace::kUniform, Binding(0_a), Group(1_a));

    Func("sub_func",
         utils::Vector{
             Param("param", ty.f32()),
         },
         ty.f32(),
         utils::Vector{
             Return(MemberAccessor(MemberAccessor(ubo, "coord"), "x")),
         });

    auto* var = Var("v", ty.f32(), Call("sub_func", 1_f));

    Func("frag_main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(var),
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

struct UBO {
  vec4 coord;
};

layout(binding = 0, std140) uniform UBO_ubo {
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
    auto* s = Structure("Uniforms", utils::Vector{Member("coord", ty.vec4<f32>())});

    GlobalVar("uniforms", ty.Of(s), builtin::AddressSpace::kUniform, Binding(0_a), Group(1_a));

    auto* var = Var("v", ty.f32(), MemberAccessor(MemberAccessor("uniforms", "coord"), "x"));

    Func("frag_main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(var),
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

struct Uniforms {
  vec4 coord;
};

layout(binding = 0, std140) uniform Uniforms_ubo {
  vec4 coord;
} uniforms;

void frag_main() {
  float v = uniforms.coord.x;
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_RW_StorageBuffer_Read) {
    auto* s = Structure("Data", utils::Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    auto* var = Var("v", ty.f32(), MemberAccessor("coord", "b"));

    Func("frag_main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(var),
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

struct Data {
  int a;
  float b;
};

layout(binding = 0, std430) buffer coord_block_ssbo {
  Data inner;
} coord;

void frag_main() {
  float v = coord.inner.b;
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_RO_StorageBuffer_Read) {
    auto* s = Structure("Data", utils::Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead,
              Binding(0_a), Group(1_a));

    auto* var = Var("v", ty.f32(), MemberAccessor("coord", "b"));

    Func("frag_main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(var),
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(),
              R"(#version 310 es
precision highp float;

struct Data {
  int a;
  float b;
};

layout(binding = 0, std430) buffer coord_block_ssbo {
  Data inner;
} coord;

void frag_main() {
  float v = coord.inner.b;
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_WO_StorageBuffer_Store) {
    auto* s = Structure("Data", utils::Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    Func("frag_main", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(MemberAccessor("coord", "b"), Expr(2_f)),
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

struct Data {
  int a;
  float b;
};

layout(binding = 0, std430) buffer coord_block_ssbo {
  Data inner;
} coord;

void frag_main() {
  coord.inner.b = 2.0f;
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_StorageBuffer_Store) {
    auto* s = Structure("Data", utils::Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    Func("frag_main", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(MemberAccessor("coord", "b"), Expr(2_f)),
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

struct Data {
  int a;
  float b;
};

layout(binding = 0, std430) buffer coord_block_ssbo {
  Data inner;
} coord;

void frag_main() {
  coord.inner.b = 2.0f;
  return;
}

void main() {
  frag_main();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_Called_By_EntryPoint_With_Uniform) {
    auto* s = Structure("S", utils::Vector{Member("x", ty.f32())});
    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kUniform, Binding(0_a), Group(1_a));

    Func("sub_func", utils::Vector{Param("param", ty.f32())}, ty.f32(),
         utils::Vector{
             Return(MemberAccessor("coord", "x")),
         });

    auto* var = Var("v", ty.f32(), Call("sub_func", 1_f));

    Func("frag_main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(var),
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

struct S {
  float x;
};

layout(binding = 0, std140) uniform S_ubo {
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
    auto* s = Structure("S", utils::Vector{Member("x", ty.f32())});
    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    Func("sub_func", utils::Vector{Param("param", ty.f32())}, ty.f32(),
         utils::Vector{
             Return(MemberAccessor("coord", "x")),
         });

    auto* var = Var("v", ty.f32(), Call("sub_func", 1_f));

    Func("frag_main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(var),
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(),
              R"(#version 310 es
precision highp float;

struct S {
  float x;
};

layout(binding = 0, std430) buffer coord_block_ssbo {
  S inner;
} coord;

float sub_func(float param) {
  return coord.inner.x;
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
    Func("centroid", utils::Empty, ty.void_(), {},
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
precision highp float;

void tint_symbol() {
}

void main() {
  tint_symbol();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute) {
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Literal) {
    Func("main", utils::Empty, ty.void_(), {},
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(2_i, 4_i, 6_i),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 2, local_size_y = 4, local_size_z = 6) in;
void main() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Const) {
    GlobalConst("width", ty.i32(), Call<i32>(2_i));
    GlobalConst("height", ty.i32(), Call<i32>(3_i));
    GlobalConst("depth", ty.i32(), Call<i32>(4_i));
    Func("main", utils::Empty, ty.void_(), {},
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize("width", "height", "depth"),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 2, local_size_y = 3, local_size_z = 4) in;
void main() {
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_Compute_WithWorkgroup_OverridableConst) {
    Override("width", ty.i32(), Call<i32>(2_i), Id(7_u));
    Override("height", ty.i32(), Call<i32>(3_i), Id(8_u));
    Override("depth", ty.i32(), Call<i32>(4_i), Id(9_u));
    Func("main", utils::Empty, ty.void_(), {},
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize("width", "height", "depth"),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_EQ(
        gen.Diagnostics().str(),
        R"(error: override-expressions should have been removed with the SubstituteOverride transform
error: override-expressions should have been removed with the SubstituteOverride transform
error: override-expressions should have been removed with the SubstituteOverride transform
error: override-expressions should have been removed with the SubstituteOverride transform)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
    Func("my_func", utils::Vector{Param("a", ty.array<f32, 5>())}, ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void my_func(float a[5]) {
  return;
}

)");
}

TEST_F(GlslGeneratorImplTest_Function, Emit_Function_WithArrayReturn) {
    Func("my_func", utils::Empty, ty.array<f32, 5>(),
         utils::Vector{
             Return(Call(ty.array<f32, 5>())),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
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
    // @compute @workgroup_size(1)
    // fn a() {
    //   var v = data.d;
    //   return;
    // }
    //
    // @compute @workgroup_size(1)
    // fn b() {
    //   var v = data.d;
    //   return;
    // }

    auto* s = Structure("Data", utils::Vector{Member("d", ty.f32())});

    GlobalVar("data", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(0_a), Group(0_a));

    {
        auto* var = Var("v", ty.f32(), MemberAccessor("data", "d"));

        Func("a", utils::Empty, ty.void_(),
             utils::Vector{
                 Decl(var),
                 Return(),
             },
             utils::Vector{
                 Stage(ast::PipelineStage::kCompute),
                 WorkgroupSize(1_i),
             });
    }

    {
        auto* var = Var("v", ty.f32(), MemberAccessor("data", "d"));

        Func("b", utils::Empty, ty.void_(),
             utils::Vector{
                 Decl(var),
                 Return(),
             },
             utils::Vector{
                 Stage(ast::PipelineStage::kCompute),
                 WorkgroupSize(1_i),
             });
    }

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

struct Data {
  float d;
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void a() {
  float v = data.inner.d;
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a();
  return;
}
void b() {
  float v = data.inner.d;
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
