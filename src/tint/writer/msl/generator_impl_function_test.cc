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

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/msl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_Function) {
    Func("my_func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  #include <metal_stdlib>

  using namespace metal;
  void my_func() {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithParams) {
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
    EXPECT_EQ(gen.result(), R"(  #include <metal_stdlib>

  using namespace metal;
  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Attribute_EntryPoint_NoReturn_Void) {
    Func("main", utils::Empty, ty.void_(), {/* no explicit return */},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
fragment void main() {
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Attribute_EntryPoint_WithInOutVars) {
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
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol_1 {
  float foo [[user(locn0)]];
};

struct tint_symbol_2 {
  float value [[color(1)]];
};

float frag_main_inner(float foo) {
  return foo;
}

fragment tint_symbol_2 frag_main(tint_symbol_1 tint_symbol [[stage_in]]) {
  float const inner_result = frag_main_inner(tint_symbol.foo);
  tint_symbol_2 wrapper_result = {};
  wrapper_result.value = inner_result;
  return wrapper_result;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Attribute_EntryPoint_WithInOut_Builtins) {
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
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol {
  float value [[depth(any)]];
};

float frag_main_inner(float4 coord) {
  return coord[0];
}

fragment tint_symbol frag_main(float4 coord [[position]]) {
  float const inner_result = frag_main_inner(coord);
  tint_symbol wrapper_result = {};
  wrapper_result.value = inner_result;
  return wrapper_result;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Attribute_EntryPoint_SharedStruct_DifferentStages) {
    // struct Interface {
    //   @location(1) col1 : f32;
    //   @location(2) col2 : f32;
    //   @builtin(position) pos : vec4<f32>;
    // };
    // fn vert_main() -> Interface {
    //   return Interface(0.5, 0.25, vec4<f32>());
    // }
    // fn frag_main(colors : Interface) {
    //   const r = colors.col1;
    //   const g = colors.col2;
    // }
    auto* interface_struct = Structure(
        "Interface",
        utils::Vector{
            Member("col1", ty.f32(), utils::Vector{Location(1_a)}),
            Member("col2", ty.f32(), utils::Vector{Location(2_a)}),
            Member("pos", ty.vec4<f32>(), utils::Vector{Builtin(builtin::BuiltinValue::kPosition)}),
        });

    Func("vert_main", utils::Empty, ty.Of(interface_struct),
         utils::Vector{Return(Call(ty.Of(interface_struct), 0.5_f, 0.25_f, vec4<f32>()))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    Func("frag_main", utils::Vector{Param("colors", ty.Of(interface_struct))}, ty.void_(),
         utils::Vector{
             WrapInStatement(Let("r", ty.f32(), MemberAccessor("colors", "col1"))),
             WrapInStatement(Let("g", ty.f32(), MemberAccessor("colors", "col2"))),
         },
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Interface {
  float col1;
  float col2;
  float4 pos;
};

struct tint_symbol {
  float col1 [[user(locn1)]];
  float col2 [[user(locn2)]];
  float4 pos [[position]];
};

Interface vert_main_inner() {
  Interface const tint_symbol_3 = Interface{.col1=0.5f, .col2=0.25f, .pos=float4(0.0f)};
  return tint_symbol_3;
}

vertex tint_symbol vert_main() {
  Interface const inner_result = vert_main_inner();
  tint_symbol wrapper_result = {};
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_2 {
  float col1 [[user(locn1)]];
  float col2 [[user(locn2)]];
};

void frag_main_inner(Interface colors) {
  float const r = colors.col1;
  float const g = colors.col2;
}

fragment void frag_main(float4 pos [[position]], tint_symbol_2 tint_symbol_1 [[stage_in]]) {
  Interface const tint_symbol_4 = {.col1=tint_symbol_1.col1, .col2=tint_symbol_1.col2, .pos=pos};
  frag_main_inner(tint_symbol_4);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Attribute_EntryPoint_SharedStruct_HelperFunction) {
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
        utils::Vector{
            Member("pos", ty.vec4<f32>(), utils::Vector{Builtin(builtin::BuiltinValue::kPosition)}),
        });

    Func("foo", utils::Vector{Param("x", ty.f32())}, ty.Of(vertex_output_struct),
         utils::Vector{
             Return(
                 Call(ty.Of(vertex_output_struct), Call(ty.vec4<f32>(), "x", "x", "x", Expr(1_f)))),
         });
    Func("vert_main1", utils::Empty, ty.Of(vertex_output_struct),
         utils::Vector{Return(Expr(Call("foo", Expr(0.5_f))))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    Func("vert_main2", utils::Empty, ty.Of(vertex_output_struct),
         utils::Vector{Return(Expr(Call("foo", Expr(0.25_f))))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct VertexOutput {
  float4 pos;
};

VertexOutput foo(float x) {
  VertexOutput const tint_symbol_2 = {.pos=float4(x, x, x, 1.0f)};
  return tint_symbol_2;
}

struct tint_symbol {
  float4 pos [[position]];
};

VertexOutput vert_main1_inner() {
  return foo(0.5f);
}

vertex tint_symbol vert_main1() {
  VertexOutput const inner_result = vert_main1_inner();
  tint_symbol wrapper_result = {};
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_1 {
  float4 pos [[position]];
};

VertexOutput vert_main2_inner() {
  return foo(0.25f);
}

vertex tint_symbol_1 vert_main2() {
  VertexOutput const inner_result_1 = vert_main2_inner();
  tint_symbol_1 wrapper_result_1 = {};
  wrapper_result_1.pos = inner_result_1.pos;
  return wrapper_result_1;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionAttribute_EntryPoint_With_RW_StorageBuffer) {
    auto* s = Structure("Data", utils::Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Group(0_a), Binding(0_a));

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
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  /* 0x0000 */ int a;
  /* 0x0004 */ float b;
};

fragment void frag_main(device Data* tint_symbol [[buffer(0)]]) {
  float v = (*(tint_symbol)).b;
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionAttribute_EntryPoint_With_RO_StorageBuffer) {
    auto* s = Structure("Data", utils::Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead,
              Group(0_a), Binding(0_a));

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
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  /* 0x0000 */ int a;
  /* 0x0004 */ float b;
};

fragment void frag_main(const device Data* tint_symbol [[buffer(0)]]) {
  float v = (*(tint_symbol)).b;
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Attribute_Called_By_EntryPoint_With_Uniform) {
    auto* ubo_ty = Structure("UBO", utils::Vector{Member("coord", ty.vec4<f32>())});
    auto* ubo =
        GlobalVar("ubo", ty.Of(ubo_ty), builtin::AddressSpace::kUniform, Group(0_a), Binding(0_a));

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
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct UBO {
  /* 0x0000 */ float4 coord;
};

float sub_func(float param, const constant UBO* const tint_symbol) {
  return (*(tint_symbol)).coord[0];
}

fragment void frag_main(const constant UBO* tint_symbol_1 [[buffer(0)]]) {
  float v = sub_func(1.0f, tint_symbol_1);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionAttribute_Called_By_EntryPoint_With_RW_StorageBuffer) {
    auto* s = Structure("Data", utils::Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Group(0_a), Binding(0_a));

    Func("sub_func",
         utils::Vector{
             Param("param", ty.f32()),
         },
         ty.f32(),
         utils::Vector{
             Return(MemberAccessor("coord", "b")),
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
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  /* 0x0000 */ int a;
  /* 0x0004 */ float b;
};

float sub_func(float param, device Data* const tint_symbol) {
  return (*(tint_symbol)).b;
}

fragment void frag_main(device Data* tint_symbol_1 [[buffer(0)]]) {
  float v = sub_func(1.0f, tint_symbol_1);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionAttribute_Called_By_EntryPoint_With_RO_StorageBuffer) {
    auto* s = Structure("Data", utils::Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead,
              Group(0_a), Binding(0_a));

    Func("sub_func",
         utils::Vector{
             Param("param", ty.f32()),
         },
         ty.f32(),
         utils::Vector{
             Return(MemberAccessor("coord", "b")),
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
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  /* 0x0000 */ int a;
  /* 0x0004 */ float b;
};

float sub_func(float param, const device Data* const tint_symbol) {
  return (*(tint_symbol)).b;
}

fragment void frag_main(const device Data* tint_symbol_1 [[buffer(0)]]) {
  float v = sub_func(1.0f, tint_symbol_1);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithArrayParams) {
    Func("my_func",
         utils::Vector{
             Param("a", ty.array<f32, 5>()),
         },
         ty.void_(),
         utils::Vector{
             Return(),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  #include <metal_stdlib>

  using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

  void my_func(tint_array<float, 5> a) {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithArrayReturn) {
    Func("my_func", utils::Empty, ty.array<f32, 5>(),
         utils::Vector{
             Return(Call(ty.array<f32, 5>())),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  #include <metal_stdlib>

  using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

  tint_array<float, 5> my_func() {
    tint_array<float, 5> const tint_symbol = tint_array<float, 5>{};
    return tint_symbol;
  }

)");
}

// https://crbug.com/tint/297
TEST_F(MslGeneratorImplTest, Emit_Function_Multiple_EntryPoint_With_Same_ModuleVar) {
    // struct Data {
    //   d : f32;
    // };
    // @binding(0) @group(0) var<storage> data : Data;
    //
    // @compute @workgroup_size(1)
    // fn a() {
    //   return;
    // }
    //
    // @compute @workgroup_size(1)
    // fn b() {
    //   return;
    // }

    auto* s = Structure("Data", utils::Vector{Member("d", ty.f32())});

    GlobalVar("data", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Group(0_a), Binding(0_a));

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
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  /* 0x0000 */ float d;
};

kernel void a(device Data* tint_symbol [[buffer(0)]]) {
  float v = (*(tint_symbol)).d;
  return;
}

kernel void b(device Data* tint_symbol_1 [[buffer(0)]]) {
  float v = (*(tint_symbol_1)).d;
  return;
}

)");
}

}  // namespace
}  // namespace tint::writer::msl
