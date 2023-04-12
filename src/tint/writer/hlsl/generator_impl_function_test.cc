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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_Function = TestHelper;

TEST_F(HlslGeneratorImplTest_Function, Emit_Function) {
    Func("my_func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  void my_func() {
    return;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_Name_Collision) {
    Func("GeometryShader", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr(R"(  void tint_symbol() {
    return;
  })"));
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithParams) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  void my_func(float a, int b) {
    return;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_NoReturn_Void) {
    Func("main", utils::Empty, ty.void_(), utils::Empty /* no explicit return */,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(void main() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, PtrParameter) {
    // fn f(foo : ptr<function, f32>) -> f32 {
    //   return *foo;
    // }
    Func("f", utils::Vector{Param("foo", ty.pointer<f32>(builtin::AddressSpace::kFunction))},
         ty.f32(), utils::Vector{Return(Deref("foo"))});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr(R"(float f(inout float foo) {
  return foo;
}
)"));
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_WithInOutVars) {
    // fn frag_main(@location(0) foo : f32) -> @location(1) f32 {
    //   return foo;
    // }
    auto* foo_in = Param("foo", ty.f32(), utils::Vector{Location(0_a)});
    Func("frag_main", utils::Vector{foo_in}, ty.f32(),
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_WithInOut_Builtins) {
    // fn frag_main(@position(0) coord : vec4<f32>) -> @frag_depth f32 {
    //   return coord.x;
    // }
    auto* coord_in =
        Param("coord", ty.vec4<f32>(), utils::Vector{Builtin(builtin::BuiltinValue::kPosition)});
    Func("frag_main", utils::Vector{coord_in}, ty.f32(),
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_SharedStruct_DifferentStages) {
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
         utils::Vector{
             Return(Call(ty.Of(interface_struct), Call(ty.vec4<f32>()), Expr(0.5_f), Expr(0.25_f))),
         },
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    Func("frag_main", utils::Vector{Param("inputs", ty.Of(interface_struct))}, ty.void_(),
         utils::Vector{
             Decl(Let("r", ty.f32(), MemberAccessor("inputs", "col1"))),
             Decl(Let("g", ty.f32(), MemberAccessor("inputs", "col2"))),
             Decl(Let("p", ty.vec4<f32>(), MemberAccessor("inputs", "pos"))),
         },
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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
  const Interface tint_symbol_3 = {(0.0f).xxxx, 0.5f, 0.25f};
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

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_SharedStruct_HelperFunction) {
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
    auto* vertex_output_struct =
        Structure("VertexOutput",
                  utils::Vector{Member("pos", ty.vec4<f32>(),
                                       utils::Vector{Builtin(builtin::BuiltinValue::kPosition)})});

    Func("foo", utils::Vector{Param("x", ty.f32())}, ty.Of(vertex_output_struct),
         utils::Vector{
             Return(
                 Call(ty.Of(vertex_output_struct), Call(ty.vec4<f32>(), "x", "x", "x", Expr(1_f)))),
         },
         utils::Empty);

    Func("vert_main1", utils::Empty, ty.Of(vertex_output_struct),
         utils::Vector{
             Return(Call("foo", Expr(0.5_f))),
         },
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    Func("vert_main2", utils::Empty, ty.Of(vertex_output_struct),
         utils::Vector{
             Return(Call("foo", Expr(0.25_f))),
         },
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_UniformStruct) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(cbuffer cbuffer_uniforms : register(b0, space1) {
  uint4 uniforms[1];
};

void frag_main() {
  float v = uniforms.coord.x;
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_RW_StorageBuffer_Read) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(),
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_RO_StorageBuffer_Read) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(),
              R"(ByteAddressBuffer coord : register(t0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_WO_StorageBuffer_Store) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(),
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_With_StorageBuffer_Store) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(),
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_Called_By_EntryPoint_With_Uniform) {
    auto* s = Structure("S", utils::Vector{Member("x", ty.f32())});
    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kUniform, Binding(0_a), Group(1_a));

    Func("sub_func",
         utils::Vector{
             Param("param", ty.f32()),
         },
         ty.f32(),
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_Called_By_EntryPoint_With_StorageBuffer) {
    auto* s = Structure("S", utils::Vector{Member("x", ty.f32())});
    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    Func("sub_func",
         utils::Vector{
             Param("param", ty.f32()),
         },
         ty.f32(),
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_WithNameCollision) {
    Func("GeometryShader", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(void tint_symbol() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute) {
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         },
         utils::Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"([numthreads(1, 1, 1)]
void main() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Literal) {
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(2_i, 4_i, 6_i),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"([numthreads(2, 4, 6)]
void main() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Const) {
    GlobalConst("width", ty.i32(), Call<i32>(2_i));
    GlobalConst("height", ty.i32(), Call<i32>(3_i));
    GlobalConst("depth", ty.i32(), Call<i32>(4_i));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize("width", "height", "depth"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"([numthreads(2, 3, 4)]
void main() {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Attribute_EntryPoint_Compute_WithWorkgroup_OverridableConst) {
    Override("width", ty.i32(), Call<i32>(2_i), Id(7_u));
    Override("height", ty.i32(), Call<i32>(3_i), Id(8_u));
    Override("depth", ty.i32(), Call<i32>(4_i), Id(9_u));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize("width", "height", "depth"),
         });

    GeneratorImpl& gen = Build();

    EXPECT_FALSE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(
        gen.Diagnostics().str(),
        R"(error: override-expressions should have been removed with the SubstituteOverride transform)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
    Func("my_func",
         utils::Vector{
             Param("a", ty.array<f32, 5>()),
         },
         ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(void my_func(float a[5]) {
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayReturn) {
    Func("my_func", utils::Empty, ty.array<f32, 5>(),
         utils::Vector{
             Return(Call(ty.array<f32, 5>())),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(typedef float my_func_ret[5];
my_func_ret my_func() {
  return (float[5])0;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithDiscardAndVoidReturn) {
    Func("my_func", utils::Vector{Param("a", ty.i32())}, ty.void_(),
         utils::Vector{
             If(Equal("a", 0_i),  //
                Block(Discard())),
             Return(),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(void my_func(int a) {
  if ((a == 0)) {
    discard;
  }
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithDiscardAndNonVoidReturn) {
    Func("my_func", utils::Vector{Param("a", ty.i32())}, ty.i32(),
         utils::Vector{
             If(Equal("a", 0_i),  //
                Block(Discard())),
             Return(42_i),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
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
TEST_F(HlslGeneratorImplTest_Function, Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(RWByteAddressBuffer data : register(u0);

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
